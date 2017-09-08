#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"

class NSCORE_URUS::CLCoreUrusRCInput : public AP_HAL::RCInput {
public:
    CLCoreUrusRCInput();
    virtual void init() {};
    bool  new_input();
    uint8_t num_channels();
    uint16_t read(uint8_t ch);
    uint8_t read(uint16_t* periods, uint8_t len);

    bool set_overrides(int16_t *overrides, uint8_t len);
    bool set_override(uint8_t channel, int16_t override);
    void clear_overrides();
};
