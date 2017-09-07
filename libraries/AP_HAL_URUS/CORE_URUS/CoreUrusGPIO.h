#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"

class NSCORE_URUS::CLCoreUrusGPIO : public AP_HAL::GPIO {
public:
    CLCoreUrusGPIO()
    {}
};

class NSCORE_URUS::CLCoreUrusDigitalSource : public AP_HAL::DigitalSource {
public:
    CLCoreUrusDigitalSource()
    {}
};
