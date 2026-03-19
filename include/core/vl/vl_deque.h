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

#ifndef VL_DEQUE_H
#define VL_DEQUE_H

#include "vl_pool.h"

/**
 * \brief Double-ended queue.
 *
 * The Deque data structure is a doubly-linked list.
 * It is implemented on top of a pool allocator, and thus requires all elements
 * in the queue to be the same size.
 *
 * Items may be added or removed from either end, but iteration is inherently
 * disallowed. Just like the vl_queue data structure, direct sampling is also
 * disallowed. Thus, all IO requires a copy. If you're using "weighty" (or,
 * large in byte size) elements and are worried about the efficiency of copying,
 * storing pointers, iterators, or offsets in the queue is a perfectly valid
 * approach.
 * \sa vl_queue
 */
typedef struct
{
    vl_pool nodes; // pool nodes
    vl_dsidx_t totalElements; // total elements in the deque
    vl_uint16_t elementSize; // size of a single element, in bytes.
    vl_pool_idx head; // first element
    vl_pool_idx tail; // last element
} vl_deque;

/**
 * \brief Initializes the specified instance of vl_deque with specific element size.
 *
 * The deque should later be de-initialized via vlDequeFree.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the `deq` struct. The function initializes the internal node pool.
 * - **Lifetime**: The deque is valid until `vlDequeFree` or `vlDequeDelete`.
 * - **Thread Safety**: Not thread-safe. Concurrent access must be synchronized.
 * - **Nullability**: `deq` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an already initialized deque without first calling `vlDequeFree` (causes memory
 * leak).
 * - **Memory Allocation Expectations**: Initializes an internal `vl_pool` which allocates management structures.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlDequeFree
 * \param deq pointer
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 */
VL_API void vlDequeInit(vl_deque* deq, vl_uint16_t elementSize);

/**
 * \brief De-initializes and frees the internal resources of the specified deque.
 *
 * The deque should have been initialized via vlDequeInit.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal node pool. Does NOT release the `deq` struct itself.
 * - **Lifetime**: The deque becomes invalid for use.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `deq` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double free.
 * - **Memory Allocation Expectations**: Deallocates internal pool structures.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlDequeInit
 * \param deq pointer
 */
VL_API void vlDequeFree(vl_deque* deq);

/**
 * \brief Allocates on the heap, initializes, and returns a new deque instance.
 *
 * The deque should later be deleted via vlDequeDelete.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_deque` pointer and is responsible for calling `vlDequeDelete`.
 * - **Lifetime**: The deque is valid until `vlDequeDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `NULL` if heap allocation for the deque struct fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_deque` struct and its internal node pool.
 * - **Return-value Semantics**: Returns a pointer to the newly allocated and initialized deque, or `NULL`.
 *
 * \sa vlDequeDelete
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 * \return deque pointer
 */
VL_API vl_deque* vlDequeNew(vl_uint16_t elementSize);

/**
 * \brief De-initializes and deletes the specified deque and its resources.
 *
 * The deque should have been initialized via vlDequeNew.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal node pool and the `vl_deque` struct.
 * - **Lifetime**: The deque pointer becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Safe to call if `deq` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Deallocates internal resources and the deque struct.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlDequeNew
 * \param deq pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlDequeDelete(vl_deque* deq);

/**
 * \brief Clears the specified deque.
 *
 * Resets the head and tail iterators and clears the internal node pool.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: All pointers to elements in the deque become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `deq` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized deque.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param deq pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlDequeClear(vl_deque* deq);

/**
 * \brief Reserves space for n-many elements in the underlying buffer of the
 * specified deque.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `deque` must not be `NULL`.
 * - **Error Conditions**: None checked.
 * - **Undefined Behavior**: Passing an uninitialized deque.
 * - **Memory Allocation Expectations**: Triggers expansion of the underlying node pool.
 * - **Return-value Semantics**: None (void).
 *
 * \param deque pointer
 * \param n total number of elements to reserve space for.
 */
VL_API void vlDequeReserve(vl_deque* deque, vl_dsidx_t n);

/**
 * \brief Clones the specified source deque to another.
 *
 * Clones the entirety of the src deque to the dest deque, including all elements and order.
 *
 * The 'src' deque pointer must be non-null and initialized.
 * The 'dest' deque pointer may be null, but if it is not null it must be
 * initialized.
 *
 * If the 'dest' deque pointer is null, a new deque is created via vlDequeNew.
 * Otherwise, its element size is set to the source's and all of its existing
 * data is replaced.
 *
 * ## Contract
 * - **Ownership**: If `dest` is `NULL`, the caller owns the returned `vl_deque`. If `dest` is provided, ownership
 * remains with the caller.
 * - **Lifetime**: The cloned deque is valid until deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` must not be `NULL`. `dest` can be `NULL`.
 * - **Error Conditions**: Returns `NULL` if allocation fails.
 * - **Undefined Behavior**: Passing an uninitialized deque.
 * - **Memory Allocation Expectations**: May allocate a new deque struct and multiple nodes.
 * - **Return-value Semantics**: Returns the pointer to the cloned deque, or `NULL` on failure.
 *
 * \param src pointer
 * \param dest pointer
 * \return pointer to deque that was copied to or created.
 */
VL_API vl_deque* vlDequeClone(const vl_deque* src, vl_deque* dest);

/**
 * \brief Returns the total number of elements in the deque.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `deq` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized deque.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the current number of elements in the deque.
 *
 * \param deq pointer
 * \par Complexity O(1) constant.
 * \return total number of elements in the deque.
 */
static inline vl_dsidx_t vlDequeSize(vl_deque* deq) { return deq->totalElements; }

/**
 * \brief Adds and copies an element to the front of the deque.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The deque maintains its own copy.
 * - **Lifetime**: Valid until popped.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `deq` must not be `NULL`. `val` should not be `NULL`.
 * - **Error Conditions**: None checked.
 * - **Undefined Behavior**: Passing an uninitialized deque.
 * - **Memory Allocation Expectations**: May trigger expansion of the underlying node pool.
 * - **Return-value Semantics**: None (void).
 *
 * \param deq pointer
 * \param val element data pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlDequePushFront(vl_deque* deq, const void* val);

/**
 * \brief Copies and removes an element from the front of the deque.
 *
 * If specified pointer val is NULL, the element is not copied, but the element
 * is still removed.
 *
 * ## Contract
 * - **Ownership**: Transfers ownership of the element slot back to the internal pool. The caller owns the data copied
 * into `val`.
 * - **Lifetime**: The popped element's storage in the deque becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `deq` must not be `NULL`. `val` can be `NULL`.
 * - **Error Conditions**: Returns 0 if the deque is empty.
 * - **Undefined Behavior**: Passing an uninitialized deque.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns 1 if an element was successfully popped, 0 if the deque was empty.
 *
 * \param deq pointer
 * \param val pointer where the element will be copied to.
 * \par Complexity O(1) constant.
 * \return 1 if success, 0 if failed
 */
VL_API int vlDequePopFront(vl_deque* deq, void* val);

/**
 * \brief Adds and copies an element to the end of the deque.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The deque maintains its own copy.
 * - **Lifetime**: Valid until popped.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `deq` must not be `NULL`. `val` should not be `NULL`.
 * - **Error Conditions**: None checked.
 * - **Undefined Behavior**: Passing an uninitialized deque.
 * - **Memory Allocation Expectations**: May trigger expansion of the underlying node pool.
 * - **Return-value Semantics**: None (void).
 *
 * \param deq pointer
 * \param val element data pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlDequePushBack(vl_deque* deq, const void* val);

/**
 * \brief Copies and removes an element from the end of the deque.
 *
 * If specified pointer val is NULL, the element is not copied, but the element
 * is still removed.
 *
 * ## Contract
 * - **Ownership**: Transfers ownership of the element slot back to the internal pool. The caller owns the data copied
 * into `val`.
 * - **Lifetime**: The popped element's storage in the deque becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `deq` must not be `NULL`. `val` can be `NULL`.
 * - **Error Conditions**: Returns 0 if the deque is empty.
 * - **Undefined Behavior**: Passing an uninitialized deque.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns 1 if an element was successfully popped, 0 if the deque was empty.
 *
 * \param deq pointer
 * \param val pointer where the element will be copied to.
 * \return 1 if success, 0 if failed.
 */
VL_API int vlDequePopBack(vl_deque* deq, void* val);

#endif // VL_DEQUE_H
