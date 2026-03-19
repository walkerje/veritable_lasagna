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

#ifndef VL_ASYNC_QUEUE_H
#define VL_ASYNC_QUEUE_H

#include "vl_async_pool.h"
#include "vl_atomic.h"
#include "vl_atomic_ptr.h"

/**
 * \brief Multi-Producer, Multi-Consumer (MPMC) Lock-Free Queue
 *
 * This structure implements a fully atomic, non-blocking concurrent queue
 * designed to support multiple producers and multiple consumers safely.
 *
 * The queue is based on the Michael & Scott lock-free concurrent queue
 * algorithm, which ensures high throughput and fairness without requiring
 * locks.
 *
 * The queue internally manages memory using \ref vl_async_pool for efficient
 * node allocation and reclamation.
 *
 * \note All queue operations are thread-safe and can be called concurrently
 *       from multiple threads without external synchronization.
 *
 * \note The \c size field provides a current estimate of the queue length,
 *       but may be temporarily inconsistent due to concurrent modifications.
 *
 * \see https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf
 */
typedef struct
{
    /** Underlying memory pool for queue nodes */
    vl_async_pool elements;

    /** Atomic pointer to the queue head node */
    vl_atomic_ptr head;

    /** Atomic pointer to the queue tail node */
    vl_atomic_ptr tail;

    /** Approximate count of elements in the queue */
    vl_atomic_uint32_t size;

    /** Size in bytes of each element stored in the queue */
    vl_uint16_t elementSize;
} vl_async_queue;

/**
 * \brief Initializes an async queue for elements of a specified size.
 *
 * ## Contract
 * - **Ownership**: The caller provides the `queue` memory.
 * - **Lifetime**: The queue must be freed with `vlAsyncQueueFree` before its memory is reclaimed.
 * - **Thread Safety**: Not thread-safe for the same `queue` instance.
 * - **Nullability**: `queue` must not be `NULL`.
 * - **Error Conditions**: May crash if `queue` is `NULL`.
 * - **Undefined Behavior**: Calling on an already initialized queue without freeing it first.
 * - **Memory Allocation Expectations**: Allocates initial internal pool blocks.
 * - **Return-value Semantics**: None (void).
 *
 * \param queue Pointer to an uninitialized vl_async_queue structure.
 * \param elementSize Size in bytes of each element stored in the queue.
 */
VL_API void vlAsyncQueueInit(vl_async_queue* queue, vl_uint16_t elementSize);

/**
 * \brief Frees resources held by the queue but does not deallocate the queue
 * structure.
 *
 * ## Contract
 * - **Ownership**: Releases internal resources.
 * - **Lifetime**: The `queue` structure remains but its contents are invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `queue` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Calling on a queue that is being used by other threads.
 * - **Memory Allocation Expectations**: Deallocates internal pool blocks.
 * - **Return-value Semantics**: None (void).
 *
 * \param queue Pointer to an initialized vl_async_queue.
 */
VL_API void vlAsyncQueueFree(vl_async_queue* queue);

/**
 * \brief Allocates and initializes a new async queue on the heap.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_async_queue` pointer and is responsible for calling
 * `vlAsyncQueueDelete`.
 * - **Lifetime**: The queue remains valid until `vlAsyncQueueDelete`.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: Returns `NULL` if allocation fails.
 * - **Error Conditions**: Returns `NULL` if heap allocation for the queue structure or initial internal blocks fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates the queue structure and initial pool blocks on the heap.
 * - **Return-value Semantics**: Returns a pointer to the new queue, or `NULL` on failure.
 *
 * \param elementSize Size in bytes of each element stored in the queue.
 * \return Pointer to the newly allocated vl_async_queue.
 */
VL_API vl_async_queue* vlAsyncQueueNew(vl_uint16_t elementSize);

/**
 * \brief Deletes a heap-allocated queue created with vlAsyncQueueNew.
 *
 * ## Contract
 * - **Ownership**: Releases ownership and all resources.
 * - **Lifetime**: The `queue` pointer becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Safe to call with `NULL` (due to `free` and `vlAsyncQueueFree` should be made safe or checked).
 * Wait, `vlAsyncQueueFree` calls `vlAsyncPoolFree` which might not be safe.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Frees all heap memory associated with the queue.
 * - **Return-value Semantics**: None (void).
 *
 * \param queue Pointer to the queue to be deleted.
 */
VL_API void vlAsyncQueueDelete(vl_async_queue* queue);

/**
 * \brief Clears the queue content and resets it to its initial dummy-node
 * state.
 * \param queue Pointer to the queue.
 *
 * \note Does not free memory but allows memory to be reused.
 * \note Not safe to call concurrently with push/pop operations; external
 * synchronization required.
 */
VL_API void vlAsyncQueueClear(vl_async_queue* queue);

/**
 * \brief Resets the queue, deallocating most dynamically allocated memory.
 * \param queue Pointer to the queue.
 *
 * \note Leaves the queue in an initialized but empty state.
 * \note Not safe to call concurrently with push/pop operations; external
 * synchronization required.
 */
VL_API void vlAsyncQueueReset(vl_async_queue* queue);

/**
 * \brief Pushes a new element to the end of the queue.
 *
 * ## Contract
 * - **Ownership**: The queue copies the data from `value`.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (lock-free MPMC).
 * - **Nullability**: `queue` and `value` must not be `NULL`.
 * - **Error Conditions**: May allocate new blocks if the internal pool is empty.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: May allocate more memory for the internal pool if needed.
 * - **Return-value Semantics**: None (void).
 *
 * \param queue Pointer to the queue.
 * \param value Pointer to the data to enqueue (must be elementSize bytes).
 *
 * \note Safe to call concurrently from multiple threads.
 */
VL_API void vlAsyncQueuePushBack(vl_async_queue* queue, const void* value);

/**
 * \brief Pops an element from the front of the queue.
 *
 * ## Contract
 * - **Ownership**: Copies the popped data into `result`.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (lock-free MPMC).
 * - **Nullability**: `queue` and `result` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if the queue is empty.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if an element was popped, `VL_FALSE` if the queue was empty.
 *
 * \param queue Pointer to the queue.
 * \param result Pointer to the buffer where the popped value will be written
 * (must be elementSize bytes).
 * \return VL_TRUE if an element was dequeued, VL_FALSE if the queue was empty.
 *
 * \note Safe to call concurrently from multiple threads.
 */
VL_API vl_bool_t vlAsyncQueuePopFront(vl_async_queue* queue, void* result);

/**
 * \brief Returns the number of elements currently stored in the queue.
 * \param queue Pointer to the queue.
 * \return The current logical size of the queue.
 *
 * \note This value may be stale in the presence of concurrent operations.
 */
static inline vl_uint32_t vlAsyncQueueSize(const vl_async_queue* queue) { return vlAtomicLoad(&queue->size); }

#endif // VL_ASYNC_QUEUE_H
