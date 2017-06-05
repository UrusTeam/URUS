#pragma once

#include <stdint.h>

namespace NSCORE_URUS {

    class CLCORE_URUS;
    class CLCoreUrusTimers;
    class CLCoreUrusScheduler;
    class CLCoreUrusUARTDriver;
    class CLCoreUrusI2CDevice;
    class CLCoreUrusI2CDeviceManager;

    const CLCORE_URUS& get_CORE();
    CLCoreUrusScheduler* get_scheduler();
    CLCoreUrusUARTDriver* get_uartDriver();
    CLCoreUrusI2CDeviceManager* get_I2CDeviceManager();

}