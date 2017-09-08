#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusRCInput.h"

class CLCoreUrusRCInput_Cygwin : public NSCORE_URUS::CLCoreUrusRCInput {
public:
    CLCoreUrusRCInput_Cygwin();
    void init() override;
};

#endif // __CYGWIN__
