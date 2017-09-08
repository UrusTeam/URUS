#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusUtil.h"

#include "CoreUrusUtil_Cygwin.h"

CLCoreUrusUtil_Cygwin::CLCoreUrusUtil_Cygwin() :
    NSCORE_URUS::CLCoreUrusUtil()
{}

uint32_t CLCoreUrusUtil_Cygwin::available_memory(void)
{
    return 128*1024;
}


#endif // __CYGWIN__
