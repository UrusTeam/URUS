#include "netsocket_win.h"
#if defined(__MINGW32__)
#include <ws2tcpip.h>
void windows_socket_start() {
        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD(2,2);
        WSAStartup( wVersionRequested, &wsaData);
}
#endif
