#include "Tracker.h"

void Tracker::init_capabilities(void)
{
#if !HAL_MINIMIZE_FEATURES_AVR
    hal.util->set_capabilities(MAV_PROTOCOL_CAPABILITY_PARAM_FLOAT |
                               MAV_PROTOCOL_CAPABILITY_COMPASS_CALIBRATION);
#endif
}
