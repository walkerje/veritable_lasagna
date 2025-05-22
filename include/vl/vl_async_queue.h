#ifndef VL_ASYNC_QUEUE_H
#define VL_ASYNC_QUEUE_H

#include "vl_atomic.h"
#include "vl_atomic_ptr.h"
#include "vl_async_pool.h"

/**
 * \brief Multi-Producer, Multi-Consumer (MPMC) Lock-Free Queue
 *
 * This structure implements a fully atomic, non-blocking concurrent queue
 * designed to support multiple producers and multiple consumers safely.
 *
 * The queue is based on the Michael & Scott lock-free concurrent queue algorithm,
 * which ensures high throughput and fairness without requiring locks.
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
typedef struct {
    /** Underlying memory pool for queue nodes */
    vl_async_pool elements;

    /** Atomic pointer to the queue head node (dummy node) */
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
 * \param queue Pointer to an uninitialized vl_async_queue structure.
 * \param elementSize Size in bytes of each element stored in the queue.
 */
void vlAsyncQueueInit(vl_async_queue* queue, vl_uint16_t elementSize);

/**
 * \brief Frees resources held by the queue but does not deallocate the queue structure.
 * \param queue Pointer to an initialized vl_async_queue.
 */
void vlAsyncQueueFree(vl_async_queue* queue);

/**
 * \brief Allocates and initializes a new async queue on the heap.
 * \param elementSize Size in bytes of each element stored in the queue.
 * \return Pointer to the newly allocated vl_async_queue.
 */
vl_async_queue* vlAsyncQueueNew(vl_uint16_t elementSize);

/**
 * \brief Deletes a heap-allocated queue created with vlAsyncQueueNew.
 * \param queue Pointer to the queue to be deleted.
 */
void vlAsyncQueueDelete(vl_async_queue* queue);

/**
 * \brief Clears the queue content and resets it to its initial dummy-node state.
 * \param queue Pointer to the queue.
 *
 * \note Does not free memory but allows memory to be reused.
 * \note Not safe to call concurrently with push/pop operations; external synchronization required.
 */
void vlAsyncQueueClear(vl_async_queue* queue);

/**
 * \brief Resets the queue, deallocating most dynamically allocated memory.
 * \param queue Pointer to the queue.
 *
 * \note Leaves the queue in an initialized but empty state.
 * \note Not safe to call concurrently with push/pop operations; external synchronization required.
 */
void vlAsyncQueueReset(vl_async_queue* queue);

/**
 * \brief Pushes a new element to the end of the queue.
 * \param queue Pointer to the queue.
 * \param value Pointer to the data to enqueue (must be elementSize bytes).
 *
 * \note Safe to call concurrently from multiple threads.
 */
void vlAsyncQueuePushBack(vl_async_queue* queue, const void* value);

/**
 * \brief Pops an element from the front of the queue.
 * \param queue Pointer to the queue.
 * \param result Pointer to the buffer where the popped value will be written (must be elementSize bytes).
 * \return VL_TRUE if an element was dequeued, VL_FALSE if the queue was empty.
 *
 * \note Safe to call concurrently from multiple threads.
 */
vl_bool_t vlAsyncQueuePopFront(vl_async_queue* queue, void* result);

/**
 * \brief Returns the number of elements currently stored in the queue.
 * \param queue Pointer to the queue.
 * \return The current logical size of the queue.
 *
 * \note This value may be stale in the presence of concurrent operations.
 */
static inline vl_uint32_t vlAsyncQueueSize(const vl_async_queue* queue){
    return vlAtomicLoad(&queue->size);
}

#endif //VL_ASYNC_QUEUE_H