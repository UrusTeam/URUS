#include <AP_HAL/AP_HAL.h>
#if defined(__CYGWIN__) && (CONFIG_SHAL_CORE_CYGWIN == SHAL_CORE_CYGWIN)

#include "../CORE_URUS.h"
#include "../CORE_URUS_NAMESPACE.h"
#include "CORE_URUS_CYGWIN.h"

#include "CoreUrusTimers_Cygwin.h"
#include "CoreUrusScheduler_Cygwin.h"
#include "CoreUrusUARTDriver_Cygwin.h"
#include "CoreUrusAnalogIn_Cygwin.h"
#include "CoreUrusI2CDevice_Cygwin.h"
#include "CoreUrusSPIDevice_Cygwin.h"
#include "CoreUrusUtil_Cygwin.h"
#include <stdio.h>

static CLCoreUrusTimers_Cygwin coreTimers;
static CLCoreUrusScheduler_Cygwin coreScheduler;
static CLCoreUrusUARTDriver_Cygwin coreUARTDriver(0, true);
static CLCoreUrusAnalogIn_Cygwin coreAnalogIn;
static CLCoreUrusI2CDeviceManager_Cygwin coreI2C_mgr;
static CLCoreUrusSPIDeviceManager_Cygwin coreSPI_mgr;
static CLCoreUrusUtil_Cygwin coreUtil;

CORE_CYGWIN::CORE_CYGWIN() :
    NSCORE_URUS::CLCORE_URUS(
        &coreTimers,
        &coreScheduler)
{}

void CORE_CYGWIN::init_core() const
{
#if 0
    printf("Cygwin Core Started!\n");
#endif
}

NSCORE_URUS::CLCoreUrusScheduler* NSCORE_URUS::get_scheduler()
{
    return &coreScheduler;
}

NSCORE_URUS::CLCoreUrusUARTDriver* NSCORE_URUS::get_uartDriver()
{
    return &coreUARTDriver;
}

NSCORE_URUS::CLCoreUrusI2CDeviceManager* NSCORE_URUS::get_I2CDeviceManager()
{
    return &coreI2C_mgr;
}

NSCORE_URUS::CLCoreUrusSPIDeviceManager* NSCORE_URUS::get_SPIDeviceManager()
{
    return &coreSPI_mgr;
}

NSCORE_URUS::CLCoreUrusAnalogSource* NSCORE_URUS::get_AnalogSource()
{
    return nullptr;
}

NSCORE_URUS::CLCoreUrusAnalogIn* NSCORE_URUS::get_AnalogIn()
{
    return &coreAnalogIn;
}

NSCORE_URUS::CLCoreUrusUtil* NSCORE_URUS::get_Util()
{
    return &coreUtil;
}

const NSCORE_URUS::CLCORE_URUS& NSCORE_URUS::get_CORE()
{
    static const CORE_CYGWIN _urus_core;
    return _urus_core;
}

#endif // __CYGWIN__
