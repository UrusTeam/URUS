#pragma once

#include <AP_HAL/AP_HAL.h>
#if defined(__CYGWIN__) && (CONFIG_SHAL_CORE_CYGWIN == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"
#include "../CoreUrusSPIDevice.h"

#include <AP_HAL/SPIDevice.h>
#include <AP_HAL/utility/OwnPtr.h>

#include "CoreUrusSemaphores_Cygwin.h"

#include <inttypes.h>

class CLCoreUrusSPIDevice_Cygwin : public NSCORE_URUS::CLCoreUrusSPIDevice {
public:
    CLCoreUrusSPIDevice_Cygwin()
    {
    }

    virtual ~CLCoreUrusSPIDevice_Cygwin() { }

    /* AP_HAL::Device implementation */

    /* See AP_HAL::Device::set_speed() */
    bool set_speed(AP_HAL::Device::Speed speed) override
    {
        return true;
    }

    /* See AP_HAL::Device::transfer() */
    bool transfer(const uint8_t *send, uint32_t send_len,
                  uint8_t *recv, uint32_t recv_len) override
    {
        return true;
    }

    /* See AP_HAL::SPIDevice::transfer_fullduplex() */
    bool transfer_fullduplex(const uint8_t *send, uint8_t *recv,
                             uint32_t len) override
    {
        return true;
    }

    /* See AP_HAL::Device::get_semaphore() */
    AP_HAL::Semaphore *get_semaphore()
    {
        return &_semaphore;
    }

    /* See AP_HAL::Device::register_periodic_callback() */
    AP_HAL::Device::PeriodicHandle register_periodic_callback(
        uint32_t period_usec, AP_HAL::Device::PeriodicCb) override
    {
        return nullptr;
    }

private:
    CLCoreUrusSemaphore_Cygwin _semaphore;
};

class CLCoreUrusSPIDeviceManager_Cygwin : public NSCORE_URUS::CLCoreUrusSPIDeviceManager {
public:
    CLCoreUrusSPIDeviceManager_Cygwin() { }
    AP_HAL::OwnPtr<AP_HAL::SPIDevice> get_device(const char *name) override
    {
        return AP_HAL::OwnPtr<AP_HAL::SPIDevice>(new NSCORE_URUS::CLCoreUrusSPIDevice_Cygwin());
    }

};

#endif // __CYGWIN__
