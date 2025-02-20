#ifndef VL_MUTEX_H
#define VL_MUTEX_H

#include "vl_numtypes.h"

#ifndef VL_MUTEX_NULL
#define VL_MUTEX_NULL 0
#endif

/**
 * \brief Mutex handle.
 *
 * In Veritable Lasagna, Mutexes are defined as Read-Write locks.
 * They offer both "shared" and "exclusive" modes; the former is usable
 * for read operations, while the latter is suitable for write operations.
 *
 * If the mutex is locked in "shared" mode, other areas of code are permitted to read
 * whatever shared resource. In "exclusive" mode, the behavior is identical to that of
 * a typical mutex: anything trying to obtain the lock in either shared or exclusive
 * mode must wait for its availability.
 *
 * If trying to obtain an exclusive lock while other thread(s) have already obtained
 * shared locks, the exclusive lock must wait until all other shared locks are released.
 */
typedef vl_uintptr_t vl_mutex;

/**
 * \brief Creates a new instance of a mutex.
 * \note May return VL_MUTEX_NULL on failure.
 * \return mutex handle
 */
vl_mutex        vlMutexNew();

/**
 * \brief Deletes the specified mutex.
 * \warning Be certain the mutex is no longer obtained by the time this function is called.
 */
void            vlMutexDelete(vl_mutex mutex);

/**
 * \brief Obtain a shared lock on the specified mutex.
 *
 * Multiple threads may obtain a shared lock at any given time.
 * This is more suitable for concurrent read operations.
 *
 * \param mutex
 */
void            vlMutexObtainShared(vl_mutex mutex);

/**
 * \brief Attempts to obtain a shared lock from the specified mutex.
 * \note This function is non-blocking.
 * \param mutex
 * \return a boolean indicating whether or not the shared lock was obtained
 */
vl_bool_t       vlMutexTryObtainShared(vl_mutex mutex);

/**
 * \brief Releases a shared lock on the specified mutex.
 * \param mutex
 */
void            vlMutexReleaseShared(vl_mutex mutex);

/**
 * \brief Obtains an exclusive lock on the specified mutex.
 *
 * Only a single thread may obtain an exclusive lock at any given time.
 * This is more suitable for write operations.
 *
 * \param mutex
 */
void            vlMutexObtainExclusive(vl_mutex mutex);

/**
 * \brief Attempts to obtain an exclusive lock on the specified mutex.
 * \note This function is non-blocking.
 * \param mutex
 * \return a boolean indicating whether or not the lock was obtained.
 */
vl_bool_t       vlMutexTryObtainExclusive(vl_mutex mutex);

/**
 * \brief Releases an exclusive lock on the specified mutex.
 * \param mutex
 */
void            vlMutexReleaseExclusive(vl_mutex mutex);

#ifndef vlMutexObtain
/*
 * \copydoc vlMutexObtainExclusive
 */
#define vlMutexObtain(mutex) vlMutexObtainExclusive(mutex)
#endif

#ifndef vlMutexRelease
/**
 * \copydoc vlMutexReleaseExclusive
 */
#define vlMutexRelease(mutex) vlMutexReleaseExclusive(mutex)
#endif

#endif //VL_MUTEX_H
