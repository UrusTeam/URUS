#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusRCInput.h"

#include "CoreUrusScheduler_Avr.h"
#include "utility/ISRRegistry.h"

#define AVR_RC_INPUT_NUM_CHANNELS 11
#define AVR_RC_INPUT_MIN_CHANNELS 5     // for ppm sum we allow less than 8 channels to make up a valid packet

#define AVR_RC_INPUT_MIN_SYNC_PULSE_WIDTH 2700

class CLCoreUrusRCInput_Avr : public NSCORE_URUS::CLCoreUrusRCInput {
public:
    CLCoreUrusRCInput_Avr();
    void init() override;
    bool  new_input();
    uint8_t num_channels();
    uint16_t read(uint8_t ch);
    uint8_t  read(uint16_t* periods, uint8_t len);
    bool set_overrides(int16_t *overrides, uint8_t len);
    bool set_override(uint8_t channel, int16_t override);
    void clear_overrides();

private:
    /* private callback for input capture ISR */
    static void _timer5_capt_cb(void);
    /* private variables to communicate with input capture isr */
    static volatile uint16_t _pulse_capt[AVR_RC_INPUT_NUM_CHANNELS];
    static volatile uint8_t  _num_channels;
    static volatile bool  _new_input;

    /* override state */
    uint16_t _override[AVR_RC_INPUT_NUM_CHANNELS];
};

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
