#include "vl_atomic.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <process.h>
#include <windows.h>

typedef struct{
    vl_thread_proc  threadProc;
    void*           userArg;
} vl_thread_args;

unsigned vl_ThreadBootstrap(void* arg){
    vl_thread_proc proc;
    void* userArg;

    {
        vl_thread_args* threadArgs = (vl_thread_args*)(arg);

        proc        = threadArgs->threadProc;
        userArg     = threadArgs->userArg;

        free(threadArgs);
    }

    proc(userArg);

    return 0;
}

vl_thread vlThreadNew(vl_thread_proc threadProc, void* userArg) {
    vl_thread_args* args = malloc(sizeof(vl_thread_args));

    if (args == NULL) {
        return 0;  // Failed to allocate arguments
    }

    args->threadProc = threadProc;
    args->userArg = userArg;

    vl_uint_t threadID;
    vl_uintptr_t threadHandle = _beginthreadex(NULL, 0, vl_ThreadBootstrap, args, 0, &threadID);
    if (threadHandle == 0) {
        free(args);  // Clean up on failure
        return 0;
    }

    return (vl_thread)threadHandle;  // Return thread handle
}

void vlThreadDelete(vl_thread thread){
    CloseHandle((HANDLE) thread);
}

vl_bool_t vlThreadJoin(vl_thread thread){
    HANDLE handle = (HANDLE) thread;
    const DWORD status = WaitForSingleObject(handle, INFINITE);

    if(status == WAIT_OBJECT_0){
        return VL_TRUE;//Finished execution.
    }

    return VL_FALSE;//Timed out or failed to wait.
}

vl_bool_t vlThreadJoinTimeout(vl_thread thread, vl_uint_t milliseconds){
    HANDLE handle = (HANDLE) thread;
    const DWORD status = WaitForSingleObject(handle, milliseconds);

    if(status == WAIT_OBJECT_0){
        return VL_TRUE;//Finished execution.
    }

    return VL_FALSE;//Timed out or failed to wait.
}

vl_bool_t vlThreadTerminate(vl_thread thread){
    if(TerminateThread((HANDLE)thread, 0)){
        return VL_TRUE;
    }

    return VL_FALSE;
}

vl_thread vlThreadCurrent(){
    return (vl_thread) GetCurrentThread();
}