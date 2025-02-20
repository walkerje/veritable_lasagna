#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

vl_mutex vlMutexNew(){
    SRWLOCK* lock = malloc(sizeof(SRWLOCK));
    if(lock == NULL)
        return VL_MUTEX_NULL;

    InitializeSRWLock(lock);

    return (vl_mutex)lock;
}

void vlMutexDelete(vl_mutex mutex){
    SRWLOCK* lock = (SRWLOCK*) mutex;
    free(lock);
}

void            vlMutexObtainShared(vl_mutex mutex){
    if(mutex == 0)
        return;
    AcquireSRWLockShared((SRWLOCK*) mutex);
}

vl_bool_t       vlMutexTryObtainShared(vl_mutex mutex){
    if(mutex == 0)
        return VL_FALSE;
    return TryAcquireSRWLockShared((SRWLOCK*) mutex);
}

void            vlMutexReleaseShared(vl_mutex mutex){
    if(mutex == 0)
        return;
    ReleaseSRWLockShared((SRWLOCK*) mutex);
}

void            vlMutexObtainExclusive(vl_mutex mutex) {
    if(mutex == 0)
        return;
    AcquireSRWLockExclusive((SRWLOCK *) mutex);
}

vl_bool_t       vlMutexTryObtainExclusive(vl_mutex mutex){
    if(mutex == 0)
        return VL_FALSE;
    return TryAcquireSRWLockExclusive((SRWLOCK*) mutex);
}

void            vlMutexReleaseExclusive(vl_mutex mutex){
    if(mutex == 0)
        return;
    ReleaseSRWLockExclusive((SRWLOCK*) mutex);
}