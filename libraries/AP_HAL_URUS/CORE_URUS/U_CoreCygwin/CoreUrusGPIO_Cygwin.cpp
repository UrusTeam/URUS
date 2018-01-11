
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusGPIO.h"
#include "CoreUrusGPIO_Cygwin.h"

extern const AP_HAL::HAL& hal;

#define GPIO_DEBUG 0

//CLCoreUrusGPIO_Cygwin Constructor
CLCoreUrusGPIO_Cygwin::CLCoreUrusGPIO_Cygwin()
{}

void CLCoreUrusGPIO_Cygwin::init()
{}

void CLCoreUrusGPIO_Cygwin::pinMode(uint8_t pin, uint8_t output)
{
    pin = pin - 1U;
    if (output == HAL_GPIO_OUTPUT) {
        _pinmode[pin / 8] |= (1U << (pin % 8));
    } else {
        _pinmode[pin / 8] &= ~(1U << (pin % 8));
    }

#if GPIO_DEBUG == 1
    const char *s;
    if (output == HAL_GPIO_OUTPUT) {
        s = "OUTPUT";
    } else {
        s = "INPUT";
    }

    hal.console->printf("PIN:[0x%2x] MODE:[%s]\n", _pinmode[pin / 8], s);
#endif

}

int8_t CLCoreUrusGPIO_Cygwin::analogPinToDigitalPin(uint8_t pin)
{
	return -1;
}


uint8_t CLCoreUrusGPIO_Cygwin::read(uint8_t pin)
{
    pin = pin - 1U;
    return (_pin[pin / 8] & (1U << (pin % 8))) ? 1 : 0;
}

void CLCoreUrusGPIO_Cygwin::write(uint8_t pin, uint8_t value)
{
    pin = pin - 1U;
    if (value == HAL_GPIO_LED_ON) {
        _pin[pin / 8] |= (1U << (pin % 8));
    } else {
        _pin[pin / 8] &= ~(1U << (pin % 8));
    }

#if GPIO_DEBUG == 1
    const char *s;
    if (value == HAL_GPIO_LED_ON) {
        s = "HIGH";
    } else {
        s = "LOW";
    }

    hal.console->printf("PIN:[0x%2x] SET:[%s]\n", _pin[pin / 8], s);
#endif

}

void CLCoreUrusGPIO_Cygwin::toggle(uint8_t pin)
{
    write(pin, !read(pin));
 #if GPIO_DEBUG == 1
    pin = pin - 1U;
    hal.console->printf("TOGGLE PIN:[0x%2x]-", _pin[pin / 8]);

    if (read(pin + 1U) ==  HAL_GPIO_LED_ON) {
        hal.console->printf("[ON]\n");
    } else {
        hal.console->printf("[OFF]\n");
    }
#endif

}

/* Alternative interface: */
AP_HAL::DigitalSource* CLCoreUrusGPIO_Cygwin::channel(uint16_t n) {
    return new CLCoreDigitalSource_Cygwin(n);
}

/* Interrupt interface: */
bool CLCoreUrusGPIO_Cygwin::attach_interrupt(uint8_t interrupt_num, AP_HAL::Proc p,
        uint8_t mode) {
    return true;
}

bool CLCoreUrusGPIO_Cygwin::usb_connected(void)
{
    return false;
}

// CLCoreDigitalSource_Cygwin Constructor
CLCoreDigitalSource_Cygwin::CLCoreDigitalSource_Cygwin(uint8_t v) :
    _v(v)
{}

void CLCoreDigitalSource_Cygwin::mode(uint8_t output)
{}

uint8_t CLCoreDigitalSource_Cygwin::read() {
    return _v;
}

void CLCoreDigitalSource_Cygwin::write(uint8_t value) {
    _v = value;
}

void CLCoreDigitalSource_Cygwin::toggle() {
    _v = !_v;
}

#endif // __CYGWIN__