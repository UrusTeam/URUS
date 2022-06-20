
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

JoypadRemote::data_controller_t JoypadRemote::controller_data_buffer1;
JoypadRemote::data_controller_t JoypadRemote::controller_data_buffer2;

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
    SCHED_TASK(update_sensor,   30,   6000),
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
                hal.gpio->write(11, LOW);
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
    _cnt_update_sensor = _cnt_update_sensor % 3;
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
            hal.console->printf("rc[%d]: dist_cm:%u filtval:%d filtbuf:%d\n", i, state[i].distance_cm, filtered_value[i], filtered_value_buf[i]);
        }
        hal.console->printf("--------\n");
#endif // DEBUG
    }
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

#ifndef ENABLED_EXT_MUX
    hal.gpio->pinMode(13, HAL_GPIO_OUTPUT);
    hal.gpio->write(13, LOW);
    hal.gpio->pinMode(12, HAL_GPIO_OUTPUT);
    hal.gpio->write(12, LOW);
    hal.gpio->pinMode(11, HAL_GPIO_OUTPUT);
    hal.gpio->write(11, HIGH);
#else
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

    for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
        state[i].pin = i;
#ifndef ENABLED_EXT_MUX
        _analogsensor[i] = new AnalogSensor(&state[i]);
#endif // ENABLED_EXT_MUX
        low_pass_filter[i] = new LowPassFilterInt(700, 15);
        rc[i]->set_angle(300);
        rc[i]->set_default_dead_zone(2);
#if CONFIG_SENSOR_CAL == DISABLED
        rc[i]->set_radio_max(1024);
        rc[i]->set_radio_trim((1024 - 1) / 2);
        rc[i]->set_radio_min(1);
        rc[i]->set_default_dead_zone(2);
        rc[i]->save_eeprom();
#endif // CONFIG_SENSOR_CAL
    }

#ifdef ENABLED_EXT_MUX
        _analogsensor[0] = new AnalogSensor(&state[0]);
        _analogsensor[1] = new AnalogSensor(&state[1]);
#endif // ENABLED_EXT_MUX

    for (int i = PIN_FIRST; i < PIN_LAST; i++){
        hal.gpio->pinMode(i, HAL_GPIO_INPUT);
        hal.gpio->write(i, HIGH);
    }

    controller_data_buffer1 = get_empty_data_controller();
    controller_data_buffer2 = get_empty_data_controller();
    controller_data1 = get_empty_data_controller();
    controller_data2 = get_empty_data_controller();

    hal.scheduler->register_timer_process(FUNCTOR_BIND_MEMBER(&JoypadRemote::send_data, void));
    // initialise the scheduler
    scheduler.init(&scheduler_tasks[0], ARRAY_SIZE(scheduler_tasks));
    nowmicros = AP_HAL::micros();
}

void JoypadRemote::loop(void)
{
    if ((AP_HAL::micros() - nowmicros) > 19986) {
        nowmicros = AP_HAL::micros();
        scheduler.tick();
        scheduler.run(20000);
    }
}

void JoypadRemote::update_sensor(void)
{
#ifndef ENABLED_EXT_MUX
    for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
        _analogsensor[i]->update();
    }
#else
    _analogsensor[0]->update();
    _analogsensor[1]->update();
#endif

    if (!_sending) {
        hal.scheduler->suspend_timer_procs();
        int16_t new_value[SENSORS_COUNT];

        for (uint8_t i = 0; i < SENSORS_COUNT; i++) {
            new_value[i] =  state[i].distance_cm;
            filtered_value_buf[i] = new_value[i];
            rc[i]->set_pwm(new_value[i]);

            filtered_value[i] = 500 + low_pass_filter[i]->apply(rc[i]->get_control_in());

            if (new_value[i] < 50) {
                filtered_value[i] = 0;
            }

        }

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

#ifndef DISABLED_ANALOG
    if ((_cnt_sw_controller > 25 ) && (_cnt_sw_filter >= 15)) {
        controller_data1.left_stick_x = filtered_value[LS_X];
        controller_data1.left_stick_y = filtered_value[LS_Y];
        controller_data1.right_stick_x = filtered_value[RS_X];
        controller_data1.right_stick_y = filtered_value[RS_Y];
        controller_data1.stick3_x = filtered_value[ST_X];
        controller_data1.stick3_y = filtered_value[ST_Y];
#if DEBUG == 1
        if (_cnt_sw_filter >= 15) {
            //hal.console->printf("Controller A detected\n");
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
            //hal.console->printf("Controller B detected\n");
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

JoypadRemote::data_controller_t JoypadRemote::get_empty_data_controller(void)
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
