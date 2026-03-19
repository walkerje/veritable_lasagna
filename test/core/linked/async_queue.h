//
// Created by silas on 5/19/2025.
//

#ifndef VL_TEST_ASYNC_QUEUE_H
#define VL_TEST_ASYNC_QUEUE_H
#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

//Basic single-threaded use case. Verify take, return, and take without corruption.
VL_TEST_API vl_bool_t vlAsyncQueueTest();

//Test high-contention scenario. A bunch of distributed takes/returns.
VL_TEST_API vl_bool_t vlAsyncQueueTestMPMC();

#ifdef __cplusplus
}
#endif
#endif //VL_TEST_ASYNC_QUEUE_H