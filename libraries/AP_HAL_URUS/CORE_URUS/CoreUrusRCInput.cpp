
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"

#include "CoreUrusRCInput.h"

namespace NSCORE_URUS {

CLCoreUrusRCInput::CLCoreUrusRCInput()
{}

bool CLCoreUrusRCInput::new_input() {
    return false;
}

uint8_t CLCoreUrusRCInput::num_channels() {
    return 0;
}

uint16_t CLCoreUrusRCInput::read(uint8_t ch) {
    if (ch == 2) return 900; /* throttle should be low, for safety */
    else return 1500;
}

uint8_t CLCoreUrusRCInput::read(uint16_t* periods, uint8_t len) {
    for (uint8_t i = 0; i < len; i++){
        if (i == 2) periods[i] = 900;
        else periods[i] = 1500;
    }
    return len;
}

bool CLCoreUrusRCInput::set_overrides(int16_t *overrides, uint8_t len) {
    return true;
}

bool CLCoreUrusRCInput::set_override(uint8_t channel, int16_t override) {
    return true;
}

void CLCoreUrusRCInput::clear_overrides()
{}

} //end namespace NSCORE_URUS