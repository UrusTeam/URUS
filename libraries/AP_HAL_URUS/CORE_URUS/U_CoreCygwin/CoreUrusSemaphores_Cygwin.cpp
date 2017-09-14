#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS.h"
#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"

#include "CoreUrusSemaphores_Cygwin.h"

static const NSCORE_URUS::CLCORE_URUS& _urus_core = NSCORE_URUS::get_CORE();

bool CLCoreUrusSemaphore_Cygwin::give() {
    return pthread_mutex_unlock(&_lock) == 0;
}

bool CLCoreUrusSemaphore_Cygwin::take(uint32_t timeout_ms) {
    if (timeout_ms == HAL_SEMAPHORE_BLOCK_FOREVER) {
        return pthread_mutex_lock(&_lock) == 0;
    }
    if (take_nonblocking()) {
        return true;
    }
    uint64_t start = AP_HAL::micros64();
    do {
        _urus_core.scheduler->delay_microseconds(200);
        if (take_nonblocking()) {
            return true;
        }
    } while ((AP_HAL::micros64() - start) < timeout_ms * 1000);
    return false;
}

bool CLCoreUrusSemaphore_Cygwin::take_nonblocking() {
    return pthread_mutex_trylock(&_lock) == 0;
}

#endif // __CYGWIN__
