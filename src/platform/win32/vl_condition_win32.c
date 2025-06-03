#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

vl_condition vlConditionNew(){
    PCONDITION_VARIABLE cond = malloc(sizeof(CONDITION_VARIABLE));

    InitializeConditionVariable(cond);

    return (vl_condition)cond;
}

void vlConditionDelete(vl_condition cond){
    free((PCONDITION_VARIABLE)cond);
}

void vlConditionWait(vl_condition cond, vl_mutex mutex){
    SRWLOCK* lock = (SRWLOCK*)mutex;
    SleepConditionVariableSRW((PCONDITION_VARIABLE) cond, lock, INFINITE, 0);
}

vl_bool_t vlConditionWaitTimeout(vl_condition cond, vl_mutex mutex, vl_ularge_t millis){
    SRWLOCK* lock = (SRWLOCK*)mutex;
    return SleepConditionVariableSRW((PCONDITION_VARIABLE) cond, lock, millis, 0) != 0;
}

void vlConditionSignal(vl_condition cond){
    WakeConditionVariable((PCONDITION_VARIABLE) cond);
}

void vlConditionBroadcast(vl_condition cond){
    WakeAllConditionVariable((PCONDITION_VARIABLE) cond);
}