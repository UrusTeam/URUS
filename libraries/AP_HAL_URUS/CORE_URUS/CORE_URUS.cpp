
#include "CORE_URUS_NAMESPACE.h"
#include "CORE_URUS.h"

NSCORE_URUS::CLCORE_URUS::CLCORE_URUS(NSCORE_URUS::CLCoreUrusTimers* _timers,
                NSCORE_URUS::CLCoreUrusScheduler* _scheduler)
    :
    timers(_timers),
    scheduler(_scheduler)
    {}
