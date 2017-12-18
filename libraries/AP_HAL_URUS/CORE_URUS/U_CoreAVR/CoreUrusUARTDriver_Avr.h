#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include <stdint.h>
#include <stdarg.h>

#include <avr/interrupt.h>

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusUARTDriver.h"

class CLCoreUrusUARTDriver_Avr : public NSCORE_URUS::CLCoreUrusUARTDriver {
public:

    CLCoreUrusUARTDriver_Avr(
        const uint8_t portNumber, volatile uint8_t *ubrrh,
        volatile uint8_t *ubrrl, volatile uint8_t *ucsra,
        volatile uint8_t *ucsrb, const uint8_t u2x,
		const uint8_t portEnableBits, const uint8_t portTxBits);

    static CLCoreUrusUARTDriver *from(AP_HAL::UARTDriver *uart) {
        return static_cast<CLCoreUrusUARTDriver_Avr*>(uart);
    }

    /* Implementations of UARTDriver virtual methods */
    void begin(uint32_t b) { begin(b, 0, 0); }
    void begin(uint32_t b, uint16_t rxS, uint16_t txS);
    void end();
    void flush();
    bool is_initialized() {
        return _initialized;
    }

    void set_blocking_writes(bool blocking)
    {
        _nonblocking_writes = !blocking;
    }

    bool tx_pending() {
        return (_txBuffer->head != _txBuffer->tail);
    }

    /* Implementations of Stream virtual methods */
    uint32_t available() override;
    uint32_t txspace() override;
    int16_t read() override;

    /* Implementations of Print virtual methods */
    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);

	/// Transmit/receive buffer descriptor.
	///
	/// Public so the interrupt handlers can see it
	struct Buffer {
		volatile uint8_t head, tail;	///< head and tail pointers
		uint8_t mask;					///< buffer size mask for pointer wrap
		uint8_t *bytes;					///< pointer to allocated buffer
	};

    void _timer_tick(void) override;

private:
    /* Instance Variables */
    bool _initialized;

	// register accessors
	volatile uint8_t * const _ubrrh;
	volatile uint8_t * const _ubrrl;
	volatile uint8_t * const _ucsra;
	volatile uint8_t * const _ucsrb;

	// register magic numbers
	const uint8_t	_u2x;
	const uint8_t	_portEnableBits;		///< rx, tx and rx interrupt enables
	const uint8_t	_portTxBits;			///< tx data and completion interrupt enables

	// ring buffers
	Buffer			* const _rxBuffer;
	Buffer			* const _txBuffer;
	bool 			_open;

	// whether writes to the port should block waiting
	// for enough space to appear
	bool			_nonblocking_writes;

    /* Class Variables */

	/// Allocates a buffer of the given size
	///
	/// @param	buffer		The buffer descriptor for which the buffer will
	///						will be allocated.
	/// @param	size		The desired buffer size.
	/// @returns			True if the buffer was allocated successfully.
	///
	static bool _allocBuffer(Buffer *buffer, uint16_t size);

	/// Frees the allocated buffer in a descriptor
	///
	/// @param	buffer		The descriptor whose buffer should be freed.
	///
	static void _freeBuffer(Buffer *buffer);

	/// default receive buffer size
	static const uint16_t _default_rx_buffer_size = 4;

	/// default transmit buffer size
	static const uint16_t _default_tx_buffer_size = 16;

	/// maxium tx/rx buffer size
	static const uint16_t _max_buffer_size;
};

extern CLCoreUrusUARTDriver_Avr::Buffer __CLCoreUrusUARTDriver_Avr__rxBuffer[];
extern CLCoreUrusUARTDriver_Avr::Buffer __CLCoreUrusUARTDriver_Avr__txBuffer[];

/// Generic Rx/Tx vectors for a serial port - needs to know magic numbers
///
#define CLCoreUrusUARTDriver_AvrHandler(_PORT, _RXVECTOR, _TXVECTOR, _UDR, _UCSRB, _TXBITS) \
ISR(_RXVECTOR, ISR_BLOCK)                                               \
{                                                                       \
        uint8_t c;                                                      \
        uint8_t i;                                                     \
                                                                        \
        /* read the byte as quickly as possible */                      \
        c = _UDR;                                                       \
        /* work out where the head will go next */                      \
        i = (__CLCoreUrusUARTDriver_Avr__rxBuffer[_PORT].head + 1) & __CLCoreUrusUARTDriver_Avr__rxBuffer[_PORT].mask; \
        /* decide whether we have space for another byte */             \
        if (i != __CLCoreUrusUARTDriver_Avr__rxBuffer[_PORT].tail) {                  \
                /* we do, move the head */                              \
                __CLCoreUrusUARTDriver_Avr__rxBuffer[_PORT].bytes[__CLCoreUrusUARTDriver_Avr__rxBuffer[_PORT].head] = c; \
                __CLCoreUrusUARTDriver_Avr__rxBuffer[_PORT].head = i;                 \
        }                                                               \
}                                                                       \
ISR(_TXVECTOR, ISR_BLOCK)                                               \
{                                                                       \
        /* if there is another character to send */                     \
        if (__CLCoreUrusUARTDriver_Avr__txBuffer[_PORT].tail != __CLCoreUrusUARTDriver_Avr__txBuffer[_PORT].head) { \
                _UDR = __CLCoreUrusUARTDriver_Avr__txBuffer[_PORT].bytes[__CLCoreUrusUARTDriver_Avr__txBuffer[_PORT].tail]; \
                /* increment the tail */                                \
                __CLCoreUrusUARTDriver_Avr__txBuffer[_PORT].tail =                    \
                        (__CLCoreUrusUARTDriver_Avr__txBuffer[_PORT].tail + 1) & __CLCoreUrusUARTDriver_Avr__txBuffer[_PORT].mask; \
        } else {                                                        \
                /* there are no more bytes to send, disable the interrupt */ \
                if (__CLCoreUrusUARTDriver_Avr__txBuffer[_PORT].head == __CLCoreUrusUARTDriver_Avr__txBuffer[_PORT].tail) \
                        _UCSRB &= ~_TXBITS;                             \
        }                                                               \
}                                                                       \
struct hack

//
// Portability; convert various older sets of defines for U(S)ART0 up
// to match the definitions for the 1280 and later devices.
//
#if !defined(USART0_RX_vect)
# if defined(USART_RX_vect)
#  define USART0_RX_vect        USART_RX_vect
#  define USART0_UDRE_vect      USART_UDRE_vect
# elif defined(UART0_RX_vect)
#  define USART0_RX_vect        UART0_RX_vect
#  define USART0_UDRE_vect      UART0_UDRE_vect
# endif
#endif

#if !defined(USART1_RX_vect)
# if defined(UART1_RX_vect)
#  define USART1_RX_vect        UART1_RX_vect
#  define USART1_UDRE_vect      UART1_UDRE_vect
# endif
#endif

#if !defined(UDR0)
# if defined(UDR)
#  define UDR0                  UDR
#  define UBRR0H                UBRRH
#  define UBRR0L                UBRRL
#  define UCSR0A                UCSRA
#  define UCSR0B                UCSRB
#  define U2X0                  U2X
#  define RXEN0                 RXEN
#  define TXEN0                 TXEN
#  define RXCIE0                RXCIE
#  define UDRIE0                UDRIE
# endif
#endif

///
/// Macro defining a CLCoreUrusUARTDriver_Avr port instance.
///
#define CLCoreUrusUARTDriver_AvrInstance(_name, _num)                              \
	CLCoreUrusUARTDriver_Avr _name(_num,                                           \
                         &UBRR##_num##H,                                \
                         &UBRR##_num##L,                                \
                         &UCSR##_num##A,                                \
                         &UCSR##_num##B,                                \
                         U2X##_num,                                     \
                         (_BV(RXEN##_num) |  _BV(TXEN##_num) | _BV(RXCIE##_num)), \
                         (_BV(UDRIE##_num)));

#define CLCoreUrusUARTDriver_AvrISRs(_num)                                         \
	CLCoreUrusUARTDriver_AvrHandler(_num,                                          \
                          USART##_num##_RX_vect,                        \
                          USART##_num##_UDRE_vect,                      \
                          UDR##_num,                                    \
                          UCSR##_num##B,                                \
                          _BV(UDRIE##_num))

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
