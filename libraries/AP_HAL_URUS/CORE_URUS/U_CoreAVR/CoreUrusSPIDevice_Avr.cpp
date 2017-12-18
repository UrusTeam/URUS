
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"

#include "CoreUrusSPIDevice_Avr.h"
#include "CoreUrusSemaphores_Avr.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include "utility/pins_arduino_mega.h"

extern const AP_HAL::HAL& hal;

#define SPI0_SPCR_8MHz   0
#define SPI0_SPSR_8MHz   _BV(SPI2X)
#define SPI0_SPCR_500kHz _BV(SPR1)
#define SPI0_SPSR_500kHz _BV(SPI2X)

#define SPI0_MISO_PIN_50   50
#define SPI0_MOSI_PIN_51   51
#define SPI0_SCK_PIN_52    52

//INS on APM2
#define SPI0_CS_PIN_53     53

//baro on APM2
#define SPI0_CS_PIN_40     40

//optflow on APM2
#define SPI0_CS_PIN_57     57

#define SPI3_MOSI_PIN_14   14
#define SPI3_MISO_PIN_15   15
#define SPI3_SCK_PIN_71    71 // PJ 2 ** 71 ** USART3 SPI SCK
#define SPI3_CS_PIN_28     28

#define MASTER_SPI  0
#define SLAVE_SPI   1

bool CLCoreUrusSPI0Device_Avr::_force_low_speed;
CLCoreUrusSemaphore_Avr CLCoreUrusSPI0Device_Avr::_semaphore;
CLCoreUrusSemaphore_Avr CLCoreUrusSPI3Device_Avr::_semaphore;

volatile bool spi0_transferflag = false;

/* SPI table descriptor
 * --------------------
 *
 * NAME | BUS | DEVICE | CS_PIN | MODE | LOW_SPEED | HIGH_SPEED
 *
 *
 */

SPIDesc CLCoreUrusSPIDeviceManager_Avr::device_table[] = {
    SPIDesc("**dummy**", 0, 0, 0, 0, 0, 0),
    SPIDesc("URUS_Cape", 0, 0, SPI0_CS_PIN_53, MASTER_SPI, SPI0_SPCR_500kHz, SPI0_SPCR_8MHz),
    SPIDesc("SPI0_M", 0, 0, SPI0_CS_PIN_53, MASTER_SPI, SPI0_SPCR_500kHz, SPI0_SPCR_8MHz),
    SPIDesc("SPI0_S", 3, 0, SPI0_CS_PIN_53, SLAVE_SPI, SPI0_SPCR_500kHz, SPI0_SPCR_8MHz),
    SPIDesc("FLASH_SPI3", 3, 0, SPI3_CS_PIN_28, MASTER_SPI, 0, 0),
    SPIDesc("MPU60XX_SPI0", 0, 0, SPI0_CS_PIN_53, MASTER_SPI, SPI0_SPCR_500kHz, SPI0_SPCR_8MHz),
    SPIDesc("MS5611_SPI0", 0, 0, SPI0_CS_PIN_40, MASTER_SPI, SPI0_SPCR_500kHz, SPI0_SPCR_8MHz),
};

#ifndef URUS_SPI_DEVICE_NUM_DEVICES
#define URUS_SPI_DEVICE_NUM_DEVICES ARRAY_SIZE(CLCoreUrusSPIDeviceManager_Avr::device_table)
#endif

const uint8_t CLCoreUrusSPIDeviceManager_Avr::_n_device_desc = URUS_SPI_DEVICE_NUM_DEVICES;
// CONSTRUCTOR FOR SPI-0
CLCoreUrusSPI0Device_Avr::CLCoreUrusSPI0Device_Avr(SPIDesc &device_desc)
    :
    NSCORE_URUS::CLCoreUrusSPIDevice(),
    _device_desc(device_desc),
    _spcr_lowspeed(device_desc.lowspeed),
    _spcr_highspeed(device_desc.highspeed),
    _spcr(device_desc.lowspeed),
    _spsr(SPI0_SPSR_8MHz)
{
    init();
#ifdef DEBUG_URUS
/*
    hal.console->printf("Name SPI: %s\n", device_desc.name);
    hal.console->printf("***SPI device %s on %u:%u at speed %u mode %u\n",
           device_desc.name,
           (unsigned)bus.bus, (unsigned)device_desc.device,
           (unsigned)frequency, (unsigned)device_desc.mode);
*/
#endif
}

CLCoreUrusSPI0Device_Avr::~CLCoreUrusSPI0Device_Avr()
{
#ifdef DEBUG_URUS
/*
    hal.console->printf("***SPI device %s on %u:%u closed\n", device_desc.name,
           (unsigned)bus.bus, (unsigned)device_desc.device);
*/
#endif
}

void CLCoreUrusSPI0Device_Avr::init()
{
    _cs_pin = new CLCoreDigitalSource_Avr(_device_desc.cs_pin);

    if (_device_desc.mode == MASTER_SPI) {
        hal.gpio->pinMode(SPI0_MISO_PIN_50, HAL_GPIO_INPUT);
        hal.gpio->pinMode(SPI0_MOSI_PIN_51, HAL_GPIO_OUTPUT);
        hal.gpio->pinMode(SPI0_SCK_PIN_52, HAL_GPIO_OUTPUT);

        _cs_pin->mode(HAL_GPIO_OUTPUT);
        _cs_pin->write(1);

        hal.gpio->pinMode(SPI0_CS_PIN_40, HAL_GPIO_OUTPUT);
        hal.gpio->write(SPI0_CS_PIN_40, 1);

        hal.gpio->pinMode(SPI0_CS_PIN_57, HAL_GPIO_OUTPUT);
        hal.gpio->write(SPI0_CS_PIN_57, 1);

        /* Enable the SPI0 peripheral as a master */

        SPCR = _BV(SPE) | _BV(MSTR);
    }
}

void CLCoreUrusSPI0Device_Avr::_cs_assert()
{
    const uint8_t valid_spcr_mask =
        (_BV(CPOL) | _BV(CPHA) | _BV(SPR1) | _BV(SPR0));
    if (_force_low_speed) {
        _spcr = _spcr_lowspeed;
    }
    uint8_t new_spcr = (SPCR & ~valid_spcr_mask) | (_spcr & valid_spcr_mask);
    SPCR = new_spcr;

    const uint8_t valid_spsr_mask = _BV(SPI2X);
    uint8_t new_spsr = (SPSR & ~valid_spsr_mask) | (_spsr & valid_spsr_mask);
    SPSR = new_spsr;

    _cs_pin->write(0);
}

void CLCoreUrusSPI0Device_Avr::_cs_release()
{

    _cs_pin->write(1);

}

bool CLCoreUrusSPI0Device_Avr::set_speed(AP_HAL::Device::Speed speed)
{
    switch (speed) {
    case AP_HAL::Device::SPEED_HIGH:
        _spcr = _spcr_highspeed;
        _force_low_speed = false;
        break;
    case AP_HAL::Device::SPEED_LOW:
        _spcr = _spcr_lowspeed;
        _force_low_speed = true;
        break;
    }
    return true;
}

uint8_t CLCoreUrusSPI0Device_Avr::_transfer(uint8_t data)
{
    if (spi0_transferflag) {
#if !HAL_MINIMIZE_FEATURES_AVR
        AP_HAL::panic("PANIC: SPI0 transfer collision");
#endif
        return 0;
    }
    spi0_transferflag = true;
    SPDR = data;
    if (SPSR & _BV(WCOL)) {
#if !HAL_MINIMIZE_FEATURES_AVR
        AP_HAL::panic("PANIC: SPI0 write collision");
#endif
        return 0;
    }
    while(!(SPSR & (1<<SPIF)));
    uint8_t read_spdr = SPDR;
    spi0_transferflag = false;
    return read_spdr;
}

bool CLCoreUrusSPI0Device_Avr::transfer(const uint8_t *send, uint32_t send_len,
                         uint8_t *recv, uint32_t recv_len)
{
    _cs_assert();
    if (send) {
        uint16_t i;
        for (i = 0; i < send_len; i++) {
            _transfer(send[i]);
        }
    }
    if (recv) {
        uint16_t i1;
        for (i1 = 0; i1 < recv_len; i1++) {
            recv[i1] = _transfer(recv[i1]);
        }
    }
    _cs_release();
    return true;
}

bool CLCoreUrusSPI0Device_Avr::transfer_fullduplex(const uint8_t *send, uint8_t *recv, uint32_t len)
{
    return true;
}

AP_HAL::Semaphore *CLCoreUrusSPI0Device_Avr::get_semaphore()
{
    return &_semaphore;
}

AP_HAL::Device::PeriodicHandle CLCoreUrusSPI0Device_Avr::register_periodic_callback(uint32_t period_usec, AP_HAL::Device::PeriodicCb cb)
{
    AP_HAL::Device::PeriodicHandle p = &cb;
    hal.scheduler->register_timer_process(cb);
    return static_cast<AP_HAL::Device::PeriodicHandle>(p);
}

bool CLCoreUrusSPI0Device_Avr::adjust_periodic_callback(AP_HAL::Device::PeriodicHandle h, uint32_t period_usec)
{
    return true;
}

/*
  allow for control of SPI chip select pin
 */
bool CLCoreUrusSPI0Device_Avr::set_chip_select(bool set)
{
    cs_forced = set;
    return false;
}

// CONSTRUCTOR FOR SPI-3
CLCoreUrusSPI3Device_Avr::CLCoreUrusSPI3Device_Avr(SPIDesc &device_desc):
    NSCORE_URUS::CLCoreUrusSPIDevice(),
    _device_desc(device_desc),
    _ucsr3c(device_desc.lowspeed),
    _ubrr3(device_desc.highspeed)
{
    init();

#ifdef DEBUG_URUS
/*
    hal.console->printf("***SPI device %s on %u:%u at speed %u mode %u\n",
           device_desc.name,
           (unsigned)bus.bus, (unsigned)device_desc.device,
           (unsigned)frequency, (unsigned)device_desc.mode);
*/
#endif
}

CLCoreUrusSPI3Device_Avr::~CLCoreUrusSPI3Device_Avr()
{
#ifdef DEBUG_URUS
/*
    hal.console->printf("***SPI device %s on %u:%u closed\n", device_desc.name,
           (unsigned)bus.bus, (unsigned)device_desc.device);
*/
#endif

}

void CLCoreUrusSPI3Device_Avr::init()
{
    /* the spi3 (USART3) sck pin PORTJ2 is not enumerated
     * by the arduino pin numbers, so we access it directly
     * with AVRDigitalSource. UPDATE: This pin was added
     * on the arduino gpio table. */

    _spi3_sck = new CLCoreDigitalSource_Avr(SPI3_SCK_PIN_71);
    _spi3_sck->mode(HAL_GPIO_OUTPUT);
    _spi3_sck->write(1);

    hal.gpio->pinMode(SPI3_MOSI_PIN_14, HAL_GPIO_OUTPUT);
    hal.gpio->pinMode(SPI3_MISO_PIN_15, HAL_GPIO_INPUT);

    _cs_pin = new CLCoreDigitalSource_Avr(_device_desc.cs_pin);
    _cs_pin->mode(HAL_GPIO_OUTPUT);
    _cs_pin->write(1);

    /* UMSELn1 and UMSELn2: USART in SPI Master mode */
    UCSR3C = _BV(UMSEL31) | _BV(UMSEL30);
    /* Enable RX and TX. */
    UCSR3B = _BV(RXEN3) | _BV(TXEN3);

}

bool CLCoreUrusSPI3Device_Avr::set_speed(AP_HAL::Device::Speed speed)
{
    return true;
}

bool CLCoreUrusSPI3Device_Avr::transfer(const uint8_t *send, uint32_t send_len,
                         uint8_t *recv, uint32_t recv_len)
{

    if (send && (send_len == 1)) {
        _transfer(send[0]);
        return true;
    }

    if (send) {
        _transfer(send, send_len);
    }

    if (recv) {
        uint8_t recv_tmp[recv_len];
        memcpy(recv_tmp, &recv[0], sizeof(recv_tmp));
        uint16_t i1;

        for (i1 = 0; i1 < recv_len; i1++) {
            recv[i1] = _transfer(recv_tmp[i1]);
        }
    }

    return true;
}

bool CLCoreUrusSPI3Device_Avr::transfer_fullduplex(const uint8_t *send, uint8_t *recv, uint32_t len)
{
    if (recv == NULL) {
        _transfer(send, (uint16_t)len);
    } else {
        for (uint16_t i = 0; i < (uint16_t)len; i++) {
            recv[i] = _transfer(send[i]);
        }
    }

    return true;
}

uint8_t CLCoreUrusSPI3Device_Avr::_transfer(uint8_t send)
{
    /* Wait for empty transmit buffer */
    while ( !( UCSR3A & _BV(UDRE3)) ) ;

    /* Put send into buffer, sends the send */
    UDR3 = send;

    /* Wait for send to be received */
    while ( !(UCSR3A & _BV(RXC3)) ) ;

    /* Get and return received data from buffer */
    return UDR3;
}

void CLCoreUrusSPI3Device_Avr::_transfer(const uint8_t *send, uint16_t len) {
    while (len--) {
        /* Wait for empty transmit buffer */
        while ( !( UCSR3A & _BV(UDRE3)) ) ;

        /* Put send into buffer, sends the send */
        UDR3 = *send++;

        /* Wait for send to be received */
        while ( !(UCSR3A & _BV(RXC3)) ) ;

        // dummy read of UDR3 to complete
        UDR3;
    }
}

AP_HAL::Semaphore *CLCoreUrusSPI3Device_Avr::get_semaphore()
{
    return &_semaphore;
}

AP_HAL::Device::PeriodicHandle CLCoreUrusSPI3Device_Avr::register_periodic_callback(uint32_t period_usec, AP_HAL::Device::PeriodicCb cb)
{
    AP_HAL::Device::PeriodicHandle p = &cb;
    hal.scheduler->register_timer_process(cb);
    return static_cast<AP_HAL::Device::PeriodicHandle>(p);
}

bool CLCoreUrusSPI3Device_Avr::adjust_periodic_callback(AP_HAL::Device::PeriodicHandle h, uint32_t period_usec)
{
    return true;
}

/*
  allow for control of SPI chip select pin
 */
bool CLCoreUrusSPI3Device_Avr::set_chip_select(bool set)
{
    _cs_forced = set;
    return true;
}

void CLCoreUrusSPI3Device_Avr::_cs_assert()
{
    /* set the device UCSRnC configuration bits.
     * only sets data order, clock phase, and clock polarity bits (lowest
     * three bits)  */
    const uint8_t new_ucsr3c = (UCSR3C & ~0x07) | (_ucsr3c & (0x07));
    UCSR3C = new_ucsr3c;
    /* set the device baud rate */
    UBRR3 = _ubrr3;

    _cs_pin->write(0);
}

void CLCoreUrusSPI3Device_Avr::_cs_release()
{
    _cs_pin->write(1);
}

uint8_t CLCoreUrusSPIDeviceManager_Avr::get_count() {
   return _n_device_desc;
}

const char* CLCoreUrusSPIDeviceManager_Avr::get_device_name(uint8_t idx)
{
    return device_table[idx].name;
}

/*
  return a SPIDevice given a string device name
 */
AP_HAL::OwnPtr<AP_HAL::SPIDevice>
CLCoreUrusSPIDeviceManager_Avr::get_device(const char *name)
{
    /* Find the bus description in the table */
    uint8_t i;

    for (i = 0; device_table[i].name; i++) {
        if (strcmp(device_table[i].name, name) == 0) {
            break;
        }
    }

    if (device_table[i].name == nullptr) {
        return AP_HAL::OwnPtr<AP_HAL::SPIDevice>(nullptr);
    }

    SPIDesc &desc = device_table[i];

    AP_HAL::OwnPtr<AP_HAL::SPIDevice> dev;

    switch (desc.bus) {
        case 0:
        {
            dev = AP_HAL::OwnPtr<AP_HAL::SPIDevice>(new CLCoreUrusSPI0Device_Avr(desc));
            break;
        }

        case 3:
        {
            dev = AP_HAL::OwnPtr<AP_HAL::SPIDevice>(new CLCoreUrusSPI3Device_Avr(desc));
            break;
        }
    }

    return dev;
}

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
