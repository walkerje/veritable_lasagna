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

#ifndef VL_THREAD_H
#define VL_THREAD_H

#include "vl_numtypes.h"

#ifndef VL_THREAD_LOCAL
#define VL_THREAD_LOCAL _Thread_local
#endif

#ifndef VL_THREAD_NULL
#define VL_THREAD_NULL 0
#endif

typedef struct vl_thread_* vl_thread;

typedef void (*vl_thread_proc)(void* usr);

/**
 * \brief Creates and begins executing a new thread.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_thread` handle and is responsible for calling `vlThreadDelete`
 * after the thread has been joined.
 * - **Lifetime**: The thread handle remains valid until `vlThreadDelete`. The thread execution is independent.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: Returns `VL_THREAD_NULL` if the thread could not be created.
 * - **Error Conditions**: Returns `VL_THREAD_NULL` if heap allocation for metadata fails or if the platform thread
 * creation call fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates metadata for the thread on the heap.
 * - **Return-value Semantics**: Returns an opaque handle to the new thread, or `VL_THREAD_NULL` on failure.
 *
 * \note May return VL_THREAD_NULL on failure.
 * \param proc the function to execute in the new thread.
 * \param userArg argument pointer passed to the thread procedure.
 * \return thread handle
 */
VL_API vl_thread vlThreadNew(vl_thread_proc proc, void* userArg);

/**
 * \brief Deletes the specified thread handle and its metadata.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the thread handle and its associated metadata.
 * - **Lifetime**: The thread handle becomes invalid immediately after this call.
 * - **Thread Safety**: Safe to call from any thread.
 * - **Nullability**: Safe to call with `VL_THREAD_NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Deleting a thread handle that has not been joined or has already been deleted.
 * - **Memory Allocation Expectations**: Deallocates heap-allocated metadata.
 * - **Return-value Semantics**: None (void).
 *
 * \warning This will result in undefined behavior if the thread has not been
 * joined prior to calling this function.
 * \sa vlThreadJoin
 * \sa vlThreadJoinTimeout
 *
 * \param thread which thread handle to delete
 */
VL_API void vlThreadDelete(vl_thread thread);

/**
 * \brief Joins the specified thread, halting the calling thread until the
 * specified thread exits.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe to call from any thread, but usually called from the owner of the handle.
 * - **Nullability**: Returns `VL_FALSE` if `thread` is `VL_THREAD_NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if the thread is the main thread or if the platform join call fails.
 * - **Undefined Behavior**: Joining the same thread from multiple threads simultaneously.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the thread was successfully joined, `VL_FALSE` otherwise.
 *
 * \note If the specified thread happens to be the main thread, this will return
 * false.
 * \param thread The thread to join.
 * \return true on success, false on error.
 */
VL_API vl_bool_t vlThreadJoin(vl_thread thread);

/**
 * \brief Attempts to join the specified thread until a maximum amount of time
 * has passed.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe to call.
 * - **Nullability**: Returns `VL_FALSE` if `thread` is `VL_THREAD_NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` on timeout, if the thread is the main thread, or on platform error.
 * - **Undefined Behavior**: Same as `vlThreadJoin`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the thread exited and was joined within the timeout, `VL_FALSE`
 * otherwise.
 *
 * \note Will always return false if trying to join the main thread.
 * \param thread The thread to join.
 * \param milliseconds Maximum time to wait in milliseconds.
 * \return true on success, false on timeout or error.
 */
VL_API vl_bool_t vlThreadJoinTimeout(vl_thread thread, vl_uint_t milliseconds);

/**
 * \brief Gets the current thread.
 *
 * ## Contract
 * - **Ownership**: Returns a handle to the current thread's metadata. The caller does not own this handle and must not
 * call `vlThreadDelete` on it.
 * - **Lifetime**: The handle is valid as long as the current thread is alive.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: Never returns `VL_THREAD_NULL` for a valid thread.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: May initialize internal thread-local storage on the first call in a thread.
 * - **Return-value Semantics**: Returns the handle to the current thread.
 *
 * \return vl_thread representing the current thread.
 */
VL_API vl_thread vlThreadCurrent(void);

/**
 * \brief Yields the execution of the current thread, allowing another thread to
 * take the remainder of its timeslice.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: N/A.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: N/A.
 * - **Error Conditions**: Returns `VL_FALSE` if the platform yield operation fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the yield was successful, `VL_FALSE` otherwise.
 *
 * \return a boolean indicating if the yield operation occurred. May return
 * false if there is no other thread to yield to.
 */
VL_API vl_bool_t vlThreadYield(void);

/**
 * \brief Sleeps the current thread a specified total number of milliseconds.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: N/A.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: N/A.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None.
 *
 * \param milliseconds
 */
VL_API void vlThreadSleep(vl_ularge_t milliseconds);

/**
 * \brief Sleeps the current thread a specified total number of nanoseconds.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: N/A.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: N/A.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None.
 *
 * \param nanoseconds
 */
VL_API void vlThreadSleepNano(vl_ularge_t nanoseconds);

/**
 * \brief Exits the calling thread.
 *
 * ## Contract
 * - **Ownership**: N/A.
 * - **Lifetime**: Terminating the current thread.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: N/A.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: This function does not return to the caller.
 *
 * \note Does not return to the caller.
 */
VL_API void vlThreadExit(void);

#endif // VL_THREAD_H
