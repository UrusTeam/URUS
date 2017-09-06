#pragma once

#include <AP_HAL/AP_HAL.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"

#include "CoreUrusSemaphores_Cygwin.h"
#include <inttypes.h>

class CLCoreUrusSemaphore_Cygwin : public  NSCORE_URUS::CLCoreUrusSemaphore {
public:
    CLCoreUrusSemaphore_Cygwin() :
        NSCORE_URUS::CLCoreUrusSemaphore(),
        _taken(false)
    {}

    bool give();
    bool take(uint32_t timeout_ms);
    bool take_nonblocking();
private:
    bool _taken;
};

#endif // __CYGWIN__
