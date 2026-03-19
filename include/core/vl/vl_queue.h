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

#ifndef VL_QUEUE_H
#define VL_QUEUE_H

#include "vl_pool.h"

/**
 * \brief First in, first out queue.
 *
 * The Queue data structure is a simplified forward-facing linked list.
 * It is implemented on top of a pool allocator, and thus requires all elements
 * in the queue to be the same size.
 *
 * Items may be added to the end and removed from the beginning.
 * Just like the vl_deque data structure, direct sampling is also disallowed.
 * Thus, all IO requires a copy.
 * \sa vl_deque
 */
typedef struct
{
    vl_pool nodes; // pool nodes
    vl_dsidx_t totalElements; // total elements in the queue.
    vl_uint16_t elementSize; // size of a single element, in bytes.
    vl_pool_idx head; // first element
    vl_pool_idx tail; // last element
} vl_queue;

/**
 * \brief Initializes the specified queue with a specific element size.
 *
 * The queue should then later be de-initialized via vlQueueFree.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the `queue` struct. The function initializes the internal node
 * pool.
 * - **Lifetime**: The queue is valid until `vlQueueFree` or `vlQueueDelete`.
 * - **Thread Safety**: Not thread-safe. Concurrent access must be synchronized.
 * - **Nullability**: `queue` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an already initialized queue without first calling `vlQueueFree` (causes memory
 * leak).
 * - **Memory Allocation Expectations**: Initializes an internal `vl_pool` which allocates management structures.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlQueueFree
 * \param queue pointer
 * \param elementSize size of a single queue element, in bytes
 * \par Complexity of O(1) constant.
 */
VL_API void vlQueueInit(vl_queue* queue, vl_uint16_t elementSize);

/**
 * \brief De-initializes and frees the internal resources of the specified queue.
 *
 * The queue should have been initialized via vlQueueInit.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal node pool. Does NOT release the `queue` struct itself.
 * - **Lifetime**: The queue becomes invalid for use.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `queue` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double free.
 * - **Memory Allocation Expectations**: Deallocates internal pool structures.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlQueueFree
 * \param queue pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlQueueFree(vl_queue* queue);

/**
 * \brief Allocates on the heap, initializes, and returns a new queue instance.
 *
 * The queue should then later be deleted via vlQueueDelete.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_queue` pointer and is responsible for calling `vlQueueDelete`.
 * - **Lifetime**: The queue is valid until `vlQueueDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `NULL` if heap allocation for the queue struct fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_queue` struct and its internal node pool.
 * - **Return-value Semantics**: Returns a pointer to the newly allocated and initialized queue, or `NULL`.
 *
 * \sa vlQueueDelete
 * \param elementSize size of a single queue element, in bytes
 * \par Complexity of O(1) constant.
 * \return pointer to queue
 */
VL_API vl_queue* vlQueueNew(vl_uint16_t elementSize);

/**
 * \brief De-initializes and deletes the specified queue and its resources.
 *
 * The queue should have been initialized via vlQueueNew.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal node pool and the `vl_queue` struct.
 * - **Lifetime**: The queue pointer becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Safe to call if `queue` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Deallocates internal resources and the queue struct.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlQueueNew
 * \param queue pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlQueueDelete(vl_queue* queue);

/**
 * \brief Clones the specified source queue to another.
 *
 * Clones the entirety of the src queue to the dest queue, including all elements and order.
 *
 * The 'src' queue pointer must be non-null and initialized.
 * The 'dest' queue pointer may be null, but if it is not null it must be
 * initialized.
 *
 * If the 'dest' queue pointer is null, a new queue is created via vlQueueNew.
 * Otherwise, its element size is set to the source's and all of its existing
 * data is replaced.
 *
 * ## Contract
 * - **Ownership**: If `dest` is `NULL`, the caller owns the returned `vl_queue`. If `dest` is provided, ownership
 * remains with the caller.
 * - **Lifetime**: The cloned queue is valid until deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` must not be `NULL`. `dest` can be `NULL`.
 * - **Error Conditions**: Returns `NULL` if allocation fails.
 * - **Undefined Behavior**: Passing an uninitialized queue.
 * - **Memory Allocation Expectations**: May allocate a new queue struct and multiple nodes.
 * - **Return-value Semantics**: Returns the pointer to the cloned queue, or `NULL` on failure.
 *
 * \param src pointer
 * \param dest pointer
 * \return pointer to queue that was copied to or created.
 */
VL_API vl_queue* vlQueueClone(const vl_queue* src, vl_queue* dest);

/**
 * \brief Reserves space for n-many elements in the underlying buffer of the
 * specified queue.
 *
 * This is done by doubling the size until the requested growth is met or
 * exceeded. This function will always result in the reallocation of the
 * underlying memory.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `queue` must not be `NULL`.
 * - **Error Conditions**: None checked.
 * - **Undefined Behavior**: Passing an uninitialized queue.
 * - **Memory Allocation Expectations**: Triggers expansion of the underlying node pool.
 * - **Return-value Semantics**: None (void).
 *
 * \param queue pointer
 * \param n total number of elements to reserve space for.
 */
VL_API void vlQueueReserve(vl_queue* queue, vl_dsidx_t n);

/**
 * \brief Clears the specified queue.
 *
 * The underlying data in the queue is untouched, but rather some book-keeping
 * variables are reset.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: All pointers/iterators to elements in the queue become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `queue` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized queue.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param queue
 * \par Complexity of O(1) constant.
 */
VL_API void vlQueueClear(vl_queue* queue);

/**
 * \brief Adds a new element to the end of the queue.
 *
 * The element data is copied into the queue's internal storage.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The queue maintains its own copy of the element.
 * - **Lifetime**: Valid until the element is popped.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `queue` must not be `NULL`. `element` must not be `NULL`.
 * - **Error Conditions**: None checked (internal node allocation failure is not currently handled).
 * - **Undefined Behavior**: Passing a `NULL` element or an uninitialized queue.
 * - **Memory Allocation Expectations**: May trigger expansion of the underlying node pool.
 * - **Return-value Semantics**: None (void).
 *
 * \param queue pointer
 * \param element data pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlQueuePushBack(vl_queue* queue, const void* element);

/**
 * \brief Copies the first element in the queue and removes it.
 *
 * This is a no-op if the queue is empty.
 *
 * ## Contract
 * - **Ownership**: Transfers ownership of the element slot back to the internal pool. The caller owns the data copied
 * into `element`.
 * - **Lifetime**: The popped element's storage in the queue becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `queue` must not be `NULL`. `element` can be `NULL` to just discard the front element.
 * - **Error Conditions**: Returns 0 if the queue is empty.
 * - **Undefined Behavior**: Passing an uninitialized queue.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns 1 if an element was successfully popped, 0 if the queue was empty.
 *
 * \param queue pointer
 * \param element data pointer
 * \par Complexity of O(1) constant.
 * \return 1 if an element was copied and removed, 0 otherwise.
 */
VL_API int vlQueuePopFront(vl_queue* queue, void* element);

/**
 * \brief Returns the total number of elements in the specified queue.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `queue` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized queue.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the current number of elements in the queue.
 *
 * \param queue pointer
 * \par Complexity of O(1) constant.
 * \return size of the queue
 */
static inline vl_dsidx_t vlQueueSize(vl_queue* queue) { return queue->totalElements; }

#endif // VL_QUEUE_H
