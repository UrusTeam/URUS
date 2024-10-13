#pragma once

#ifdef ENABLE_GET_IP_HOST
#if defined(__MINGW32__)
#include <ws2tcpip.h>

char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
#endif
#endif
