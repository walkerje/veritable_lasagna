#ifndef VL_THREAD_H
#define VL_THREAD_H

#include "vl_numtypes.h"

#ifndef VL_THREAD_LOCAL
#define VL_THREAD_LOCAL _Thread_local
#endif

typedef vl_uintptr_t vl_thread;
typedef void (*vl_thread_proc)(void* usr);

vl_thread   vlThreadNew(vl_thread_proc, void* userArg);

void        vlThreadDelete(vl_thread thread);

vl_bool_t   vlThreadJoin(vl_thread thread);

vl_bool_t   vlThreadJoinTimeout(vl_thread thread, vl_uint_t milliseconds);

vl_thread   vlThreadCurrent();

#endif //VL_THREAD_H
