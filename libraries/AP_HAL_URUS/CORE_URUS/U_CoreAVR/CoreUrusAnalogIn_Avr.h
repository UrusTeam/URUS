#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusAnalogIn.h"

#define AVR_INPUT_MAX_CHANNELS 12

class CLCoreUrusAnalogSource_Avr : public NSCORE_URUS::CLCoreUrusAnalogSource {
public:
    friend class CLCoreUrusAnalogIn_Avr;

    CLCoreUrusAnalogSource_Avr(uint8_t v);
    float read_average();
    float read_latest();
    void set_pin(uint8_t p);
    void set_stop_pin(uint8_t p);
    void set_settle_time(uint16_t settle_time_ms);
    float voltage_average();
    float voltage_latest();
    float voltage_average_ratiometric();

    /* new_sample(): called with value of ADC measurments, from interrput */
    void new_sample(uint16_t);
    /* setup_read(): called to setup ADC registers for next measurment,
     * from interrupt */
    void setup_read();

    /* read_average: called to calculate and clear the internal average.
     * implements read_average(), unscaled. */
    float _read_average();

    /* stop_read(): called to stop device measurement */
    void stop_read();

    /* reading_settled(): called to check if we have read for long enough */
    bool reading_settled();
private:
    /* following three are used from both an interrupt and normal thread */
    volatile uint8_t _sum_count;
    volatile uint16_t _sum;
    volatile uint16_t _latest;
    float _last_average;

    /* _pin designates the ADC input mux for the sample */
    uint8_t _pin;

    /* _stop_pin designates a digital pin to use for
       enabling/disabling the analog device */
    uint8_t _stop_pin;
    uint16_t _settle_time_ms;
    uint32_t _read_start_time_ms;

};

class CLCoreUrusAnalogIn_Avr : public NSCORE_URUS::CLCoreUrusAnalogIn {
public:
    CLCoreUrusAnalogIn_Avr();
    void init() override;
    AP_HAL::AnalogSource* channel(int16_t n);
    float board_voltage(void);

protected:
    CLCoreUrusAnalogSource_Avr* _create_channel(int16_t num);
    void _register_channel(CLCoreUrusAnalogSource_Avr*);
    void _timer_event(void);
    CLCoreUrusAnalogSource_Avr* _channels[AVR_INPUT_MAX_CHANNELS];
    int16_t _num_channels;
    int16_t _active_channel;
    uint16_t _channel_repeat_count;

private:
    CLCoreUrusAnalogSource_Avr _vcc;
};

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
