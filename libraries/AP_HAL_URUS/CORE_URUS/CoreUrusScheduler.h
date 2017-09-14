#pragma once

#include <stdint.h>
#include <stdio.h>

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"

class NSCORE_URUS::CLCoreUrusScheduler : public AP_HAL::Scheduler {
public:
    CLCoreUrusScheduler();
    virtual void start_sched();

    /* We need it to be called in to the Core target,
     * try to call it at low level as possible (ISR Timer Vector),
     * we need the HARDWARE TIMER counter to get it working at
     * realtime.
     * Without it the system will process everything
     * as interactive mode and not at realtime mode.
     * POSIX threading can make it working on the core target,
     * but maybe it could not run at precise timming for URUS SHAL.
     * URUS SHAL need the physical TIMER and CPU frequency
     * to calculate our own TICK system and the JIFFY counter
     * operation per seconds.
    */
    static void fire_isr_timer();
    static void fire_isr_sched();

    /* This need to be overridden into the Core Target,
     * otherwise IO operation at realtime will do nothing
     * and a warning will be displayed on the console at
     * first execution.
     * This function is monitored in the URUS SHAL.
    */
    virtual void timer_event();

    uint64_t get_isr_timer_tick();

    enum CLK_TIMERS {
        CLK_TIME_MSEC = 1,
        CLK_TIME_USEC,
        CLK_TIME_NSEC,
    };

    struct clk_core_timers_s {
        uint32_t clk_per_sec;
        uint32_t timer_divider;
        float dial;
        CLK_TIMERS timer_type;
    } clk_core_timers;

private:

    bool _timer_event_eval = false;
    static bool _isr_timer_running_shal;
    static uint64_t _timer_tick_shal;
    static uint32_t _shal_tick_hz;
};
