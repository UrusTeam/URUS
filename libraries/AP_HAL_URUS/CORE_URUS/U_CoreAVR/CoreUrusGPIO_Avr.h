#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusGPIO.h"

#if defined(SHAL_CORE_APM1)
 # define HAL_GPIO_A_LED_PIN        37
 # define HAL_GPIO_B_LED_PIN        36
 # define HAL_GPIO_C_LED_PIN        35
 # define HAL_GPIO_LED_ON           HIGH
 # define HAL_GPIO_LED_OFF          LOW
 # define HAL_GPIO_USB_MUX_PIN	    -1
#elif defined(SHAL_CORE_APM2)
 # define HAL_GPIO_A_LED_PIN        27
 # define HAL_GPIO_B_LED_PIN        26
 # define HAL_GPIO_C_LED_PIN        25
 # define HAL_GPIO_LED_ON           LOW
 # define HAL_GPIO_LED_OFF          HIGH
 # define HAL_GPIO_USB_MUX_PIN	    23
#endif

class CLCoreUrusGPIO_Avr : public NSCORE_URUS::CLCoreUrusGPIO {
public:
    CLCoreUrusGPIO_Avr();
    void    init();
    void    pinMode(uint8_t pin, uint8_t output);
    int8_t  analogPinToDigitalPin(uint8_t pin);
    uint8_t read(uint8_t pin);
    void    write(uint8_t pin, uint8_t value);
    void    toggle(uint8_t pin);

    /* Alternative interface: */
    AP_HAL::DigitalSource* channel(uint16_t n);

    /* Interrupt interface: */
    bool    attach_interrupt(uint8_t interrupt_num, AP_HAL::Proc p,
            uint8_t mode);

    /* return true if USB cable is connected */
    bool    usb_connected(void);
/* private-ish: only to be used from the appropriate interrupt */
    static AP_HAL::Proc _interrupt_6;
};

class CLCoreDigitalSource_Avr : public NSCORE_URUS::CLCoreUrusDigitalSource {
public:
    CLCoreDigitalSource_Avr(uint8_t v);
    void    mode(uint8_t output);
    uint8_t read();
    void    write(uint8_t value); 
    void    toggle();
private:
    uint8_t _v;
};

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM