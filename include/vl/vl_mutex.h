#ifndef VL_MUTEX_H
#define VL_MUTEX_H

#include "vl_numtypes.h"

#ifndef VL_MUTEX_NULL
#define VL_MUTEX_NULL 0
#endif

typedef struct vl_mutex_* vl_mutex;

/**
 * \brief Creates a new instance of a mutex.
 * \note May return VL_MUTEX_NULL on failure.
 * \return mutex handle
 */
VL_API vl_mutex vlMutexNew();

/**
 * \brief Deletes the specified mutex.
 * \warning Be certain the mutex is no longer obtained by the time this function is called.
 */
VL_API void vlMutexDelete(vl_mutex mutex);

/**
 * \brief Obtains an exclusive lock on the specified mutex.
 *
 * Only a single thread may obtain an exclusive lock at any given time.
 * This is more suitable for write operations.
 *
 * \param mutex
 */
VL_API void vlMutexObtain(vl_mutex mutex);

/**
 * \brief Attempts to obtain an exclusive lock on the specified mutex.
 * \note This function is non-blocking.
 * \param mutex
 * \return a boolean indicating whether or not the lock was obtained.
 */
VL_API vl_bool_t vlMutexTryObtain(vl_mutex mutex);

/**
 * \brief Releases an exclusive lock on the specified mutex.
 * \param mutex
 */
VL_API void vlMutexRelease(vl_mutex mutex);

#endif //VL_MUTEX_H
