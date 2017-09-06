#pragma once

#include <AP_HAL/AP_HAL.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusAnalogIn.h"

class CLCoreUrusAnalogSource_Cygwin : public NSCORE_URUS::CLCoreUrusAnalogSource {
public:
    CLCoreUrusAnalogSource_Cygwin(float v);
    float read_average();
    float read_latest();
    void set_pin(uint8_t p);
    void set_stop_pin(uint8_t p);
    void set_settle_time(uint16_t settle_time_ms);
    float voltage_average();
    float voltage_latest();
    float voltage_average_ratiometric() { return voltage_average(); }
private:
    float _v;
};

class CLCoreUrusAnalogIn_Cygwin : public NSCORE_URUS::CLCoreUrusAnalogIn {
public:
    CLCoreUrusAnalogIn_Cygwin();
    void init();
    AP_HAL::AnalogSource* channel(int16_t n);
    float board_voltage(void);
};

#endif // __CYGWIN__
