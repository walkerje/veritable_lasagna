#include <pthread.h>

vl_srwlock        vlSRWLockNew(){
    pthread_rwlock_t* lock = malloc(sizeof(pthread_rwlock_t));

    if(lock == NULL){
        return VL_SRWLOCK_NULL;
    }

    if(pthread_rwlock_init(lock, NULL) != 0){
        free(lock);
        return VL_SRWLOCK_NULL;
    }

    return (vl_srwlock)lock;
}

void            vlSRWLockDelete(vl_srwlock lock){
    if(lock == 0){
        return;
    }

    pthread_rwlock_t* lockPtr = (pthread_rwlock_t*)lock;
    pthread_rwlock_destroy(lockPtr);
    free(lockPtr);
}

void            vlSRWLockObtainShared(vl_srwlock lock){
    if(lock == 0){
        return;
    }
    pthread_rwlock_rdlock((pthread_rwlock_t*)lock);
}

vl_bool_t       vlSRWLockTryObtainShared(vl_srwlock lock){
    if(lock == 0){
        return VL_FALSE;
    }

    return (pthread_rwlock_tryrdlock((pthread_rwlock_t*)lock) == 0);
}

void            vlSRWLockReleaseShared(vl_srwlock lock){
    if(lock == 0){
        return;
    }

    pthread_rwlock_unlock((pthread_rwlock_t*)lock);//Same as exclusive lock.
}

void            vlSRWLockObtainExclusive(vl_srwlock lock){
    if(lock == 0){
        return;
    }

    pthread_rwlock_wrlock((pthread_rwlock_t*)lock);
}

vl_bool_t       vlSRWLockTryObtainExclusive(vl_srwlock lock){
    if(lock == 0){
        return VL_FALSE;
    }

    return (pthread_rwlock_trywrlock((pthread_rwlock_t*)lock) == 0);
}

void            vlSRWLockReleaseExclusive(vl_srwlock lock){
    if(lock == 0){
        return;
    }

    pthread_rwlock_unlock((pthread_rwlock_t*)lock);//Same as shared lock.
}