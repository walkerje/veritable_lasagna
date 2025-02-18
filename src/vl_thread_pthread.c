#include <time.h>
#include <pthread.h>

typedef struct{
    pthread_t threadHandle;

    pthread_cond_t      timeoutCondition;
    pthread_mutex_t     timeoutConditionMutex;
} vl_posix_thread;

typedef struct{
    vl_posix_thread* meta;

    vl_thread_proc  threadProc;
    void*           userArg;
} vl_thread_args;

void* vl_ThreadBootstrap(void* arg) {
    vl_posix_thread* meta;
    vl_thread_proc proc;
    void* userArg;

    {
        vl_thread_args* threadArgs = (vl_thread_args*)(arg);

        meta        = threadArgs->meta;
        proc        = threadArgs->threadProc;
        userArg     = threadArgs->userArg;

        free(threadArgs);
    }

    proc(userArg);

    pthread_mutex_lock(&meta->timeoutConditionMutex);
    pthread_cond_signal(&meta->timeoutCondition);
    pthread_mutex_unlock(&meta->timeoutConditionMutex);

    return NULL;
}

vl_thread   vlThreadNew(vl_thread_proc threadProc, void* userArg){
    vl_thread_args* args = malloc(sizeof(vl_thread_args));

    if (args == NULL) {
        return (vl_thread) NULL;  // Failed to allocate arguments
    }

    vl_posix_thread* meta = malloc(sizeof(vl_posix_thread));
    if (meta == NULL) {
        free(args);
        return (vl_thread) NULL;  // Failed to allocate arguments
    }

    args->meta = meta;
    args->threadProc = threadProc;
    args->userArg = userArg;

    if(pthread_create(&meta->threadHandle, NULL, vl_ThreadBootstrap, args) != 0){
        free(args);
        free(meta);
        return 0;
    }

    pthread_mutex_init(&meta->timeoutConditionMutex, NULL);
    pthread_cond_init(&meta->timeoutCondition, NULL);

    return (vl_thread) meta;
}

void        vlThreadDelete(vl_thread thread){
    if(thread == 0)
        return;

    vl_posix_thread* meta = (vl_posix_thread*) thread;

    pthread_mutex_destroy(&meta->timeoutConditionMutex);
    pthread_cond_destroy(&meta->timeoutCondition);
    free((vl_posix_thread*) thread);
}

vl_bool_t   vlThreadJoin(vl_thread thread){
    int result = pthread_join(((vl_posix_thread*)thread)->threadHandle, NULL);
    if (result == 0) {
        return VL_TRUE; // Finished execution.
    }
    return VL_FALSE; // Error or failure to join
}

vl_bool_t   vlThreadJoinTimeout(vl_thread thread, vl_uint_t milliseconds){
    vl_posix_thread* meta = (vl_posix_thread*) thread;
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
        return VL_TRUE;
    }

    // An error occurred
    return VL_FALSE;
}


vl_thread   vlThreadCurrent(){
    return (vl_thread) pthread_self();
}