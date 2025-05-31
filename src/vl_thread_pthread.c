#include <time.h>
#include <pthread.h>
#include <unistd.h>

/**
 * \private
 */
typedef struct {
    pthread_t threadHandle;

    pthread_cond_t timeoutCondition;
    pthread_mutex_t timeoutConditionMutex;
} vl_posix_thread;

/**
 * \private
 */
vl_posix_thread mainThread;

/**
 * \brief Thread local pointer to the metadata of the current thread.
 */
VL_THREAD_LOCAL vl_posix_thread *currentThread = NULL;

/**
 * \private
 */
typedef struct {
    vl_posix_thread *meta;

    vl_thread_proc threadProc;
    void *userArg;
} vl_thread_args;

void *vl_ThreadBootstrap(void *arg) {
    vl_posix_thread *meta;
    vl_thread_proc proc;
    void *userArg;


    {
        vl_thread_args *threadArgs = (vl_thread_args *) (arg);

        meta = threadArgs->meta;
        proc = threadArgs->threadProc;
        userArg = threadArgs->userArg;

        free(threadArgs);
    }

    currentThread = meta;

    proc(userArg);

    currentThread = NULL;

    pthread_mutex_lock(&meta->timeoutConditionMutex);
    pthread_cond_signal(&meta->timeoutCondition);
    pthread_mutex_unlock(&meta->timeoutConditionMutex);

    return NULL;
}

vl_thread vlThreadNew(vl_thread_proc threadProc, void *userArg) {
    vlThreadCurrent();

    vl_thread_args *args = malloc(sizeof(vl_thread_args));

    if (args == NULL) {
        return (vl_thread) NULL;  // Failed to allocate arguments
    }

    vl_posix_thread *meta = malloc(sizeof(vl_posix_thread));
    if (meta == NULL) {
        free(args);
        return (vl_thread) NULL;  // Failed to allocate arguments
    }

    args->meta = meta;
    args->threadProc = threadProc;
    args->userArg = userArg;

    pthread_mutex_init(&meta->timeoutConditionMutex, NULL);
    pthread_cond_init(&meta->timeoutCondition, NULL);

    if (pthread_create(&meta->threadHandle, NULL, vl_ThreadBootstrap, args) != 0) {
        free(args);
        free(meta);
        return 0;
    }

    return (vl_thread) meta;
}

void vlThreadDelete(vl_thread thread) {
    if (thread == 0 || thread == ((vl_uintptr_t) &mainThread))
        return;

    vl_posix_thread *meta = (vl_posix_thread *) thread;

    pthread_mutex_destroy(&meta->timeoutConditionMutex);
    pthread_cond_destroy(&meta->timeoutCondition);
    free((vl_posix_thread *) thread);
}

vl_bool_t vlThreadJoin(vl_thread thread) {
    if (thread == 0 || thread == ((vl_uintptr_t) &mainThread))
        return VL_FALSE;

    int result = pthread_join(((vl_posix_thread *) thread)->threadHandle, NULL);
    if (result == 0) {
        return VL_TRUE; // Finished execution.
    }
    return VL_FALSE; // Error or failure to join
}

vl_bool_t vlThreadJoinTimeout(vl_thread thread, vl_uint_t milliseconds) {
    if (thread == 0 || thread == ((vl_uintptr_t) &mainThread))
        return VL_FALSE;

    vl_posix_thread *meta = (vl_posix_thread *) thread;
    vl_int_t result;

    // Lock the mutex to protect the condition variable
    pthread_mutex_lock(&meta->timeoutConditionMutex);

    // Get the current time and add the timeout value
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += milliseconds / 1000;
    ts.tv_nsec += (milliseconds % 1000) * 1000000;

    // Normalize timespec in case of overflow in nsec
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }

    // Wait for the thread to finish or timeout
    result = pthread_cond_timedwait(&meta->timeoutCondition, &meta->timeoutConditionMutex, &ts);

    // Unlock the mutex
    pthread_mutex_unlock(&meta->timeoutConditionMutex);

    if (result == 0) {
        // Thread finished within the timeout
        // Join it "for real" for cleanup.
        pthread_join(meta->threadHandle, NULL);
        return VL_TRUE;
    }

    // An error occurred
    return VL_FALSE;
}

vl_thread vlThreadCurrent() {
    switch ((vl_uintptr_t) currentThread) {
        case 0:
            currentThread = &mainThread;
            currentThread->threadHandle = pthread_self();
            //Timeout condition & mutex left uninitialized for the main thread.
        default:
            return (vl_thread) currentThread;
    }
}

vl_bool_t vlThreadYield() {
    return sched_yield() == 0;
}

void vlThreadSleep(vl_ularge_t milliseconds) {
    usleep(milliseconds * 1000);
}

void vlThreadSleepNano(vl_ularge_t nanoseconds) {
    const vl_ularge_t busy_threshold = 10000; // 10,000 ns = 10 µs
    if (nanoseconds < busy_threshold) {
        struct timespec start, current;
        clock_gettime(CLOCK_MONOTONIC, &start);

        vl_ularge_t elapsed;
        do {
            clock_gettime(CLOCK_MONOTONIC, &current);
            elapsed = (current.tv_sec - start.tv_sec) * 1000000000ull +
                      (current.tv_nsec - start.tv_nsec);
        } while (elapsed < nanoseconds);
        return;
    }

    const vl_ularge_t nsecs = nanoseconds % 1000000000ull;
    const vl_ularge_t seconds = nanoseconds / 1000000000ull;

    struct timespec request;
    request.tv_sec = seconds;
    request.tv_nsec = nsecs;
    nanosleep(&request, NULL);
}

void vlThreadExit() {
    pthread_exit(NULL);
}