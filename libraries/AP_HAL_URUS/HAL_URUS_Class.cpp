#include <AP_HAL/AP_HAL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_URUS

#include <assert.h>

#include "AP_HAL_URUS.h"
#include "HAL_URUS_Class.h"
#include "CORE_URUS/CORE_URUS.h"

static const NSCORE_URUS::CLCORE_URUS& _urus_core = NSCORE_URUS::get_CORE();

HAL_URUS::HAL_URUS() :
    AP_HAL::HAL(
        nullptr,  /* uartA */
        nullptr,  /* uartB */
        nullptr,  /* uartC */
        nullptr,  /* uartD */
        nullptr,  /* uartE */
        nullptr,  /* uartF */
        nullptr,
        nullptr, /* spi */
        nullptr, /* analogin */
        nullptr, /* storage */
        nullptr, /* console */
        nullptr, /* gpio */
        nullptr,  /* rcinput */
        nullptr, /* rcoutput */
        nullptr, /* scheduler */
        nullptr, /* util */
        nullptr, /* onboard optical flow */
        nullptr) /* can */
{
    scheduler = NSCORE_URUS::get_scheduler();
    uartA = NSCORE_URUS::get_uartDriver();
    console = uartA;
    i2c_mgr = NSCORE_URUS::get_I2CDeviceManager();
    spi = NSCORE_URUS::get_SPIDeviceManager();
    analogin = NSCORE_URUS::get_AnalogIn();
}

void HAL_URUS::run(int argc, char * const argv[], Callbacks* callbacks) const
{
    assert(callbacks);

    _urus_core.init_core();

    scheduler->init();
    uartA->begin(115200);

    callbacks->setup();

    for (;;) {
        callbacks->loop();
    }
}

const AP_HAL::HAL& AP_HAL::get_HAL() {
    static const HAL_URUS hal;
    return hal;
}

#endif // CONFIG_HAL_BOARD == HAL_BOARD_URUS
