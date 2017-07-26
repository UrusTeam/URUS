#pragma once

#include <stdint.h>

namespace NSCORE_URUS {

    class CLCORE_URUS;
    class CLCoreUrusTimers;
    class CLCoreUrusScheduler;
    class CLCoreUrusUARTDriver;
    class CLCoreUrusI2CDevice;
    class CLCoreUrusI2CDeviceManager;
    class CLCoreUrusSPIDevice;
    class CLCoreUrusSPIDeviceManager;
    class CLCoreUrusSemaphore;
    class CLCoreUrusAnalogSource;
    class CLCoreUrusAnalogIn;
    class CLCoreUrusUtil;

    const CLCORE_URUS& get_CORE();
    CLCoreUrusScheduler* get_scheduler();
    CLCoreUrusUARTDriver* get_uartDriver();
    CLCoreUrusI2CDeviceManager* get_I2CDeviceManager();
    CLCoreUrusSPIDeviceManager* get_SPIDeviceManager();
    CLCoreUrusAnalogSource* get_AnalogSource();
    CLCoreUrusAnalogIn* get_AnalogIn();
    CLCoreUrusUtil* get_Util();

}
