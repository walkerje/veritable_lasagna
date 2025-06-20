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

typedef void (*vl_thread_proc)(void *usr);

/**
 * \brief Creates and begins executing a new thread.
 *
 * \note May return VL_THREAD_NULL on failure.
 * \param userArg argument pointer
 * \return thread handle
 */
VL_API vl_thread vlThreadNew(vl_thread_proc, void *userArg);

/**
 * \brief Deletes the specified thread.
 * \warning This will result in undefined behavior if the thread has not been joined prior to calling this function.
 * \sa vlThreadJoin
 * \sa vlThreadJoinTimeout
 *
 * \param thread which thread to delete
 */
VL_API void vlThreadDelete(vl_thread thread);

/**
 * \brief Joins the specified thread, halting the calling thread until the specified thread exits.
 *
 * \note If the specified thread happens to be the main thread, this will return false.
 * \param thread
 * \return true on success, false on error.
 */
VL_API vl_bool_t vlThreadJoin(vl_thread thread);

/**
 * \brief Attempts to join the specified thread until a maximum amount of time has passed.
 * \note Will always return false is trying to join the main thread.
 * \param thread
 * \param milliseconds
 * \return true on success, false on timeout or error.
 */
VL_API vl_bool_t vlThreadJoinTimeout(vl_thread thread, vl_uint_t milliseconds);

/**
 * \brief Gets the current thread.
 * \return vl_thread representing the current thread.
 */
VL_API vl_thread vlThreadCurrent();

/**
 * \brief Yields the execution of the current thread, allowing another thread to take the remainder of its timeslice.
 * \return a boolean indicating if the yield operation occurred. May return false if there is no other thread to yield to.
 */
VL_API vl_bool_t vlThreadYield();

/**
 * \brief Sleeps the current thread a specified total number of milliseconds.
 * \param milliseconds
 */
VL_API void vlThreadSleep(vl_ularge_t milliseconds);

/**
 * \brief Sleeps the current thread a specified total number of nanoseconds.
 * \param nanoseconds
 */
VL_API void vlThreadSleepNano(vl_ularge_t nanoseconds);

/**
 * \brief Exits the calling thread.
 * \note Does not return to the caller.
 */
VL_API void vlThreadExit();

#endif //VL_THREAD_H
