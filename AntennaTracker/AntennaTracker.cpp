/*
   Lead developers: Matthew Ridley and Andrew Tridgell

   Please contribute your ideas! See http://dev.ardupilot.org for details

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

#include "Tracker.h"

#define FORCE_VERSION_H_INCLUDE
#include "version.h"
#undef FORCE_VERSION_H_INCLUDE

#define SCHED_TASK(func, _interval_ticks, _max_time_micros) SCHED_TASK_CLASS(Tracker, &tracker, func, _interval_ticks, _max_time_micros)

/*
  scheduler table - all regular tasks apart from the fast_loop()
  should be listed here, along with how often they should be called
  (in 20ms units) and the maximum time they are expected to take (in
  microseconds)
 */
const AP_Scheduler::Task Tracker::scheduler_tasks[] PROGMEM = {
    SCHED_TASK(update_ahrs,            50,   1000),
    SCHED_TASK(read_radio,             50,    200),
    SCHED_TASK(update_tracking,        50,   1000),
    SCHED_TASK(update_GPS,             10,   4000),
    SCHED_TASK(update_compass,         10,   1500),
#if !HAL_MINIMIZE_FEATURES_AVR
    SCHED_TASK_CLASS(AP_BattMonitor,    &tracker.battery,   read,           10, 1500),
#endif
    SCHED_TASK(update_barometer,       10,   1500),
    SCHED_TASK(gcs_update,             50,   1700),
    SCHED_TASK(gcs_data_stream_send,   50,   3000),
    SCHED_TASK(compass_accumulate,     50,   1500),
    SCHED_TASK_CLASS(AP_Baro,           &tracker.barometer, accumulate,     50,  900),
    SCHED_TASK(ten_hz_logging_loop,    10,    300),
#if !HAL_MINIMIZE_FEATURES_AVR
    SCHED_TASK_CLASS(DataFlash_Class,   &tracker.DataFlash, periodic_tasks, 50,  300),
    SCHED_TASK_CLASS(AP_InertialSensor, &tracker.ins,       periodic,       50,   50),
#endif
    SCHED_TASK_CLASS(AP_Notify,         &tracker.notify,    update,         50,  100),
    SCHED_TASK(check_usb_mux,          10,    300),
    SCHED_TASK(gcs_retry_deferred,     50,   1000),
    SCHED_TASK(one_second_loop,         1,   3900),
    SCHED_TASK(compass_cal_update,     50,    100),
    SCHED_TASK(accel_cal_update,       10,    100)
};

/**
  setup the sketch - called once on startup
 */
void Tracker::setup()
{
    // load the default values of variables listed in var_info[]
    AP_Param::setup_sketch_defaults();

    loc_sta_airport.alt = 0;
    loc_sta_airport.lat = -17.672422 * 10000000UL;// * LOCATION_SCALING_FACTOR * 1000000000UL;
    loc_sta_airport.lng = -63.190124 * 10000000UL;// * LOCATION_SCALING_FACTOR * longitude_scale_float(-17.6586422) * 1000000000UL;

    loc_vor_plane.alt = 0;
    loc_vor_plane.lat = -17.670534 * 10000000UL;// * LOCATION_SCALING_FACTOR * 1000000000UL;
    loc_vor_plane.lng = -63.189188 * 10000000UL;// * LOCATION_SCALING_FACTOR * longitude_scale_float(-17.6573385) * 1000000000UL;

    init_tracker();

    // initialise the main loop scheduler
#if !HAL_MINIMIZE_FEATURES_AVR
    scheduler.init(&scheduler_tasks[0], ARRAY_SIZE(scheduler_tasks), (uint32_t)-1);
#else
    scheduler.init(&scheduler_tasks[0], ARRAY_SIZE(scheduler_tasks));
#endif
}

/**
   loop() is called continuously
 */
void Tracker::loop()
{
    // wait for an INS sample
    ins.wait_for_sample();

    // tell the scheduler one tick has passed
    scheduler.tick();

    scheduler.run(19900UL);
}

void Tracker::one_second_loop()
{
    float yaw = wrap_180_cd((nav_status.bearing+g.yaw_trim)*100); // target yaw in centidegrees
    float pitch = constrain_float(nav_status.pitch+g.pitch_trim, -90, 90) * 100; // target pitch in centidegrees

    bool direction_reversed = get_ef_yaw_direction();

    calc_angle_error(pitch, yaw, direction_reversed);

    float bf_pitch;
    float bf_yaw;
    convert_ef_to_bf(pitch, yaw, bf_pitch, bf_yaw);

    hal.console->printf("dist:%.2f bear:%.2f ahr_yawerr:%.2f bf_yaw:%.2f armed:%d\n", nav_status.distance, nav_status.bearing, nav_status.angle_error_yaw, bf_yaw, AP_Notify::flags.armed);
    // send a heartbeat
#if !HAL_MINIMIZE_FEATURES_AVR
    gcs().send_message(MSG_HEARTBEAT);
#endif

    // make it possible to change orientation at runtime
    ahrs.set_orientation();

    // sync MAVLink system ID
#if !HAL_MINIMIZE_FEATURES_AVR
    mavlink_system.sysid = g.sysid_this_mav;
#endif

    // updated armed/disarmed status LEDs
    update_armed_disarmed();

    one_second_counter++;

    if (one_second_counter >= 60) {
        if (g.compass_enabled) {
            compass.save_offsets();
        }
        one_second_counter = 0;
    }
}

void Tracker::ten_hz_logging_loop()
{
#if !HAL_MINIMIZE_FEATURES_AVR
    if (should_log(MASK_LOG_IMU)) {
        DataFlash.Log_Write_IMU(ins);
    }
    if (should_log(MASK_LOG_ATTITUDE)) {
        Log_Write_Attitude();
    }
    if (should_log(MASK_LOG_RCIN)) {
        DataFlash.Log_Write_RCIN();
    }
    if (should_log(MASK_LOG_RCOUT)) {
        DataFlash.Log_Write_RCOUT();
    }
#endif
}

const AP_HAL::HAL& hal = AP_HAL::get_HAL();

Tracker::Tracker(void)
#if !HAL_MINIMIZE_FEATURES_AVR
    : DataFlash(fwver.fw_string, g.log_bitmask)
#endif
{
    memset(&current_loc, 0, sizeof(current_loc));
    memset(&vehicle, 0, sizeof(vehicle));
}

Tracker tracker;

AP_HAL_MAIN_CALLBACKS(&tracker);
