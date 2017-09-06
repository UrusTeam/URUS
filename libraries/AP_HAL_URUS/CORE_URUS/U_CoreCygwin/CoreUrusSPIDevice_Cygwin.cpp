
#include <AP_HAL/AP_HAL.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "CoreUrusSPIDevice_Cygwin.h"
#include <stdio.h>

SPIDesc CLCoreUrusSPIDeviceManager_Cygwin::device_table[] = {
    SPIDesc("**dummy**", 0, 0, 0, 0, 0),
    SPIDesc("URUS_Cape", 0, 0, 0, 0, 0),
};

#ifndef URUS_SPI_DEVICE_NUM_DEVICES
#define URUS_SPI_DEVICE_NUM_DEVICES ARRAY_SIZE(CLCoreUrusSPIDeviceManager_Cygwin::device_table)
#endif

const uint8_t CLCoreUrusSPIDeviceManager_Cygwin::_n_device_desc = URUS_SPI_DEVICE_NUM_DEVICES;

CLCoreUrusSPIDevice_Cygwin::CLCoreUrusSPIDevice_Cygwin(SPIBus &_bus, SPIDesc &_device_desc)
    :
    NSCORE_URUS::CLCoreUrusSPIDevice(),
    bus(_bus),
    device_desc(_device_desc)
{
    set_device_bus(_bus.bus);
    set_device_address(_device_desc.device);
    set_speed(AP_HAL::Device::SPEED_LOW);
#ifdef DEBUG_URUS
    asprintf(&pname, "---SPI:%s:%u:%u",
             device_desc.name,
             (unsigned)bus.bus, (unsigned)device_desc.device);
    printf("***SPI device %s on %u:%u at speed %u mode %u\n",
           device_desc.name,
           (unsigned)bus.bus, (unsigned)device_desc.device,
           (unsigned)frequency, (unsigned)device_desc.mode);
#endif
}

CLCoreUrusSPIDevice_Cygwin::~CLCoreUrusSPIDevice_Cygwin()
{
#ifdef DEBUG_URUS
    printf("***SPI device %s on %u:%u closed\n", device_desc.name,
           (unsigned)bus.bus, (unsigned)device_desc.device);
#endif
    free(pname);
}

bool CLCoreUrusSPIDevice_Cygwin::set_speed(AP_HAL::Device::Speed speed)
{
    switch (speed) {
    case AP_HAL::Device::SPEED_HIGH:
        frequency = device_desc.highspeed;
        break;
    case AP_HAL::Device::SPEED_LOW:
        frequency = device_desc.lowspeed;
        break;
    }
    return true;
}

/*
  low level transfer function
 */
void CLCoreUrusSPIDevice_Cygwin::do_transfer(const uint8_t *send, uint8_t *recv, uint32_t len)
{
    uint8_t data;
    data = *recv;
    data += 0x80;
    recv[3] = data;
#ifdef DEBUG_URUS
    printf("data send: %x data recv: %x len: %u\n",*send,data,len);
#endif
}

bool CLCoreUrusSPIDevice_Cygwin::transfer(const uint8_t *send, uint32_t send_len,
                         uint8_t *recv, uint32_t recv_len)
{
    if (send_len == recv_len && send == recv) {
        // simplest cases, needed for DMA
        do_transfer(send, recv, recv_len);
        return true;
    }
    uint8_t buf[send_len+recv_len];
    if (send_len > 0) {
        memcpy(buf, send, send_len);
    }
    if (recv_len > 0) {
        memset(&buf[send_len], 0, recv_len);
    }
    do_transfer(buf, buf, send_len+recv_len);
    if (recv_len > 0) {
        memcpy(recv, &buf[send_len], recv_len);
    }
    return true;
}

bool CLCoreUrusSPIDevice_Cygwin::transfer_fullduplex(const uint8_t *send, uint8_t *recv, uint32_t len)
{
    uint8_t buf[len];
    memcpy(buf, send, len);
    do_transfer(buf, buf, len);
    memcpy(recv, buf, len);
    return true;
}

AP_HAL::Semaphore *CLCoreUrusSPIDevice_Cygwin::get_semaphore()
{
    return &bus.semaphore;
}

AP_HAL::Device::PeriodicHandle CLCoreUrusSPIDevice_Cygwin::register_periodic_callback(uint32_t period_usec, AP_HAL::Device::PeriodicCb cb)
{
    return bus.register_periodic_callback(period_usec, cb, this);
}

bool CLCoreUrusSPIDevice_Cygwin::adjust_periodic_callback(AP_HAL::Device::PeriodicHandle h, uint32_t period_usec)
{
    return bus.adjust_timer(h, period_usec);
}

/*
  allow for control of SPI chip select pin
 */
bool CLCoreUrusSPIDevice_Cygwin::set_chip_select(bool set)
{
    cs_forced = set;
    return true;
}

uint8_t CLCoreUrusSPIDeviceManager_Cygwin::get_count() {
   return _n_device_desc;
}

const char* CLCoreUrusSPIDeviceManager_Cygwin::get_device_name(uint8_t idx)
{
    return device_table[idx].name;
}

/*
  return a SPIDevice given a string device name
 */
AP_HAL::OwnPtr<AP_HAL::SPIDevice>
CLCoreUrusSPIDeviceManager_Cygwin::get_device(const char *name)
{
    /* Find the bus description in the table */
    uint8_t i;

    for (i = 0; device_table[i].name; i++) {
        if (strcmp(device_table[i].name, name) == 0) {
            break;
        }
    }
    if (device_table[i].name == nullptr) {
        printf("SPI: Invalid device name: %s\n", name);
        return AP_HAL::OwnPtr<AP_HAL::SPIDevice>(nullptr);
    }

    SPIDesc &desc = device_table[i];

    // find the bus
    SPIBus *busp;
    for (busp = buses; busp; busp = (SPIBus *)busp->next) {
        if (busp->bus == desc.bus) {
            break;
        }
    }
    if (busp == nullptr) {
        // create a new one
        busp = new SPIBus;
        if (busp == nullptr) {
            return nullptr;
        }
        busp->next = buses;
        busp->bus = desc.bus;
        busp->dev = nullptr;
        buses = busp;
    }

    return AP_HAL::OwnPtr<AP_HAL::SPIDevice>(new CLCoreUrusSPIDevice_Cygwin(*busp, desc));
}

#endif // __CYGWIN__
