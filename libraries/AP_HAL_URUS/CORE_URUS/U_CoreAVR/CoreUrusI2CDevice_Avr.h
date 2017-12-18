#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusI2CDevice.h"
#include "CoreUrusSemaphores_Avr.h"
#include <inttypes.h>
#include <stdio.h>

class CLCoreUrusI2CDevice_Avr : public NSCORE_URUS::CLCoreUrusI2CDevice {
public:
    CLCoreUrusI2CDevice_Avr(uint8_t bus, uint8_t address);

    virtual ~CLCoreUrusI2CDevice_Avr() { }

    void begin() override;
    void end();
    /* AP_HAL::I2CDevice implementation */

    /* See AP_HAL::I2CDevice::set_address() */
    void set_address(uint8_t address) override;

    /* See AP_HAL::I2CDevice::set_retries() */
    void set_retries(uint8_t retries) override;

    /* AP_HAL::Device implementation */

    /* See AP_HAL::Device::transfer() */
    bool transfer(const uint8_t *send, uint32_t send_len,
                  uint8_t *recv, uint32_t recv_len) override;

    bool read_registers_multiple(uint8_t first_reg, uint8_t *recv,
                                 uint32_t recv_len, uint8_t times);


    /* See AP_HAL::Device::set_speed() */
    bool set_speed(enum AP_HAL::Device::Speed speed) override;

    /* See AP_HAL::Device::get_semaphore() */
    AP_HAL::Semaphore *get_semaphore() override;

    /* See AP_HAL::Device::register_periodic_callback() */
    AP_HAL::Device::PeriodicHandle register_periodic_callback(
        uint32_t period_usec, AP_HAL::Device::PeriodicCb) override;

    /* See Device::adjust_periodic_callback() */
    bool adjust_periodic_callback(
        AP_HAL::Device::PeriodicHandle h, uint32_t period_usec) override;

    void setTimeout(uint16_t ms) { _timeoutDelay = ms; }

    uint8_t write(uint8_t addr, uint32_t len, const uint8_t* data);
    uint8_t writeRegister(uint8_t addr, uint8_t reg, uint8_t val);
    uint8_t writeRegisters(uint8_t addr, uint8_t reg,
                           uint32_t len, uint8_t* data);
    uint8_t read(uint8_t addr, uint32_t len, uint8_t* data);
    uint8_t readRegister(uint8_t addr, uint8_t reg, uint8_t* data);
    uint8_t readRegisters(uint8_t addr, uint8_t reg,
                          uint32_t len, uint8_t* data);
    uint8_t lockup_count() { return _lockup_count; }

    bool set_chip_select(bool set) override
    {
        _chip_select = set;
         return true;
    }
private:

    bool _chip_select;
    uint8_t _start();
    uint8_t _stop();
    uint8_t _sendAddress(uint8_t addr);
    uint8_t _sendByte(uint8_t data);
    uint8_t _receiveByte(bool ack);
    void    _handleLockup();

    uint8_t _waitInterrupt();
    uint8_t _waitStop();

    uint8_t _lockup_count;
    uint16_t _timeoutDelay;
    bool _ignore_errors = true;
    uint8_t _address;

    static CLCoreUrusSemaphore_Avr semaphore;
};

class CLCoreUrusI2CDeviceManager_Avr : public NSCORE_URUS::CLCoreUrusI2CDeviceManager {
public:
    friend class CLCoreUrusI2CDevice_Avr;
    CLCoreUrusI2CDeviceManager_Avr();

    /* AP_HAL::I2CDeviceManager implementation */
    AP_HAL::OwnPtr<AP_HAL::I2CDevice> get_device(uint8_t bus, uint8_t address) override;

    /* See AP_HAL::I2CDeviceManager::get_count() */
    uint8_t get_count() override {return 0;}

    /* See AP_HAL::I2CDeviceManager::get_device_name() */
    const char *get_device_name(uint8_t idx) override
    {
        return "EMPTY";
    }
};
#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM

