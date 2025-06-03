/**
 * This file serves as configuration for which library implementation to use.
 */

#include <stdlib.h>
#include "vl/vl_libconfig.h"
#include "vl_condition.h"

#ifdef VL_THREADS_WIN32

#include "platform/win32/vl_condition_win32.c"

#elif defined VL_THREADS_PTHREAD

#include "platform/posix/vl_condition_pthread.c"

#else
#error Failed to configure vl_condition implementation.
#endif