/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//
//      Copyright (c) 2010 Michael Smith. All rights reserved.
//
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>

#include <avr/pgmspace.h>

#include <AP_Math/AP_Math.h>

#include "../CORE_URUS_NAMESPACE.h"

#include "../CoreUrusUARTDriver.h"
#include "CoreUrusUARTDriver_Avr.h"

#define FS_MAX_PORTS 4
CLCoreUrusUARTDriver_Avr::Buffer __CLCoreUrusUARTDriver_Avr__rxBuffer[FS_MAX_PORTS];
CLCoreUrusUARTDriver_Avr::Buffer __CLCoreUrusUARTDriver_Avr__txBuffer[FS_MAX_PORTS];

const uint16_t CLCoreUrusUARTDriver_Avr::_max_buffer_size = 256;

/* CLCoreUrusUARTDriver_Avr method implementations */

CLCoreUrusUARTDriver_Avr::CLCoreUrusUARTDriver_Avr(
        const uint8_t portNumber, volatile uint8_t *ubrrh,
        volatile uint8_t *ubrrl, volatile uint8_t *ucsra,
        volatile uint8_t *ucsrb, const uint8_t u2x,
		const uint8_t portEnableBits, const uint8_t portTxBits) :
			NSCORE_URUS::CLCoreUrusUARTDriver(),
			_ubrrh(ubrrh),
			_ubrrl(ubrrl),
			_ucsra(ucsra),
			_ucsrb(ucsrb),
			_u2x(u2x),
			_portEnableBits(portEnableBits),
			_portTxBits(portTxBits),
			_rxBuffer(&__CLCoreUrusUARTDriver_Avr__rxBuffer[portNumber]),
			_txBuffer(&__CLCoreUrusUARTDriver_Avr__txBuffer[portNumber])
{
	_initialized = true;
	//begin(57600);
}

void CLCoreUrusUARTDriver_Avr::begin(uint32_t baud, uint16_t rxSpace, uint16_t txSpace)
{
	uint16_t ubrr;
	bool use_u2x = true;
	bool need_allocate = true;

	// if we are currently open...
	if (_open) {
		// If the caller wants to preserve the buffer sizing, work out what
		// it currently is...
		if (0 == rxSpace)
			rxSpace = _rxBuffer->mask + 1;
		if (0 == txSpace)
			txSpace = _txBuffer->mask + 1;

		if (rxSpace == (_rxBuffer->mask + 1U) &&
			txSpace == (_txBuffer->mask + 1U)) {
			// avoid re-allocating the buffers if possible
			need_allocate = false;
			*_ucsrb &= ~(_portEnableBits | _portTxBits);
		} else {
			// close the port in its current configuration, clears _open
			end();
		}
	}

	if (need_allocate) {
		// allocate buffers
		if (!_allocBuffer(_rxBuffer, rxSpace ? : _default_rx_buffer_size)
        || !_allocBuffer(_txBuffer, txSpace ? : _default_tx_buffer_size)) {
			end();
			return; // couldn't allocate buffers - fatal
		}
	}

	// reset buffer pointers
	_txBuffer->head = _txBuffer->tail = 0;
	_rxBuffer->head = _rxBuffer->tail = 0;

	// mark the port as open
	_open = true;

	// If the user has supplied a new baud rate, compute the new UBRR value.
	if (baud > 0) {
#if F_CPU == 16000000UL
		// hardcoded exception for compatibility with the bootloader shipped
		// with the Duemilanove and previous boards and the firmware on the 8U2
		// on the Uno and Mega 2560.
		if (baud == 57600)
			use_u2x = false;
#endif

		if (use_u2x) {
			*_ucsra = 1 << _u2x;
			ubrr = (F_CPU / 4 / baud - 1) / 2;
		} else {
			*_ucsra = 0;
			ubrr = (F_CPU / 8 / baud - 1) / 2;
		}

		*_ubrrh = ubrr >> 8;
		*_ubrrl = ubrr;
	}

	*_ucsrb |= _portEnableBits;
}

void CLCoreUrusUARTDriver_Avr::end()
{
	*_ucsrb &= ~(_portEnableBits | _portTxBits);

	_freeBuffer(_rxBuffer);
	_freeBuffer(_txBuffer);
	_open = false;
}

uint32_t CLCoreUrusUARTDriver_Avr::available(void)
{
	if (!_open)
		return (-1);
	return ((_rxBuffer->head - _rxBuffer->tail) & _rxBuffer->mask);
}

uint32_t CLCoreUrusUARTDriver_Avr::txspace(void)
{
	if (!_open)
		return (-1);
	return ((_txBuffer->mask+1) - ((_txBuffer->head - _txBuffer->tail) & _txBuffer->mask));
}

int16_t CLCoreUrusUARTDriver_Avr::read(void)
{
	uint8_t c;

	// if the head and tail are equal, the buffer is empty
	if (!_open || (_rxBuffer->head == _rxBuffer->tail))
		return (-1);

	// pull character from tail
	c = _rxBuffer->bytes[_rxBuffer->tail];
	_rxBuffer->tail = (_rxBuffer->tail + 1) & _rxBuffer->mask;

	return (c);
}

void CLCoreUrusUARTDriver_Avr::flush(void)
{
	// don't reverse this or there may be problems if the RX interrupt
	// occurs after reading the value of _rxBuffer->head but before writing
	// the value to _rxBuffer->tail; the previous value of head
	// may be written to tail, making it appear as if the buffer
	// don't reverse this or there may be problems if the RX interrupt
	// occurs after reading the value of head but before writing
	// the value to tail; the previous value of rx_buffer_head
	// may be written to tail, making it appear as if the buffer
	// were full, not empty.
	_rxBuffer->head = _rxBuffer->tail;

	// don't reverse this or there may be problems if the TX interrupt
	// occurs after reading the value of _txBuffer->tail but before writing
	// the value to _txBuffer->head.
	_txBuffer->tail = _txBuffer->head;
}

size_t CLCoreUrusUARTDriver_Avr::write(uint8_t c)
{
	uint8_t i;

	if (!_open) // drop bytes if not open
		return 0;

	// wait for room in the tx buffer
	i = (_txBuffer->head + 1) & _txBuffer->mask;

	// if the port is set into non-blocking mode, then drop the byte
	// if there isn't enough room for it in the transmit buffer
	if (_nonblocking_writes && i == _txBuffer->tail) {
		return 0;
	}

	while (i == _txBuffer->tail)
		;

	// add byte to the buffer
	_txBuffer->bytes[_txBuffer->head] = c;
	_txBuffer->head = i;

	// enable the data-ready interrupt, as it may be off if the buffer is empty
	*_ucsrb |= _portTxBits;

	// return number of bytes written (always 1)
	return 1;
}

size_t CLCoreUrusUARTDriver_Avr::write(const uint8_t *buffer, size_t size)
{
    if (!_open) {
        return 0;
    }

    if (!_nonblocking_writes) {
        /*
          use the per-byte delay loop in write() above for blocking writes
         */
        size_t ret = 0;
        while (size--) {
            if (write(*buffer++) != 1) break;
            ret++;
        }
        return ret;
    }

    int16_t space = txspace();
    if (space <= 0) {
        return 0;
    }
    if (size > (size_t)space) {
        // throw away remainder if too much data
        size = space;
    }
    if (_txBuffer->tail > _txBuffer->head) {
        // perform as single memcpy
        memcpy(&_txBuffer->bytes[_txBuffer->head], buffer, size);
        _txBuffer->head = (_txBuffer->head + size) & _txBuffer->mask;
        // enable the data-ready interrupt, as it may be off if the buffer is empty
        *_ucsrb |= _portTxBits;
        return size;
    }

    // perform as two memcpy calls
    uint16_t n = (_txBuffer->mask+1) - _txBuffer->head;
    if (n > size) n = size;
    memcpy(&_txBuffer->bytes[_txBuffer->head], buffer, n);
    _txBuffer->head = (_txBuffer->head + n) & _txBuffer->mask;
    buffer += n;
    n = size - n;
    if (n > 0) {
        memcpy(&_txBuffer->bytes[0], buffer, n);
        _txBuffer->head = (_txBuffer->head + n) & _txBuffer->mask;
    }

    // enable the data-ready interrupt, as it may be off if the buffer is empty
    *_ucsrb |= _portTxBits;
    return size;
}

// Buffer management ///////////////////////////////////////////////////////////


bool CLCoreUrusUARTDriver_Avr::_allocBuffer(Buffer *buffer, uint16_t size)
{
	uint8_t mask;
	uint8_t	 shift;

	// init buffer state
	buffer->head = buffer->tail = 0;

	// Compute the power of 2 greater or equal to the requested buffer size
	// and then a mask to simplify wrapping operations.  Using __builtin_clz
	// would seem to make sense, but it uses a 256(!) byte table.
	// Note that we ignore requests for more than BUFFER_MAX space.
	for (shift = 1; (1U << shift) < MIN(_max_buffer_size, size); shift++)
		;
	mask = (1U << shift) - 1;

	// If the descriptor already has a buffer allocated we need to take
	// care of it.
	if (buffer->bytes) {

		// If the allocated buffer is already the correct size then
		// we have nothing to do
		if (buffer->mask == mask)
			return true;

		// Dispose of the old buffer.
		free(buffer->bytes);
	}
	buffer->mask = mask;

	// allocate memory for the buffer - if this fails, we fail.
	buffer->bytes = (uint8_t *) malloc(buffer->mask + (size_t)1);

	return (buffer->bytes != NULL);
}

void CLCoreUrusUARTDriver_Avr::_freeBuffer(Buffer *buffer)
{
	buffer->head = buffer->tail = 0;
	buffer->mask = 0;
	if (NULL != buffer->bytes) {
		free(buffer->bytes);
		buffer->bytes = NULL;
	}
}

void CLCoreUrusUARTDriver_Avr::_timer_tick(void)
{}

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM

