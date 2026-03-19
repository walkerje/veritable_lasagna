/**
 * ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
 * ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
 * ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
 *  ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
 *   ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
 * ====---: A Data Structure and Algorithms library for C11.  :---====
 *
 * Copyright 2026 Jesse Walker, released under the MIT license.
 * Git Repository:  https://github.com/walkerje/veritable_lasagna
 * \private
 */

#ifndef VL_MUTEX_H
#define VL_MUTEX_H

#include "vl_numtypes.h"

#ifndef VL_MUTEX_NULL
#define VL_MUTEX_NULL 0
#endif

typedef struct vl_mutex_* vl_mutex;

/**
 * \brief Creates a new instance of a mutex.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_mutex` handle and is responsible for calling `vlMutexDelete`.
 * - **Lifetime**: The mutex remains valid until `vlMutexDelete`.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: Returns `VL_MUTEX_NULL` if the mutex could not be created.
 * - **Error Conditions**: Returns `VL_MUTEX_NULL` if heap allocation fails or if platform-specific mutex initialization
 * fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates mutex metadata on the heap.
 * - **Return-value Semantics**: Returns an opaque handle to the new mutex, or `VL_MUTEX_NULL` on failure.
 *
 * \note May return VL_MUTEX_NULL on failure.
 * \return mutex handle
 */
VL_API vl_mutex vlMutexNew(void);

/**
 * \brief De-initializes and deletes the specified mutex.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the mutex handle and its associated resources.
 * - **Lifetime**: The mutex handle becomes invalid immediately after this call.
 * - **Thread Safety**: Safe to call from any thread, provided no other thread is using the mutex.
 * - **Nullability**: Safe to call with `VL_MUTEX_NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Deleting a mutex that is currently locked or has threads waiting on it. Double deletion.
 * - **Memory Allocation Expectations**: Deallocates heap-allocated resources.
 * - **Return-value Semantics**: None (void).
 *
 * \warning Be certain the mutex is no longer obtained by the time this function
 * is called.
 * \param mutex The mutex handle to delete.
 */
VL_API void vlMutexDelete(vl_mutex mutex);

/**
 * \brief Obtains an exclusive lock on the specified mutex.
 *
 * Only a single thread may obtain an exclusive lock at any given time.
 * This is more suitable for write operations.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The calling thread gains exclusive logical ownership of the lock.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (blocking).
 * - **Nullability**: Safe to call with `VL_MUTEX_NULL` (no-op).
 * - **Error Conditions**: Deadlock if the same thread attempts to obtain a non-recursive mutex it already holds.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param mutex The mutex handle.
 */
VL_API void vlMutexObtain(vl_mutex mutex);

/**
 * \brief Attempts to obtain an exclusive lock on the specified mutex without blocking.
 *
 * ## Contract
 * - **Ownership**: If successful, the calling thread gains exclusive logical ownership of the lock.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (non-blocking).
 * - **Nullability**: Safe to call with `VL_MUTEX_NULL` (returns `VL_FALSE`).
 * - **Error Conditions**: Returns `VL_FALSE` if the lock is already held by another thread.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the lock was successfully obtained, `VL_FALSE` otherwise.
 *
 * \note This function is non-blocking.
 * \param mutex The mutex handle.
 * \return a boolean indicating whether or not the lock was obtained.
 */
VL_API vl_bool_t vlMutexTryObtain(vl_mutex mutex);

/**
 * \brief Releases an exclusive lock on the specified mutex.
 *
 * ## Contract
 * - **Ownership**: The calling thread relinquishes logical ownership of the lock.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: Safe to call with `VL_MUTEX_NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Releasing a mutex not held by the calling thread.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param mutex The mutex handle.
 */
VL_API void vlMutexRelease(vl_mutex mutex);

#endif // VL_MUTEX_H
