#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusSemaphores.h"

#include <stdint.h>

class CLCoreUrusSemaphore_Avr : public  NSCORE_URUS::CLCoreUrusSemaphore {
public:
    CLCoreUrusSemaphore_Avr();
    bool give();
    bool take(uint32_t timeout_ms);
    bool take_nonblocking();

protected:
    bool _take_from_mainloop(uint32_t timeout_ms);
    bool _take_nonblocking();

    volatile bool _taken;
    //void _delay_microseconds(uint16_t us);
};

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
