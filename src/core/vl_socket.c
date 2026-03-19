/**
 * This file serves as configuration for which library implementation to use.
 */

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <vl/vl_numtypes.h>
#include <vl/vl_socket.h>

#ifdef VL_SOCKET_WIN32

#include "platform/win32/vl_socket_win32.c"

#elif defined(VL_SOCKET_POSIX)

#include "platform/posix/vl_socket_posix.c"

#else
#error Failed to configure vl_socket implementation.
#endif
