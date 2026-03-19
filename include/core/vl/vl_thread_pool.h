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

#ifndef VL_THREAD_POOL_H
#define VL_THREAD_POOL_H

#include "vl_async_queue.h"
#include "vl_atomic.h"
#include "vl_condition.h"
#include "vl_mutex.h"
#include "vl_numtypes.h"
#include "vl_semaphore.h"
#include "vl_thread.h"

/**
 * \brief Priority-aware work-stealing thread pool for scalable task scheduling.
 *
 * This structure implements a multi-tier work-stealing scheduler with three
 * priority levels (HIGH, MEDIUM, LOW). Workers prefer high-priority work but
 * will steal from lower tiers when their current tier is empty, ensuring:
 *
 * - **Work starvation prevention**: Low-priority tasks eventually execute
 * - **Priority respect**: High-priority work gets preferential execution
 * - **Load balancing**: Idle workers steal across priority tiers
 * - **Lock-free enqueueing**: All priority levels use atomic queues
 * - **Efficient signaling**: Per-tier semaphores reduce spurious wakeups
 *
 * ## Architecture
 *
 * Each priority tier (HIGH, MEDIUM, LOW) has its own:
 * - Atomic MPMC work queue (vl_async_queue)
 * - Counting semaphore (vl_semaphore)
 *
 * Worker threads employ the following strategy:
 * 1. Try to pop from HIGH priority queue
 * 2. If empty, try MEDIUM priority queue
 * 3. If empty, try LOW priority queue
 * 4. If all empty, wait on semaphore (blocking)
 * 5. Upon wakeup, repeat from step 1
 *
 * ## Typical Usage
 *
 * \code
 * // Create pool with 4 worker threads
 * vl_thread_pool *pool = vlThreadPoolNew(4);
 *
 * // Enqueue high-priority work (e.g., user input, time-critical)
 * vl_thread_pool_task task = {
 *     .proc = handle_user_input,
 *     .user_data = event_ptr
 * };
 * vlThreadPoolEnqueuePriority(pool, VL_THREAD_POOL_PRIORITY_HIGH, &task);
 *
 * // Enqueue medium-priority work (e.g., rendering, IO)
 * vlThreadPoolEnqueuePriority(pool, VL_THREAD_POOL_PRIORITY_MEDIUM, &task);
 *
 * // Enqueue low-priority work (e.g., cleanup, caching)
 * vlThreadPoolEnqueuePriority(pool, VL_THREAD_POOL_PRIORITY_LOW, &task);
 *
 * // Wait for all queues to drain
 * vlThreadPoolWait(pool, 0);
 *
 * vlThreadPoolDelete(pool);
 * \endcode
 *
 * ## Thread Safety
 *
 * - `vlThreadPoolEnqueuePriority()` is fully thread-safe for any priority
 * - `vlThreadPoolEnqueueBatchPriority()` is fully thread-safe
 * - `vlThreadPoolWait()` is thread-safe; multiple threads can wait
 * simultaneously
 * - `vlThreadPoolGetStats()` is thread-safe; returns atomic snapshot
 * - `vlThreadPoolShutdown()` and `vlThreadPoolDelete()` must be called after
 *   all external enqueueing has stopped and waiters have been signaled
 *
 * ## Work Item Semantics
 *
 * Each enqueued task is executed exactly once. Execution order respects
 * priority:
 * - HIGH tasks execute before MEDIUM when both are available
 * - MEDIUM tasks execute before LOW when both are available
 * - Within a priority tier, tasks execute FIFO
 * - Multiple workers execute concurrently across all tiers
 *
 * Tasks themselves are responsible for any necessary synchronization if
 * accessing shared state.
 *
 * ## Starvation Prevention
 *
 * While the scheduler respects priorities, it prevents LOW-priority starvation
 * through work-stealing:
 * - When a worker's preferred tier is empty, it steals from lower tiers
 * - This ensures LOW-priority work eventually gets CPU time
 * - Adjust priority distribution based on your workload
 *
 * ## Shutdown Behavior
 *
 * Call `vlThreadPoolShutdown()` to initiate graceful shutdown:
 * - No new tasks are accepted
 * - In-flight tasks complete across all priority tiers
 * - Workers exit naturally
 * - Call `vlThreadPoolDelete()` to reclaim resources
 *
 * \note Work items are copied into queues, so originals can be stack-allocated.
 *
 * \see vl_async_queue, vl_semaphore, vl_thread, vl_thread_pool_priority
 */

/**
 * \brief Priority tiers for work-stealing scheduler.
 */
typedef enum
{
    VL_THREAD_POOL_PRIORITY_HIGH = 0, /**< Highest priority; checked first */
    VL_THREAD_POOL_PRIORITY_MEDIUM = 1, /**< Medium priority; fallback tier */
    VL_THREAD_POOL_PRIORITY_LOW = 2, /**< Lowest priority; work-steal tier */
    VL_THREAD_POOL_PRIORITY_COUNT = 3 /**< Total number of priority levels */
} vl_thread_pool_priority;

typedef enum
{
    VL_THREAD_POOL_RUNNING = 0,
    VL_THREAD_POOL_SHUTTING_DOWN = 1,
    VL_THREAD_POOL_SHUT_DOWN = 2
} vl_thread_pool_state;

/**
 * \brief Function signature for worker thread task procedures.
 *
 * \param user_data Arbitrary pointer passed during task enqueueing.
 */
typedef void (*vl_thread_pool_task_proc)(void* user_data);

/**
 * \brief A single work item for the thread pool.
 *
 * \field proc The function to execute
 * \field user_data Context pointer passed to the function
 */
typedef struct
{
    vl_thread_pool_task_proc proc;
    void* user_data;
} vl_thread_pool_task;

/**
 * \brief Opaque thread pool handle.
 */
typedef struct vl_thread_pool_
{
    /* Work queues per priority tier */
    vl_async_queue* workQueues[VL_THREAD_POOL_PRIORITY_COUNT];

    /* Semaphore per tier: counts available work at that priority */
    vl_semaphore semaphores[VL_THREAD_POOL_PRIORITY_COUNT];

    /* Worker thread management */
    vl_thread* workers;
    vl_uint_t workerCount;

    /* State & statistics */
    VL_ATOMIC vl_thread_pool_state state;
    VL_ATOMIC vl_ularge_t tasksCompleted;

    /* Idle synchronization: for vlThreadPoolWait() */
    vl_condition all_idle;
    vl_mutex idle_lock;
    VL_ATOMIC vl_uint_t active_workers;
} vl_thread_pool;

/**
 * \brief Statistics snapshot from a thread pool.
 *
 * \field tasks_completed Total tasks executed since pool creation
 * \field tasks_pending_by_priority Queue depth for each priority tier
 * \field worker_count Number of active worker threads
 *
 * \note Counts may be slightly stale due to concurrent modifications.
 */
typedef struct
{
    vl_ularge_t tasks_completed;
    vl_uint32_t tasksPending[VL_THREAD_POOL_PRIORITY_COUNT];
    vl_uint_t worker_count;
} vl_thread_pool_stats;

/**
 * \brief Creates a new priority-aware thread pool with work-stealing.
 *
 * Worker threads are created immediately and begin waiting for work. Threads
 * employ a work-stealing strategy: HIGH -> MEDIUM -> LOW, with stealing from
 * lower tiers when higher tiers are empty.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_thread_pool` handle and is responsible for calling
 * `vlThreadPoolDelete`.
 * - **Lifetime**: The thread pool remains valid until `vlThreadPoolDelete`.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: Returns `NULL` if `worker_count` is 0 or if the pool could not be created.
 * - **Error Conditions**: Returns `NULL` if any heap allocation fails or if worker threads cannot be spawned.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates the pool structure, worker handles, queues, and synchronization
 * primitives on the heap.
 * - **Return-value Semantics**: Returns an opaque handle to the new thread pool, or `NULL` on failure.
 *
 * \param worker_count Number of worker threads to create (must be > 0)
 * \return Thread pool handle, or VL_THREAD_POOL_NULL on failure
 *
 * \note If worker_count is 0 or thread creation fails, returns null.
 *
 * \sa vlThreadPoolDelete
 */
VL_API vl_thread_pool* vlThreadPoolNew(vl_uint_t worker_count);

/**
 * \brief Deletes a thread pool and frees all associated resources.
 *
 * Threads must have been joined via shutdown. If threads are still active,
 * behavior is undefined.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the thread pool handle and all its internal resources.
 * - **Lifetime**: The thread pool handle becomes invalid immediately after this call.
 * - **Thread Safety**: Safe to call from any thread, provided no other thread is concurrently using the pool.
 * - **Nullability**: Safe to call with `NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion. Deleting a pool that is being used by another thread.
 * - **Memory Allocation Expectations**: Deallocates all heap-allocated resources associated with the pool.
 * - **Return-value Semantics**: None (void).
 *
 * \param pool Thread pool handle to delete
 *
 * \warning Call vlThreadPoolShutdown() first and ensure all threads have
 * exited.
 *
 * \sa vlThreadPoolShutdown, vlThreadPoolWait
 */
VL_API void vlThreadPoolDelete(vl_thread_pool* pool);

/**
 * \brief Enqueues a single work item at the specified priority level.
 *
 * The task is atomically added to the appropriate priority queue and a waiting
 * worker thread is signaled. If no workers are waiting, the task is queued for
 * later execution.
 *
 * ## Contract
 * - **Ownership**: The pool copies the `task` data into its internal storage. The caller retains ownership of the
 * `task` pointer.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (lock-free).
 * - **Nullability**: Returns `VL_FALSE` if `pool` or `task` is `NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if the pool is in the process of shutting down.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: May trigger node allocation in the underlying async queues.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the task was successfully enqueued, `VL_FALSE` otherwise.
 *
 * \param pool Thread pool handle
 * \param priority Priority level (HIGH, MEDIUM, or LOW)
 * \param task Pointer to task structure (copied internally)
 * \return VL_TRUE on success, VL_FALSE if pool is shutting down
 *
 * \note This function is lock-free and safe to call concurrently from any
 * thread.
 *
 * \sa vlThreadPoolEnqueueBatchPriority, vlThreadPoolWait
 */
VL_API vl_bool_t vlThreadPoolEnqueuePriority(vl_thread_pool* pool, vl_thread_pool_priority priority,
                                             const vl_thread_pool_task* task);

/**
 * \brief Enqueues multiple work items at the same priority level in a batch.
 *
 * This is more efficient than multiple individual enqueues. All tasks are
 * added to the same priority queue atomically.
 *
 * ## Contract
 * - **Ownership**: The pool copies the tasks in the `tasks` array into its internal storage.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (lock-free).
 * - **Nullability**: Returns 0 if `pool` or `tasks` is `NULL`.
 * - **Error Conditions**: Returns 0 if the pool is shutting down.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: May trigger node allocation in the underlying async queues.
 * - **Return-value Semantics**: Returns the number of tasks successfully enqueued.
 *
 * \param pool Thread pool handle
 * \param priority Priority level for all tasks
 * \param tasks Pointer to array of task structures
 * \param count Number of tasks in the array
 * \return Number of tasks successfully enqueued (< count if shutdown in
 * progress)
 *
 * \note This function is lock-free and safe to call concurrently.
 *
 * \sa vlThreadPoolEnqueuePriority
 */
VL_API vl_uint_t vlThreadPoolEnqueueBatchPriority(vl_thread_pool* pool, vl_thread_pool_priority priority,
                                                  const vl_thread_pool_task* tasks, vl_uint_t count);

/**
 * \brief Convenience wrapper: enqueues at DEFAULT (MEDIUM) priority.
 *
 * Equivalent to `vlThreadPoolEnqueuePriority(pool,
 * VL_THREAD_POOL_PRIORITY_MEDIUM, task)`.
 *
 * \param pool Thread pool handle
 * \param task Pointer to task structure
 * \return VL_TRUE on success, VL_FALSE if pool is shutting down
 *
 * \sa vlThreadPoolEnqueuePriority
 */
static inline vl_bool_t vlThreadPoolEnqueue(vl_thread_pool* pool, const vl_thread_pool_task* task)
{
    return vlThreadPoolEnqueuePriority(pool, VL_THREAD_POOL_PRIORITY_MEDIUM, task);
}

/**
 * \brief Convenience wrapper: batch-enqueues at DEFAULT (MEDIUM) priority.
 *
 * Equivalent to `vlThreadPoolEnqueueBatchPriority(pool,
 * VL_THREAD_POOL_PRIORITY_MEDIUM, ...)`.
 *
 * \param pool Thread pool handle
 * \param tasks Pointer to task array
 * \param count Number of tasks
 * \return Number of tasks successfully enqueued
 *
 * \sa vlThreadPoolEnqueueBatchPriority
 */
static inline vl_uint_t vlThreadPoolEnqueueBatch(vl_thread_pool* pool, const vl_thread_pool_task* tasks,
                                                 vl_uint_t count)
{
    return vlThreadPoolEnqueueBatchPriority(pool, VL_THREAD_POOL_PRIORITY_MEDIUM, tasks, count);
}

/**
 * \brief Waits until all enqueued tasks across all priority tiers have
 * completed.
 *
 * Blocks the calling thread until all HIGH, MEDIUM, and LOW priority queues
 * are empty and all workers are idle. May be called concurrently from multiple
 * threads; all will unblock when the pool becomes empty.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (blocking).
 * - **Nullability**: Returns `VL_FALSE` if `pool` is `NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if the timeout expires.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the pool became idle within the timeout, `VL_FALSE` otherwise.
 *
 * \param pool Thread pool handle
 * \param timeout_ms Maximum time to wait in milliseconds (0 = infinite)
 * \return VL_TRUE if all tasks completed, VL_FALSE if timeout expired
 *
 * \note If new tasks are enqueued from other threads while waiting, behavior
 *       is implementation-specific (may or may not wait for them).
 *
 * \sa vlThreadPoolEnqueuePriority
 */
VL_API vl_bool_t vlThreadPoolWait(vl_thread_pool* pool, vl_uint_t timeout_ms);

/**
 * \brief Initiates graceful shutdown of the thread pool.
 *
 * After calling this function:
 * - No new tasks are accepted (vlThreadPoolEnqueuePriority returns VL_FALSE)
 * - In-flight tasks across all priorities continue to completion
 * - Worker threads exit naturally
 *
 * This function does not wait for threads. Use vlThreadPoolWait() to block
 * until idle, or vlThreadPoolDelete() to join and cleanup.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: The pool enters a shutting-down state.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: Safe to call with `NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param pool Thread pool handle
 *
 * \note Safe to call multiple times (idempotent).
 *
 * \sa vlThreadPoolDelete, vlThreadPoolWait
 */
VL_API void vlThreadPoolShutdown(vl_thread_pool* pool);

/**
 * \brief Retrieves a snapshot of current thread pool statistics.
 *
 * Returns completed task count and per-priority queue depths.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (atomic reads).
 * - **Nullability**: `pool` and `out_stats` should not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param pool Thread pool handle
 * \param out_stats Pointer to stats structure to populate
 *
 * \note Statistics are atomic but may be stale by the time this function
 * returns.
 *
 * \sa vl_thread_pool_stats
 */
VL_API void vlThreadPoolGetStats(vl_thread_pool* pool, vl_thread_pool_stats* out_stats);

/**
 * \brief Returns the total approximate queue depth across all priorities.
 *
 * Convenience function equivalent to summing all priority tiers in stats.
 *
 * \param pool Thread pool handle
 * \return Approximate total number of pending tasks
 *
 * \note This value may change immediately after the function returns.
 */
static inline vl_uint32_t vlThreadPoolQueueDepth(vl_thread_pool* pool)
{
    vl_thread_pool_stats stats;
    vlThreadPoolGetStats(pool, &stats);
    return stats.tasksPending[0] + stats.tasksPending[1] + stats.tasksPending[2];
}

#endif // VL_THREAD_POOL_H
