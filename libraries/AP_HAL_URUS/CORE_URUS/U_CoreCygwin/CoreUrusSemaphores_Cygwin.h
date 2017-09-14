#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"

#include "CoreUrusSemaphores_Cygwin.h"
#include <stdint.h>
#include <pthread.h>

class CLCoreUrusSemaphore_Cygwin : public  NSCORE_URUS::CLCoreUrusSemaphore {
public:
    CLCoreUrusSemaphore_Cygwin() :
        NSCORE_URUS::CLCoreUrusSemaphore()
    {
        pthread_mutex_init(&_lock, nullptr);
    }

    bool give();
    bool take(uint32_t timeout_ms);
    bool take_nonblocking();
private:
    pthread_mutex_t _lock;
};

#endif // __CYGWIN__
