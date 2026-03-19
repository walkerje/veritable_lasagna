/**
 * This file serves as configuration for which library implementation to use.
 */

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "vl_condition.h"

#include <stdlib.h>

#include "vl/vl_libconfig.h"

#ifdef VL_THREADS_WIN32

#include "platform/win32/vl_condition_win32.c"

#elif defined VL_THREADS_PTHREAD

#include "platform/posix/vl_condition_pthread.c"

#else
#error Failed to configure vl_condition implementation.
#endif
