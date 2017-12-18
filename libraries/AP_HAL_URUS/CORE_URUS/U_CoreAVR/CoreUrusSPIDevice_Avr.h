#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include <AP_HAL/SPIDevice.h>
#include <AP_HAL/utility/OwnPtr.h>

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"
#include "../CoreUrusSPIDevice.h"
#include "../CoreUrusGPIO.h"

#include "CoreUrusSemaphores_Avr.h"
#include "CoreUrusGPIO_Avr.h"

#include <inttypes.h>

class SPIDesc;

struct SPIDesc {
    SPIDesc(const char *_name, uint8_t _bus,
            uint8_t _device, uint8_t _cs_pin, uint8_t _mode,
            uint32_t _lowspeed, uint32_t _highspeed)
        : name(_name), bus(_bus), device(_device), cs_pin(_cs_pin),
        mode(_mode), lowspeed(_lowspeed), highspeed(_highspeed)
    {
    }

    const char *name;
    uint8_t bus;
    uint8_t device;
    uint8_t cs_pin;
    uint8_t mode;
    uint8_t lowspeed;
    uint8_t highspeed;
};

class CLCoreUrusSPI0Device_Avr : public NSCORE_URUS::CLCoreUrusSPIDevice {
public:
    CLCoreUrusSPI0Device_Avr(SPIDesc &device_desc);

    virtual ~CLCoreUrusSPI0Device_Avr();

    void init();

    /* See AP_HAL::Device::set_speed() */
    bool set_speed(AP_HAL::Device::Speed speed) override;

    // low level transfer function
    uint8_t _transfer(uint8_t data);

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
    SPIDesc &_device_desc;
    static CLCoreUrusSemaphore_Avr _semaphore;
    CLCoreDigitalSource_Avr *_cs_pin;

    uint32_t frequency;
    bool cs_forced;
    static bool _force_low_speed;

    const uint8_t _spcr_lowspeed;
    const uint8_t _spcr_highspeed;
    uint8_t _spcr;
    const uint8_t _spsr;

    void _cs_assert();
    void _cs_release();
};

class CLCoreUrusSPI3Device_Avr : public NSCORE_URUS::CLCoreUrusSPIDevice {
public:
    CLCoreUrusSPI3Device_Avr(SPIDesc &device_desc);

    virtual ~CLCoreUrusSPI3Device_Avr();

    void init();

    /* See AP_HAL::Device::set_speed() */
    bool set_speed(AP_HAL::Device::Speed speed) override;

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

    void cs_assert() override { _cs_assert(); };
    void cs_release() override { _cs_release(); };

private:
    SPIDesc &_device_desc;
    static CLCoreUrusSemaphore_Avr _semaphore;

    CLCoreDigitalSource_Avr *_spi3_sck;
    CLCoreDigitalSource_Avr *_cs_pin;

    bool _cs_forced;
    uint8_t _ucsr3c;
    uint16_t _ubrr3;
    // low level transfer function
    uint8_t _transfer(uint8_t data);
    void _transfer(const uint8_t *data, uint16_t len);

    void _cs_assert();

    void _cs_release();

};

class CLCoreUrusSPIDeviceManager_Avr : public NSCORE_URUS::CLCoreUrusSPIDeviceManager {
public:
    friend class CLCoreUrusSPI0Device_Avr;
    friend class CLCoreUrusSPI3Device_Avr;

    static CLCoreUrusSPIDeviceManager_Avr *from(AP_HAL::SPIDeviceManager *spi_mgr)
    {
        return static_cast<CLCoreUrusSPIDeviceManager_Avr*>(spi_mgr);
    }

    AP_HAL::OwnPtr<AP_HAL::SPIDevice> get_device(const char *name);

    /* See AP_HAL::SPIDeviceManager::get_count() */
    uint8_t get_count();

    /* See AP_HAL::SPIDeviceManager::get_device_name() */
    const char *get_device_name(uint8_t idx);

private:
    static SPIDesc device_table[];

    static const uint8_t _n_device_desc;
};

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
