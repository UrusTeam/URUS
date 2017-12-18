
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusI2CDevice.h"
#include "../CoreUrusSemaphores.h"

#include "CoreUrusI2CDevice_Avr.h"
#include "CoreUrusSemaphores_Avr.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#define CPU_FREQ 16000000L
#else
#define CPU_FREQ F_CPU
#endif

#define START           0x08
#define REPEATED_START  0x10
#define MT_SLA_ACK      0x18
#define MT_DATA_ACK     0x28
#define MR_SLA_ACK      0x40
#define MR_DATA_ACK     0x50
#define MR_DATA_NACK    0x58
#define TWI_STATUS      (TWSR & 0xF8)

#define SLA_W(address)  (address << 1)
#define SLA_R(address)  ((address << 1) + 0x01)

#define cbi(sfr, bit)   (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit)   (_SFR_BYTE(sfr) |= _BV(bit))

extern const AP_HAL::HAL& hal;

CLCoreUrusSemaphore_Avr CLCoreUrusI2CDevice_Avr::semaphore;

CLCoreUrusI2CDevice_Avr::CLCoreUrusI2CDevice_Avr(uint8_t bus, uint8_t address) :
    NSCORE_URUS::CLCoreUrusI2CDevice(),
    _address(address)
{
    begin();
    setTimeout(100);
}

void CLCoreUrusI2CDevice_Avr::begin()
{
    // activate internal pull-ups for twi
    // as per note from atmega128 manual pg204
    sbi(PORTD, 0);
    sbi(PORTD, 1);

    // initialize twi prescaler and bit rate
    cbi(TWSR, TWPS0);
    cbi(TWSR, TWPS1);

    // start in high speed. When a driver gets an error it drops it to
    // low speed
    set_speed(AP_HAL::Device::SPEED_HIGH);

    // enable twi module, acks, and twi interrupt
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
}

void CLCoreUrusI2CDevice_Avr::end()
{
    TWCR = 0;
}

uint8_t CLCoreUrusI2CDevice_Avr::write(uint8_t addr, uint32_t len, const uint8_t* data)
{
    uint8_t stat = _start();
    if (stat) goto error;
    stat = _sendAddress(SLA_W(addr));
    if (stat) goto error;
    for (uint32_t i = 0; i < len; i++)
    {
        stat = _sendByte(data[i]);
        if (stat) goto error;
    }
    stat = _stop();
    if (stat) goto error;
    return stat;
error:
    _lockup_count++;
    return stat;
}

uint8_t CLCoreUrusI2CDevice_Avr::writeRegisters(uint8_t addr, uint8_t reg,
                                    uint32_t len, uint8_t* data)
{
    uint8_t stat = _start();
    if (stat) goto error;
    stat = _sendAddress(SLA_W(addr));
    if (stat) goto error;
    stat = _sendByte(reg);
    if (stat) goto error;
    for (uint32_t i = 0; i < len; i++)
    {
        stat = _sendByte(data[i]);
        if (stat) goto error;
    }
    stat = _stop();
    if (stat) goto error;
    return stat;
error:
    if (!_ignore_errors) {
        _lockup_count++;
    }
    return stat;
}

uint8_t CLCoreUrusI2CDevice_Avr::writeRegister(uint8_t addr, uint8_t reg, uint8_t val)
{
        /* Sometimes avr-gcc fails at dereferencing a uint8_t arg. */
        uint8_t data[1];
        data[0] = val;
        return writeRegisters(addr, reg, 1, data);
}

uint8_t CLCoreUrusI2CDevice_Avr::read(uint8_t addr, uint32_t len, uint8_t* data)
{
    uint8_t stat;
    if ( len == 0) {
        len = 1;
    }

    uint32_t nackposition = len - 1;
    stat = 0;
    stat = _start();
    if(stat) goto error;
    stat = _sendAddress(SLA_R(addr));
    if(stat) goto error;
    for(uint32_t i = 0; i < len ; i++) {
        if ( i == nackposition ) {
            stat = _receiveByte(false);
            if (stat != MR_DATA_NACK) goto error;
        } else {
            stat = _receiveByte(true);
            if (stat != MR_DATA_ACK) goto error;
        }
        data[i] = TWDR;
    }
    stat = _stop();
    if (stat) goto error;
    return stat;
error:
    _lockup_count++;
    return stat;
}

uint8_t CLCoreUrusI2CDevice_Avr::readRegisters(uint8_t addr, uint8_t reg,
                                    uint32_t len, uint8_t* data)
{
    uint8_t stat;
    if ( len == 0) {
        len = 1;
    }

    uint32_t nackposition = len - 1;
    stat = 0;
    stat = _start();
    if(stat) goto error;
    stat = _sendAddress(SLA_W(addr));
    if(stat) goto error;
    stat = _sendByte(reg);
    if(stat) goto error;
    stat = _start();
    if(stat) goto error;
    stat = _sendAddress(SLA_R(addr));
    if(stat) goto error;
    for(uint32_t i = 0; i < len ; i++) {
        if ( i == nackposition ) {
            stat = _receiveByte(false);
            if (stat != MR_DATA_NACK) goto error;
        } else {
            stat = _receiveByte(true);
            if (stat != MR_DATA_ACK) goto error;
        }
        data[i] = TWDR;
    }
    stat = _stop();
    if (stat) goto error;
    return stat;
error:
    _lockup_count++;
    return stat;
}

uint8_t CLCoreUrusI2CDevice_Avr::readRegister(uint8_t addr, uint8_t reg, uint8_t* data)
{
    return readRegisters(addr, reg, 1, data);
}

uint8_t CLCoreUrusI2CDevice_Avr::_start()
{
    TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN);
    uint8_t stat = _waitInterrupt();
    if (stat) return stat;

    if ((TWI_STATUS == START) || (TWI_STATUS == REPEATED_START)) {
        return 0;
    } else {
        return TWI_STATUS;
    }
}

uint8_t CLCoreUrusI2CDevice_Avr::_stop() {
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
    return _waitStop();
}

uint8_t CLCoreUrusI2CDevice_Avr::_sendAddress(uint8_t addr)
{
    TWDR = addr;
    TWCR = _BV(TWINT) | _BV(TWEN);
    return _waitInterrupt();
}

uint8_t CLCoreUrusI2CDevice_Avr::_sendByte(uint8_t data)
{
    TWDR = data;
    TWCR = _BV(TWINT) | _BV(TWEN);
    uint8_t stat = _waitInterrupt();
    if (stat) return stat;

    if (TWI_STATUS  == MT_DATA_ACK) {
        return 0;
    } else {
        return TWI_STATUS;
    }
}

uint8_t CLCoreUrusI2CDevice_Avr::_receiveByte(bool ack)
{
    if (ack) {
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
    } else {
        TWCR = _BV(TWINT) | _BV(TWEN);
    }
    uint8_t stat = _waitInterrupt();
    if (stat) return stat;
    return TWI_STATUS;
}

void CLCoreUrusI2CDevice_Avr::_handleLockup()
{
    TWCR = 0; /* Releases SDA and SCL lines to high impedance */
    TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA); /* Reinitialize TWI */
    _lockup_count++;
}

uint8_t CLCoreUrusI2CDevice_Avr::_waitInterrupt()
{
    uint32_t start = AP_HAL::millis();
    if (_timeoutDelay == 0) {
        /* Wait indefinitely for interrupt to go off */
        while (!(TWCR & _BV(TWINT))) { }
    } else {
        /* Wait while polling for timeout */
        while (!(TWCR & _BV(TWINT))) {
            uint32_t current = AP_HAL::millis();
            if ( current - start >= _timeoutDelay ) {
                _handleLockup();
                return 1;
            }
        }
    }
    return 0;
}

uint8_t CLCoreUrusI2CDevice_Avr::_waitStop()
{
    uint32_t start = AP_HAL::millis();
    if (_timeoutDelay == 0) {
        /* Wait indefinitely for stop condition */
        while( TWCR & _BV(TWSTO) ) { }
    } else  {
        /* Wait while polling for timeout */
        while( TWCR & _BV(TWSTO) ) {
            uint32_t current = AP_HAL::millis();
            if (current - start >= _timeoutDelay) {
                _handleLockup();
                return 1;
            }
        }
    }
    return 0;
}

ISR(TWI_vect)
{
    switch(TWI_STATUS) {
    case 0x20:
    case 0x30:
    case 0x48:
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);  // send a stop
        break;
    case 0x38:
    case 0x68:
    case 0x78:
    case 0xB0:
        TWCR = 0;  //releases SDA and SCL lines to high impedance
        TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);  //reinitialize TWI
        break;
    }
}

/* See AP_HAL::I2CDevice::set_address() */
void CLCoreUrusI2CDevice_Avr::set_address(uint8_t address)
{
    _address = address;
}

/* See AP_HAL::I2CDevice::set_retries() */
void CLCoreUrusI2CDevice_Avr::set_retries(uint8_t retries)
{}

/* AP_HAL::Device implementation */

/* See AP_HAL::Device::transfer() */
bool CLCoreUrusI2CDevice_Avr::transfer(const uint8_t *send, uint32_t send_len,
              uint8_t *recv, uint32_t recv_len)
{

    if ((send) && (send_len > 1)) {
        if (send_len == 2) {
            CLCoreUrusI2CDevice_Avr::writeRegister(_address, send[0], send[1]);
        } else {
            CLCoreUrusI2CDevice_Avr::write(_address, send_len, send);
        }
    } else {
        if ((send_len == 1) && (!recv)) {
            CLCoreUrusI2CDevice_Avr::write(_address, send_len, send);
        }
    }

    if (recv) {
        if (send_len == 1) {
            CLCoreUrusI2CDevice_Avr::readRegisters(_address, send[0], recv_len, recv);
        } else {
            CLCoreUrusI2CDevice_Avr::read(_address, recv_len, recv);
        }
    }

    return true;
}

bool CLCoreUrusI2CDevice_Avr::read_registers_multiple(uint8_t first_reg, uint8_t *recv,
                             uint32_t recv_len, uint8_t times)
{
    return true;
}

/* See AP_HAL::Device::set_speed() */
bool CLCoreUrusI2CDevice_Avr::set_speed(enum AP_HAL::Device::Speed speed)
{
    if (speed == AP_HAL::Device::SPEED_HIGH) {
        TWBR = ((CPU_FREQ / 400000) - 16) / 2;
    } else {
        TWBR = ((CPU_FREQ / 100000) - 16) / 2;
    }
    return true;
}

/* See AP_HAL::Device::get_semaphore() */
AP_HAL::Semaphore *CLCoreUrusI2CDevice_Avr::get_semaphore()
{
    return &semaphore;
}

/* See AP_HAL::Device::register_periodic_callback() */
AP_HAL::Device::PeriodicHandle CLCoreUrusI2CDevice_Avr::register_periodic_callback(
    uint32_t period_usec, AP_HAL::Device::PeriodicCb cb)
{
    AP_HAL::Device::PeriodicHandle p = &cb;
    hal.scheduler->register_timer_process(cb);
    return static_cast<AP_HAL::Device::PeriodicHandle>(p);
}

/* See Device::adjust_periodic_callback() */
bool CLCoreUrusI2CDevice_Avr::adjust_periodic_callback(
    AP_HAL::Device::PeriodicHandle h, uint32_t period_usec)
{
    return true;
}

CLCoreUrusI2CDeviceManager_Avr::CLCoreUrusI2CDeviceManager_Avr() :
    NSCORE_URUS::CLCoreUrusI2CDeviceManager()
{ }

/* AP_HAL::I2CDeviceManager implementation */
AP_HAL::OwnPtr<AP_HAL::I2CDevice>
CLCoreUrusI2CDeviceManager_Avr::get_device(uint8_t bus, uint8_t address)
{
    auto dev = AP_HAL::OwnPtr<AP_HAL::I2CDevice>(new CLCoreUrusI2CDevice_Avr(bus, address));
    return dev;
}


#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM

