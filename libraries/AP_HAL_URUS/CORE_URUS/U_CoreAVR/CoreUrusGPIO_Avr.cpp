
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusGPIO.h"
#include "CoreUrusGPIO_Avr.h"

#include <avr/interrupt.h>
#include <avr/io.h>

#include "utility/pins_arduino_mega.h"

#define analogInPinToBit(P) (P)


// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.
//
// These perform slightly better as macros compared to inline functions
//
#define digitalPinToPort(P) ( pgm_read_byte( digital_pin_to_port_PGM + (P) ) )
#define digitalPinToBitMask(P) ( pgm_read_byte( digital_pin_to_bit_mask_PGM + (P) ) )
#define digitalPinToTimer(P) ( pgm_read_byte( digital_pin_to_timer_PGM + (P) ) )
#define analogInPinToBit(P) (P)
#define portOutputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_output_PGM + (P))) )
#define portInputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_input_PGM + (P))) )
#define portModeRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_mode_PGM + (P))) )

extern const AP_HAL::HAL& hal;

AP_HAL::Proc CLCoreUrusGPIO_Avr::_interrupt_6 = NULL;

SIGNAL(INT6_vect) {
    if (CLCoreUrusGPIO_Avr::_interrupt_6) {
        CLCoreUrusGPIO_Avr::_interrupt_6();
    }
}

//CLCoreUrusGPIO_Avr Constructor
CLCoreUrusGPIO_Avr::CLCoreUrusGPIO_Avr()
{}

void CLCoreUrusGPIO_Avr::init()
{}

void CLCoreUrusGPIO_Avr::pinMode(uint8_t pin, uint8_t output)
{
    uint8_t bit = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);
    volatile uint8_t *reg;

    if (port == NOT_A_PIN) return;

    // JWS: can I let the optimizer do this?
    reg = portModeRegister(port);

    if (output == HAL_GPIO_INPUT)
    {
        uint8_t oldSREG = SREG;
        cli();
        *reg &= ~bit;
        SREG = oldSREG;
    }
    else
    {
        uint8_t oldSREG = SREG;
        cli();
        *reg |= bit;
        SREG = oldSREG;
    }

}

int8_t CLCoreUrusGPIO_Avr::analogPinToDigitalPin(uint8_t pin)
{
    return analogInputToDigitalPin(pin);
}

uint8_t CLCoreUrusGPIO_Avr::read(uint8_t pin) {
    uint8_t bit = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);

    if (port == NOT_A_PIN) return 0;

    if (*portInputRegister(port) & bit) return 1;
    return 0;
}

void CLCoreUrusGPIO_Avr::write(uint8_t pin, uint8_t value)
{
    uint8_t bit = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);
    volatile uint8_t *out;

    if (port == NOT_A_PIN) return;

    out = portOutputRegister(port);

    uint8_t oldSREG = SREG;
    cli();

    if (value == 0)
    {
        *out &= ~bit;
    }
    else
    {
        *out |= bit;
    }

    SREG = oldSREG;
}

void CLCoreUrusGPIO_Avr::toggle(uint8_t pin)
{
    uint8_t bit = digitalPinToBitMask(pin);
    uint8_t port = digitalPinToPort(pin);
    volatile uint8_t *out;

    if (port == NOT_A_PIN) return;

    out = portOutputRegister(port);

    uint8_t oldSREG = SREG;
    cli();

    *out ^= bit;

    SREG = oldSREG;
}

/* Alternative interface: */
AP_HAL::DigitalSource* CLCoreUrusGPIO_Avr::channel(uint16_t n)
{
    return new CLCoreDigitalSource_Avr(n);
}

/* Interrupt interface: */
bool CLCoreUrusGPIO_Avr::attach_interrupt(uint8_t interrupt_num, AP_HAL::Proc p,
        uint8_t mode)
{
    /* Mode is to set the ISCn0 and ISCn1 bits.
     * These correspond to the GPIO_INTERRUPT_ defs in AP_HAL.h */
    if (!((mode == HAL_GPIO_INTERRUPT_LOW)||
          (mode == HAL_GPIO_INTERRUPT_HIGH)||
          (mode == HAL_GPIO_INTERRUPT_FALLING)||
          (mode == HAL_GPIO_INTERRUPT_RISING))) return false;
    if (interrupt_num == 6) {
	uint8_t oldSREG = SREG;
	cli();
        _interrupt_6 = p;
        /* Set the ISC60 and ICS61 bits in EICRB according to the value
         * of mode. */
        EICRB = (EICRB & ~((1 << ISC60) | (1 << ISC61))) | (mode << ISC60);
        EIMSK |= (1 << INT6);
	SREG = oldSREG;
        return true;
    } else {
        return false;
    }
}

bool CLCoreUrusGPIO_Avr::usb_connected(void)
{
#if HAL_GPIO_USB_MUX_PIN != -1
    pinMode(HAL_GPIO_USB_MUX_PIN, HAL_GPIO_INPUT);
    return !read(HAL_GPIO_USB_MUX_PIN);
#else
    return false;
#endif
}

// CLCoreDigitalSource_Avr Constructor
CLCoreDigitalSource_Avr::CLCoreDigitalSource_Avr(uint8_t v) :
    _v(v)
{}

void CLCoreDigitalSource_Avr::mode(uint8_t output)
{
    hal.gpio->pinMode(_v, output);
}

uint8_t CLCoreDigitalSource_Avr::read()
{
    return hal.gpio->read(_v);
}

void CLCoreDigitalSource_Avr::write(uint8_t value)
{
    return hal.gpio->write(_v,value);
}

void CLCoreDigitalSource_Avr::toggle()
{
    write(!read());
}

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
