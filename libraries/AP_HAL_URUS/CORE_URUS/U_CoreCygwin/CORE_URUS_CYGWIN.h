#pragma once
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS.h"
#include "../CORE_URUS_NAMESPACE.h"

class CORE_CYGWIN : public NSCORE_URUS::CLCORE_URUS {
public:
    CORE_CYGWIN();
    void init_core() const override;
};

#endif // __CYGWIN__
