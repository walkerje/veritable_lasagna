#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <process.h>
#include <stdlib.h> /* malloc/free */
#include <windows.h>

/* ----------------------------------------------------------------------------
 * Internal representation matches public header:
 *   typedef struct vl_thread_* vl_thread;
 * ----------------------------------------------------------------------------
 */

struct vl_thread_
{
    HANDLE handle; /* real handle for VL-created threads; pseudo-handle for
                      foreign threads */
    DWORD tid; /* thread id (debug/identity) */
    vl_bool_t is_main;
    vl_bool_t is_foreign; /* not created by vlThreadNew; handle is pseudo-handle */
};

static struct vl_thread_ g_mainThreadMeta;
static vl_bool_t g_mainThreadInitialized = VL_FALSE;

/* For threads created by VL, TLS points at the heap meta.
   For "foreign" threads, TLS points at a thread-local meta struct (no heap, no
   leaks). */
VL_THREAD_LOCAL vl_thread currentThread = NULL;
VL_THREAD_LOCAL struct vl_thread_ tlsForeignMeta;

/* ----------------------------------------------------------------------------
 * Bootstrap
 * ----------------------------------------------------------------------------
 */

typedef struct
{
    vl_thread meta;
    vl_thread_proc threadProc;
    void* userArg;
} vl_thread_args;

static unsigned __stdcall vl_ThreadBootstrap(void* arg)
{
    vl_thread meta;
    vl_thread_proc proc;
    void* userArg;

    {
        vl_thread_args* threadArgs = (vl_thread_args*)arg;
        meta = threadArgs->meta;
        proc = threadArgs->threadProc;
        userArg = threadArgs->userArg;
        free(threadArgs);
    }

    currentThread = meta;
    proc(userArg);
    currentThread = NULL;

    _endthreadex(0);
    return 0;
}

/* ----------------------------------------------------------------------------
 * Public API
 * ----------------------------------------------------------------------------
 */

vl_thread vlThreadNew(vl_thread_proc threadProc, void* userArg)
{
    vlThreadCurrent(); /* ensure main meta is set */

    vl_thread meta = (vl_thread)malloc(sizeof(struct vl_thread_));
    if (!meta)
        return VL_THREAD_NULL;

    meta->handle = NULL;
    meta->tid = 0;
    meta->is_main = VL_FALSE;
    meta->is_foreign = VL_FALSE;

    vl_thread_args* args = (vl_thread_args*)malloc(sizeof(vl_thread_args));
    if (!args)
    {
        free(meta);
        return VL_THREAD_NULL;
    }

    args->meta = meta;
    args->threadProc = threadProc;
    args->userArg = userArg;

    unsigned threadID = 0;
    uintptr_t h = _beginthreadex(NULL, 0, vl_ThreadBootstrap, args, 0, &threadID);
    if (h == 0)
    {
        free(args);
        free(meta);
        return VL_THREAD_NULL;
    }

    meta->handle = (HANDLE)h; /* real, closable handle */
    meta->tid = (DWORD)threadID;

    return meta;
}

void vlThreadDelete(vl_thread thread)
{
    if (thread == VL_THREAD_NULL)
        return;

    /* Never delete the static main thread object. */
    if (thread->is_main)
        return;

    /* "Foreign" thread meta is thread-local; not heap-owned. */
    if (thread->is_foreign)
        return;

    if (thread->handle)
    {
        CloseHandle(thread->handle);
        thread->handle = NULL;
    }
    free(thread);
}

vl_bool_t vlThreadJoin(vl_thread thread)
{
    if (thread == VL_THREAD_NULL)
        return VL_FALSE;
    if (thread->is_main)
        return VL_FALSE;

    /* Only join VL-created threads (real handles). */
    if (thread->is_foreign || thread->handle == NULL)
        return VL_FALSE;

    return (WaitForSingleObject(thread->handle, INFINITE) == WAIT_OBJECT_0) ? VL_TRUE : VL_FALSE;
}

vl_bool_t vlThreadJoinTimeout(vl_thread thread, vl_uint_t milliseconds)
{
    if (thread == VL_THREAD_NULL)
        return VL_FALSE;
    if (thread->is_main)
        return VL_FALSE;

    if (thread->is_foreign || thread->handle == NULL)
        return VL_FALSE;

    return (WaitForSingleObject(thread->handle, (DWORD)milliseconds) == WAIT_OBJECT_0) ? VL_TRUE : VL_FALSE;
}

vl_thread vlThreadCurrent()
{
    if (currentThread != NULL)
    {
        return currentThread;
    }

    /* First call on this OS thread: decide if this is "main" or "foreign". */
    const DWORD tid = GetCurrentThreadId();

    if (!g_mainThreadInitialized)
    {
        /* First ever call: designate this as the main thread. */
        g_mainThreadMeta.handle = GetCurrentThread(); /* pseudo-handle is fine for identity */
        g_mainThreadMeta.tid = tid;
        g_mainThreadMeta.is_main = VL_TRUE;
        g_mainThreadMeta.is_foreign = VL_TRUE; /* not heap, not joinable via handle */
        g_mainThreadInitialized = VL_TRUE;

        currentThread = &g_mainThreadMeta;
        return currentThread;
    }

    /* Not main: represent as "foreign" thread using thread-local storage. */
    tlsForeignMeta.handle = GetCurrentThread(); /* pseudo-handle; must not be closed */
    tlsForeignMeta.tid = tid;
    tlsForeignMeta.is_main = VL_FALSE;
    tlsForeignMeta.is_foreign = VL_TRUE;

    currentThread = &tlsForeignMeta;
    return currentThread;
}

vl_bool_t vlThreadYield() { return SwitchToThread() ? VL_TRUE : VL_FALSE; }

void vlThreadSleep(vl_ularge_t milliseconds) { Sleep((DWORD)milliseconds); }

void vlThreadSleepNano(vl_ularge_t nanoseconds)
{
    const vl_ularge_t busy_threshold = 10000; // 10,000 ns = 10 µs
    if (nanoseconds <= busy_threshold)
    {
        LARGE_INTEGER freq, start, now;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        LONGLONG wait_ticks = (nanoseconds * freq.QuadPart) / 1000000000LL;
        do
        {
            QueryPerformanceCounter(&now);
        }
        while ((now.QuadPart - start.QuadPart) < wait_ticks);
    }
    else
    {
        HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
        LARGE_INTEGER li;
        li.QuadPart = -(LONGLONG)(nanoseconds / 100); /* 100ns intervals, relative */

        (void)SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
        (void)WaitForSingleObject(timer, INFINITE);
        CloseHandle(timer);
    }
}

void vlThreadExit()
{
    /* Ensure TLS/meta is set so we can decide the safest exit primitive. */
    vl_thread self = vlThreadCurrent();

    /* Threads created by _beginthreadex should exit via _endthreadex to keep the
       CRT happy. For "foreign" threads (including main), ExitThread is the least
       surprising choice. */
    if (self && self->is_foreign)
    {
        ExitThread(0);
    }

    _endthreadex(0);
}
