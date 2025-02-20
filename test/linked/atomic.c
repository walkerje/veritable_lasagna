#include "atomic.h"
#include <vl/vl_atomic.h>
#include <vl/vl_thread.h>
#include <vl/vl_mutex.h>
#include <stdio.h>

typedef struct{
    vl_uint_t maxIterations;

    vl_uint32_t naiveCounter;
    vl_atomic_uint32_t atomicCounter;

    vl_mutex goLock;
} vl_counter_test;

void vlTestAtomicCounterThread(void* arg){
    vl_counter_test* test = (vl_counter_test*)arg;

    //Wait to obtain a shared lock.
    vlMutexObtainShared(test->goLock);

    //Then, slam the counters with increments.
    for(int i = 0; i < test->maxIterations; i++){
        test->naiveCounter++;
        vlAtomicFetchAdd(&test->atomicCounter, 1);
    }

    vlMutexReleaseShared(test->goLock);
}

#define VL_TEST_ATOMIC_COUNTER_THREADS 8

/**
 * \brief Demonstrate the race condition on naive addition in comparison to an atomic counter.
 * \private
 * \return a boolean indicating some variance
 */
vl_bool_t vlTestAtomicCounter(){
    const int maxAttempts = 1024;

    vl_counter_test testVars;
    testVars.maxIterations = 1024;
    testVars.goLock = vlMutexNew();

    vl_thread threads[VL_TEST_ATOMIC_COUNTER_THREADS];
    const vl_uint32_t expectedNaiveCounter = testVars.maxIterations * VL_TEST_ATOMIC_COUNTER_THREADS;
    printf("Expected result: %u\n", expectedNaiveCounter);

    for (int i = 0; i < maxAttempts; i++) {
        printf("Attempt %d\n", (i+1));

        testVars.naiveCounter = 0;
        vlAtomicStore(&testVars.atomicCounter, 0);

        {
            vlMutexObtain(testVars.goLock);

            for(int j = 0; j < VL_TEST_ATOMIC_COUNTER_THREADS; j++){
                threads[j] = vlThreadNew(vlTestAtomicCounterThread, &testVars);
            }

            //Tell the threads to start incrementing.
            vlMutexRelease(testVars.goLock);

            for(int j = 0; j < VL_TEST_ATOMIC_COUNTER_THREADS; j++){
                vl_thread thread = threads[j];
                vlThreadJoin(thread);
                vlThreadDelete(thread);
            }
        }

        const vl_uint32_t atomicResult = vlAtomicLoad(&testVars.atomicCounter);

        printf("Naive Result: %u\nAtomicResult: %u\n",testVars.naiveCounter, atomicResult);

        if((atomicResult == expectedNaiveCounter) && (testVars.naiveCounter != expectedNaiveCounter)) {
            vlMutexDelete(testVars.goLock);
            return VL_TRUE;
        }
    }

    vlMutexDelete(testVars.goLock);

    /**
     * THIS TEST MAY FAIL SPURIOUSLY DUE TO ITS ATOMIC NATURE.
     * We're trying to encourage a failure here as a demonstration.
     * The mutex is intended to encourage a race condition on the non-atomic counter.
     */
    return VL_FALSE;
}

#undef VL_TEST_ATOMIC_COUNTER_THREADS