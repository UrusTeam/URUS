#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include <AP_HAL/I2CDevice.h>

#include "CORE_URUS_NAMESPACE.h"

class NSCORE_URUS::CLCoreUrusI2CDevice : public AP_HAL::I2CDevice {
public:
    CLCoreUrusI2CDevice()
    {}
};

class NSCORE_URUS::CLCoreUrusI2CDeviceManager : public AP_HAL::I2CDeviceManager {
public:
    CLCoreUrusI2CDeviceManager()
    {}
};

