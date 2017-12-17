#include <stdint.h>

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"

#include "CORE_URUS.h"
#include "CoreUrusScheduler.h"
#include <string.h>

#define URUS_DEBUG 0

namespace NSCORE_URUS {

static const CLCORE_URUS& _urus_core = get_CORE();
bool CLCoreUrusScheduler::_isr_timer_running_shal = false;
uint32_t CLCoreUrusScheduler::_timer_tick_shal = 0;
uint32_t CLCoreUrusScheduler::_shal_tick_hz = 0;

struct CLCoreUrusScheduler::clk_core_timers_s CLCoreUrusScheduler::clk_core_timers;

CLCoreUrusScheduler::CLCoreUrusScheduler ()
{}

void CLCoreUrusScheduler::start_sched()
{

    uint32_t result;
    uint8_t pos = 0;
    uint32_t timer_divider = URUS_MAGIC_TIME;
    clk_core_timers.isr_time = CORE_ISR_TIMER_FREQ;

    result = clk_core_timers.clk_per_sec;
    while (result > 1) {
        result = result / timer_divider;
        pos++;
    }

    switch (pos) {
        case CLK_TIME_MSEC:
        {
            clk_core_timers.timer_divider = pow(URUS_MAGIC_TIME, pos);
#if URUS_DEBUG == 1
            fprintf(stdout, "CPU CLOCK have milliseconds range\n");
#endif
            break;
        }
        case CLK_TIME_USEC:
        {
            clk_core_timers.timer_divider = pow(URUS_MAGIC_TIME, pos - 1);
#if URUS_DEBUG == 1
            fprintf(stdout, "CPU CLOCK have microseconds range %u\n", clk_core_timers.timer_divider);
#endif
            break;
        }
        case CLK_TIME_NSEC:
        {
            clk_core_timers.timer_divider = pow(URUS_MAGIC_TIME, pos);
#if URUS_DEBUG == 1
            fprintf(stdout, "CPU CLOCK have nanosconds range\n");
#endif
            break;
        }
    }

    /* This will give to us the frequency of core speed sched and isr timer. */
    uint32_t _time_factor = (URUS_MAGIC_TIME * clk_core_timers.timer_divider);
    clk_core_timers.isr_time = _time_factor / clk_core_timers.isr_time;
    _shal_tick_hz = (_time_factor / clk_core_timers.isr_time) / SHAL_ISR_SCHED_FREQ;
#if URUS_DEBUG == 1
    fprintf(stdout, "_shal_tick_hz: %u isr_time: %u time_factor: %u divider: %u\n", _shal_tick_hz, (uint32_t)clk_core_timers.isr_time, _time_factor, clk_core_timers.timer_divider);
#endif
}

uint32_t CLCoreUrusScheduler::get_isr_timer_tick()
{
    return _timer_tick_shal;
}

void CLCoreUrusScheduler::fire_isr_sched()
{
    static uint32_t now_tick = 0;
    static uint32_t last_tick = 0;

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
#if CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN
        fprintf(stdout, "\nWARNING!\nCORE target has no realtime support."
               "\nRunning Interactive Mode!\n");
#endif
    }
}

} //end namespace NSCORE_URUS
