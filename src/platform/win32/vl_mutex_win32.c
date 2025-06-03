#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

/**
 * \brief Mutex primitives are implemented using the Windows SRWLOCK API.
 * This is due to Win32 mutexes being "heavyweight" and being historically
 * less performant than other comparable APIs within the same process.
 * (E.g, if inter-process sharing isn't needed, then SRWLOCK is a good
 * choice.)
 *
 * https://github.com/markwaterman/MutexShootout
 *
 */

#include <windows.h>

vl_mutex vlMutexNew() {
    SRWLOCK *lock = malloc(sizeof(SRWLOCK));
    if (lock == NULL)
        return VL_MUTEX_NULL;

    InitializeSRWLock(lock);

    return (vl_mutex) lock;
}

void vlMutexDelete(vl_mutex mutex) {
    SRWLOCK *lock = (SRWLOCK *) mutex;
    free(lock);
}

void vlMutexObtain(vl_mutex mutex) {
    if (mutex == 0)
        return;
    AcquireSRWLockExclusive((SRWLOCK *) mutex);
}

vl_bool_t vlMutexTryObtain(vl_mutex mutex) {
    if (mutex == 0)
        return VL_FALSE;
    return TryAcquireSRWLockExclusive((SRWLOCK *) mutex);
}

void vlMutexRelease(vl_mutex mutex) {
    if (mutex == 0)
        return;
    ReleaseSRWLockExclusive((SRWLOCK *) mutex);
}