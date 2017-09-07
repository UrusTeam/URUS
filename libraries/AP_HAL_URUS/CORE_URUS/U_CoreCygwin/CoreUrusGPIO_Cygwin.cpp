
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusGPIO.h"
#include "CoreUrusGPIO_Cygwin.h"

//CLCoreUrusGPIO_Cygwin Constructor
CLCoreUrusGPIO_Cygwin::CLCoreUrusGPIO_Cygwin()
{}

void CLCoreUrusGPIO_Cygwin::init()
{}

void CLCoreUrusGPIO_Cygwin::pinMode(uint8_t pin, uint8_t output)
{}

int8_t CLCoreUrusGPIO_Cygwin::analogPinToDigitalPin(uint8_t pin)
{
	return -1;
}


uint8_t CLCoreUrusGPIO_Cygwin::read(uint8_t pin) {
    return 0;
}

void CLCoreUrusGPIO_Cygwin::write(uint8_t pin, uint8_t value)
{}

void CLCoreUrusGPIO_Cygwin::toggle(uint8_t pin)
{}

/* Alternative interface: */
AP_HAL::DigitalSource* CLCoreUrusGPIO_Cygwin::channel(uint16_t n) {
    return new CLCoreDigitalSource_Cygwin(0);
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