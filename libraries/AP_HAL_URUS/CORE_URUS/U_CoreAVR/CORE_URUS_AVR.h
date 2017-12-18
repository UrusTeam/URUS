#pragma once
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS.h"
#include "../CORE_URUS_NAMESPACE.h"

#include "utility/ISRRegistry.h"

class CORE_AVR : public NSCORE_URUS::CLCORE_URUS {
public:
    CORE_AVR();
    void init_core() const override;
    static ISRRegistry isrregistry;
};

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
