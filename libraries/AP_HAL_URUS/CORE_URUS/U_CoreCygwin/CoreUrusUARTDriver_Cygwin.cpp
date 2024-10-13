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
//      Copyright (c) 2024 Hiroshi Takey F. All rights reserved.
//          - MingW support UART
//
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <AP_Math/AP_Math.h>

#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <termios.h>

#if defined(SHAL_CORE_MINGW)
#include <winsock2.h>
#include <windows.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <wininet.h>
//#include "netsocket_win.h"
//#include "dirent_win.h"
#define MSG_DONTWAIT 0
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/select.h>
#endif

#include "../CORE_URUS_NAMESPACE.h"
#include "../CORE_URUS.h"

#include "../CoreUrusUARTDriver.h"
#include "../CoreUrusScheduler.h"

#include "CoreUrusUARTDriver_Cygwin.h"
#include "CoreUrusScheduler_Cygwin.h"

#define UART_TYPE_UNIX_ACM    0
#define UART_TYPE_UNIX_USB    1 << 6
#define UART_TYPE_WIN_COM     2 << 6
#define UART_TYPE_MASK        0xC0

extern const NSCORE_URUS::CLCORE_URUS& _urus_core;

const char* CLCoreUrusUARTDriver_Cygwin::_uartnum[] = {
    "UART_NUM_0",
    "UART_NUM_1",
    "UART_NUM_2",
    "UART_NUM_3",
    "UART_NUM_4",
    "UART_NUM_5",
    "UART_NUM_6",
    "UART_NUM_7",
    "UART_NUM_8",
    "UART_NUM_9",
};

/*
AP_Int8 CLCoreUrusUARTDriver_Cygwin::uartnum[10];

#define class_core CLCoreUrusUARTDriver_Cygwin
#define URGSCALAR(v, idx, name, def) { v[idx].vtype, name, class_core::k_param_ ## v ## idx, &v[idx], {def_value : def} }

const AP_Param::Info CLCoreUrusUARTDriver_Cygwin::var_info[] = {
    URGSCALAR(uartnum, 0, "UART_NUM_0",  0),
    URGSCALAR(uartnum, 1, "UART_NUM_1",  1),
    URGSCALAR(uartnum, 2, "UART_NUM_2",  2),
    URGSCALAR(uartnum, 3, "UART_NUM_3",  3),
    URGSCALAR(uartnum, 4, "UART_NUM_4",  4),
    URGSCALAR(uartnum, 5, "UART_NUM_5",  5),
    URGSCALAR(uartnum, 6, "UART_NUM_6",  6),
    URGSCALAR(uartnum, 7, "UART_NUM_7",  7),
    URGSCALAR(uartnum, 8, "UART_NUM_8",  8),
    URGSCALAR(uartnum, 9, "UART_NUM_9",  9),

    AP_VAREND
};
*/
/* CLCoreUrusUARTDriver_Cygwin method implementations */

void CLCoreUrusUARTDriver_Cygwin::begin(uint32_t baud, uint16_t rxSpace, uint16_t txSpace)
{
    /* parse type:args:flags string for path.
       For example:
         tcp:5760:wait    // tcp listen on port 5760
         tcp:0:wait       // tcp listen on use base_port + 0
         tcpclient:192.168.2.15:5762
         uart:/dev/ttyUSB0:57600
     */
#if defined(SHAL_CORE_MINGW)
    char errMsg[256];
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0) {
        sprintf(errMsg, "DataLink: Failed to initialise Windows Sockets: %d\n", err);
        fprintf(stdout, "ERROR: %s\n", errMsg);
    }
#endif
/*
    uartnum[_portNumber].load();
    uint8_t uartnum1 = uartnum[_portNumber];
    fprintf(stdout, "UART PORT:%d\n", uartnum1);
*/
    //uartnum[_portNumber] = 9;
    //uartnum[_portNumber].save();

/*
    if (AP_Param::set_and_save_by_name("UART_NUM_1", 12)) {
        fprintf(stdout, "ok UART_NUM_1\n");
    } else {
        fprintf(stdout, "can't set UART_NUM_1\n");
    }
*/
    _status_scheduling = !_urus_core.scheduler->get_timer_event_eval();
    char *saveptr = nullptr;
    char *s = strdup(path[_portNumber]);
    char *devtype = strtok_r(s, ":", &saveptr);
    char *args1 = strtok_r(nullptr, ":", &saveptr);
    char *args2 = strtok_r(nullptr, ":", &saveptr);
    if (strcmp(devtype, "tcp") == 0) {
        uint16_t port = atoi(args1);
        bool wait = (args2 && strcmp(args2, "wait") == 0);
        _tcp_start_connection(port, wait);
    } else if (strcmp(devtype, "tcpclient") == 0) {
        if (args2 == nullptr) {
            AP_HAL::panic("Invalid tcp client path: %s\n", path[_portNumber]);
        }
        uint16_t port = atoi(args2);
        _tcp_start_client(args1, port);
    } else if (strcmp(devtype, "uart") == 0) {
        _uartn = (AP_Int8*)AP_Param::find_object(_uartnum[_portNumber]);
        if (_uartn) {
            _uartn->load();
            uint8_t uartnum1 = _uartn->get();
            uint8_t uart_type = uartnum1 & UART_TYPE_MASK;
            uint8_t devn = uartnum1 & ~UART_TYPE_MASK;
            switch (uart_type) {
            case UART_TYPE_UNIX_ACM:
                sprintf(_uartdynpath, "/dev/ttyACM%d", devn);
                //fprintf(stdout, "Type uart: %s\n", _uartdynpath);
                break;
            case UART_TYPE_UNIX_USB:
                sprintf(_uartdynpath, "/dev/ttyUSB%d", devn);
                //fprintf(stdout, "Type uart: %s\n", _uartdynpath);
                break;
            case UART_TYPE_WIN_COM:
                sprintf(_uartdynpath, "COM%d", devn);
                //fprintf(stdout, "Type uart: %s\n", _uartdynpath);
                break;
            default:
                sprintf(_uartdynpath, "UNKNOWN");
                //fprintf(stdout, "unknown uart type\n");
            }

            fprintf(stdout, "[%s:%d] on DEVPATH:\"%s\"\n", _uartnum[_portNumber], devn, _uartdynpath);
        }

        if (_uartn) {
            _uart_path = strdup(_uartdynpath);
        } else {
            _uart_path = strdup(args1);
        }

        uint32_t baudrate = args2? atoi(args2) : baud;
        ::printf("uart%c connection %s:%u\n", (char)(0x41 + _portNumber), _uart_path, baudrate);

        _uart_baudrate = baudrate;

        if (rxSpace != 0) {
            _readbuffer.set_size(rxSpace);
        } else {
            _readbuffer.set_size(512);
        }

        if (txSpace != 0) {
            _writebuffer.set_size(txSpace);
        } else {
            _writebuffer.set_size(512);
        }

        _use_rtscts = false;

        _uart_start_connection();
    } else {
        AP_HAL::panic("Invalid device path: %s\n", path[_portNumber]);
    }
    free(s);

}

void CLCoreUrusUARTDriver_Cygwin::end()
{
}

uint32_t CLCoreUrusUARTDriver_Cygwin::available(void)
{
    _check_connection();

    if (!_connected) {
        _check_reconnect();
        return 0;
    }

    return _readbuffer.available();
}

uint32_t CLCoreUrusUARTDriver_Cygwin::txspace(void)
{
    _check_connection();

    if (!_connected) {
        _check_reconnect();
        return 0;
    }
    return _writebuffer.space();
}

int16_t CLCoreUrusUARTDriver_Cygwin::read(void)
{
    if (available() <= 0) {
        return -1;
    }

    uint8_t c;
    _readbuffer.read(&c, 1);
    return c;
}

void CLCoreUrusUARTDriver_Cygwin::flush(void)
{
    if (_fd != -1) {
        tcflush(_fd, TCIOFLUSH);
    }

	_readbuffer.clear();
	_writebuffer.clear();
}

size_t CLCoreUrusUARTDriver_Cygwin::write(uint8_t c)
{
    if (txspace() <= 0) {
        //_timer_tick();
        return 0;
    }

    _writebuffer.write(&c, 1);

    /*  If we aren't using task manager (AP_Scheduler), then we
        process data byte each.
    */
    if (!_status_scheduling) {
        //_timer_tick();
    }

    return 1;
}

size_t CLCoreUrusUARTDriver_Cygwin::write(const uint8_t *buffer, size_t size)
{
    if (txspace() <= (ssize_t)size) {
        size = txspace();
    }
    if (size <= 0) {
        return 0;
    }
    _writebuffer.write(buffer, size);
    return size;
}


/*
  start a TCP connection for the serial port. If wait_for_connection
  is true then block until a client connects
 */
void CLCoreUrusUARTDriver_Cygwin::_tcp_start_connection(uint16_t port, bool wait_for_connection)
{
    int one=1;
    struct sockaddr_in sockaddr;
    int ret;

    if (_connected) {
        return;
    }

    _use_send_recv = true;

    if (_console) {
        // hack for console access
        _connected = true;
        _use_send_recv = false;
        _listen_fd = -1;
        _fd = STDOUT_FILENO;
        _set_nonblocking(_fd, _console);
        return;
    }

    if (_fd != -1) {
        close(_fd);
    }

    if (_listen_fd == -1) {
        memset(&sockaddr,0,sizeof(sockaddr));

#ifdef HAVE_SOCK_SIN_LEN
        sockaddr.sin_len = sizeof(sockaddr);
#endif
        if (port > 1000) {
            sockaddr.sin_port = htons(port);
        } else {
            sockaddr.sin_port = htons(_base_port + port + _portNumber);
        }
        sockaddr.sin_family = AF_INET;

        _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (_listen_fd == -1) {
            fprintf(stderr, "socket failed - %s\n", strerror(errno));
            exit(1);
        }
#if !defined(SHAL_CORE_MINGW)
        fcntl(_listen_fd, F_SETFD, FD_CLOEXEC);
#endif

        /* we want to be able to re-use ports quickly */
#if !defined(SHAL_CORE_MINGW)
        //setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR &one, sizeof(one));
        setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));
#else
        setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));
#endif

        fprintf(stderr, "bind port %u for %u\n",
                (unsigned)ntohs(sockaddr.sin_port),
                (unsigned)_portNumber);

        ret = bind(_listen_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
        if (ret == -1) {
            fprintf(stderr, "bind failed on port %u - %s\n",
                    (unsigned)ntohs(sockaddr.sin_port),
                    strerror(errno));
            exit(1);
        }

        ret = listen(_listen_fd, 5);
        if (ret == -1) {
            fprintf(stderr, "listen failed - %s\n", strerror(errno));
            exit(1);
        }

        fprintf(stderr, "Serial port %u on TCP port %u\n", _portNumber,
                _base_port + _portNumber);
        fflush(stdout);
    }

    if (wait_for_connection) {
        fprintf(stdout, "Waiting for connection ....\n");
        fflush(stdout);
        _fd = accept(_listen_fd, nullptr, nullptr);
#if !defined(SHAL_CORE_MINGW)
        fcntl(_fd, F_SETFD, FD_CLOEXEC);
#endif

        if (_fd == -1) {
            fprintf(stderr, "accept() error - %s\n", strerror(errno));
            exit(1);
        }
#if !defined(SHAL_CORE_MINGW)
        setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
#else
        setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));
        setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&one, sizeof(one));
#endif
        _connected = true;
    }
}


/*
  start a TCP client connection for the serial port.
 */
void CLCoreUrusUARTDriver_Cygwin::_tcp_start_client(const char *address, uint16_t port)
{
    int one=1;
    struct sockaddr_in sockaddr;
    int ret;

    if (_connected) {
        return;
    }

    _use_send_recv = true;

    if (_fd != -1) {
        close(_fd);
    }

    memset(&sockaddr,0,sizeof(sockaddr));

#ifdef HAVE_SOCK_SIN_LEN
    sockaddr.sin_len = sizeof(sockaddr);
#endif
    sockaddr.sin_port = htons(port);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(address);

    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1) {
        fprintf(stderr, "socket failed - %s\n", strerror(errno));
        exit(1);
    }

    /* we want to be able to re-use ports quickly */
#if !defined(SHAL_CORE_MINGW)
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#else
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));
#endif

    ret = connect(_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
    if (ret == -1) {
        fprintf(stderr, "connect failed on port %u - %s\n",
                (unsigned)ntohs(sockaddr.sin_port),
                strerror(errno));
        exit(1);
    }
#if !defined(SHAL_CORE_MINGW)
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
#else
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));
    setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&one, sizeof(one));
#endif
    _connected = true;
}


/*
  start a UART connection for the serial port
 */
void CLCoreUrusUARTDriver_Cygwin::_uart_start_connection(void)
{
    struct termios t {};
    if (!_connected) {
#if !defined(SHAL_CORE_MINGW)
        _fd = ::open(_uart_path, O_RDWR | O_CLOEXEC);
#else
        _fd = ::open_serial(_uart_path, O_RDWR | O_CLOEXEC);
#endif
        if (_fd == -1) {
            if ((_urus_core.timers->get_core_millis32() - _timeout_unable_port) > 5000) {
                _timeout_unable_port = _urus_core.timers->get_core_millis32();
                ::printf("\nUnable to open uart%c on %s !!!\n", (char)(0x41 + _portNumber), _uart_path);
                fflush(stdout);
            }
            return;
        }
        ::printf("Opened uart%c on %s\n", (char)(0x41 + _portNumber), _uart_path);
        fflush(stdout);
        _urus_core.scheduler->delay(3000);
    }

    if (_fd == -1) {
        ::printf("\nUnable to open uart%c on %s !!!\n\n", (char)(0x41 + _portNumber), _uart_path);
    }

    // set non-blocking
#if !defined(SHAL_CORE_MINGW)
    int flags = fcntl(_fd, F_GETFL, 0);
    flags = flags | O_NONBLOCK;
    fcntl(_fd, F_SETFL, flags);
#endif
    // disable LF -> CR/LF
    tcgetattr(_fd, &t);

#if !defined(SHAL_CORE_MINGW)
    t.c_iflag &= ~(BRKINT | ICRNL | IMAXBEL | IXON | IXOFF);
    t.c_oflag &= ~(OPOST | ONLCR);
    t.c_lflag &= ~(ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE);
#else
    t.c_iflag &= ~(BRKINT | ICRNL | IXON | IXOFF);
    t.c_oflag &= ~(OPOST);
    t.c_lflag &= ~(ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK);
#endif
    t.c_cc[VMIN] = 0;

    //t.c_cflag &= ~CRTSCTS;

    if (_use_rtscts) {
        t.c_cflag |= CRTSCTS;
    }

    tcflush(_fd, TCIOFLUSH);   // clear the input and output buffers

    tcsetattr(_fd, TCSANOW, &t);

    // set baudrate
    tcgetattr(_fd, &t);
    cfsetspeed(&t, _uart_baudrate);
    cfmakeraw(&t);
    tcsetattr(_fd, TCSANOW, &t);

    _connected = true;
    _use_send_recv = false;
}

/*
  see if a new connection is coming in
 */
void CLCoreUrusUARTDriver_Cygwin::_check_connection(void)
{
    /*  If we aren't using task manager (AP_Scheduler), then we
        process data byte each.
    */
    if (!_status_scheduling) {
        //_timer_tick();
    }

    if (_connected) {
        // we only want 1 connection at a time
        return;
    }
    if (_select_check(_listen_fd)) {
        _fd = accept(_listen_fd, NULL, NULL);
#if !defined(SHAL_CORE_MINGW)
        fcntl(_fd, F_SETFD, FD_CLOEXEC);
#endif
        if (_fd != -1) {
            int one = 1;
            _connected = true;
#if !defined(SHAL_CORE_MINGW)
            setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
            setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#else
            setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&one, sizeof(one));
            setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));
#endif
            fprintf(stdout, "New connection on uart%c serial port %u\n", (char)(0x41 + _portNumber), _portNumber);
        }
    }
}

/*
  use select() to see if something is pending
 */
bool CLCoreUrusUARTDriver_Cygwin::_select_check(int fd)
{
    if (fd == -1) {
        return false;
    }
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    // zero time means immediate return from select()
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if (select(fd+1, &fds, nullptr, nullptr, &tv) == 1) {
        return true;
    }
    return false;
/*
#if !defined(SHAL_CORE_MINGW)
    if (fd == -1) {
        return false;
    }
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    // zero time means immediate return from select()
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if (select(fd+1, &fds, nullptr, nullptr, &tv) == 1) {
        return true;
    }
    return false;
#else

    if (fd == -1) {
        return false;
    }

    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    //tv.tv_sec = 1 / 1000;
    //tv.tv_usec = (1 % 1000) * 1000UL;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if (select(FD_SETSIZE, &fds, NULL, NULL, &tv) != 1) {
        return false;
    }

    return true;
#endif
*/
}

void CLCoreUrusUARTDriver_Cygwin::_set_nonblocking(int fd, bool is_console)
{
    unsigned rd_flags;
    unsigned wr_flags;
    int _wr_fd = fd;
    int _rd_fd = STDIN_FILENO;
#if !defined(SHAL_CORE_MINGW)
    rd_flags  = fcntl(_rd_fd, F_GETFL, 0);
    wr_flags  = fcntl(_wr_fd, F_GETFL, 0);

    rd_flags = rd_flags | O_NONBLOCK;
    wr_flags = wr_flags | O_NONBLOCK;

    if (fcntl(_rd_fd, F_SETFL, rd_flags) < 0) {
        ::printf("Failed to set STDIN Console nonblocking %s\n", strerror(errno));
    }
#endif
    if (is_console) {
        struct termios custom;
        tcgetattr(_wr_fd, &custom);
        tcgetattr(_wr_fd, &_termiostmp);
        custom.c_lflag &= ~(ICANON|ECHO);
        tcsetattr(_wr_fd,TCSANOW,&custom);
#if !defined(SHAL_CORE_MINGW)
        if (fcntl(_wr_fd, F_SETFL, wr_flags) < 0) {
            ::printf("Failed to set STDOUT Console nonblocking %s\n",strerror(errno));
        }
#endif
    }
}

void CLCoreUrusUARTDriver_Cygwin::_check_reconnect(void)
{
    if (!_uart_path) {
        return;
    }
    _uart_start_connection();
}

void CLCoreUrusUARTDriver_Cygwin::_timer_tick(void)
{
    if (!_connected) {
        //_check_reconnect();
        return;
    }
    uint32_t navail = 0;
    ssize_t nwritten = 0;

    const uint8_t *readptr = _writebuffer.readptr(navail);

    if (readptr && navail > 0) {
        if (!_use_send_recv) {
            int fd = _console?0:_fd;
#if !defined(SHAL_CORE_MINGW)
            nwritten = ::write(fd, readptr, navail);
#else
            nwritten = ::write_serial(fd, readptr, navail);
#endif
            if (nwritten <= 0 && errno != EAGAIN && _uart_path) {
#if !defined(SHAL_CORE_MINGW)
                ::close(fd);
#else
                ::close_serial(fd);
#endif
                _fd = -1;
                _connected = false;
                fprintf(stdout, "Closed [0] connection on uart%c serial port %u\n", (char)(0x41 + _portNumber), _portNumber);
                fflush(stdout);
            }
        } else {
            //if (_select_check(_fd)) {
                nwritten = ::send(_fd, readptr, navail, MSG_DONTWAIT);
            //} else {
                //nwritten = 0;
            //}
        }
        if (nwritten > 0) {
            _writebuffer.advance(nwritten);
        }
    }

    uint32_t space = _readbuffer.space();
    if (space == 0) {
        return;
    }

    char buf[space] = {0};
    ssize_t nread = 0;
    if (!_use_send_recv) {
        int fd = _console?0:_fd;
        //if (_select_check(fd)) {
#if !defined(SHAL_CORE_MINGW)
            nread = ::read(fd, buf, space);
#else
            nread = ::read_serial(fd, buf, space);
#endif
            if (nread <= 0 && errno != EAGAIN && _uart_path) {
#if !defined(SHAL_CORE_MINGW)
                ::close(fd);
#else
                ::close_serial(fd);
#endif
                _fd = -1;
                _connected = false;
                fprintf(stdout, "Closed [1] connection on uart%c serial port %u\n", (char)(0x41 + _portNumber), _portNumber);
                fflush(stdout);
                return;
            }
        //} else {
            //nread = 0;
        //}
    } else {
        if (_select_check(_fd)) {
            nread = ::recv(_fd, buf, space, MSG_DONTWAIT);
            if (nread <= 0) {
                // the socket has reached EOF
                ::close(_fd);
                _connected = false;
                fprintf(stdout, "Closed [2] connection on uart%c serial port %u\n", (char)(0x41 + _portNumber), _portNumber);
                fflush(stdout);
                return;
            }
        } else {
            nread = 0;
        }
    }
    if (nread > 0) {
        _readbuffer.write((uint8_t *)buf, nread);
    }
}

#endif // __CYGWIN__

