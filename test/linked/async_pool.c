#include "async_pool.h"
#include "vl/vl_rand.h"
#include <vl/vl_async_pool.h>
#include <vl/vl_thread.h>
#include <vl/vl_mutex.h>
#include <vl/vl_condition.h>
#include <stdio.h>

#define VL_ASYNC_POOL_TEST_CONTENTION_THREADS 8

typedef struct {
    vl_async_pool atomicPool;
    vl_condition goCondition;
    vl_mutex goMutex;
    vl_uint_t iterations;
    vl_atomic_uint32_t waitingThreads;
} vl_async_pool_test_mpmc_args;

void vl_AsyncPoolTestWorkerMPMC(void *argPtr) {
    vl_async_pool_test_mpmc_args *args = argPtr;

    //wait for signal to proceed
    vlAtomicFetchAdd(&args->waitingThreads, 1);
    vlMutexObtain(args->goMutex);
    vlConditionWait(args->goCondition, args->goMutex);
    vlMutexRelease(args->goMutex);

    vl_rand rand = vlRandInit();
    for (vl_uint_t i = 0; i < args->iterations; i++) {
        //Take an item from the pool.
        vl_uint32_t *taken = vlAsyncPoolTake(&args->atomicPool);
        //Assign it a random number.
        *taken = vlRandUInt32(&rand);
        //Sleep for a small delay to emulate activity.
        vlThreadSleepNano(vlRandNext(&rand) % 100);
        //Then return the node to the pool.
        vlAsyncPoolReturn(&args->atomicPool, taken);
    }
}

vl_bool_t vlTestAsyncPoolBasic() {
    vl_async_pool pool;
    vlAsyncPoolInit(&pool, sizeof(vl_uint32_t));

    const vl_uint32_t nodeValue = 0x600DCAFE;
    vl_uint32_t *elemA = vlAsyncPoolTake(&pool);
    *elemA = nodeValue;

    vlAsyncPoolReturn(&pool, elemA);

    vl_uint32_t *elemB = vlAsyncPoolTake(&pool);
    const vl_bool_t elementsMatch = *elemB == nodeValue;
    vlAsyncPoolFree(&pool);

    return elementsMatch && (elemA == elemB);//Values and pointers should match due to reuse.
}

vl_bool_t vlTestAsyncPoolMPMC() {
    vl_thread threads[VL_ASYNC_POOL_TEST_CONTENTION_THREADS];
    vl_async_pool_test_mpmc_args args;

    args.iterations = 2048;
    args.goMutex = vlMutexNew();
    args.goCondition = vlConditionNew();
    vlAtomicInit(&args.waitingThreads, 0);

    //Pool of unsigned integers
    vlAsyncPoolInit(&args.atomicPool, sizeof(vl_uint32_t));

    //Spawn some threads.
    for (int i = 0; i < VL_ASYNC_POOL_TEST_CONTENTION_THREADS; i++)
        threads[i] = vlThreadNew(vl_AsyncPoolTestWorkerMPMC, &args);

    //Wait for all threads to wait for the start condition.
    while (vlAtomicLoad(&args.waitingThreads) < VL_ASYNC_POOL_TEST_CONTENTION_THREADS) {
        vlThreadYield();
    }

    vlConditionBroadcast(args.goCondition);

    for (int i = 0; i < VL_ASYNC_POOL_TEST_CONTENTION_THREADS; i++) {
        vlThreadJoin(threads[i]);
        vlThreadDelete(threads[i]);
    }

    vlConditionDelete(args.goCondition);
    vlMutexDelete(args.goMutex);

    vl_uint32_t actualLength = 0;
    {
        vl_tagged_ptr stackTop = vlAtomicLoad(&args.atomicPool.freeStack);
        vl_uintptr_t current = stackTop.ptr;
        for (int i = 0; i < (vlAtomicLoad(&args.atomicPool.freeLength)); i++) {
            vl_async_pool_header *header = (vl_async_pool_header *) current;
            if (!header)
                break;

            actualLength++;
            current = header->next;
        }
    }
    const vl_uint32_t expectedLength = vlAtomicLoad(&args.atomicPool.freeLength);
    printf("Expected %u, got %u.", (unsigned int) (expectedLength), (unsigned int) actualLength);

    vlAsyncPoolFree(&args.atomicPool);

    //Total of free nodes should ALWAYS be <= the number of threads that work with the pool.
    //All nodes in this test should be returned after use. If properly re-using pool nodes
    //between threads, this assertion should always be TRUE.
    const vl_bool_t freeReasonable = expectedLength <= VL_ASYNC_POOL_TEST_CONTENTION_THREADS;
    return freeReasonable &&
           (actualLength == expectedLength);//Should not deadlock, and total free should match traversed sum.
}

vl_bool_t vlTestAsyncPoolClearAndReuse() {
    vl_async_pool pool;
    vlAsyncPoolInit(&pool, sizeof(vl_uint32_t));

    const vl_uint32_t nodeValue = 0x600DCAFE;
    vl_uint32_t *elemA = vlAsyncPoolTake(&pool);
    *elemA = nodeValue;

    vlAsyncPoolClear(&pool);

    vl_uint32_t *elemB = vlAsyncPoolTake(&pool);
    const vl_bool_t elementsMatch = *elemB == nodeValue;
    vlAsyncPoolFree(&pool);
    return elementsMatch && (elemA == elemB);//Values and pointers should match due to reuse.
}

vl_bool_t vlTestAsyncPoolAlign() {
    // Test various alignment values
    vl_async_pool pool;
    vlAsyncPoolInitAligned(&pool, sizeof(double), 16);

    // Verify element alignment
    void *ptr = vlAsyncPoolTake(&pool);
    const int result = (((vl_uintptr_t) ptr % 16) == 0);

    vlAsyncPoolFree(&pool);
    return result;
}