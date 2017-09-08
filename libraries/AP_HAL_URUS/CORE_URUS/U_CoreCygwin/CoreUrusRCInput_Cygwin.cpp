
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "CoreUrusRCInput_Cygwin.h"

CLCoreUrusRCInput_Cygwin::CLCoreUrusRCInput_Cygwin() :
    NSCORE_URUS::CLCoreUrusRCInput()
{}

void CLCoreUrusRCInput_Cygwin::init()
{}

#endif // __CYGWIN__
