#include <pthread.h>

vl_mutex vlMutexNew() {
    pthread_mutex_t *lock = malloc(sizeof(pthread_mutex_t));

    if (lock == NULL) {
        return VL_MUTEX_NULL;
    }

    if (pthread_mutex_init(lock, NULL) != 0) {
        free(lock);
        return VL_MUTEX_NULL;
    }

    return (vl_mutex) lock;
}

void vlMutexDelete(vl_mutex mutex) {
    if (mutex == 0) {
        return;
    }

    pthread_mutex_t *lock = (pthread_mutex_t *) mutex;
    pthread_mutex_destroy(lock);
    free(lock);
}

void vlMutexObtain(vl_mutex mutex) {
    if (mutex == 0) {
        return;
    }

    pthread_mutex_lock((pthread_mutex_t *) mutex);
}

vl_bool_t vlMutexTryObtain(vl_mutex mutex) {
    if (mutex == 0) {
        return VL_FALSE;
    }

    return (pthread_mutex_trylock((pthread_mutex_t *) mutex) == 0);
}

void vlMutexRelease(vl_mutex mutex) {
    if (mutex == 0) {
        return;
    }

    pthread_mutex_unlock((pthread_mutex_t *) mutex);//Same as shared lock.
}