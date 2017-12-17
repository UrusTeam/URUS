#pragma once
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"

#include "../CoreUrusTimers.h"
#include <errno.h>

class CLCoreUrusTimers_Cygwin : public NSCORE_URUS::CLCoreUrusTimers {
public:
    CLCoreUrusTimers_Cygwin();

    uint64_t get_core_hrdtime ();

    uint32_t get_core_micros32 () override;
    uint64_t get_core_micros64 () override;

    uint32_t get_core_millis32 () override;
    uint64_t get_core_millis64 () override;

private:

    void _measure_time_proccess();
    void _micro_sleep(uint32_t);
    uint64_t _micros64ts();
    uint64_t _micros64tv();
    uint64_t nowt;
};

#endif // __CYGWIN__
