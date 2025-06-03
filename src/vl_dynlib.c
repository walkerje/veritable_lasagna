/**
 * This file serves as configuration for which library implementation to use.
 */

#include <stdlib.h>
#include "vl/vl_libconfig.h"
#include "vl_dynlib.h"

#ifdef VL_DYNLIB_WIN32

#include "platform/win32/vl_dynlib_win32.c"

#elif defined VL_DYNLIB_POSIX

#include "platform/posix/vl_dynlib_posix.c"

#else

#error Failed to configure vl_dynlib implementation.

#endif