#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#define ISR_REGISTRY_TIMER2_OVF  0
#define ISR_REGISTRY_TIMER4_CAPT 1
#define ISR_REGISTRY_TIMER5_CAPT 2
#define ISR_REGISTRY_NUM_SLOTS   3

class ISRRegistry {
public:
    int register_signal(int isr_number, AP_HAL::Proc proc);
    int unregister_signal(int isr_number);
 
    static AP_HAL::Proc _registry[ISR_REGISTRY_NUM_SLOTS];
};

#endif  // CONFIG_SHAL_CORE == SHAL_CORE_APM

