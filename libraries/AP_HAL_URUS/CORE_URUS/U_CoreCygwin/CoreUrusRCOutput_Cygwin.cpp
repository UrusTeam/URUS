
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "CoreUrusRCOutput_Cygwin.h"

CLCoreUrusRCOutput_Cygwin::CLCoreUrusRCOutput_Cygwin() :
    NSCORE_URUS::CLCoreUrusRCOutput()
{}

void CLCoreUrusRCOutput_Cygwin::init()
{}

#endif // __CYGWIN__
