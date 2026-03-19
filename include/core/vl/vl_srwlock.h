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

#ifndef VL_SRWLOCK_H
#define VL_SRWLOCK_H

#include "vl_numtypes.h"

#ifndef VL_SRWLOCK_NULL
#define VL_SRWLOCK_NULL 0
#endif

/**
 * \brief Slim Read-Write Lock handle for thread synchronization.
 *
 * SRWLocks (Slim Read-Write Locks) provide efficient synchronization for
 * scenarios where a resource can be accessed concurrently by multiple readers
 * but requires exclusive access by writers. They offer two locking modes:
 *
 * - <b>Shared mode (for readers):</b> Multiple threads can simultaneously hold
 *   shared locks, enabling concurrent read access to the protected resource.
 *   Use \c vlSRWLockObtainShared() or \c vlSRWLockTryObtainShared().
 *
 * - <b>Exclusive mode (for writers):</b> Only one thread can hold an exclusive
 *   lock, preventing all other threads from accessing the resource. Use
 *   \c vlSRWLockObtainExclusive() or \c vlSRWLockTryObtainExclusive().
 *
 * \par Thread Behavior:
 * - When a thread requests an exclusive lock while shared locks are held, it
 * will block until all shared locks are released.
 * - When a thread requests a shared lock while an exclusive lock is held, it
 * will block until the exclusive lock is released.
 * - Non-blocking variants (Try* functions) allow testing lock availability
 * without waiting.
 *
 * \note Improper use of read-write locks can lead to writer starvation or
 * deadlocks. Always release locks in the reverse order they were acquired.
 */
typedef struct vl_srwlock_* vl_srwlock;

/**
 * \brief Creates a new instance of a lock.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_srwlock` handle and is responsible for calling `vlSRWLockDelete`.
 * - **Lifetime**: The lock remains valid until `vlSRWLockDelete`.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: Returns `VL_SRWLOCK_NULL` if the lock could not be created.
 * - **Error Conditions**: Returns `VL_SRWLOCK_NULL` if heap allocation fails or platform-specific initialization fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates lock metadata on the heap.
 * - **Return-value Semantics**: Returns an opaque handle to the new SRW lock, or `NULL` on failure.
 *
 * \note May return VL_SRWLOCK_NULL on failure.
 * \return lock handle
 */
VL_API vl_srwlock vlSRWLockNew(void);

/**
 * \brief Deletes the specified lock.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the lock handle and its associated resources.
 * - **Lifetime**: The lock handle becomes invalid immediately after this call.
 * - **Thread Safety**: Safe to call from any thread, provided no other thread is using the lock.
 * - **Nullability**: Safe to call with `NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Deleting a lock that is currently held or has threads waiting on it. Double deletion.
 * - **Memory Allocation Expectations**: Deallocates heap-allocated resources.
 * - **Return-value Semantics**: None (void).
 *
 * \warning Be certain the lock is no longer obtained by the time this function
 * is called.
 * \param lock The lock handle to delete.
 */
VL_API void vlSRWLockDelete(vl_srwlock lock);

/**
 * \brief Obtain a shared lock on the specified lock.
 *
 * Multiple threads may obtain a shared lock at any given time.
 * This is more suitable for concurrent read operations.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The calling thread gains shared logical ownership of the lock.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (blocking).
 * - **Nullability**: Safe to call with `VL_SRWLOCK_NULL` (no-op).
 * - **Error Conditions**: Potential deadlock if not used carefully.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param lock The lock handle.
 */
VL_API void vlSRWLockObtainShared(vl_srwlock lock);

/**
 * \brief Attempts to obtain a shared lock from the specified lock.
 *
 * ## Contract
 * - **Ownership**: If successful, the calling thread gains shared logical ownership of the lock.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (non-blocking).
 * - **Nullability**: Safe to call with `VL_SRWLOCK_NULL` (returns `VL_FALSE`).
 * - **Error Conditions**: Returns `VL_FALSE` if an exclusive lock is already held by another thread.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the shared lock was successfully obtained, `VL_FALSE` otherwise.
 *
 * \note This function is non-blocking.
 * \param lock The lock handle.
 * \return a boolean indicating whether or not the shared lock was obtained
 */
VL_API vl_bool_t vlSRWLockTryObtainShared(vl_srwlock lock);

/**
 * \brief Releases a shared lock on the specified lock.
 *
 * ## Contract
 * - **Ownership**: The calling thread relinquishes shared logical ownership of the lock.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: Safe to call with `VL_SRWLOCK_NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Releasing a shared lock not held by the calling thread.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param lock The lock handle.
 */
VL_API void vlSRWLockReleaseShared(vl_srwlock lock);

/**
 * \brief Obtains an exclusive lock on the specified lock.
 *
 * Only a single thread may obtain an exclusive lock at any given time.
 * This is more suitable for write operations.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The calling thread gains exclusive logical ownership of the lock.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (blocking).
 * - **Nullability**: Safe to call with `VL_SRWLOCK_NULL` (no-op).
 * - **Error Conditions**: Potential deadlock if not used carefully.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param lock The lock handle.
 */
VL_API void vlSRWLockObtainExclusive(vl_srwlock lock);

/**
 * \brief Attempts to obtain an exclusive lock on the specified lock.
 *
 * ## Contract
 * - **Ownership**: If successful, the calling thread gains exclusive logical ownership of the lock.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (non-blocking).
 * - **Nullability**: Safe to call with `VL_SRWLOCK_NULL` (returns `VL_FALSE`).
 * - **Error Conditions**: Returns `VL_FALSE` if any lock (shared or exclusive) is already held by other threads.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the exclusive lock was successfully obtained, `VL_FALSE`
 * otherwise.
 *
 * \note This function is non-blocking.
 * \param lock The lock handle.
 * \return a boolean indicating whether or not the lock was obtained.
 */
VL_API vl_bool_t vlSRWLockTryObtainExclusive(vl_srwlock lock);

/**
 * \brief Releases an exclusive lock on the specified lock.
 *
 * ## Contract
 * - **Ownership**: The calling thread relinquishes exclusive logical ownership of the lock.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: Safe to call with `VL_SRWLOCK_NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Releasing an exclusive lock not held by the calling thread.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param lock The lock handle.
 */
VL_API void vlSRWLockReleaseExclusive(vl_srwlock lock);

#endif // VL_SRWLOCK_H
