#include <semaphore.h>
#include <time.h>
#include "vl_atomic.h"

vl_semaphore vlSemaphoreNew(vl_uint_t initialCount){
    sem_t* sem = malloc(sizeof(sem_t));

    if(sem == NULL){
        return 0;
    }

    if(sem_init(sem, 0, initialCount) != 0){
        free(sem);
        return 0;
    }

    return (vl_semaphore) sem;
}

void vlSemaphoreDelete(vl_semaphore sem){
    sem_destroy((sem_t*)sem);
    free((sem_t*)sem);
}

vl_bool_t vlSemaphoreWait(vl_semaphore sem, vl_uint_t timeoutMs){
    struct timespec waitTime, now;
    clock_gettime(CLOCK_REALTIME, &now);

    vl_ularge_t sec = timeoutMs / 1000;
    vl_ularge_t nsec = (timeoutMs % 1000) * 1000000; // Convert remaining ms to ns

    waitTime.tv_sec = now.tv_sec + sec;
    waitTime.tv_nsec = now.tv_nsec + nsec;

    if (waitTime.tv_nsec >= 1000000000) {
        waitTime.tv_sec += 1;
        waitTime.tv_nsec -= 1000000000;
    }
    return sem_timedwait((sem_t*)sem, &waitTime) == 0;
}

void vlSemaphorePost(vl_semaphore sem){
    sem_post((sem_t*)sem);
}

vl_bool_t vlSemaphoreTryWait(vl_semaphore sem){
    return sem_trywait((sem_t*)sem) == 0;
}

vl_uint_t vlSemaphoreGetCount(vl_semaphore sem){
    int val = 0;
    sem_getvalue((sem_t*)sem, &val);
    return val;
}