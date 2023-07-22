
#include <AP_HAL/AP_HAL.h>
#include <AP_Scheduler/AP_Scheduler.h>
#include <Filter/Filter.h>
#include <Filter/ModeFilter.h>
#include <Filter/AverageFilter.h>
#include <Filter/LowPassFilter2p.h>
#include <AP_Param/AP_Param.h>

#include "joypad_remote.h"

#define DEBUG 0

const AP_HAL::HAL& hal = AP_HAL::get_HAL();

volatile bool JoypadRemote::_sending = false;
volatile uint8_t JoypadRemote::inByte = 0;

data_controller_t JoypadRemote::controller_data_buffer1;
data_controller_t JoypadRemote::controller_data_buffer2;

JoypadRemote joypadremote;

static LowPassFilterInt *low_pass_filter[SENSORS_COUNT];
int16_t filter_tmp;

#define SCHED_TASK(func, _interval_ticks, _max_time_micros) SCHED_TASK_CLASS(JoypadRemote, &joypadremote, func, _interval_ticks, _max_time_micros)

/*
  scheduler table - all regular tasks are listed here, along with how
  often they should be called (in 20ms units) and the maximum time
  they are expected to take (in microseconds)
 */
const AP_Scheduler::Task JoypadRemote::scheduler_tasks[] = {
    SCHED_TASK(update_sensor,   50,   6000),
    SCHED_TASK(live,            10,   5000),
    SCHED_TASK(beep,            20,   6000),
};

JoypadRemote::JoypadRemote():
    sw_pins(false),
    sw_pins_pushing(false),
    _cal_ch_mask(0),
    _tone_speed(10),
    _tone_on_off(false),
    _in_calibration(false)
{
}

void JoypadRemote::live()
{
    if (_tone_on_off) {
        hal.gpio->pinMode(12, HAL_GPIO_OUTPUT);
    } else {
        hal.gpio->pinMode(12, HAL_GPIO_INPUT);
        //hal.gpio->write(12, LOW);
    }
#if CONFIG_JOYPAD != CFG_BCH_EXTRA
#ifndef ENABLED_EXT_MUX
#if CONFIG_SENSOR_CAL == ENABLED
    if (_cnt_sw_filter < 15) {
        for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
            if (filtered_value_buf[i] < 100) {
                    _cnt_sw_controller++;
            }

            if ((filtered_value_buf[i] > 5) && (filtered_value_buf[i] < 500)) {
                _cal_ch_mask |= (1 << i);
#if DEBUG == 1
                hal.console->printf("CH[%d] CHMASK: 0x%02x\n", i, _cal_ch_mask);
#endif // DEBUG
            }
        }
        _cnt_sw_filter++;
    }

    if ((1 & (_cal_ch_mask >> 2)) &&
        (1 & (_cal_ch_mask >> 1)) && (_cnt_ch_cal_min < 20)) {
        _in_calibration = true;
        _tone_on_off = true;
        hal.gpio->write(13, HAL_GPIO_LED_ON);

        if (_cnt_ch_cal_min < 10) {
            for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
                if (1 & (_cal_ch_mask >> i)) {
                    rc[i]->set_radio_min(filtered_value_buf[i]);
                }
            }
        }

        if (_cnt_ch_cal_min > 0) {
            _tone_speed = 10;
        }

        if (_cnt_ch_cal_min > 15) {
            _cnt_ch_cal_min = 20;
            _tone_speed = 2;
        }
        _cnt_ch_cal_min++;
#if DEBUG == 1
        hal.console->printf("CAL MIN: %d MASK: 0x%02x\n", _cnt_ch_cal_min, _cal_ch_mask);
#endif // DEBUG
    } else {
        if ((filtered_value_buf[2] > (rc[2]->get_radio_min() + 100)) && (_cnt_ch_cal_max < 30) && (_cnt_ch_cal_min > 15)) {
            _tone_speed = 7;
            if (_cnt_ch_cal_max > 28) {
                _cnt_ch_cal_max = 30;
                for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
                    if (1 & (_cal_ch_mask >> i)) {
                        rc[i]->set_radio_max(filtered_value_buf[i]);
                        rc[i]->set_radio_trim(filtered_value_buf[i] - ((filtered_value_buf[i]) / 3));
                        rc[i]->save_eeprom();
                    }
                }
                hal.gpio->write(13, LOW);
                hal.gpio->write(PIN_REBOOT, LOW);
                _tone_on_off = false;
                _in_calibration = false;
            }
            _cnt_ch_cal_max++;
#if DEBUG == 1
            hal.console->printf("CHAN MASK AFTER: 0x%02x\n", _cal_ch_mask);
#endif // DEBUG
        }
    }
#elif CONFIG_SENSOR_CAL == DISABLED
    _cnt_sw_filter = 100;
    _cnt_sw_controller = 100;
#endif // CONFIG_SENSOR_CAL
#else
/*
    if (_cnt_update_sensor == 0) {
        hal.gpio->write( 8, 0);
        hal.gpio->write( 9, 0);
        hal.gpio->write(10, 0);
    }

    if (_cnt_update_sensor == 1) {
        hal.gpio->write( 8, 1);
        hal.gpio->write( 9, 0);
        hal.gpio->write(10, 0);
    }

    if (_cnt_update_sensor == 2) {
        hal.gpio->write( 8, 1);
        hal.gpio->write( 9, 1);
        hal.gpio->write(10, 0);
    }

    _cnt_update_sensor++;
    _cnt_update_sensor = _cnt_update_sensor % 4;
*/
#endif
#endif // CONFIG_JOYPAD

    static uint8_t cntlive = 0;
    cntlive++;
    cntlive %= 10;
    if (cntlive == 0) {
        hal.gpio->toggle(13);
#if DEBUG == 1
        hal.console->printf("CNTContrllr: %u CNTfilt %u\n", _cnt_sw_controller, _cnt_sw_filter);
        //hal.console->printf("rc1: %u rc2: %u\n", state[0].distance_cm, state[1].distance_cm);
        for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
            int16_t maxrc = rc[i]->get_radio_max();
            int16_t trimrc = rc[i]->get_radio_trim();
            int16_t minrc = rc[i]->get_radio_min();

            //hal.console->printf("rc[%d]: dist_cm:%u filtval:%d maxrc:%u trimrc:%d minrc:%d\n", i, state[i].distance_cm, filtered_value[i], maxrc, trimrc, minrc);
            hal.console->printf("rc[%d]: dist_cm:%u filtval:%d filtbuf:%d\n", i, state[i].distance_cm, filtered_value[i], filtered_value_buf[i]);
        }
        hal.console->printf("--------\n");
#endif // DEBUG
    }
}

mincenmax_ir_t JoypadRemote::_get_mincenmax(uint8_t idx, int16_t sval)
{
    mincenmax_ir_t mincemaxtmp;
    int16_t last_sensor_max = 0;
    int16_t last_sensor_min = 0;

    //rc[idx]->load_eeprom();

    last_sensor_max = rc[idx]->get_radio_max();
    last_sensor_min = rc[idx]->get_radio_min();

    int16_t resmax = MAX(sval, last_sensor_max);
    if (resmax == 0) {
        resmax = sval;
    }

    int16_t resmin = MIN(sval, last_sensor_min);
    if (resmin == 0) {
        resmin = sval;
    }

    mincemaxtmp.min = resmin;
    mincemaxtmp.cen = sval;
    mincemaxtmp.max = resmax;

    return mincemaxtmp;
}

void JoypadRemote::_set_conf_sensor_ir(uint8_t idx, mincenmax_ir_t mincendat)
{
    rc[idx]->set_radio_max(mincendat.max);
    rc[idx]->set_radio_trim(mincendat.cen);
    rc[idx]->set_radio_min(mincendat.min);
}

void JoypadRemote::_calibrate_sens()
{
    uint32_t sw_timer = 0;
    uint8_t sw_cnt = 0;
    uint8_t sw_stat_last = hal.gpio->read(PIN_SWCONFIG);

    sw_timer = AP_HAL::millis();

    int16_t sensor_val = 0;
    uint8_t sw_stat = 0;
    uint32_t now_time = 0;

    while (_in_calibration) {
        _update_sens(1);

        sw_stat = hal.gpio->read(PIN_SWCONFIG);
        if (sw_stat != sw_stat_last) {
            sw_stat_last = sw_stat;
            if (sw_stat == 0) {
                sw_cnt++;
                for (uint8_t j = 0; j < SENSORS_COUNT; j++) {
                    sensor_val = state[j].distance_cm;
                    mincenmax_ir_t mincenmaxdat = _get_mincenmax(j, sensor_val);
                    _set_conf_sensor_ir(j, mincenmaxdat);
                }
            }
        }

        if (sw_stat == 1) {
            sw_timer = AP_HAL::millis();
        }

        now_time = AP_HAL::millis();

        if ((now_time - sw_timer) > 4000) {
            for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
                rc[i]->save_eeprom();
            }

            _in_calibration = false;
            hal.gpio->write(13, LOW);
            hal.gpio->write(PIN_REBOOT, LOW);
            hal.scheduler->reboot(true);
        }

        hal.scheduler->delay(50);
        beep();
    }
}

void JoypadRemote::_update_sens(uint16_t delayms)
{
    if (delayms <= 1) {
        for (uint8_t j = 0; j < SENSORS_COUNT; j++) {
#if CONFIG_JOYPAD == CFG_ELIPEDAL
            //_analogsensor[0]->update();
            _analogsensor[1]->update();
#else
            _analogsensor[j]->update();
#endif
        }
        return;
    }

    for (uint8_t i = 0; i < 40; i++) {
        hal.scheduler->delay(delayms);
        for (uint8_t j = 0; j < SENSORS_COUNT; j++) {
#if CONFIG_JOYPAD == CFG_ELIPEDAL
            //_analogsensor[0]->update();
            _analogsensor[1]->update();
#else
            _analogsensor[j]->update();
#endif
        }
        beep();
    }
}

void JoypadRemote::_detect_controller()
{
    _update_sens(50);

    if ((state[3].distance_cm < 100) && (state[4].distance_cm < 100) && (state[5].distance_cm < 100)) {
        _cnt_sw_controller = 1;
    } else {
        _cnt_sw_controller = 100;
    }
}

void JoypadRemote::_check_cal_mode()
{
    _sending = true;
    _tone_on_off = true;
    _tone_speed = 10;

    _cnt_sw_filter = 100;

    uint8_t sw_stat = hal.gpio->read(PIN_SWCONFIG);
    if (sw_stat == 0) {
        mincenmax_ir_t mincenmaxdat;
        mincenmaxdat.max = 0;
        mincenmaxdat.cen = 0;
        mincenmaxdat.min = 0;

        hal.scheduler->delay(500);

        for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
            _set_conf_sensor_ir(i, mincenmaxdat);
            rc[i]->save_eeprom();
        }

        _tone_speed = 2;
        _update_sens(5);
        hal.scheduler->delay(500);
        _update_sens(5);
        hal.scheduler->delay(500);
        _in_calibration = false;
        hal.gpio->write(13, LOW);
        hal.gpio->write(PIN_REBOOT, LOW);
        hal.scheduler->reboot(true);
    }

    if (state[2].distance_cm < 200) {
        _tone_speed = 2;
        _in_calibration = true;
        _calibrate_sens();
    }

    _tone_on_off = false;

    _sending = false;
}

void JoypadRemote::setup(void)
{
#if DEBUG == 1
    hal.uartA->begin(115200);
#endif

    hal.scheduler->delay(300);
    hal.uartA->flush();

    hal.gpio->pinMode(5, HAL_GPIO_OUTPUT);
    hal.gpio->write(5, LOW);

#if CONFIG_SENSOR_CAL == NULL_SENSOR_CAL
    hal.gpio->pinMode(PIN_SWCONFIG, HAL_GPIO_INPUT);
    hal.gpio->write(PIN_SWCONFIG, HIGH);
#endif // CONFIG_SENSOR_CAL

#ifndef ENABLED_EXT_MUX
    hal.gpio->pinMode(13, HAL_GPIO_OUTPUT);
    hal.gpio->write(13, LOW);
    hal.gpio->pinMode(12, HAL_GPIO_OUTPUT);
    hal.gpio->write(12, LOW);
    hal.gpio->pinMode(PIN_REBOOT, HAL_GPIO_OUTPUT);
    hal.gpio->write(PIN_REBOOT, HIGH);
#else
    hal.gpio->pinMode(13, HAL_GPIO_OUTPUT);
    hal.gpio->write(13, LOW);
    hal.gpio->pinMode(8, HAL_GPIO_OUTPUT);
    hal.gpio->write(8, 0);
    hal.gpio->pinMode(9, HAL_GPIO_OUTPUT);
    hal.gpio->write(9, 0);
    hal.gpio->pinMode(10, HAL_GPIO_OUTPUT);
    hal.gpio->write(10, 0);
#endif

    hal.scheduler->delay(2000);

    _tone_cnt = -1;
    _tone_on_off = true;
    beep();
    hal.scheduler->delay(200);
    _tone_cnt = -1;
    beep();
    hal.scheduler->delay(200);
    _tone_on_off = false;

    _cal_ch_mask = 0;
    load_parameters();

    for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
        state[i].pin = i;
#ifndef ENABLED_EXT_MUX
        _analogsensor[i] = new AnalogSensor(&state[i]);
        low_pass_filter[i] = new LowPassFilterInt(700, 15);
#else
        low_pass_filter[i] = new LowPassFilterInt(350, 50);
#endif // ENABLED_EXT_MUX
    }

#ifdef ENABLED_EXT_MUX
        _cnt_sw_controller = 0;
        _cnt_sw_filter = 100;
        _cnt_update_sensor = 0;

        //_analogsensor[0] = new AnalogSensor(&state[0]);
        _analogsensor[1] = new AnalogSensor(&state[1]);
#endif // ENABLED_EXT_MUX

#if CONFIG_SENSOR_CAL == NULL_SENSOR_CAL
    _tone_on_off = true;
    _tone_speed = 10;
    _detect_controller();
    _tone_on_off = false;

    if (_cnt_sw_controller <= 1) {
        rc[0] = &g.rc_1;
        rc[1] = &g.rc_2;
        rc[2] = &g.rc_3;
        rc[3] = &g.rc_4;
        rc[4] = &g.rc_5;
        rc[5] = &g.rc_6;
        rc[6] = &g.rc_7;
#if SENSORS_COUNT >= 8
        rc[7] = &g.rc_8;
        rc[8] = &g.rc_9;
        rc[9] = &g.rc_10;
#endif
    } else {
        rc[0] = &g.rc_1B;
        rc[1] = &g.rc_2B;
        rc[2] = &g.rc_3B;
        rc[3] = &g.rc_4B;
        rc[4] = &g.rc_5B;
        rc[5] = &g.rc_6B;
        rc[6] = &g.rc_7B;
#if SENSORS_COUNT >= 8
        rc[7] = &g.rc_8B;
        rc[8] = &g.rc_9B;
        rc[9] = &g.rc_10B;
#endif
    }
#else
    rc[0] = &g.rc_1;
    rc[1] = &g.rc_2;
    rc[2] = &g.rc_3;
    rc[3] = &g.rc_4;
    rc[4] = &g.rc_5;
    rc[5] = &g.rc_6;
    rc[6] = &g.rc_7;
#if SENSORS_COUNT >= 8
    rc[7] = &g.rc_8;
    rc[8] = &g.rc_9;
    rc[9] = &g.rc_10;
#endif
#endif // CONFIG_SENSOR_CAL

    for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
        rc[i]->set_angle(300);
        rc[i]->set_default_dead_zone(2);
#if CONFIG_SENSOR_CAL == DISABLED
        rc[i]->set_radio_max(1024);
        rc[i]->set_radio_trim((1024 - 1) / 2);
        rc[i]->set_radio_min(1);
        rc[i]->set_default_dead_zone(2);
#endif // CONFIG_SENSOR_CAL
    }

    for (int i = PIN_FIRST; i < PIN_LAST; i++){
        hal.gpio->pinMode(i, HAL_GPIO_INPUT);
        hal.gpio->write(i, HIGH);
    }

    controller_data_buffer1 = get_empty_data_controller();
    controller_data_buffer2 = get_empty_data_controller();
    controller_data1 = get_empty_data_controller();
    controller_data2 = get_empty_data_controller();

    _sending = true;

    hal.scheduler->register_timer_process(FUNCTOR_BIND_MEMBER(&JoypadRemote::send_data, void));
    // initialise the scheduler
    _scheduler.init(&scheduler_tasks[0], ARRAY_SIZE(scheduler_tasks));

#if CONFIG_SENSOR_CAL == NULL_SENSOR_CAL
    _check_cal_mode();
#endif // CONFIG_SENSOR_CAL

    _sending = false;
    _nowmicros = AP_HAL::micros();
}

void JoypadRemote::loop(void)
{
    if ((AP_HAL::micros() - _nowmicros) >= 20000LU) {
        _scheduler.tick();
        _scheduler.run(20000);
        _load_avg = (uint32_t)(_scheduler.load_average() * 20000.0f);
        _nowmicros = AP_HAL::micros() - _load_avg;
    }
}

void JoypadRemote::update_sensor(void)
{
#ifndef ENABLED_EXT_MUX
    for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
        _analogsensor[i]->update();
    }
#else
    //_analogsensor[0]->update();
    for (uint8_t i = 0; i < 30; i++) {
        _analogsensor[1]->update();
    }
#endif

    if (!_sending) {
        hal.scheduler->suspend_timer_procs();
        int16_t new_value[SENSORS_COUNT];

#ifndef ENABLED_EXT_MUX
        for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
            new_value[i] =  state[i].distance_cm;
            filtered_value_buf[i] = new_value[i];
            rc[i]->set_pwm(new_value[i]);

            filtered_value[i] = 500 + low_pass_filter[i]->apply(rc[i]->get_control_in());

            if (new_value[i] < 50) {
                filtered_value[i] = 0;
            }
        }
#else
        if (_cnt_update_sensor < 3) {
            new_value[_cnt_update_sensor] =  state[1].distance_cm;
            filtered_value_buf[_cnt_update_sensor] = new_value[_cnt_update_sensor];
            rc[_cnt_update_sensor]->set_pwm(new_value[_cnt_update_sensor]);

            filtered_value[_cnt_update_sensor] = 500 + low_pass_filter[_cnt_update_sensor]->apply(rc[_cnt_update_sensor]->get_control_in());
        }
/*
        if (new_value[_cnt_update_sensor] < 50) {
            filtered_value[_cnt_update_sensor] = 0;
        }
*/

    if (_cnt_update_sensor == 0) {
        hal.gpio->write( 8, 0);
        hal.gpio->write( 9, 0);
        hal.gpio->write(10, 0);
    }

    if (_cnt_update_sensor == 1) {
        hal.gpio->write( 8, 1);
        hal.gpio->write( 9, 0);
        hal.gpio->write(10, 0);
    }

    if (_cnt_update_sensor == 2) {
        hal.gpio->write( 8, 1);
        hal.gpio->write( 9, 1);
        hal.gpio->write(10, 0);
    }

    _cnt_update_sensor++;
    _cnt_update_sensor = _cnt_update_sensor % 4;

#endif // ENABLED_EXT_MUX
        set_data();
        hal.scheduler->resume_timer_procs();
    }
}

void JoypadRemote::set_data(void)
{

    for (int i = 0; i < BUTTON_ARRAY_LENGTH; i++) {
        controller_data1.button_array[i] = 0;
    }

    for (int i = PIN_FIRST; i < PIN_LAST; i++){
        controller_data1.button_array[(i - PIN_FIRST) / 8] |= (!hal.gpio->read(i)) << ((i - PIN_FIRST) % 8);
    }

#ifndef DISABLED_SWITCHING
    if (!hal.gpio->read(25) && !sw_pins_pushing) {
        sw_pins = !sw_pins;
        sw_pins_pushing = true;
        hal.gpio->toggle(13);
    } else if (hal.gpio->read(25) && sw_pins_pushing) {
        sw_pins_pushing = false;
    }

    if (sw_pins) {
        if (!hal.gpio->read(26)) {
            controller_data1.button_array[(26 - PIN_FIRST) / 8] &= ~(1 << ((26 - PIN_FIRST) % 8));
            controller_data1.button_array[(28 - PIN_FIRST) / 8] |= !hal.gpio->read(26) << ((28 - PIN_FIRST) % 8);
        }

        if (!hal.gpio->read(27)) {
            controller_data1.button_array[(27 - PIN_FIRST) / 8] &= ~(1 << ((27 - PIN_FIRST) % 8));
            controller_data1.button_array[(29 - PIN_FIRST) / 8] |= !hal.gpio->read(27) << ((29 - PIN_FIRST) % 8);
        }
    }
#endif
#if DEBUG == 1
    static uint8_t cntlive = 0;
    cntlive++;
    cntlive %= 100;
#endif // DEBUG
#ifndef DISABLED_ANALOG
    if (!((_cnt_sw_controller > 25 ) && (_cnt_sw_filter >= 15))) {
#ifndef ENABLED_EXT_MUX
        controller_data1.left_stick_x = filtered_value[LS_X];
        controller_data1.left_stick_y = filtered_value[LS_Y];
        controller_data1.right_stick_x = filtered_value[RS_X];
        controller_data1.right_stick_y = filtered_value[RS_Y];
        controller_data1.stick3_x = filtered_value[ST_X];
        controller_data1.stick3_y = filtered_value[ST_Y];
#else
        controller_data1.left_stick_x = filtered_value[RS_Y];
        controller_data1.left_stick_y = filtered_value[ST_X];
        controller_data1.right_stick_x = filtered_value[ST_Y];
        controller_data1.right_stick_y = filtered_value[LS_X];
        controller_data1.stick3_x = filtered_value[LS_Y];
        controller_data1.stick3_y = filtered_value[RS_X];
#endif // ENABLED_EXT_MUX
#if DEBUG == 1
        if (_cnt_sw_filter >= 15) {
            if (cntlive == 0) {
                hal.console->printf("Controller A detected\n");
            }
        }
#endif // DEBUG
    } else {
        controller_data2.left_stick_x = filtered_value[LS_X];
        controller_data2.left_stick_y = filtered_value[LS_Y];
        controller_data2.right_stick_x = filtered_value[RS_X];
        controller_data2.right_stick_y = filtered_value[RS_Y];
        controller_data2.stick3_x = filtered_value[ST_X];
        controller_data2.stick3_y = filtered_value[ST_Y];
#if DEBUG == 1
        if (_cnt_sw_filter >= 15) {
            if (cntlive == 0) {
                hal.console->printf("Controller B detected\n");
            }
        }
#endif // DEBUG
    }
#endif

    if (_cnt_sw_filter >= 15) {
        set_controller_data(controller_data1, 0);
    }

    for (int i = 0; i < BUTTON_ARRAY_LENGTH; i++) {
        controller_data2.button_array[i] = 0;
    }

#if CONTROLLER_DATA_CNT > 1
    if (!hal.gpio->read(27)) {
        controller_data2.button_array[0] = 63;
    } else {
        controller_data2.button_array[0] = 0;
    }

    controller_data2.right_stick_x = filtered_value[LS_X1];
    controller_data2.right_stick_y = filtered_value[LS_Y1];
    controller_data2.stick3_x = filtered_value[RS_X1];
#endif

    if (_cnt_sw_filter >= 15) {
        set_controller_data(controller_data2, 1);
    }
}

void JoypadRemote::send_data(void)
{
    if (_in_calibration) {
        return;
    }
#if DEBUG == 1
    _sending = false;
    return;
#endif // DEBUG
    if (_cnt_sw_filter < 15) {
        return;
    }

    if (_sending) {
        return;
    }

    hal.gpio->write(5, HIGH);

    _sending = true;

    if (hal.uartA->available() > 0) {
        inByte = hal.uartA->read();
        if (inByte < sizeof(data_controller_t)) {
            hal.uartA->write(((uint8_t*)&controller_data_buffer1)[inByte]);
        } else {
            inByte = inByte - sizeof(data_controller_t);
            hal.uartA->write(((uint8_t*)&controller_data_buffer2)[inByte]);
        }
    }

    _sending = false;
}

data_controller_t JoypadRemote::get_empty_data_controller(void)
{
    data_controller_t controller_data_empty;
    // Make the buttons zero
    for (int i = 0; i < BUTTON_ARRAY_LENGTH; i++) {
        controller_data_empty.button_array[i] = 0;
    }

    controller_data_empty.dpad_left_on = 0;
    controller_data_empty.dpad_up_on = 0;
    controller_data_empty.dpad_right_on = 0;
    controller_data_empty.dpad_down_on = 0;
    controller_data_empty.dummy = 0;

    // Center the sticks
    controller_data_empty.left_stick_x = 0x3E8;
    controller_data_empty.left_stick_y = 0x3E8;
    controller_data_empty.right_stick_x = 0x3E8;
    controller_data_empty.right_stick_y = 0x3E8;
    controller_data_empty.stick3_x = 0x3E8;
    controller_data_empty.stick3_y = 0x3E8;

    return controller_data_empty;
}

void JoypadRemote::set_controller_data(data_controller_t controller_data_set, uint8_t id)
{
    if (!_sending) {
        switch (id) {
        case 0:
            memcpy(&controller_data_buffer1, &controller_data_set, sizeof(data_controller_t));
            break;
        case 1:
            memcpy(&controller_data_buffer2, &controller_data_set, sizeof(data_controller_t));
            break;
        default:
            break;
        }
    }
}

void JoypadRemote::beep(void) {

    if (_tone_on_off) {
        if (_tone_cnt > _tone_speed) {
            for (uint8_t i = 0; i < 25; i++) {
                hal.gpio->write(12, HIGH);
                hal.scheduler->delay_microseconds(105);
                hal.gpio->write(12, LOW);
                hal.scheduler->delay_microseconds(115);
            }
            _tone_cnt = 0;
        }
        _tone_cnt++;
    }
}

/*
  compatibility with old pde style build
 */
void setup(void);
void loop(void);

void setup(void)
{
    joypadremote.setup();
}

void loop(void)
{
    joypadremote.loop();
}

AP_HAL_MAIN();
