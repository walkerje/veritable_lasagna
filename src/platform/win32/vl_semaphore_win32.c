#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

vl_semaphore vlSemaphoreNew(vl_uint_t initialCount) {
    HANDLE sem = CreateSemaphore(
            NULL,                   // Default security attributes
            initialCount,           // Initial count
            0x7FFFFFFF,             // Maximum count (max LONG value)
            NULL                    // Unnamed semaphore
    );

    if (sem == NULL) {
        return 0;
    }

    return (vl_semaphore)sem;
}

void vlSemaphoreDelete(vl_semaphore sem) {
    CloseHandle((HANDLE)sem);
}

vl_bool_t vlSemaphoreWait(vl_semaphore sem, vl_uint_t timeoutMs) {
    DWORD result = WaitForSingleObject(
            (HANDLE)sem,
            timeoutMs == 0 ? INFINITE : timeoutMs
    );

    return result == WAIT_OBJECT_0;
}

void vlSemaphorePost(vl_semaphore sem) {
    ReleaseSemaphore((HANDLE)sem, 1, NULL);
}

vl_bool_t vlSemaphoreTryWait(vl_semaphore sem) {
    DWORD result = WaitForSingleObject((HANDLE)sem, 0);
    return result == WAIT_OBJECT_0;
}

vl_uint_t vlSemaphoreGetCount(vl_semaphore sem) {
    LONG previousCount;
    // Query the count by attempting to reduce by 0
    ReleaseSemaphore((HANDLE)sem, 0, &previousCount);
    return (vl_uint_t)previousCount;
}
