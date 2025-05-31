#ifndef VL_SEMAPHORE_H
#define VL_SEMAPHORE_H

#include "vl_numtypes.h"

/**
 * \brief Opaque semaphore handle for synchronization
 *
 * Provides a lightweight counting semaphore implementation that can be used
 * for signaling between threads and managing access to a pool of resources.
 */
typedef vl_uintptr_t vl_semaphore;

/**
 * \brief Creates a new semaphore with the specified initial count
 * \param initialCount Initial number of available resources
 * \return Semaphore handle or VL_SEMAPHORE_NULL on failure
 */
VL_API vl_semaphore vlSemaphoreNew(vl_uint_t initialCount);

/**
 * \brief Deletes a semaphore handle and releases associated resources
 * \param sem Semaphore handle to delete
 */
VL_API void vlSemaphoreDelete(vl_semaphore sem);

/**
 * \brief Decrements (acquires) the semaphore count
 *
 * If the count is zero, blocks until either:
 * - Another thread increments the count
 * - The specified timeout expires
 *
 * \param sem Semaphore handle
 * \param timeoutMs Maximum time to wait in milliseconds (0 = infinite)
 * \return VL_TRUE if acquired, VL_FALSE if timeout occurred
 */
VL_API vl_bool_t vlSemaphoreWait(vl_semaphore sem, vl_uint_t timeoutMs);

/**
 * \brief Increments (releases) the semaphore count
 *
 * If threads are waiting, one will be unblocked.
 *
 * \param sem Semaphore handle
 */
VL_API void vlSemaphorePost(vl_semaphore sem);

/**
 * \brief Attempts to decrement (acquire) the semaphore without blocking
 * \param sem Semaphore handle
 * \return VL_TRUE if acquired, VL_FALSE if would block
 */
VL_API vl_bool_t vlSemaphoreTryWait(vl_semaphore sem);

/**
 * \brief Gets the current semaphore count
 * \param sem Semaphore handle
 * \return Current count value
 * \note This value may change immediately after reading
 */
VL_API vl_uint_t vlSemaphoreGetCount(vl_semaphore sem);

#endif // VL_SEMAPHORE_H