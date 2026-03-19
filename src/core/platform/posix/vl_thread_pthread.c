#include <pthread.h>
#include <stdlib.h> /* malloc/free */
#include <time.h>
#include <unistd.h>

/**
 * \private
 */
struct vl_thread_
{
    pthread_t threadHandle;

    pthread_cond_t timeoutCondition;
    pthread_mutex_t timeoutConditionMutex;

    /* Main-thread handling + robust timeout join */
    vl_bool_t is_main;
    vl_bool_t finished; /* protected by timeoutConditionMutex */
};

/**
 * \private
 */
static struct vl_thread_ mainThread;

/**
 * \brief Thread local pointer to the metadata of the current thread.
 */
VL_THREAD_LOCAL vl_thread currentThread = NULL;

/**
 * \private
 */
typedef struct
{
    vl_thread meta;

    vl_thread_proc threadProc;
    void* userArg;
} vl_thread_args;

void* vl_ThreadBootstrap(void* arg)
{
    vl_thread meta;
    vl_thread_proc proc;
    void* userArg;

    {
        vl_thread_args* threadArgs = (vl_thread_args*)(arg);

        meta = threadArgs->meta;
        proc = threadArgs->threadProc;
        userArg = threadArgs->userArg;

        free(threadArgs);
    }

    currentThread = meta;

    proc(userArg);

    /* Mark finished under lock to avoid lost wakeups for timed joiners. */
    pthread_mutex_lock(&meta->timeoutConditionMutex);
    meta->finished = VL_TRUE;
    pthread_cond_broadcast(&meta->timeoutCondition);
    pthread_mutex_unlock(&meta->timeoutConditionMutex);

    currentThread = NULL;

    return NULL;
}

vl_thread vlThreadNew(vl_thread_proc threadProc, void* userArg)
{
    vlThreadCurrent();

    vl_thread_args* args = (vl_thread_args*)malloc(sizeof(vl_thread_args));

    if (args == NULL)
    {
        return (vl_thread)NULL; // Failed to allocate arguments
    }

    vl_thread meta = (vl_thread)malloc(sizeof(struct vl_thread_));
    if (meta == NULL)
    {
        free(args);
        return (vl_thread)NULL; // Failed to allocate arguments
    }

    meta->is_main = VL_FALSE;
    meta->finished = VL_FALSE;

    args->meta = meta;
    args->threadProc = threadProc;
    args->userArg = userArg;

    pthread_mutex_init(&meta->timeoutConditionMutex, NULL);
    pthread_cond_init(&meta->timeoutCondition, NULL);

    if (pthread_create(&meta->threadHandle, NULL, vl_ThreadBootstrap, args) != 0)
    {
        pthread_cond_destroy(&meta->timeoutCondition);
        pthread_mutex_destroy(&meta->timeoutConditionMutex);
        free(args);
        free(meta);
        return (vl_thread)NULL;
    }

    return (vl_thread)meta;
}

void vlThreadDelete(vl_thread thread)
{
    if (thread == NULL)
        return;

    vl_thread meta = thread;

    /* Never destroy/free the static main thread metadata. */
    if (meta->is_main)
    {
        return;
    }

    pthread_mutex_destroy(&meta->timeoutConditionMutex);
    pthread_cond_destroy(&meta->timeoutCondition);
    free(meta);
}

vl_bool_t vlThreadJoin(vl_thread thread)
{
    if (thread == NULL)
        return VL_FALSE;

    vl_thread meta = thread;

    if (meta->is_main)
    {
        /* Joining the main thread via this API doesn't make sense. */
        return VL_FALSE;
    }

    int result = pthread_join(meta->threadHandle, NULL);
    if (result == 0)
    {
        return VL_TRUE; // Finished execution.
    }
    return VL_FALSE; // Error or failure to join
}

vl_bool_t vlThreadJoinTimeout(vl_thread thread, vl_uint_t milliseconds)
{
    if (thread == NULL)
        return VL_FALSE;

    vl_thread meta = thread;
    vl_int_t result;

    if (meta->is_main)
    {
        return VL_FALSE;
    }

    pthread_mutex_lock(&meta->timeoutConditionMutex);

    /* Fast path: already finished (prevents lost wakeups). */
    if (meta->finished)
    {
        pthread_mutex_unlock(&meta->timeoutConditionMutex);
        (void)pthread_join(meta->threadHandle, NULL);
        return VL_TRUE;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += milliseconds / 1000;
    ts.tv_nsec += (milliseconds % 1000) * 1000000;

    if (ts.tv_nsec >= 1000000000L)
    {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000L;
    }

    /* Wait until finished or timeout; loop handles spurious wakeups. */
    while (!meta->finished)
    {
        result = pthread_cond_timedwait(&meta->timeoutCondition, &meta->timeoutConditionMutex, &ts);
        if (result != 0)
        {
            break; /* timeout or error */
        }
    }

    const vl_bool_t done = meta->finished ? VL_TRUE : VL_FALSE;
    pthread_mutex_unlock(&meta->timeoutConditionMutex);

    if (done)
    {
        (void)pthread_join(meta->threadHandle, NULL);
        return VL_TRUE;
    }

    return VL_FALSE;
}

vl_thread vlThreadCurrent(void)
{
    if (currentThread == NULL)
    {
        /* Initialize main thread metadata once, including cond/mutex so the struct
         * is fully valid. */
        mainThread.threadHandle = pthread_self();
        mainThread.is_main = VL_TRUE;
        mainThread.finished = VL_FALSE;

        pthread_mutex_init(&mainThread.timeoutConditionMutex, NULL);
        pthread_cond_init(&mainThread.timeoutCondition, NULL);

        currentThread = &mainThread;
    }
    return (vl_thread)currentThread;
}

vl_bool_t vlThreadYield(void) { return sched_yield() == 0; }

void vlThreadSleep(vl_ularge_t milliseconds) { usleep(milliseconds * 1000); }

void vlThreadSleepNano(vl_ularge_t nanoseconds)
{
    const vl_ularge_t busy_threshold = 10000; // 10,000 ns = 10 µs
    if (nanoseconds < busy_threshold)
    {
        struct timespec start, current;
        clock_gettime(CLOCK_MONOTONIC, &start);

        vl_ularge_t elapsed;
        do
        {
            clock_gettime(CLOCK_MONOTONIC, &current);
            elapsed = (current.tv_sec - start.tv_sec) * 1000000000ull + (current.tv_nsec - start.tv_nsec);
        }
        while (elapsed < nanoseconds);
        return;
    }

    const vl_ularge_t nsecs = nanoseconds % 1000000000ull;
    const vl_ularge_t seconds = nanoseconds / 1000000000ull;

    struct timespec request;
    request.tv_sec = seconds;
    request.tv_nsec = nsecs;
    nanosleep(&request, NULL);
}

void vlThreadExit(void) { pthread_exit(NULL); }
