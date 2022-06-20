#pragma once

#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include <stdint.h>
#include <stdarg.h>

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusUARTDriver.h"

#include <AP_HAL/utility/Socket.h>
#include <AP_HAL/utility/RingBuffer.h>

class CLCoreUrusUARTDriver_Cygwin : public NSCORE_URUS::CLCoreUrusUARTDriver {
public:

    CLCoreUrusUARTDriver_Cygwin(const uint8_t portNumber, const bool console) :
        NSCORE_URUS::CLCoreUrusUARTDriver(),
        _fd(-1),
        _portNumber(portNumber),
        _listen_fd(-1),
        _console(console)
    {
        _initialized = true;
    }

    static CLCoreUrusUARTDriver *from(AP_HAL::UARTDriver *uart) {
        return static_cast<CLCoreUrusUARTDriver_Cygwin*>(uart);
    }

    /* Implementations of UARTDriver virtual methods */
    void begin(uint32_t b) override {
        begin(b, 0, 0);
    }
    void begin(uint32_t b, uint16_t rxS, uint16_t txS) override;
    void end() override;
    void flush() override;
    bool is_initialized() override {
        return _initialized;
    }

    void set_blocking_writes(bool blocking) override
    {
        _nonblocking_writes = !blocking;
    }

    bool tx_pending() override {
        return (_writebuffer.space() != 0);
    }

    /* Implementations of Stream virtual methods */
    uint32_t available() override;
    uint32_t txspace() override;
    int16_t read() override;

    /* Implementations of Print virtual methods */
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;

    // file descriptor, exposed so SITL_State::loop_hook() can use it
    int _fd;

    enum flow_control get_flow_control(void) override { return FLOW_CONTROL_ENABLE; }

    void _timer_tick(void) override;

    /* Set scheduling status. */
    void set_scheduling_status(bool status) {
        _status_scheduling = status;
    }

private:
    uint8_t _portNumber;
    bool _connected = false; // true if a client has connected
    bool _use_send_recv = false;
    int _listen_fd;  // socket we are listening on
    int _serial_port;
    bool _console;
    bool _nonblocking_writes;
    ByteBuffer _readbuffer{16384};
    ByteBuffer _writebuffer{16384};
    bool _initialized = false;

    const char *_uart_path;
    uint32_t _uart_baudrate;

    // IPv4 address of target for uartC
    const char *_tcp_client_addr;

    void _tcp_start_connection(uint16_t port, bool wait_for_connection);
    void _uart_start_connection(void);
    void _check_reconnect();
    void _tcp_start_client(const char *address, uint16_t port);
    void _check_connection(void);
    bool _select_check(int );
    void _set_nonblocking(int , bool is_console);
    bool _use_rtscts;

    /* default configuration for uart driver */
    const char* path[5] = {
        "tcp:0:nowait",
        "uart:/dev/ttyUSB0:115200",
        "uart:/dev/ttyACM0:115200",
        "tcp:0:nowait",
        "tcp:0:nowait",
    };

    uint16_t _base_port = 5760;

    /*  Is scheduling using and active?
        It's activated if we are using the AP_Scheduler.
    */
    bool _status_scheduling = false;
};

#endif // __CYGWIN__
