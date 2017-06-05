#pragma once

#include "CORE_URUS_NAMESPACE.h"
#include "CoreUrusSemaphores.h"

class NSCORE_URUS::CLCoreUrusSPIDevice : public AP_HAL::SPIDevice {
public:
    CLCoreUrusSPIDevice()
    {}
};

class NSCORE_URUS::CLCoreUrusSPIDeviceManager : public AP_HAL::SPIDeviceManager {
public:
    CLCoreUrusSPIDeviceManager()
    {}
};
