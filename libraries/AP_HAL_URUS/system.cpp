#include <stdarg.h>

#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/system.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_URUS

#include "CORE_URUS/CORE_URUS.h"

#include <stdio.h>

static const NSCORE_URUS::CLCORE_URUS& _urus_core = NSCORE_URUS::get_CORE();

namespace AP_HAL {

void init()
{
#if 0
    printf("system!\n");
#endif
}

void panic(const char *errormsg, ...)
{
    va_list ap;

    fflush(stdout);
    va_start(ap, errormsg);
    vprintf(errormsg, ap);
    va_end(ap);
    printf("\n");

    for(;;);
}

uint32_t micros()
{
    return _urus_core.timers->get_core_micros32();
}

uint32_t millis()
{
    return _urus_core.timers->get_core_millis32();
}

uint64_t micros64()
{
    return _urus_core.timers->get_core_micros64();
}

uint64_t millis64()
{
    return _urus_core.timers->get_core_millis64();
}

} // namespace AP_HAL

#endif // CONFIG_HAL_BOARD == HAL_BOARD_URUS
