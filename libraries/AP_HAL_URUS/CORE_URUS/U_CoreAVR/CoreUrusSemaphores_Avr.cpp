#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS.h"
#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"
#include "../CoreUrusScheduler.h"

#include "CoreUrusScheduler_Avr.h"
#include "CoreUrusSemaphores_Avr.h"

#include <avr/io.h>
#include <avr/interrupt.h>

extern const AP_HAL::HAL& hal;

CLCoreUrusSemaphore_Avr::CLCoreUrusSemaphore_Avr() :
    NSCORE_URUS::CLCoreUrusSemaphore(),
    _taken(false)
{}

bool CLCoreUrusSemaphore_Avr::give()
{
    if (!_taken) {
        return false;
    } else {
        _taken = false;
        return true;
    }
}

bool CLCoreUrusSemaphore_Avr::take(uint32_t timeout_ms)
{
    return _take_from_mainloop(timeout_ms);
}

bool CLCoreUrusSemaphore_Avr::take_nonblocking()
{
    if (hal.scheduler->in_main_thread()) {
        return _take_nonblocking();
    } else {
        return _take_from_mainloop(0);
    }
}

bool CLCoreUrusSemaphore_Avr::_take_from_mainloop(uint32_t timeout_ms)
{
    /* Try to take immediately */
    if (_take_nonblocking()) {
        return true;
    } else if (timeout_ms == 0) {
        /* Return immediately if timeout is 0 */
        return false;
    }

    uint16_t timeout_ticks = timeout_ms*100;
    do {
        /* Delay 1ms until we can successfully take, or we timed out */
        hal.scheduler->delay_microseconds(10);
        timeout_ticks--;
        if (_take_nonblocking()) {
            return true;
        }
    } while (timeout_ticks > 0);

    return false;
}

bool CLCoreUrusSemaphore_Avr::_take_nonblocking()
{
    bool result = false;
    uint8_t sreg = SREG;
    cli();
    if (!_taken) {
        _taken = true;
        result = true;
    }
    SREG = sreg;
    return result;
}


#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
