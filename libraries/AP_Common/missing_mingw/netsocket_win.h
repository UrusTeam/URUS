#pragma once

#include "inet_ntop.h"

#if defined(__MINGW32__)

#ifndef SHUT_RDWR
#define SHUT_RD   0x00
#define SHUT_WR   0x01
#define SHUT_RDWR 0x02
#endif

#define MSG_DONTWAIT 0

void windows_socket_start();
#endif // __MINGW32__
