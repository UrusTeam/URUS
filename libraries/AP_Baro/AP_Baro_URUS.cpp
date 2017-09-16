#include "AP_Baro_URUS.h"

#if CONFIG_HAL_BOARD == HAL_BOARD_URUS
#include <AP_HAL_URUS/AP_HAL_URUS.h>

AP_Baro_URUS::AP_Baro_URUS(AP_Baro &baro) :
    AP_Baro_Backend(baro)
{
    _instance = _frontend.register_sensor();
    _frontend.setHIL(0,1013,30,300,2,AP_HAL::millis());
}

// Read the sensor
void AP_Baro_URUS::update(void)
{
    _frontend.set_hil_mode();
    if (_frontend._hil.updated) {
        _frontend._hil.updated = false;
        _copy_to_frontend(0, _frontend._hil.pressure, _frontend._hil.temperature);
        _frontend.setHIL(0,1013,30,300,2,AP_HAL::millis());
    }
}
#endif // CONFIG_HAL_BOARD == HAL_BOARD_URUS
