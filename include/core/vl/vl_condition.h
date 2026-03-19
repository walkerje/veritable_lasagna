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

#ifndef VL_ASYNC_COND_H
#define VL_ASYNC_COND_H

#include "vl_mutex.h"
#include "vl_numtypes.h"

typedef struct vl_condition_* vl_condition;

/**
 * \brief Creates and initializes a new condition variable.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_condition` handle and is responsible for calling
 * `vlConditionDelete`.
 * - **Lifetime**: The condition variable remains valid until `vlConditionDelete`.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: Will attempt to return `NULL` if allocation fails, but the implementation may crash due to
 * unhandled `NULL` in `pthread_cond_init`.
 * - **Error Conditions**: Crashes if heap allocation fails.
 * - **Undefined Behavior**: Allocation failure.
 * - **Memory Allocation Expectations**: Allocates condition variable metadata on the heap.
 * - **Return-value Semantics**: Returns an opaque handle to the new condition variable.
 *
 * \return A vl_condition handle representing the newly created condition
 * variable. The caller is responsible for deallocating the associated resources
 *         when they are no longer needed.
 */
VL_API vl_condition vlConditionNew(void);

/**
 * \brief De-initializes and deletes a condition variable.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the condition variable handle and its associated resources.
 * - **Lifetime**: The condition variable handle becomes invalid immediately after this call.
 * - **Thread Safety**: Safe to call from any thread, provided no thread is waiting on it.
 * - **Nullability**: Passing `NULL` results in undefined behavior (will attempt to destroy a null pointer).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing `NULL`. Deleting a condition variable that still has threads waiting on it. Double
 * deletion.
 * - **Memory Allocation Expectations**: Deallocates heap-allocated resources.
 * - **Return-value Semantics**: None (void).
 *
 * \param cond The condition variable handle to delete.
 */
VL_API void vlConditionDelete(vl_condition cond);

/**
 * \brief Waits on a condition variable.
 *
 * This function causes the calling thread to block until the specified
 * condition variable has been signaled. It atomically releases the provided
 * mutex and suspends the thread execution until the condition is signaled.
 * After being signaled, the mutex is reacquired by the calling thread before
 * the function returns.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The calling thread must hold the `mutex`.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (blocking).
 * - **Nullability**: `cond` and `mutex` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Calling without holding the `mutex`. Spurious wakeups (caller should use a loop).
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \warning The behavior is undefined if the mutex is not locked by the calling
 * thread prior to calling this function.
 *
 * \param cond The condition variable on which to wait.
 * \param mutex The mutex associated with the condition variable. It must be
 * locked by the calling thread before calling this function.
 *
 * \note This function relies on the proper behavior of the underlying condition
 * and mutex mechanisms to ensure correct synchronization between threads.
 */
VL_API void vlConditionWait(vl_condition cond, vl_mutex mutex);

/**
 * \brief Waits on a condition variable with a timeout.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (blocking).
 * - **Nullability**: `cond` and `mutex` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if the timeout expires.
 * - **Undefined Behavior**: Calling without holding the `mutex`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the condition was signaled, `VL_FALSE` on timeout or error.
 *
 * \warning The behavior is undefined if the mutex is not locked by the calling
 * thread prior to calling this function.
 *
 * \param cond The condition variable to wait on.
 * \param mutex The mutex associated with the condition.
 * \param millis The timeout period in milliseconds.
 * \return A boolean value that indicates whether the condition was
 *         signaled within the timeout.
 *         Returns true if the condition was signaled, or false
 *         if the timeout period elapsed without the condition being signaled.
 */
VL_API vl_bool_t vlConditionWaitTimeout(vl_condition cond, vl_mutex mutex, vl_ularge_t millis);

/**
 * \brief Signals a condition variable, waking up at least one thread waiting on
 * it.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: `cond` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param cond The condition variable to signal.
 */
VL_API void vlConditionSignal(vl_condition cond);

/**
 * \brief Broadcasts a condition variable to wake up all waiting threads.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: `cond` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param cond The condition variable to broadcast. It must be a valid
 *                 vl_condition that has been properly initialized.
 */
VL_API void vlConditionBroadcast(vl_condition cond);

#endif // VL_ASYNC_COND_H
