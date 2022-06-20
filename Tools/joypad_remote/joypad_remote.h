
#pragma once

#include <AP_HAL/AP_HAL.h>
#include <AP_InertialSensor/AP_InertialSensor.h>
#include <AP_Scheduler/AP_Scheduler.h>
#include <RC_Channel/RC_Channel.h>

#include "analog_sensor.h"
#include "Parameters.h"

#define BUTTON_ARRAY_LENGTH 4
#define PIN_FIRST   22
#define PIN_LAST    44

#define CFG_B737        1
#define CFG_BCH         2
#define CFG_ELIPEDAL    3
#define CFG_BCH_EXTRA   4
#define CFG_ELIPANEL    5

#define CONFIG_JOYPAD CFG_B737

#if CONFIG_JOYPAD == CFG_B737
#define CONTROLLER_DATA_CNT 2
#define SENSORS_COUNT 10
#define DISABLED_SWITCHING 1
#define CONFIG_SENSOR_CAL DISABLED
#elif CONFIG_JOYPAD == CFG_BCH
#define CONTROLLER_DATA_CNT 1
#define SENSORS_COUNT 7
#define CONFIG_SENSOR_CAL DISABLED
#elif CONFIG_JOYPAD == CFG_ELIPANEL
#define CONTROLLER_DATA_CNT 1
#define SENSORS_COUNT 7
#define DISABLED_SWITCHING 1
#define CONFIG_SENSOR_CAL ENABLED
#elif CONFIG_JOYPAD == CFG_ELIPEDAL
#define CONTROLLER_DATA_CNT 1
#define DISABLED_SWITCHING 1
#define ENABLED_EXT_MUX 1
#define SENSORS_COUNT 7
#define CONFIG_SENSOR_CAL DISABLED
#elif CONFIG_JOYPAD == CFG_BCH_EXTRA
#define CONTROLLER_DATA_CNT 1
#define SENSORS_COUNT 7
#define DISABLED_SWITCHING 1
#define CONFIG_SENSOR_CAL DISABLED
//#define ENABLED_EXT_MUX 1
#endif

class JoypadRemote {
public:

    JoypadRemote();

    void setup();
    void loop();
    void load_parameters(void);

private:

    enum STICKS {
        LS_X = 0,
        LS_Y,
        RS_X,
        RS_Y,
        ST_X,
        ST_Y,
        LS_X1,
        LS_Y1,
        RS_X1,
        RS_Y1,
        ST_X1,
        ST_Y1
    };

#pragma pack(push, 1)
    typedef struct __data_controller
    {
        uint8_t button_array[BUTTON_ARRAY_LENGTH];
        uint8_t dpad_left_on : 1;
        uint8_t dpad_up_on : 1;
        uint8_t dpad_right_on : 1;
        uint8_t dpad_down_on : 1;
        uint8_t dummy : 4;

        int16_t left_stick_x;
        int16_t left_stick_y;
        int16_t right_stick_x;
        int16_t right_stick_y;
        int16_t stick3_x;
        int16_t stick3_y;

    } data_controller_t;
#pragma pack(pop)

    Parameters g;
    AP_Param param_loader{var_info};

    AP_Scheduler scheduler;
    AnalogSensor::state_t state[SENSORS_COUNT];
#ifndef ENABLED_EXT_MUX
    AnalogSensor *_analogsensor[SENSORS_COUNT];
#else
    AnalogSensor *_analogsensor[2];
#endif // ENABLED_EXT_MUX

    static data_controller_t controller_data_buffer1;
    static data_controller_t controller_data_buffer2;
    data_controller_t controller_data1;
    data_controller_t controller_data2;

    uint32_t ins_counter;
    static const AP_Scheduler::Task scheduler_tasks[] PROGMEM;

    uint32_t nowmicros;
    uint8_t pin;

    int16_t filtered_value[SENSORS_COUNT];
    int16_t filtered_value_buf[SENSORS_COUNT];

    static volatile bool _sending;
    static volatile uint8_t inByte;

    bool sw_pins;
    bool sw_pins_pushing;

    RC_Channel *rc[SENSORS_COUNT];

    uint8_t _cnt_sw_filter;
    uint8_t _cal_ch_mask;

    uint8_t _tone_cnt;
    uint8_t _tone_speed;
    uint8_t _cnt_ch_cal_min;
    uint8_t _cnt_ch_cal_max;
    bool _tone_on_off;
    uint8_t _cnt_sw_controller;
    uint8_t _cnt_update_sensor;

    bool _in_calibration;

    void update_sensor(void);
    void send_data(void);
    void set_data(void);
    void live(void);
    void beep(void);
    data_controller_t get_empty_data_controller(void);
    void set_controller_data(data_controller_t controller_data_set, uint8_t id);

    static const AP_Param::Info var_info[];
};

extern const AP_HAL::HAL& hal;
extern JoypadRemote joypadremote;
