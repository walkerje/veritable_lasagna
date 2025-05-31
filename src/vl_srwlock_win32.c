#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

vl_srwlock vlSRWLockNew() {
    SRWLOCK *lock = malloc(sizeof(SRWLOCK));
    if (lock == NULL)
        return VL_SRWLOCK_NULL;

    InitializeSRWLock(lock);

    return (vl_srwlock) lock;
}

void vlSRWLockDelete(vl_srwlock lock) {
    SRWLOCK *lockPtr = (SRWLOCK *)lock;
    free(lockPtr);
}

void vlSRWLockObtainShared(vl_srwlock lock) {
    if (lock == 0)
        return;
    AcquireSRWLockShared((SRWLOCK *)lock);
}

vl_bool_t vlSRWLockTryObtainShared(vl_srwlock lock) {
    if (lock == 0)
        return VL_FALSE;
    return TryAcquireSRWLockShared((SRWLOCK *)lock);
}

void vlSRWLockReleaseShared(vl_srwlock lock) {
    if (lock == 0)
        return;
    ReleaseSRWLockShared((SRWLOCK *)lock);
}

void vlSRWLockObtainExclusive(vl_srwlock lock) {
    if (lock == 0)
        return;
    AcquireSRWLockExclusive((SRWLOCK *)lock);
}

vl_bool_t vlSRWLockTryObtainExclusive(vl_srwlock lock) {
    if (lock == 0)
        return VL_FALSE;
    return TryAcquireSRWLockExclusive((SRWLOCK *)lock);
}

void vlSRWLockReleaseExclusive(vl_srwlock lock) {
    if (lock == 0)
        return;
    ReleaseSRWLockExclusive((SRWLOCK *)lock);
}