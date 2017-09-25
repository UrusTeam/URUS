#pragma once

#include <stdint.h>
#include <stdio.h>

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"
#include <math.h>

#define FATHER          1
#define SON             1
#define SPIRIT_SAINT    1

#define TRINITY_SAINT (FATHER + SON + SPIRIT_SAINT)

#define DA_VINCI_NUMBER 5
#define CUBE_PERFECT_TIME_BASE pow(DA_VINCI_NUMBER, TRINITY_SAINT)

#define GEORGE_BOOLE_BASE 2
#define CUBE_PERFECT_BOOL_BASE pow(GEORGE_BOOLE_BASE, TRINITY_SAINT)

#define URUS_MAGIC_TIME (CUBE_PERFECT_TIME_BASE * CUBE_PERFECT_BOOL_BASE)

/* Frequency in HZ for CORE isr timer.
 * Maximum frequency is 1000000HZ - 1MHZ
 */
#define CORE_ISR_TIMER_FREQ 5000
#define CORE_SPEED_PERCENT 25.0
#define CORE_SPEED_FREQ_PERCENT CORE_SPEED_PERCENT / 100.0

/* Frequency in HZ for SHAL isr scheduler.
 * SHAL_ISR_SCHED_FREQ < CORE_ISR_TIMER_FREQ ever.
 */
#define SHAL_ISR_SCHED_FREQ 1000

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

    uint32_t get_isr_timer_tick();

    enum CLK_TIMERS {
        CLK_TIME_MSEC = 1,
        CLK_TIME_USEC,
        CLK_TIME_NSEC,
    };

    static struct clk_core_timers_s {
        uint32_t clk_per_sec;
        uint32_t timer_divider;
        float isr_time;
        CLK_TIMERS timer_type;
    } clk_core_timers;

    float get_timer_dial() {
        return clk_core_timers.isr_time;
    }

    bool get_timer_event_eval() {
        return _timer_event_eval;
    }
private:

    bool _timer_event_eval = false;
    static bool _isr_timer_running_shal;

    /* @_timer_tick_shal will overflow aprox.
     * if CORE_ISR_TIMER_FREQ is set at:
     * 1000  KHZ = 71 Minutes
     *  100  KHZ = 12 Hours
     *   10  KHZ =  5 Days
     *    5  Khz =  9 Days
     *    1  Khz = 49 Days
     */
    static uint32_t _timer_tick_shal;
    static uint32_t _shal_tick_hz;

};
