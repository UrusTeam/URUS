#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"

#include "CoreUrusSemaphores_Cygwin.h"

bool CLCoreUrusSemaphore_Cygwin::give() {
    if (_taken) {
        _taken = false;
        return true;
    } else {
        return false;
    }
}

bool CLCoreUrusSemaphore_Cygwin::take(uint32_t timeout_ms) {
    return take_nonblocking();
}

bool CLCoreUrusSemaphore_Cygwin::take_nonblocking() {
    /* No syncronisation primitives to garuntee this is correct */
    if (!_taken) {
        _taken = true;
        return true;
    } else {
        return false;
    }
}

#endif // __CYGWIN__