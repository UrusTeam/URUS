#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"

#include "../CoreUrusUtil.h"
#include "CoreUrusSemaphores_Avr.h"

class CLCoreUrusUtil_Avr : public NSCORE_URUS::CLCoreUrusUtil {
public:
    CLCoreUrusUtil_Avr();
    static CLCoreUrusUtil *from(AP_HAL::Util *Util) {
        return static_cast<CLCoreUrusUtil_Avr*>(Util);
    }

    bool run_debug_shell(AP_HAL::BetterStream *stream) { return false; }

    AP_HAL::Semaphore *new_semaphore(void) override { return new CLCoreUrusSemaphore_Avr; }
    uint32_t available_memory(void) override;
    void memcheck_init(void);

private:

};
#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
