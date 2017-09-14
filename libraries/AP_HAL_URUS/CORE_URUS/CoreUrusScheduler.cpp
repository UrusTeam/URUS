#include <stdint.h>

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"

#include "CORE_URUS.h"
#include "CoreUrusScheduler.h"
#include <string.h>

namespace NSCORE_URUS {

static const CLCORE_URUS& _urus_core = get_CORE();
bool CLCoreUrusScheduler::_isr_timer_running_shal = false;
uint64_t CLCoreUrusScheduler::_timer_tick_shal = 0;
uint32_t CLCoreUrusScheduler::_shal_tick_hz = 0;

CLCoreUrusScheduler::CLCoreUrusScheduler ()
{}

void CLCoreUrusScheduler::start_sched()
{

    uint32_t result;
    uint8_t pos = 0;
    uint32_t timer_divider = 1000;

    result = clk_core_timers.clk_per_sec;
    while (result > 1) {
        result = result / timer_divider;
        pos++;
    }

    switch (pos) {
        case CLK_TIME_MSEC:
        {
            clk_core_timers.timer_divider = 1;
            printf("CPU CLOCK have milliseconds range\n");
            break;
        }
        case CLK_TIME_USEC:
        {
            clk_core_timers.timer_divider = 1000;
            printf("CPU CLOCK have microseconds range\n");
            break;
        }
        case CLK_TIME_NSEC:
        {
            clk_core_timers.timer_divider = 1000000;
            printf("CPU CLOCK have nanosconds range\n");
            break;
        }
    }

    /* This will give to us 1KHZ of core speed use timer
     * for this isr.
     * clk_core_timers.dial the dial scale is from 0.001 to 1.
     */
    _shal_tick_hz = (clk_core_timers.clk_per_sec / clk_core_timers.timer_divider) * clk_core_timers.dial;
}

uint64_t CLCoreUrusScheduler::get_isr_timer_tick()
{
    return _timer_tick_shal;
}

void CLCoreUrusScheduler::fire_isr_sched()
{
    static uint64_t now_tick = 0;
    static uint64_t last_tick = 0;

    now_tick = _urus_core.scheduler->get_isr_timer_tick();

    if ((now_tick - last_tick) > _shal_tick_hz ) {
        last_tick = _urus_core.scheduler->get_isr_timer_tick();
        _urus_core.scheduler->timer_event();
    }
}

void CLCoreUrusScheduler::fire_isr_timer()
{

    if (_isr_timer_running_shal) {
        return;
    }

    _isr_timer_running_shal = true;
    _timer_tick_shal++;
    _isr_timer_running_shal = false;
}

void CLCoreUrusScheduler::timer_event()
{
    if (!_timer_event_eval) {
        _timer_event_eval = true;
        printf("\nWARNING!\nCORE target has no realtime support."
               "\nRunning Interactive Mode!\n");
    }
}

} //end namespace NSCORE_URUS
