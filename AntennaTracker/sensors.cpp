#include "Tracker.h"

void Tracker::init_barometer(bool full_calibration)
{
    gcs().send_text(MAV_SEVERITY_INFO, "Calibrating barometer");
    if (full_calibration) {
        barometer.calibrate();
    } else {
        barometer.update_calibration();
    }
    gcs().send_text(MAV_SEVERITY_INFO, "Barometer calibration complete");
}

// read the barometer and return the updated altitude in meters
void Tracker::update_barometer(void)
{
    barometer.update();
    if (should_log(MASK_LOG_IMU)) {
        Log_Write_Baro();
    }
}


/*
  update INS and attitude
 */
void Tracker::update_ahrs()
{
    ahrs.update();
}


/*
  read and update compass
 */
void Tracker::update_compass(void)
{
    if (g.compass_enabled && compass.read()) {
        ahrs.set_compass(&compass);
#if !HAL_MINIMIZE_FEATURES_AVR
        if (should_log(MASK_LOG_COMPASS)) {
            DataFlash.Log_Write_Compass(compass);
        }
#endif
    }
}

/*
  if the compass is enabled then try to accumulate a reading
 */
void Tracker::compass_accumulate(void)
{
    if (g.compass_enabled) {
        compass.accumulate();
    }
}

/*
 calibrate compass
*/
void Tracker::compass_cal_update() {
#if !HAL_MINIMIZE_FEATURES_AVR
    if (!hal.util->get_soft_armed()) {
        compass.compass_cal_update();
    }
#endif
}

/*
    Accel calibration
*/
void Tracker::accel_cal_update() {
    if (hal.util->get_soft_armed()) {
        return;
    }
    ins.acal_update();
    float trim_roll, trim_pitch;
    if (ins.get_new_trim(trim_roll, trim_pitch)) {
        ahrs.set_trim(Vector3f(trim_roll, trim_pitch, 0));
    }
}

/*
  read the GPS
 */
void Tracker::update_GPS(void)
{
/*
    current_loc = loc_sta_airport;

    compass.set_initial_location(current_loc.lat, current_loc.lng);
    set_home(current_loc);
*/
/*
    dpoint_t rot_plane((double)loc_sta_airport.lng, (double)loc_sta_airport.lat);

    dpoint_t degrot= rotate_deg_point(loc_sta_airport.lng, loc_sta_airport.lat, 0.5, rot_plane);
    loc_sta_airport.lng = (int32_t)degrot.x;
    loc_sta_airport.lat = (int32_t)degrot.y;
*/
    Vector3f velocity;

    gps.setHIL(0, AP_GPS::GPS_Status::GPS_OK_FIX_3D, hal.util->get_system_clock_ms() * 1000, loc_sta_airport, velocity, 8, 5);

    gps.update();

    static uint32_t last_gps_msg_ms;
    static uint8_t ground_start_count = 5;
    if (gps.last_message_time_ms() != last_gps_msg_ms &&
        gps.status() >= AP_GPS::GPS_OK_FIX_3D) {
        last_gps_msg_ms = gps.last_message_time_ms();

        if (ground_start_count > 1) {
            ground_start_count--;
        } else if (ground_start_count == 1) {
            // We countdown N number of good GPS fixes
            // so that the altitude is more accurate
            // -------------------------------------
            if (current_loc.lat == 0 && current_loc.lng == 0) {
                ground_start_count = 5;

            } else {
                // Now have an initial GPS position
                // use it as the HOME position in future startups
                current_loc = gps.location();
                set_home(current_loc);

                // set system clock for log timestamps
                uint64_t gps_timestamp = gps.time_epoch_usec();

                hal.util->set_system_clock(gps_timestamp);

                // update signing timestamp
#if !HAL_MINIMIZE_FEATURES_AVR
                GCS_MAVLINK::update_signing_timestamp(gps_timestamp);
#endif

                if (g.compass_enabled) {
                    // Set compass declination automatically
                    compass.set_initial_location(gps.location().lat, gps.location().lng);
                }
                ground_start_count = 0;
            }
        }
#if !HAL_MINIMIZE_FEATURES_AVR
        // log GPS data
        if (should_log(MASK_LOG_GPS)) {
            DataFlash.Log_Write_GPS(gps, 0);
        }
#endif
    }

}

