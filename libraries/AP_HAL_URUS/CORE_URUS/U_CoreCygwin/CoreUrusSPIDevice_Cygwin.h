#pragma once

#include <AP_HAL/AP_HAL.h>
#if defined(__CYGWIN__) && (CONFIG_SHAL_CORE_CYGWIN == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"
#include "../CoreUrusSPIDevice.h"

#include <AP_HAL/SPIDevice.h>
#include <AP_HAL/utility/OwnPtr.h>

#include "CoreUrusSemaphores_Cygwin.h"
#include "Device.h"

#include <inttypes.h>

class SPIDesc;
    
class SPIBus : public DeviceBus {
public:
    SPIBus(void) :
        DeviceBus(APM_SPI_PRIORITY) {}
    struct spi_dev_s *dev;
    uint8_t bus;
};

struct SPIDesc {
    SPIDesc(const char *_name, uint8_t _bus, 
            uint16_t _device, uint8_t _mode,
            uint32_t _lowspeed, uint32_t _highspeed)
        : name(_name), bus(_bus), device(_device), mode(_mode),
          lowspeed(_lowspeed), highspeed(_highspeed)
    {
    }

    const char *name;
    uint8_t bus;
    uint16_t device;
    uint8_t mode;
    uint32_t lowspeed;
    uint32_t highspeed;
};

class CLCoreUrusSPIDevice_Cygwin : public NSCORE_URUS::CLCoreUrusSPIDevice {
public:
    CLCoreUrusSPIDevice_Cygwin(SPIBus &_bus, SPIDesc &_device_desc);

    virtual ~CLCoreUrusSPIDevice_Cygwin();

    /* See AP_HAL::Device::set_speed() */
    bool set_speed(AP_HAL::Device::Speed speed) override;

    // low level transfer function
    void do_transfer(const uint8_t *send, uint8_t *recv, uint32_t len);

    /* See AP_HAL::Device::transfer() */
    bool transfer(const uint8_t *send, uint32_t send_len,
                  uint8_t *recv, uint32_t recv_len) override;

    /* See AP_HAL::SPIDevice::transfer_fullduplex() */
    bool transfer_fullduplex(const uint8_t *send, uint8_t *recv,
                             uint32_t len) override;

    /* See AP_HAL::Device::get_semaphore() */
    AP_HAL::Semaphore *get_semaphore() override;

    /* See AP_HAL::Device::register_periodic_callback() */
    AP_HAL::Device::PeriodicHandle register_periodic_callback(
        uint32_t period_usec, AP_HAL::Device::PeriodicCb) override;

    /* See AP_HAL::Device::adjust_periodic_callback() */
    bool adjust_periodic_callback(AP_HAL::Device::PeriodicHandle h, uint32_t period_usec) override;

    bool set_chip_select(bool set) override;

private:
    SPIBus &bus;
    SPIDesc &device_desc;
    uint32_t frequency;
    //perf_counter_t perf;
    char *pname;
    bool cs_forced;
    static void *spi_thread(void *arg);
};

class CLCoreUrusSPIDeviceManager_Cygwin : public NSCORE_URUS::CLCoreUrusSPIDeviceManager {
public:
    friend class CLCoreUrusSPIDevice_Cygwin;

    static CLCoreUrusSPIDeviceManager_Cygwin *from(AP_HAL::SPIDeviceManager *spi_mgr)
    {
        return static_cast<CLCoreUrusSPIDeviceManager_Cygwin*>(spi_mgr);
    }

    AP_HAL::OwnPtr<AP_HAL::SPIDevice> get_device(const char *name);

    /* See AP_HAL::SPIDeviceManager::get_count() */
    uint8_t get_count();

    /* See AP_HAL::SPIDeviceManager::get_device_name() */
    const char *get_device_name(uint8_t idx);

private:
    static SPIDesc device_table[];
    SPIBus *buses;
    static const uint8_t _n_device_desc;
};

#endif // __CYGWIN__
