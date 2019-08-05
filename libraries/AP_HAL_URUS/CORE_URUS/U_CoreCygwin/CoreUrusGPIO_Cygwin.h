#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusGPIO.h"

#define PORTS_NUM   4

class CLCoreUrusGPIO_Cygwin : public NSCORE_URUS::CLCoreUrusGPIO {
public:
    CLCoreUrusGPIO_Cygwin();
    void    init();
    void    pinMode(uint8_t pin, uint8_t output);
    int8_t  analogPinToDigitalPin(uint8_t pin);
    uint8_t read(uint8_t pin);
    void    write(uint8_t pin, uint8_t value);
    void    toggle(uint8_t pin);

    void    write_port(uint8_t portnr, uint8_t value) {};
    uint8_t read_port(uint8_t portnr) { return 0;};

    /* Alternative interface: */
    AP_HAL::DigitalSource* channel(uint16_t n);

    /* Interrupt interface: */
    bool    attach_interrupt(uint8_t interrupt_num, AP_HAL::Proc p,
            uint8_t mode);

    /* return true if USB cable is connected */
    bool    usb_connected(void);

private:
    uint8_t _pin[PORTS_NUM];
    uint8_t _pinmode[PORTS_NUM];
};

class CLCoreDigitalSource_Cygwin : public NSCORE_URUS::CLCoreUrusDigitalSource {
public:
    CLCoreDigitalSource_Cygwin(uint8_t v);
    void    mode(uint8_t output);
    uint8_t read();
    void    write(uint8_t value);
    void    toggle();
private:
    uint8_t _v;
};

#endif // __CYGWIN__
