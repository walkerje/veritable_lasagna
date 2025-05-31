#ifndef VL_ASYNC_COND_H
#define VL_ASYNC_COND_H

#include "vl_numtypes.h"
#include "vl_mutex.h"

typedef vl_uintptr_t vl_condition;

/**
 * \brief Creates and initializes a new condition variable.
 *
 * This function allocates memory for a condition variable, initializes it,
 * and returns a pointer wrapped as a vl_condition.
 *
 * \return A vl_condition handle representing the newly created condition variable.
 *         The caller is responsible for deallocating the associated resources
 *         when they are no longer needed.
 */
VL_API vl_condition    vlConditionNew();

/**
 * Deletes a condition variable.
 *
 * This function destroys a condition variable and releases the memory
 * occupied by it. It is necessary to ensure that no other thread is
 * waiting on the condition variable before invoking this function to
 * avoid undefined behavior.
 *
 * \param cond The condition variable to delete.
 */
VL_API void            vlConditionDelete(vl_condition);

/**
 * \brief Waits on a condition variable.
 *
 * This function causes the calling thread to block until the specified condition
 * variable has been signaled. It atomically releases the provided mutex and suspends
 * the thread execution until the condition is signaled. After being signaled, the mutex
 * is reacquired by the calling thread before the function returns.
 *
 * \warning The behavior is undefined if the mutex is not locked by the calling thread
 *          prior to calling this function.
 *
 * \param cond The condition variable on which to wait.
 * \param mutex The mutex associated with the condition variable. It must be locked
 *              by the calling thread before calling this function.
 *
 * \note This function relies on the proper behavior of the underlying condition and
 *       mutex mechanisms to ensure correct synchronization between threads.
 */
VL_API void            vlConditionWait(vl_condition cond, vl_mutex mutex);

/**
 * \brief Waits on a condition variable with a timeout.
 *
 * This function uses the associated mutex to wait for a specified condition
 * to be signaled or until the timeout period elapses. It allows for 
 * thread synchronization with a limited wait time.
 *
 * \warning The behavior is undefined if the mutex is not locked by the calling thread
 *          prior to calling this function.
 *
 * \param cond The condition variable to wait on.
 * \param mutex The mutex associated with the condition.
 * \param millis The timeout period in milliseconds.
 * \return A boolean value that indicates whether the condition was 
 *         signaled within the timeout. 
 *         Returns true if the condition was signaled, or false 
 *         if the timeout period elapsed without the condition being signaled.
 */
VL_API vl_bool_t       vlConditionWaitTimeout(vl_condition cond, vl_mutex mutex, vl_ularge_t millis);

/**
 * \brief Signals a condition variable, waking up at least one thread waiting on it.
 * 
 * This function sends a signal to the specified condition variable `cond`,
 * causing at least one thread that is blocked waiting for the condition to be
 * awakened. If no threads are currently waiting on the condition, the signal
 * is ignored.
 * 
 * \param cond The condition variable to signal.
 *
 *
 * This function does not ensure that all waiting threads are unblocked, but
 * instead wakes up a single waiting thread (if any).
 */
VL_API void            vlConditionSignal(vl_condition cond);

/**
 * \brief Broadcasts a condition variable to wake up all waiting threads.
 *
 * This function signals all threads that are waiting on the specified
 * condition variable, causing them to resume execution.
 *
 * \param[in] cond The condition variable to broadcast. It must be a valid 
 *                 vl_condition that has been properly initialized.
 *
 * \note The behavior is undefined if the condition variable is not valid
 *       or was not properly initialized before calling this function.
 *       This function is typically used in multi-threaded programs 
 *       to synchronize threads waiting on a condition.
 */
VL_API void            vlConditionBroadcast(vl_condition cond);

#endif //VL_ASYNC_COND_H
