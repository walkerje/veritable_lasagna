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
vl_bool_t vlTestAsyncPoolBasic();

//Test high-contention scenario. A bunch of distributed takes/returns.
vl_bool_t vlTestAsyncPoolMPMC();

//Ensure all internal counters are reset and allow for re-use.
vl_bool_t vlTestAsyncPoolClearAndReuse();

#ifdef __cplusplus
}
#endif
#endif //VL_TEST_ASYNC_QUEUE_H


#ifndef VL_TEST_ASYNC_POOL_H
#define VL_TEST_ASYNC_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

// Test multi-producer multi-consumer correctness and concurrency
vl_bool_t vlAsyncQueueTestMPMC();

//Test high-contention scenario. A bunch of distributed takes/returns.
vl_bool_t vlAsyncQueueTest();


#ifdef __cplusplus
}
#endif
#endif //VL_TEST_ASYNC_POOL_H
