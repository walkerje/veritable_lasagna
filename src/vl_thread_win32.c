#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <process.h>
#include <windows.h>

HANDLE mainThread = NULL;
VL_THREAD_LOCAL HANDLE currentThread = NULL;

typedef struct{
    HANDLE threadHandle;

    vl_thread_proc  threadProc;
    void*           userArg;
} vl_thread_args;

unsigned __stdcall vl_ThreadBootstrap(void* arg){
    HANDLE threadHandle;
    vl_thread_proc proc;
    void* userArg;

    {
        vl_thread_args* threadArgs = (vl_thread_args*)(arg);

        threadHandle    = threadArgs->threadHandle;
        proc            = threadArgs->threadProc;
        userArg         = threadArgs->userArg;

        free(threadArgs);
    }

    currentThread = threadHandle;
    proc(userArg);
    currentThread = NULL;

    _endthreadex(0);
    return 0;
}

vl_thread vlThreadNew(vl_thread_proc threadProc, void* userArg) {
    vlThreadCurrent();

    vl_thread_args* args = malloc(sizeof(vl_thread_args));

    if (args == NULL) {
        return 0;  // Failed to allocate arguments
    }

    args->threadProc = threadProc;
    args->userArg = userArg;

    vl_uint_t threadID;
    vl_uintptr_t threadHandle = _beginthreadex(NULL, 0, vl_ThreadBootstrap, args, CREATE_SUSPENDED, &threadID);
    if (threadHandle == 0) {
        free(args);  // Clean up on failure
        return 0;
    }

    args->threadHandle = (HANDLE)threadHandle;
    ResumeThread(args->threadHandle);

    return (vl_thread)threadHandle;  // Return thread handle
}

void vlThreadDelete(vl_thread thread){
    if(thread == 0 || thread == ((vl_uintptr_t)&mainThread))
        return;
    CloseHandle((HANDLE) thread);
}

vl_bool_t vlThreadJoin(vl_thread thread){
    if(thread == 0 || thread == ((vl_uintptr_t)&mainThread))
        return VL_FALSE;
    HANDLE handle = (HANDLE) thread;
    const DWORD status = WaitForSingleObject(handle, INFINITE);

    if(status == WAIT_OBJECT_0){
        return VL_TRUE;//Finished execution.
    }

    return VL_FALSE;//Timed out or failed to wait.
}

vl_bool_t vlThreadJoinTimeout(vl_thread thread, vl_uint_t milliseconds){
    if(thread == 0 || thread == ((vl_uintptr_t)&mainThread))
        return VL_FALSE;
    HANDLE handle = (HANDLE) thread;
    const DWORD status = WaitForSingleObject(handle, milliseconds);

    if(status == WAIT_OBJECT_0){
        return VL_TRUE;//Finished execution.
    }

    return VL_FALSE;//Timed out or failed to wait.
}
vl_thread vlThreadCurrent(){
    switch((vl_uintptr_t)currentThread){
        case 0:
            mainThread = GetCurrentThread();
            currentThread = &mainThread;
        default:
            return (vl_thread) currentThread;
    }
}

vl_bool_t vlThreadYield(){
    return SwitchToThread();
}

void vlThreadSleep(vl_ularge_t milliseconds){
    Sleep(milliseconds);
}

void vlThreadExit(){
    ExitThread(0);
}