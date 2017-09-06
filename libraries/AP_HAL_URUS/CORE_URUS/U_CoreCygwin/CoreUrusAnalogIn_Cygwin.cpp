#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusAnalogIn.h"

#include "CoreUrusAnalogIn_Cygwin.h"

CLCoreUrusAnalogSource_Cygwin::CLCoreUrusAnalogSource_Cygwin(float v) :
    NSCORE_URUS::CLCoreUrusAnalogSource(),
    _v(v)
{}

float CLCoreUrusAnalogSource_Cygwin::read_average() {
    return _v;
}

float CLCoreUrusAnalogSource_Cygwin::voltage_average() {
    return 5.0f * _v / 1024.0f;
}

float CLCoreUrusAnalogSource_Cygwin::voltage_latest() {
    return 5.0f * _v / 1024.0f;
}

float CLCoreUrusAnalogSource_Cygwin::read_latest() {
    return _v;
}

void CLCoreUrusAnalogSource_Cygwin::set_pin(uint8_t p)
{}

void CLCoreUrusAnalogSource_Cygwin::set_stop_pin(uint8_t p)
{}

void CLCoreUrusAnalogSource_Cygwin::set_settle_time(uint16_t settle_time_ms)
{}

CLCoreUrusAnalogIn_Cygwin::CLCoreUrusAnalogIn_Cygwin()
{}

void CLCoreUrusAnalogIn_Cygwin::init()
{}

AP_HAL::AnalogSource* CLCoreUrusAnalogIn_Cygwin::channel(int16_t n) {
    return new CLCoreUrusAnalogSource_Cygwin(1.11);
}

float CLCoreUrusAnalogIn_Cygwin::board_voltage(void)
{
    return 5.0f;
}

#endif // __CYGWIN__
