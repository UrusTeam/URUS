/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *       AP_Compass_URUS.cpp - URUS backend for AP_Compass
 *
 */


#include <AP_HAL/AP_HAL.h>
#include "AP_Compass_URUS.h"

extern const AP_HAL::HAL& hal;

// constructor
AP_Compass_URUS::AP_Compass_URUS(Compass &compass):
    AP_Compass_Backend(compass)
{
    memset(_compass_instance, 0, sizeof(_compass_instance));
    _compass._setup_earth_field();
    _compass.setHIL(0,1,1,1);
    _compass.setHIL(1,1,1,1);
}

// detect the sensor
AP_Compass_Backend *AP_Compass_URUS::detect(Compass &compass)
{
    AP_Compass_URUS *sensor = new AP_Compass_URUS(compass);
    if (sensor == nullptr) {
        return nullptr;
    }
    if (!sensor->init()) {
        delete sensor;
        return nullptr;
    }
    return sensor;
}

bool
AP_Compass_URUS::init(void)
{
    // register two compass instances
    for (uint8_t i=0; i<URUS_NUM_COMPASSES; i++) {
        _compass_instance[i] = register_compass();
    }
    return true;
}

void AP_Compass_URUS::read()
{
    for (uint8_t i=0; i < ARRAY_SIZE(_compass_instance); i++) {
        if (_compass._hil.healthy[i]) {
            uint8_t compass_instance = _compass_instance[i];
            Vector3f field = _compass._hil.field[compass_instance];
            rotate_field(field, compass_instance);
            publish_raw_field(field, compass_instance);
            correct_field(field, compass_instance);
            uint32_t saved_last_update = _compass.last_update_usec(compass_instance);
            publish_filtered_field(field, compass_instance);
            set_last_update_usec(saved_last_update, compass_instance);
        }
    }
}
