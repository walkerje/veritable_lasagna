#ifndef VL_SRWLOCK_H
#define VL_SRWLOCK_H

#include "vl_numtypes.h"

#ifndef VL_SRWLOCK_NULL
#define VL_SRWLOCK_NULL 0
#endif

/**
 * \brief Slim Read-Write Lock handle for thread synchronization.
 *
 * SRWLocks (Slim Read-Write Locks) provide efficient synchronization for scenarios
 * where a resource can be accessed concurrently by multiple readers but requires
 * exclusive access by writers. They offer two locking modes:
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
 * - When a thread requests an exclusive lock while shared locks are held, it will
 *   block until all shared locks are released.
 * - When a thread requests a shared lock while an exclusive lock is held, it will
 *   block until the exclusive lock is released.
 * - Non-blocking variants (Try* functions) allow testing lock availability without waiting.
 *
 * \note Improper use of read-write locks can lead to writer starvation or deadlocks.
 *       Always release locks in the reverse order they were acquired.
 */
typedef vl_uintptr_t vl_srwlock;

/**
 * \brief Creates a new instance of a lock.
 * \note May return VL_SRWLOCK_NULL on failure.
 * \return lock handle
 */
VL_API vl_srwlock vlSRWLockNew();

/**
 * \brief Deletes the specified lock.
 * \warning Be certain the lock is no longer obtained by the time this function is called.
 */
VL_API void vlSRWLockDelete(vl_srwlock lock);

/**
 * \brief Obtain a shared lock on the specified lock.
 *
 * Multiple threads may obtain a shared lock at any given time.
 * This is more suitable for concurrent read operations.
 *
 * \param lock
 */
VL_API void vlSRWLockObtainShared(vl_srwlock lock);

/**
 * \brief Attempts to obtain a shared lock from the specified lock.
 * \note This function is non-blocking.
 * \param lock
 * \return a boolean indicating whether or not the shared lock was obtained
 */
VL_API vl_bool_t vlSRWLockTryObtainShared(vl_srwlock lock);

/**
 * \brief Releases a shared lock on the specified lock.
 * \param lock
 */
VL_API void vlSRWLockReleaseShared(vl_srwlock lock);

/**
 * \brief Obtains an exclusive lock on the specified lock.
 *
 * Only a single thread may obtain an exclusive lock at any given time.
 * This is more suitable for write operations.
 *
 * \param lock
 */
VL_API void vlSRWLockObtainExclusive(vl_srwlock lock);

/**
 * \brief Attempts to obtain an exclusive lock on the specified lock.
 * \note This function is non-blocking.
 * \param lock
 * \return a boolean indicating whether or not the lock was obtained.
 */
VL_API vl_bool_t vlSRWLockTryObtainExclusive(vl_srwlock lock);

/**
 * \brief Releases an exclusive lock on the specified lock.
 * \param lock
 */
VL_API void vlSRWLockReleaseExclusive(vl_srwlock lock);

#endif //VL_SRWLOCK_H
