#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusUtil.h"

#include "CoreUrusUtil_Cygwin.h"

CLCoreUrusUtil_Cygwin::CLCoreUrusUtil_Cygwin() :
    NSCORE_URUS::CLCoreUrusUtil()
{}


#endif // __CYGWIN__
