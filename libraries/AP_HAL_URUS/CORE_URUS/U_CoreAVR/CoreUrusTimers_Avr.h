#pragma once
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if CONFIG_SHAL_CORE == SHAL_CORE_APM

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusTimers.h"

class AVRTimer {
public:

    AVRTimer();
    static void     init();
    static uint32_t micros();
    static uint32_t millis();
    //static void delay_microseconds(uint16_t us);
};

class CLCoreUrusTimers_Avr : public NSCORE_URUS::CLCoreUrusTimers {
public:
    CLCoreUrusTimers_Avr();

    uint32_t get_core_micros32 () override;
    uint64_t get_core_micros64 () override;

    uint32_t get_core_millis32 () override;
    uint64_t get_core_millis64 () override;

    static AVRTimer avr_timer;
};

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
