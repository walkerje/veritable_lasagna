#include "vl_thread_pool.h"

#include "vl_condition.h"
#include "vl_memory.h"

/**
 * \brief Main worker thread loop with work-stealing strategy.
 *
 * Strategy:
 * 1. Attempt non-blocking pop from HIGH priority queue
 * 2. If empty, attempt MEDIUM priority queue
 * 3. If empty, attempt LOW priority queue
 * 4. If all empty and RUNNING, mark idle and block on semaphore
 * 5. On wakeup or work found, execute and repeat
 *
 * On shutdown (SHUTTING_DOWN state):
 * - Continue processing remaining work in all tiers
 * - Exit when all queues are empty
 */
static void vl_thread_pool_worker_proc(void* user_arg)
{
    vl_thread_pool* pool = (vl_thread_pool*)user_arg;
    vl_thread_pool_task task;
    vl_thread_pool_state state;

    while (VL_TRUE)
    {
        /* Work-stealing loop: HIGH → MEDIUM → LOW */
        vl_bool_t found_work = VL_FALSE;

        for (vl_int_t pri = VL_THREAD_POOL_PRIORITY_HIGH; pri < VL_THREAD_POOL_PRIORITY_COUNT; pri++)
        {
            if (vlAsyncQueuePopFront(pool->workQueues[pri], &task))
            {
                found_work = VL_TRUE;
                break;
            }
        }

        if (found_work)
        {
            /* Execute work and update statistics */
            task.proc(task.user_data);
            vlAtomicFetchAdd(&pool->tasksCompleted, 1);
            continue;
        }

        /* No work found; check shutdown state */
        state = vlAtomicLoad(&pool->state);
        if (state != VL_THREAD_POOL_RUNNING)
        {
            /* Pool is shutting down and all queues empty; exit worker */
            break;
        }

        /* Mark worker as idle */
        vlAtomicFetchSub(&pool->active_workers, 1);

        /* Signal all waiters if all workers are now idle */
        vlMutexObtain(pool->idle_lock);
        if (vlAtomicLoad(&pool->active_workers) == 0)
        {
            vlConditionBroadcast(pool->all_idle);
        }
        vlMutexRelease(pool->idle_lock);

        /* Try non-blocking wait on each semaphore in order of priority */
        vl_bool_t got_signal = VL_FALSE;
        for (vl_int_t pri = VL_THREAD_POOL_PRIORITY_HIGH; pri < VL_THREAD_POOL_PRIORITY_COUNT; pri++)
        {
            if (vlSemaphoreTryWait(pool->semaphores[pri]))
            {
                got_signal = VL_TRUE;
                break;
            }
        }

        /* If no immediate signal, block on HIGH priority semaphore */
        if (!got_signal)
        {
            vlSemaphoreWait(pool->semaphores[VL_THREAD_POOL_PRIORITY_HIGH], 0);
        }

        /* Mark worker as active again */
        vlAtomicFetchAdd(&pool->active_workers, 1);
    }
}

/* ============================================================================
 * Public API Implementation
 * ============================================================================
 */

VL_API vl_thread_pool* vlThreadPoolNew(vl_uint_t worker_count)
{
    if (worker_count == 0)
    {
        return NULL;
    }

    /* Allocate pool structure */
    vl_thread_pool* pool = vlMemAllocType(vl_thread_pool);
    if (pool == NULL)
    {
        return NULL;
    }

    /* Initialize all queues and semaphores */
    for (vl_int_t i = 0; i < VL_THREAD_POOL_PRIORITY_COUNT; i++)
    {
        pool->workQueues[i] = vlAsyncQueueNew(sizeof(vl_thread_pool_task));
        if (pool->workQueues[i] == NULL)
        {
            /* Cleanup on failure */
            for (vl_int_t j = 0; j < i; j++)
            {
                vlAsyncQueueDelete(pool->workQueues[j]);
            }
            vlMemFree((vl_memory*)pool);
            return NULL;
        }

        /* Initialize semaphore with count 0 (no work initially) */
        pool->semaphores[i] = vlSemaphoreNew(0);
        if (pool->semaphores[i] == NULL)
        {
            /* Cleanup on failure */
            for (vl_int_t j = 0; j <= i; j++)
            {
                vlAsyncQueueDelete(pool->workQueues[j]);
                if (j < i)
                {
                    vlSemaphoreDelete(pool->semaphores[j]);
                }
            }
            vlMemFree((vl_memory*)pool);
            return NULL;
        }
    }

    /* Initialize synchronization primitives for waiting */
    pool->all_idle = vlConditionNew();
    if (pool->all_idle == NULL)
    {
        for (vl_int_t i = 0; i < VL_THREAD_POOL_PRIORITY_COUNT; i++)
        {
            vlAsyncQueueDelete(pool->workQueues[i]);
            vlSemaphoreDelete(pool->semaphores[i]);
        }
        vlMemFree((vl_memory*)pool);
        return NULL;
    }

    pool->idle_lock = vlMutexNew();
    if (pool->idle_lock == NULL)
    {
        vlConditionDelete(pool->all_idle);
        for (vl_int_t i = 0; i < VL_THREAD_POOL_PRIORITY_COUNT; i++)
        {
            vlAsyncQueueDelete(pool->workQueues[i]);
            vlSemaphoreDelete(pool->semaphores[i]);
        }
        vlMemFree((vl_memory*)pool);
        return NULL;
    }

    /* Allocate worker thread array */
    pool->workers = vlMemAllocTypeArray(vl_thread, worker_count);
    if (pool->workers == NULL)
    {
        vlConditionDelete(pool->all_idle);
        vlMutexDelete(pool->idle_lock);
        for (vl_int_t i = 0; i < VL_THREAD_POOL_PRIORITY_COUNT; i++)
        {
            vlAsyncQueueDelete(pool->workQueues[i]);
            vlSemaphoreDelete(pool->semaphores[i]);
        }
        vlMemFree((vl_memory*)pool);
        return NULL;
    }

    pool->workerCount = worker_count;

    /* Initialize atomic state */
    vlAtomicInit(&pool->state, VL_THREAD_POOL_RUNNING);
    vlAtomicInit(&pool->tasksCompleted, 0);
    vlAtomicInit(&pool->active_workers, worker_count);

    /* Create worker threads */
    for (vl_uint_t i = 0; i < worker_count; i++)
    {
        pool->workers[i] = vlThreadNew(vl_thread_pool_worker_proc, (void*)pool);
        if (pool->workers[i] == VL_THREAD_NULL)
        {
            /* Cleanup: shutdown existing threads */
            vlAtomicStore(&pool->state, VL_THREAD_POOL_SHUTTING_DOWN);

            /* Signal all semaphores to wake threads */
            for (vl_int_t j = 0; j < VL_THREAD_POOL_PRIORITY_COUNT; j++)
            {
                for (vl_uint_t k = 0; k < i; k++)
                {
                    vlSemaphorePost(pool->semaphores[j]);
                }
            }

            /* Join created threads */
            for (vl_uint_t j = 0; j < i; j++)
            {
                vlThreadJoin(pool->workers[j]);
                vlThreadDelete(pool->workers[j]);
            }

            /* Free resources */
            vlMemFree((vl_memory*)pool->workers);
            vlConditionDelete(pool->all_idle);
            vlMutexDelete(pool->idle_lock);
            for (vl_int_t j = 0; j < VL_THREAD_POOL_PRIORITY_COUNT; j++)
            {
                vlAsyncQueueDelete(pool->workQueues[j]);
                vlSemaphoreDelete(pool->semaphores[j]);
            }
            vlMemFree((vl_memory*)pool);
            return NULL;
        }
    }

    return pool;
}

VL_API void vlThreadPoolDelete(vl_thread_pool* pool)
{
    if (pool == NULL)
    {
        return;
    }

    /* Ensure shutdown is initiated */
    vlThreadPoolShutdown(pool);

    /* Join all worker threads */
    for (vl_uint_t i = 0; i < pool->workerCount; i++)
    {
        vlThreadJoin(pool->workers[i]);
        vlThreadDelete(pool->workers[i]);
    }

    /* Free worker array */
    vlMemFree((vl_memory*)pool->workers);

    /* Free synchronization primitives */
    vlConditionDelete(pool->all_idle);
    vlMutexDelete(pool->idle_lock);

    /* Free queues and semaphores */
    for (vl_int_t i = 0; i < VL_THREAD_POOL_PRIORITY_COUNT; i++)
    {
        vlAsyncQueueDelete(pool->workQueues[i]);
        vlSemaphoreDelete(pool->semaphores[i]);
    }

    /* Free pool structure */
    vlMemFree((vl_memory*)pool);
}

VL_API vl_bool_t vlThreadPoolEnqueuePriority(vl_thread_pool* pool, vl_thread_pool_priority priority,
                                             const vl_thread_pool_task* task)
{
    if (pool == NULL || task == NULL)
    {
        return VL_FALSE;
    }

    /* Check if pool is shutting down */
    vl_thread_pool_state state = vlAtomicLoad(&pool->state);
    if (state != VL_THREAD_POOL_RUNNING)
    {
        return VL_FALSE;
    }

    /* Enqueue task to appropriate priority queue (lock-free) */
    vlAsyncQueuePushBack(pool->workQueues[priority], (const void*)task);

    /* Signal a waiting worker at this priority tier */
    vlSemaphorePost(pool->semaphores[priority]);

    return VL_TRUE;
}

VL_API vl_uint_t vlThreadPoolEnqueueBatchPriority(vl_thread_pool* pool, vl_thread_pool_priority priority,
                                                  const vl_thread_pool_task* tasks, vl_uint_t count)
{
    if (pool == NULL || tasks == NULL || count == 0)
    {
        return 0;
    }

    /* Check if pool is shutting down */
    vl_thread_pool_state state = vlAtomicLoad(&pool->state);
    if (state != VL_THREAD_POOL_RUNNING)
    {
        return 0;
    }

    /* Enqueue all tasks */
    vl_uint_t enqueued = 0;
    for (vl_uint_t i = 0; i < count; i++)
    {
        vlAsyncQueuePushBack(pool->workQueues[priority], (const void*)&tasks[i]);
        enqueued++;

        vlSemaphorePost(pool->semaphores[priority]);
    }

    return enqueued;
}

VL_API vl_bool_t vlThreadPoolWait(vl_thread_pool* pool, vl_uint_t timeout_ms)
{
    if (pool == NULL)
    {
        return VL_FALSE;
    }

    vlMutexObtain(pool->idle_lock);

    /* Wait until all queues are empty and all workers are idle */
    vl_bool_t result = VL_TRUE;

    while (VL_TRUE)
    {
        /* Check if all workers are idle */
        vl_uint_t active = vlAtomicLoad(&pool->active_workers);
        if (active == 0)
        {
            /* Double-check: verify all queues are empty */
            vl_bool_t all_empty = VL_TRUE;
            for (vl_int_t i = 0; i < VL_THREAD_POOL_PRIORITY_COUNT; i++)
            {
                if (vlAsyncQueueSize(pool->workQueues[i]) > 0)
                {
                    all_empty = VL_FALSE;
                    break;
                }
            }

            if (all_empty)
            {
                /* All workers idle and all queues empty; we're done */
                break;
            }
        }

        /* Wait for broadcast signal or timeout */
        if (timeout_ms == 0)
        {
            /* Infinite wait */
            vlConditionWait(pool->all_idle, pool->idle_lock);
        }
        else
        {
            /* Timed wait */
            if (!vlConditionWaitTimeout(pool->all_idle, pool->idle_lock, (vl_ularge_t)timeout_ms))
            {
                result = VL_FALSE;
                break;
            }
        }
    }

    vlMutexRelease(pool->idle_lock);
    return result;
}

VL_API void vlThreadPoolShutdown(vl_thread_pool* pool)
{
    if (pool == NULL)
    {
        return;
    }

    /* Set state to shutting down (idempotent) */
    vl_thread_pool_state old_state = vlAtomicExchange(&pool->state, VL_THREAD_POOL_SHUTTING_DOWN);

    if (old_state == VL_THREAD_POOL_SHUTTING_DOWN || old_state == VL_THREAD_POOL_SHUT_DOWN)
    {
        /* Already shutting down or shut down */
        return;
    }

    /* Wake all workers by posting to all semaphores */
    for (vl_uint_t i = 0; i < pool->workerCount; i++)
    {
        for (vl_int_t pri = 0; pri < VL_THREAD_POOL_PRIORITY_COUNT; pri++)
        {
            vlSemaphorePost(pool->semaphores[pri]);
        }
    }
}

VL_API void vlThreadPoolGetStats(vl_thread_pool* pool, vl_thread_pool_stats* out_stats)
{
    if (pool == NULL || out_stats == NULL)
    {
        return;
    }

    out_stats->tasks_completed = vlAtomicLoad(&pool->tasksCompleted);
    out_stats->worker_count = pool->workerCount;

    for (vl_int_t i = 0; i < VL_THREAD_POOL_PRIORITY_COUNT; i++)
    {
        out_stats->tasksPending[i] = vlAsyncQueueSize(pool->workQueues[i]);
    }
}
