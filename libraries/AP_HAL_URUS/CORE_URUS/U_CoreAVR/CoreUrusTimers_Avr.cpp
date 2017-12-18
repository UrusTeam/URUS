
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if CONFIG_SHAL_CORE == SHAL_CORE_APM

#include "../CORE_URUS_NAMESPACE.h"

#include "../CoreUrusScheduler.h"
#include "../CoreUrusTimers.h"

#include "CoreUrusTimers_Avr.h"
#include "CoreUrusScheduler_Avr.h"

AVRTimer CLCoreUrusTimers_Avr::avr_timer;

CLCoreUrusTimers_Avr::CLCoreUrusTimers_Avr() :
    NSCORE_URUS::CLCoreUrusTimers()
{}

uint32_t CLCoreUrusTimers_Avr::get_core_micros32()
{
    return avr_timer.micros();
}

uint64_t CLCoreUrusTimers_Avr::get_core_micros64()
{
    return get_core_micros32();
}

uint32_t CLCoreUrusTimers_Avr::get_core_millis32()
{
    return avr_timer.millis();
}

uint64_t CLCoreUrusTimers_Avr::get_core_millis64()
{
    return get_core_millis32();
}

#endif // __SHAL_CORE_APM__
