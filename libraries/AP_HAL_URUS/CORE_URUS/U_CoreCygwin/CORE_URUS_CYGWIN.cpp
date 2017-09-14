#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

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
#include "CoreUrusStorage_Cygwin.h"
#include "CoreUrusGPIO_Cygwin.h"
#include "CoreUrusRCInput_Cygwin.h"
#include "CoreUrusRCOutput_Cygwin.h"
#include <stdio.h>

static CLCoreUrusTimers_Cygwin coreTimers;
static CLCoreUrusScheduler_Cygwin coreScheduler;
static CLCoreUrusUARTDriver_Cygwin coreUARTA_Driver(0, true);
static CLCoreUrusUARTDriver_Cygwin coreUARTB_Driver(1, false);
static CLCoreUrusUARTDriver_Cygwin coreUARTC_Driver(2, false);
static CLCoreUrusUARTDriver_Cygwin coreUARTD_Driver(3, false);
static CLCoreUrusUARTDriver_Cygwin coreUARTE_Driver(4, false);
static CLCoreUrusUARTDriver_Cygwin coreUARTF_Driver(5, false);
static CLCoreUrusAnalogIn_Cygwin coreAnalogIn;
static CLCoreUrusI2CDeviceManager_Cygwin coreI2C_mgr;
static CLCoreUrusSPIDeviceManager_Cygwin coreSPI_mgr;
static CLCoreUrusUtil_Cygwin coreUtil;
static CLCoreUrusEEStorage_Cygwin coreStorage;
static CLCoreUrusGPIO_Cygwin coreGPIO;
static CLCoreUrusRCInput_Cygwin coreRCInput;
static CLCoreUrusRCOutput_Cygwin coreRCOutput;

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

NSCORE_URUS::CLCoreUrusUARTDriver* NSCORE_URUS::get_uartA_Driver()
{
    return &coreUARTA_Driver;
}

NSCORE_URUS::CLCoreUrusUARTDriver* NSCORE_URUS::get_uartB_Driver()
{
    return &coreUARTB_Driver;
}

NSCORE_URUS::CLCoreUrusUARTDriver* NSCORE_URUS::get_uartC_Driver()
{
    return &coreUARTC_Driver;
}

NSCORE_URUS::CLCoreUrusUARTDriver* NSCORE_URUS::get_uartD_Driver()
{
    return &coreUARTD_Driver;
}

NSCORE_URUS::CLCoreUrusUARTDriver* NSCORE_URUS::get_uartE_Driver()
{
    return &coreUARTE_Driver;
}

NSCORE_URUS::CLCoreUrusUARTDriver* NSCORE_URUS::get_uartF_Driver()
{
    return &coreUARTF_Driver;
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

NSCORE_URUS::CLCoreUrusEEStorage* NSCORE_URUS::get_Storage()
{
    return &coreStorage;
}

NSCORE_URUS::CLCoreUrusGPIO* NSCORE_URUS::get_GPIO()
{
    return &coreGPIO;
}

NSCORE_URUS::CLCoreUrusRCInput* NSCORE_URUS::get_RCInput()
{
    return &coreRCInput;
}

NSCORE_URUS::CLCoreUrusRCOutput* NSCORE_URUS::get_RCOutput()
{
    return &coreRCOutput;
}

const NSCORE_URUS::CLCORE_URUS& NSCORE_URUS::get_CORE()
{
    static const CORE_CYGWIN _urus_core;
    return _urus_core;
}

#endif // __CYGWIN__
