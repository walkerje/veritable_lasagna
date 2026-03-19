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

#ifndef VL_SEMAPHORE_H
#define VL_SEMAPHORE_H

#include "vl_numtypes.h"

/**
 * \brief Opaque semaphore handle for synchronization
 *
 * Provides a lightweight counting semaphore implementation that can be used
 * for signaling between threads and managing access to a pool of resources.
 */
typedef struct vl_semaphore_* vl_semaphore;

/**
 * \brief Creates a new semaphore with the specified initial count.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_semaphore` handle and is responsible for calling
 * `vlSemaphoreDelete`.
 * - **Lifetime**: The semaphore remains valid until `vlSemaphoreDelete`.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: Returns `VL_SEMAPHORE_NULL` if the semaphore could not be created.
 * - **Error Conditions**: Returns `VL_SEMAPHORE_NULL` if heap allocation fails or platform-specific initialization
 * fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates semaphore metadata on the heap.
 * - **Return-value Semantics**: Returns an opaque handle to the new semaphore, or `NULL` on failure.
 *
 * \param initialCount Initial number of available resources
 * \return Semaphore handle or VL_SEMAPHORE_NULL on failure
 */
VL_API vl_semaphore vlSemaphoreNew(vl_uint_t initialCount);

/**
 * \brief Deletes a semaphore handle and releases associated resources.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the semaphore handle and its associated resources.
 * - **Lifetime**: The semaphore handle becomes invalid immediately after this call.
 * - **Thread Safety**: Safe to call from any thread, provided no other thread is using the semaphore.
 * - **Nullability**: Passing `NULL` results in undefined behavior (will attempt to destroy a null pointer).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing `NULL`. Deleting a semaphore that still has threads waiting on it. Double deletion.
 * - **Memory Allocation Expectations**: Deallocates heap-allocated resources.
 * - **Return-value Semantics**: None (void).
 *
 * \param sem Semaphore handle to delete
 */
VL_API void vlSemaphoreDelete(vl_semaphore sem);

/**
 * \brief Decrements (acquires) the semaphore count.
 *
 * If the count is zero, blocks until either:
 * - Another thread increments the count
 * - The specified timeout expires
 *
 * ## Contract
 * - **Ownership**: Unchanged. The calling thread gains logical "access" to a pooled resource.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (blocking).
 * - **Nullability**: `sem` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if the timeout expires before the semaphore is acquired.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the semaphore was successfully acquired, `VL_FALSE` on timeout or
 * error.
 *
 * \param sem Semaphore handle
 * \param timeoutMs Maximum time to wait in milliseconds. Note: 0 results in an immediate return if the semaphore is not
 * available. \return VL_TRUE if acquired, VL_FALSE if timeout occurred
 */
VL_API vl_bool_t vlSemaphoreWait(vl_semaphore sem, vl_uint_t timeoutMs);

/**
 * \brief Increments (releases) the semaphore count.
 *
 * If threads are waiting, one will be unblocked.
 *
 * ## Contract
 * - **Ownership**: The calling thread relinquishes logical "access" to a resource.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: `sem` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Incrementing the count beyond its platform-defined maximum. Passing `NULL`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param sem Semaphore handle
 */
VL_API void vlSemaphorePost(vl_semaphore sem);

/**
 * \brief Attempts to decrement (acquire) the semaphore without blocking.
 *
 * ## Contract
 * - **Ownership**: If successful, the calling thread gains logical access to a resource.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (non-blocking).
 * - **Nullability**: `sem` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if the count is zero.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the semaphore was successfully acquired, `VL_FALSE` otherwise.
 *
 * \param sem Semaphore handle
 * \return VL_TRUE if acquired, VL_FALSE if would block
 */
VL_API vl_bool_t vlSemaphoreTryWait(vl_semaphore sem);

/**
 * \brief Gets the current semaphore count.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe to call, but the returned value is a point-in-time snapshot.
 * - **Nullability**: `sem` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the current non-negative count of the semaphore.
 *
 * \param sem Semaphore handle
 * \return Current count value
 * \note This value may change immediately after reading
 */
VL_API vl_uint_t vlSemaphoreGetCount(vl_semaphore sem);

#endif // VL_SEMAPHORE_H
