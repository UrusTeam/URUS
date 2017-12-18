#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusAnalogIn.h"

#include "CoreUrusAnalogIn_Avr.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define CHANNEL_READ_REPEAT 2

extern const AP_HAL::HAL& hal;

CLCoreUrusAnalogSource_Avr::CLCoreUrusAnalogSource_Avr(uint8_t v) :
    NSCORE_URUS::CLCoreUrusAnalogSource(),
    _sum_count(0),
    _sum(0),
    _last_average(0),
    _pin(ANALOG_INPUT_NONE),
    _stop_pin(ANALOG_INPUT_NONE),
    _settle_time_ms(0)
{
    set_pin(v);
}

float CLCoreUrusAnalogSource_Avr::read_average()
{
    if (_pin == ANALOG_INPUT_BOARD_VCC) {
        uint16_t v = (uint16_t) _read_average();
        return 1126400UL / v;
    } else {
        return _read_average();
    }
}

/* read_average is called from the normal thread (not an interrupt). */
float CLCoreUrusAnalogSource_Avr::_read_average()
{
    uint16_t sum;
    uint8_t sum_count;

    if (_sum_count == 0) {
        // avoid blocking waiting for new samples
        return _last_average;
    }

    /* Read and clear in a critical section */
    uint8_t sreg = SREG;
    cli();

    sum = _sum;
    sum_count = _sum_count;
    _sum = 0;
    _sum_count = 0;

    SREG = sreg;

    float avg = sum / (float) sum_count;

    _last_average = avg;
    return avg;
}

float CLCoreUrusAnalogSource_Avr::voltage_average()
{
    float vcc_mV = hal.analogin->channel(ANALOG_INPUT_BOARD_VCC)->read_average();
    float v = read_average();
    // constrain Vcc reading so that a bad Vcc doesn't throw off
    // the reading of other sources too badly
    if (vcc_mV < 4000) {
        vcc_mV = 4000;
    } else if (vcc_mV > 6000) {
        vcc_mV = 6000;
    }
    return v * vcc_mV * 9.765625e-7; // 9.765625e-7 = 1.0/(1024*1000)
}

float CLCoreUrusAnalogSource_Avr::voltage_latest()
{
    if (_pin == ANALOG_INPUT_BOARD_VCC) {
        return read_latest() * 0.001f;
    }
    float vcc_mV = hal.analogin->channel(ANALOG_INPUT_BOARD_VCC)->read_average();
    float v = read_latest();
    // constrain Vcc reading so that a bad Vcc doesn't throw off
    // the reading of other sources too badly
    if (vcc_mV < 4000) {
        vcc_mV = 4000;
    } else if (vcc_mV > 6000) {
        vcc_mV = 6000;
    }
    return v * vcc_mV * 9.765625e-7; // 9.765625e-7 = 1.0/(1024*1000)
}

float CLCoreUrusAnalogSource_Avr::read_latest()
{
    uint8_t sreg = SREG;
    cli();
    uint16_t latest = _latest;
    SREG = sreg;
    if (_pin == ANALOG_INPUT_BOARD_VCC) {
        return 1126400UL / latest;
    } else {
        return latest;
    }
}

void CLCoreUrusAnalogSource_Avr::set_pin(uint8_t p)
{
    if (p != _pin) {
        // ensure the pin is marked as an INPUT pin
        if (p != ANALOG_INPUT_NONE && p != ANALOG_INPUT_BOARD_VCC) {
            int8_t dpin = hal.gpio->analogPinToDigitalPin(p);
            if (dpin != -1) {
                // enable as input without a pull-up. This gives the
                // best results for our analog sensors
                hal.gpio->pinMode(dpin, HAL_GPIO_INPUT);
                hal.gpio->write(dpin, 0);
            }
        }
        uint8_t sreg = SREG;
        cli();
        _sum = 0;
        _sum_count = 0;
        _last_average = 0;
        _latest = 0;
        _pin = p;
        SREG = sreg;
    }
}

/*
  return voltage from 0.0 to 5.0V, assuming a ratiometric sensor. This
  means the result is really a pseudo-voltage, that assumes the supply
  voltage is exactly 5.0V.
 */
float CLCoreUrusAnalogSource_Avr::voltage_average_ratiometric(void)
{
    float v = read_average();
    return v * (5.0f / 1023.0f);
}


void CLCoreUrusAnalogSource_Avr::set_stop_pin(uint8_t p)
{
    _stop_pin = p;
}

void CLCoreUrusAnalogSource_Avr::set_settle_time(uint16_t settle_time_ms)
{
    _settle_time_ms = settle_time_ms;
}

void CLCoreUrusAnalogSource_Avr::setup_read()
{
    if (_stop_pin != ANALOG_INPUT_NONE) {
        uint8_t digital_pin = hal.gpio->analogPinToDigitalPin(_stop_pin);
        hal.gpio->pinMode(digital_pin, HAL_GPIO_OUTPUT);
        hal.gpio->write(digital_pin, 1);
    }
    if (_settle_time_ms != 0) {
        _read_start_time_ms = AP_HAL::millis();
    }
    if (_pin == ANALOG_INPUT_BOARD_VCC) {
        ADCSRB = (ADCSRB & ~(1 << MUX5));
        ADMUX = _BV(REFS0)|_BV(MUX4)|_BV(MUX3)|_BV(MUX2)|_BV(MUX1);
    } else if (_pin == ANALOG_INPUT_NONE) {
        /* noop */
    } else {
        ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((_pin >> 3) & 0x01) << MUX5);
        ADMUX = _BV(REFS0) | (_pin & 0x07);
    }
}

void CLCoreUrusAnalogSource_Avr::stop_read()
{
    if (_stop_pin != ANALOG_INPUT_NONE) {
        uint8_t digital_pin = hal.gpio->analogPinToDigitalPin(_stop_pin);
        hal.gpio->pinMode(digital_pin, HAL_GPIO_OUTPUT);
        hal.gpio->write(digital_pin, 0);
    }
}

bool CLCoreUrusAnalogSource_Avr::reading_settled()
{
    if (_settle_time_ms != 0 && (AP_HAL::millis() - _read_start_time_ms) < _settle_time_ms) {
        return false;
    }
    return true;
}

/* new_sample is called from an interrupt. It always has access to
 *  _sum and _sum_count. Lock out the interrupts briefly with
 * cli/sei to read these variables from outside an interrupt. */
void CLCoreUrusAnalogSource_Avr::new_sample(uint16_t sample)
{
    _sum += sample;
    _latest = sample;
    if (_sum_count >= 63) {
        _sum >>= 1;
        _sum_count = 32;
    } else {
        _sum_count++;
    }
}

// CLCoreUrusAnalogIn_Avr constructor
CLCoreUrusAnalogIn_Avr::CLCoreUrusAnalogIn_Avr() :
    _vcc(CLCoreUrusAnalogSource_Avr(ANALOG_INPUT_BOARD_VCC))
{}

void CLCoreUrusAnalogIn_Avr::init()
{
    /* Register CLCoreUrusAnalogIn_Avr::_timer_event with the scheduler. */
    hal.scheduler->register_timer_process(FUNCTOR_BIND_MEMBER(&CLCoreUrusAnalogIn_Avr::_timer_event, void));
    /* Register each private channel with CLCoreUrusAnalogIn_Avr. */
    _register_channel(&_vcc);
}

AP_HAL::AnalogSource* CLCoreUrusAnalogIn_Avr::channel(int16_t n)
{
    if (n == ANALOG_INPUT_BOARD_VCC) {
            return &_vcc;
    } else {
        return _create_channel(n);
    }
}

float CLCoreUrusAnalogIn_Avr::board_voltage(void)
{
    return _vcc.voltage_latest();
}

CLCoreUrusAnalogSource_Avr* CLCoreUrusAnalogIn_Avr::_create_channel(int16_t chnum)
{
    CLCoreUrusAnalogSource_Avr *ch = new CLCoreUrusAnalogSource_Avr(chnum);
    _register_channel(ch);
    return ch;
}

void CLCoreUrusAnalogIn_Avr::_register_channel(CLCoreUrusAnalogSource_Avr* ch)
{
    if (_num_channels >= AVR_INPUT_MAX_CHANNELS) {
        for(;;) {
            hal.console->printf(
                "Error: AP_HAL_AVR::CLCoreUrusAnalogIn_Avr out of channels\n");
            hal.scheduler->delay(1000);
        }
    }
    _channels[_num_channels] = ch;
    /* Need to lock to increment _num_channels as it is used
     * by the interrupt to access _channels */
    uint8_t sreg = SREG;
    cli();
    _num_channels++;
    SREG = sreg;

    if (_num_channels == 1) {
        /* After registering the first channel, we can enable the ADC */
        PRR0 &= ~_BV(PRADC);
        ADCSRA |= _BV(ADEN);
    }
}

void CLCoreUrusAnalogIn_Avr::_timer_event(void)
{
    if (_channels[_active_channel]->_pin == ANALOG_INPUT_NONE) {
        _channels[_active_channel]->new_sample(0);
        goto next_channel;
    }

    if (ADCSRA & _BV(ADSC)) {
        /* ADC Conversion is still running - this should not happen, as we
         * are called at 1khz. */
        return;
    }

    if (_num_channels == 0) {
        /* No channels are registered - nothing to be done. */
        return;
    }

    _channel_repeat_count++;
    if (_channel_repeat_count < CHANNEL_READ_REPEAT ||
        !_channels[_active_channel]->reading_settled()) {
        /* Start a new conversion, throw away the current conversion */
        ADCSRA |= _BV(ADSC);
        return;
    }

    _channel_repeat_count = 0;

    /* Read the conversion registers. */
    {
        uint8_t low = ADCL;
        uint8_t high = ADCH;
        uint16_t sample = low | (((uint16_t)high) << 8);
        /* Give the active channel a new sample */
        _channels[_active_channel]->new_sample( sample );
    }
next_channel:
    /* stop the previous channel, if a stop pin is defined */
    _channels[_active_channel]->stop_read();
    /* Move to the next channel */
    _active_channel = (_active_channel + 1) % _num_channels;
    /* Setup the next channel's conversion */
    _channels[_active_channel]->setup_read();
    /* Start conversion */
    ADCSRA |= _BV(ADSC);
}

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
