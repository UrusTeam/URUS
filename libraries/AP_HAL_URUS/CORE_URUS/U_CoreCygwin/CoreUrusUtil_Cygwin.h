#pragma once

#include <AP_HAL/AP_HAL.h>
#if defined(__CYGWIN__) && (CONFIG_SHAL_CORE_CYGWIN == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"

#include "../CoreUrusUtil.h"
#include "CoreUrusSemaphores_Cygwin.h"

class CLCoreUrusUtil_Cygwin : public NSCORE_URUS::CLCoreUrusUtil {
public:
    CLCoreUrusUtil_Cygwin();
    static CLCoreUrusUtil *from(AP_HAL::Util *Util) {
        return static_cast<CLCoreUrusUtil_Cygwin*>(Util);
    }

    bool run_debug_shell(AP_HAL::BetterStream *stream) { return false; }

    AP_HAL::Semaphore *new_semaphore(void) override { return new CLCoreUrusSemaphore_Cygwin; }

private:

};
#endif // __CYGWIN__