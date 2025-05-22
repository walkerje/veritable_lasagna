#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vl/vl_thread.h>
#include <vl/vl_mutex.h>
#include <vl/vl_async_queue.h>

#define TEST_COUNT 10000
#define PRODUCER_COUNT 4
#define CONSUMER_COUNT 4

typedef struct {
    vl_async_queue* queue;
    int start;
    int count;
    vl_bool_t* results; // per-thread success results
    int index;
} producer_args;

typedef struct {
    vl_async_queue* queue;
    int totalCount;
    int* consumedCount;
    vl_mutex mutex;
} consumer_args;

vl_bool_t vlAsyncQueueTest() {
    vl_async_queue q;
    vlAsyncQueueInit(&q, sizeof(int));

    int vals[3] = {10, 20, 30};
    for (int i = 0; i < 3; ++i) {
        vlAsyncQueuePushBack(&q, &vals[i]);
    }

    int out = 0;
    for (int i = 0; i < 3; ++i) {
        if (!vlAsyncQueuePopFront(&q, &out)) {
            vlAsyncQueueFree(&q);
            return VL_FALSE;
        }
        if (out != vals[i]) {
            vlAsyncQueueFree(&q);
            return VL_FALSE;
        }
    }

    // Queue should now be empty
    if (vlAsyncQueuePopFront(&q, &out)) {
        vlAsyncQueueFree(&q);
        return VL_FALSE;
    }

    vlAsyncQueueFree(&q);
    return VL_TRUE;
}

// Producer thread function
static void producer_thread(void* arg) {
    producer_args* args = (producer_args*)arg;
    for (int i = 0; i < args->count; ++i) {
        int val = args->start + i;
        vlAsyncQueuePushBack(args->queue, &val);
    }
    args->results[args->index] = VL_TRUE;
    vlThreadExit();
}

// Consumer thread function
static void consumer_thread(void* arg) {
    consumer_args* args = (consumer_args*)arg;
    int val = 0;
    while (1) {
        if (*args->consumedCount >= args->totalCount)
            break;
        else if (vlAsyncQueuePopFront(args->queue, &val)) {
            vlMutexObtainExclusive(args->mutex);
            (*args->consumedCount)++;
            vlMutexReleaseExclusive(args->mutex);
        } else {
            vlThreadYield(); // queue empty, yield and try again
        }
    }
    vlThreadExit();
}

// Test multi-producer multi-consumer correctness and concurrency
vl_bool_t vlAsyncQueueTestMPMC() {
    vl_async_queue q;
    vlAsyncQueueInit(&q, sizeof(int));

    vl_mutex countMutex = vlMutexNew();
    int consumed = 0;
    int totalToProduce = TEST_COUNT * PRODUCER_COUNT;

    producer_args producers[PRODUCER_COUNT];
    vl_bool_t producerResults[PRODUCER_COUNT];
    vl_thread producerThreads[PRODUCER_COUNT];

    consumer_args consumers[CONSUMER_COUNT];
    vl_thread consumerThreads[CONSUMER_COUNT];

    // Initialize producer args
    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        producers[i].queue = &q;
        producers[i].start = i * TEST_COUNT;
        producers[i].count = TEST_COUNT;
        producers[i].results = producerResults;
        producers[i].index = i;
        producerResults[i] = VL_FALSE;
    }

    // Initialize consumer args
    for (int i = 0; i < CONSUMER_COUNT; ++i) {
        consumers[i].queue = &q;
        consumers[i].totalCount = totalToProduce;
        consumers[i].consumedCount = &consumed;
        consumers[i].mutex = countMutex;
    }

    // Start consumers
    for (int i = 0; i < CONSUMER_COUNT; ++i) {
        consumerThreads[i] = vlThreadNew(consumer_thread, &consumers[i]);
        if (consumerThreads[i] == VL_THREAD_NULL) {
            vlAsyncQueueFree(&q);
            vlMutexDelete(countMutex);
            return VL_FALSE;
        }
    }

    // Start producers
    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        producerThreads[i] = vlThreadNew(producer_thread, &producers[i]);
        if (producerThreads[i] == VL_THREAD_NULL) {
            vlAsyncQueueFree(&q);
            vlMutexDelete(countMutex);
            return VL_FALSE;
        }
    }

    // Join producers
    for (int i = 0; i < PRODUCER_COUNT; ++i) {
        vlThreadJoin(producerThreads[i]);
        vlThreadDelete(producerThreads[i]);
        if (!producerResults[i]) {
            vlAsyncQueueFree(&q);
            vlMutexDelete(countMutex);
            return VL_FALSE;
        }
    }

    // Join consumers
    for (int i = 0; i < CONSUMER_COUNT; ++i) {
        vlThreadJoin(consumerThreads[i]);
        vlThreadDelete(consumerThreads[i]);
    }

    // Check if consumed all produced items
    vl_bool_t pass = (consumed == totalToProduce);

    vlAsyncQueueFree(&q);
    vlMutexDelete(countMutex);

    return pass;
}