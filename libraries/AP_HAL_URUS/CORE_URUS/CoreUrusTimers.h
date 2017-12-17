#pragma once
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"

#include <stdio.h>
#include <stdint.h>

class NSCORE_URUS::CLCoreUrusTimers {
public:
    CLCoreUrusTimers()
    {}

    virtual uint32_t get_core_micros32 () = 0;
    virtual uint64_t get_core_micros64 () = 0;

    virtual uint32_t get_core_millis32 () = 0;
    virtual uint64_t get_core_millis64 () = 0;
};
