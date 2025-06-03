#include <pthread.h>

vl_condition    vlConditionNew(){
    pthread_cond_t* cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    pthread_cond_init(cond, NULL);
    return (vl_condition)cond;;
}

void            vlConditionDelete(vl_condition cond){
    pthread_cond_destroy((pthread_cond_t*)cond);
    free((pthread_cond_t*)cond);
}

void            vlConditionWait(vl_condition cond, vl_mutex mutex){
    pthread_cond_wait((pthread_cond_t*)cond, (pthread_mutex_t*)mutex);
}

vl_bool_t       vlConditionWaitTimeout(vl_condition cond, vl_mutex mutex, vl_ularge_t millis){
    struct timespec waitTime, now;
    clock_gettime(CLOCK_REALTIME, &now);

    vl_ularge_t sec = millis / 1000;
    vl_ularge_t nsec = (millis % 1000) * 1000000; // Convert remaining ms to ns

    waitTime.tv_sec = now.tv_sec + sec;
    waitTime.tv_nsec = now.tv_nsec + nsec;

    if (waitTime.tv_nsec >= 1000000000) {
        waitTime.tv_sec += 1;
        waitTime.tv_nsec -= 1000000000;
    }

    return pthread_cond_timedwait((pthread_cond_t*)cond, (pthread_mutex_t*)mutex, &waitTime) == 0;
}

void            vlConditionSignal(vl_condition cond){
    pthread_cond_signal((pthread_cond_t*)cond);
}

void            vlConditionBroadcast(vl_condition cond){
    pthread_cond_broadcast((pthread_cond_t*)cond);
}