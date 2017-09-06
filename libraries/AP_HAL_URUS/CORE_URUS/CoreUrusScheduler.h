#pragma once

#include <stdint.h>

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"

class NSCORE_URUS::CLCoreUrusScheduler : public AP_HAL::Scheduler {
public:
    CLCoreUrusScheduler() {}
};
