#include <pthread.h>

vl_mutex        vlMutexNew(){
    pthread_rwlock_t* lock = malloc(sizeof(pthread_rwlock_t));

    if(lock == NULL){
        return VL_MUTEX_NULL;
    }

    if(pthread_rwlock_init(lock, NULL) != 0){
        free(lock);
        return VL_MUTEX_NULL;
    }

    return (vl_mutex)lock;
}

void            vlMutexDelete(vl_mutex mutex){
    if(mutex == 0){
        return;
    }

    pthread_rwlock_t* lock = (pthread_rwlock_t*)mutex;
    pthread_rwlock_destroy(lock);
    free(lock);
}

void            vlMutexObtainShared(vl_mutex mutex){
    if(mutex == 0){
        return;
    }
    pthread_rwlock_rdlock((pthread_rwlock_t*)mutex);
}

vl_bool_t       vlMutexTryObtainShared(vl_mutex mutex){
    if(mutex == 0){
        return VL_FALSE;
    }

    return (pthread_rwlock_tryrdlock((pthread_rwlock_t*)mutex) == 0);
}

void            vlMutexReleaseShared(vl_mutex mutex){
    if(mutex == 0){
        return;
    }

    pthread_rwlock_unlock((pthread_rwlock_t*)mutex);//Same as exclusive lock.
}

void            vlMutexObtainExclusive(vl_mutex mutex){
    if(mutex == 0){
        return;
    }

    pthread_rwlock_wrlock((pthread_rwlock_t*)mutex);
}

vl_bool_t       vlMutexTryObtainExclusive(vl_mutex mutex){
    if(mutex == 0){
        return VL_FALSE;
    }

    return (pthread_rwlock_trywrlock((pthread_rwlock_t*)mutex) == 0);
}

void            vlMutexReleaseExclusive(vl_mutex mutex){
    if(mutex == 0){
        return;
    }

    pthread_rwlock_unlock((pthread_rwlock_t*)mutex);//Same as shared lock.
}