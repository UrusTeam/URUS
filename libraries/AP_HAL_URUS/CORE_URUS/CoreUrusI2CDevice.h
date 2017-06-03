#pragma once

#include <inttypes.h>

#include <AP_HAL/HAL.h>
#include <AP_HAL/I2CDevice.h>
#include <AP_HAL/utility/OwnPtr.h>

#include "CORE_URUS_NAMESPACE.h"

class NSCORE_URUS::CLCoreUrusI2CDevice : public AP_HAL::I2CDevice {
public:
    CLCoreUrusI2CDevice()
    {
    }

    virtual ~CLCoreUrusI2CDevice() { }

    /* AP_HAL::I2CDevice implementation */

    /* See AP_HAL::I2CDevice::set_address() */
    void set_address(uint8_t address) override { }

    /* See AP_HAL::I2CDevice::set_retries() */
    void set_retries(uint8_t retries) override { }


    /* AP_HAL::Device implementation */

    /* See AP_HAL::Device::transfer() */
    bool transfer(const uint8_t *send, uint32_t send_len,
                  uint8_t *recv, uint32_t recv_len) override
    {
        return true;
    }

    bool read_registers_multiple(uint8_t first_reg, uint8_t *recv,
                                 uint32_t recv_len, uint8_t times)
    {
        return true;
    }


    /* See AP_HAL::Device::set_speed() */
    bool set_speed(enum AP_HAL::Device::Speed speed) override { return true; }

    /* See AP_HAL::Device::get_semaphore() */
    AP_HAL::Semaphore *get_semaphore() { return nullptr; }

    /* See AP_HAL::Device::register_periodic_callback() */
    AP_HAL::Device::PeriodicHandle register_periodic_callback(
        uint32_t period_usec, AP_HAL::Device::PeriodicCb) override
    {
        return nullptr;
    }

    /* See Device::adjust_periodic_callback() */
    virtual bool adjust_periodic_callback(
        AP_HAL::Device::PeriodicHandle h, uint32_t period_usec) override
    {
        return true;
    }
};

class NSCORE_URUS::CLCoreUrusI2CDeviceManager : public AP_HAL::I2CDeviceManager {
public:
    CLCoreUrusI2CDeviceManager() { }

    /* AP_HAL::I2CDeviceManager implementation */
    AP_HAL::OwnPtr<AP_HAL::I2CDevice> get_device(uint8_t bus, uint8_t address)
    {
        return nullptr;
    }
};

