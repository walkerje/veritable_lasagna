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

#ifndef VL_LIST_H
#define VL_LIST_H

#include "vl_compare.h"
#include "vl_pool.h"

#define VL_LIST_ITER_INVALID VL_POOL_INVALID_IDX

/**
 * This is a simple macro for iterating a list.
 * \param list pointer vl_list pointer
 * \param trackVar identifier of the iterator. Always of type vl_list_iter.
 * \sa vl_list_iter
 */
#define VL_LIST_FOREACH(list, trackVar)                                                                                \
    for (vl_list_iter trackVar = (list)->head; (trackVar) != VL_LIST_ITER_INVALID;                                     \
         (trackVar) = vlListNext(list, trackVar))

/**
 * This is a simple macro for iterating a list, in reverse.
 * \param list pointer vl_list pointer
 * \param trackVar identifier of the iterator. Always of type vl_list_iter.
 * \sa vl_list_iter
 */
#define VL_LIST_FOREACH_REVERSE(list, trackVar)                                                                        \
    for (vl_list_iter trackVar = (list)->tail; (trackVar) != VL_LIST_ITER_INVALID;                                     \
         (trackVar) = vlListPrev(list, trackVar))

/**
 * \brief List iterator type. Represents a location within a vl_linked_list.
 */
typedef vl_pool_idx vl_list_iter;

/**
 * \brief A doubly-linked list with pool-based memory management
 *
 * A doubly-linked list implementation that uses a fixed-size memory pool for
 * efficient node allocation and deallocation. Each node contains both the
 * element data and bidirectional links to adjacent nodes.
 *
 * Key features:
 * - Memory efficient: Uses pool allocation to minimize fragmentation
 * - Constant-time operations: O(1) for insertions and deletions at known
 * positions
 * - Bidirectional traversal: Supports both forward and backward iteration
 * - Cache-friendly: Nodes are allocated in contiguous memory blocks
 *
 * Memory management:
 * - Nodes are allocated from an internal vl_pool
 * - The pool pre-allocates memory in larger blocks
 * - Deleted nodes are recycled for future allocations
 * - No per-operation dynamic allocation after initial pool setup
 *
 * \sa vl_pool, vl_list_iter
 */

typedef struct
{
    vl_pool nodePool;
    vl_uint16_t elementSize;
    vl_list_iter head;
    vl_list_iter tail;
    vl_dsidx_t length;
} vl_linked_list;

/**
 * \brief Initializes the specified list instance.
 *
 * The initialized list should be freed via vlListFree.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the `list` struct. The function initializes the internal node
 * pool.
 * - **Lifetime**: The list is valid until `vlListFree` or `vlListDelete`.
 * - **Thread Safety**: Not thread-safe. Concurrent access must be synchronized.
 * - **Nullability**: `list` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an already initialized list without first calling `vlListFree` (causes memory
 * leak).
 * - **Memory Allocation Expectations**: Initializes an internal `vl_pool` which allocates management structures.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlListFree
 * \param list pointer
 * \param elementSize size of a single list element, in bytes.
 * \par Complexity of O(1) constant.
 */
VL_API void vlListInit(vl_linked_list* list, vl_uint16_t elementSize);

#ifndef vlListFree

/**
 * \brief Frees the specified list instance.
 *
 * The specified list should be initialized via vlListInit.
 *
 * \sa vlListInit
 * \param list pointer
 * \par Complexity of O(1) constant.
 */
#define vlListFree(listPtr) vlPoolFree(&((listPtr)->nodePool))

#endif

/**
 * \brief Allocates on the heap, initializes, and returns a new list instance.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_linked_list` pointer and is responsible for calling `vlListDelete`.
 * - **Lifetime**: The list is valid until `vlListDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `NULL` if heap allocation for the list struct fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_linked_list` struct and its internal pool.
 * - **Return-value Semantics**: Returns a pointer to the new list, or `NULL`.
 *
 * \param elementSize size of a single list element, in bytes.
 * \par Complexity of O(1) constant.
 * \return pointer to created list
 */
VL_API vl_linked_list* vlListNew(vl_uint16_t elementSize);

/**
 * \brief Deletes the specified list instance and its internal pool.
 *
 * The specified list should be created via vlListNew.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal node pool and the `vl_linked_list` struct.
 * - **Lifetime**: The list pointer and all its iterators become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Safe to call if `list` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Deallocates the internal node pool and the list struct.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlListNew
 * \param list pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlListDelete(vl_linked_list* list);

/**
 * \brief Adds a new element to the front of the list.
 *
 * Element data is copied to the internal pool allocator.
 *
 * ## Contract
 * - **Ownership**: Does not transfer ownership of `elem`. The list maintains a copy.
 * - **Lifetime**: The returned iterator is valid until the element is removed or the list is cleared/destroyed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `list` must not be `NULL`. `elem` can be `NULL` (initializes with zero/garbage depending on pool
 * implementation).
 * - **Error Conditions**: Returns `VL_LIST_ITER_INVALID` if node allocation fails.
 * - **Undefined Behavior**: Passing an uninitialized list.
 * - **Memory Allocation Expectations**: May trigger expansion of the underlying node pool.
 * - **Return-value Semantics**: Returns a `vl_list_iter` handle to the new element, or `VL_LIST_ITER_INVALID` on
 * failure.
 *
 * \param list pointer
 * \param elem pointer to element data
 * \par Complexity of O(1) constant.
 * \return iterator referring to added element
 */
VL_API vl_list_iter vlListPushFront(vl_linked_list* list, const void* elem);

/**
 * \brief Removes whatever element is at the front of the list.
 *
 * This is a no-op if the list is empty.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the front element node back to the pool.
 * - **Lifetime**: Iterators to the former front element become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `list` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized list.
 * - **Memory Allocation Expectations**: None (returns node to pool).
 * - **Return-value Semantics**: None (void).
 *
 * \param list pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlListPopFront(vl_linked_list* list);

/**
 * \brief Adds a new element to the end of the list.
 *
 * Element data is copied to the internal pool allocator.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The list maintains a copy.
 * - **Lifetime**: Valid until removed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `list` must not be `NULL`. `elem` can be `NULL`.
 * - **Error Conditions**: Returns `VL_LIST_ITER_INVALID` if allocation fails.
 * - **Undefined Behavior**: Passing an uninitialized list.
 * - **Memory Allocation Expectations**: May trigger pool expansion.
 * - **Return-value Semantics**: Returns a `vl_list_iter` handle to the new element, or `VL_LIST_ITER_INVALID`.
 *
 * \param list pointer
 * \param elem pointer to element data
 * \par Complexity of O(1) constant.
 * \return iterator referring to added element
 */
VL_API vl_list_iter vlListPushBack(vl_linked_list* list, const void* elem);

/**
 * \brief Removes whatever element is at the end of the list.
 *
 * This is a no-op if the list is empty.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the back element node.
 * - **Lifetime**: Iterators to the former back element become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `list` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized list.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param list pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlListPopBack(vl_linked_list* list);

/**
 * \brief Inserts an element immediately after the specified target.
 *
 * Element data is copied to the internal pool allocator.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The list maintains a copy.
 * - **Lifetime**: Valid until removed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `list` must not be `NULL`. `target` must not be `VL_LIST_ITER_INVALID`. `elem` can be `NULL`.
 * - **Error Conditions**: Returns `VL_LIST_ITER_INVALID` if allocation fails.
 * - **Undefined Behavior**: Passing an invalid iterator or an iterator from a different list.
 * - **Memory Allocation Expectations**: May trigger pool expansion.
 * - **Return-value Semantics**: Returns a `vl_list_iter` handle to the new element, or `VL_LIST_ITER_INVALID`.
 *
 * \param list pointer
 * \param target iterator to element that will have something inserted after it.
 * \param elem pointer
 * \par Complexity of O(1) constant.
 * \return iterator to inserted element.
 */
VL_API vl_list_iter vlListInsertAfter(vl_linked_list* list, vl_list_iter target, const void* elem);

/**
 * \brief Inserts an element immediately before the specified target.
 *
 * Element data is copied to the internal pool allocator.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The list maintains a copy.
 * - **Lifetime**: Valid until removed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Same as `vlListInsertAfter`.
 * - **Error Conditions**: Same as `vlListInsertAfter`.
 * - **Undefined Behavior**: Same as `vlListInsertAfter`.
 * - **Memory Allocation Expectations**: May trigger pool expansion.
 * - **Return-value Semantics**: Returns a `vl_list_iter` handle to the new element, or `VL_LIST_ITER_INVALID`.
 *
 * \param list pointer
 * \param target iterator to element that will have something inserted before
 * it.
 * \param elem
 * \par Complexity of O(1) constant.
 * \return iterator to inserted element.
 */
VL_API vl_list_iter vlListInsertBefore(vl_linked_list* list, vl_list_iter target, const void* elem);

#ifndef vlListSize
/**
 * \brief Returns the total number of elements in the specified list.
 * \param list pointer
 * \par Complexity of O(1) constant.
 * \return total elements
 */
#define vlListSize(listPtr) (vl_dsidx_t)((listPtr)->length)
#endif

#ifndef vlListReserve
/**
 * \brief Reserves space for n-many elements in the specified list.
 *
 * \param list pointer
 * \param n number of elements to reserve space for.
 */
#define vlListReserve(listPtr, n) vlFixedPoolReserve(&((listPtr)->nodePool))
#endif

#ifndef vlListClear
/**
 * \brief Clears the specified list so it can be used as if it was just
 * initialized.
 *
 * This function does not touch any information in the underlying buffer, but
 * rather resets some book-keeping variables.
 *
 * \param list pointer
 * \par Complexity of O(1) constant.
 */
#define vlListClear(listPtr)                                                                                           \
    (((listPtr)->head = (listPtr)->tail = VL_LIST_ITER_INVALID));                                                      \
    vlFixedPoolClear(&((listPtr)->nodePool))
#endif

/**
 * \brief Clones the specified list to another.
 *
 * Clones the entirety of the src list to the dest list, including all element data and order.
 *
 * The 'src' list pointer must be non-null and initialized.
 * The 'dest' list pointer may be null, but if it is not null it must be
 * initialized.
 *
 * If the 'dest' list pointer is null, a new list is created via vlListNew.
 * Otherwise, its element size is set to the source's and all of its existing
 * data is replaced.
 *
 * ## Contract
 * - **Ownership**: If `dest` is `NULL`, the caller owns the returned `vl_linked_list`. If `dest` is provided, ownership
 * remains with the caller.
 * - **Lifetime**: Valid until deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` must not be `NULL`. `dest` can be `NULL`.
 * - **Error Conditions**: Returns `NULL` if allocation fails.
 * - **Undefined Behavior**: Passing an uninitialized list.
 * - **Memory Allocation Expectations**: Allocates a new list struct (if `dest` is `NULL`) and multiple nodes.
 * - **Return-value Semantics**: Returns the pointer to the cloned list, or `NULL` on failure.
 *
 * \param src
 * \param dest
 * \return pointer to list that was copied to or created.
 */
VL_API vl_linked_list* vlListClone(const vl_linked_list* src, vl_linked_list* dest);

/**
 * \brief Copies a range of elements from one list to another.
 *
 * Both the src list and the dest list must have equivalent element sizes,
 * otherwise this is a no-op.
 *
 * This is an inclusive range, and as such, the elements referred to by the
 * begin and end iterators are also copied to the target list.
 *
 * The begin and end iterators are expected to be in logical iterative order,
 * meaning that if iterating through the entire src list, begin would be found
 * before end.
 *
 * ## Contract
 * - **Ownership**: Unchanged. `dest` maintains copies of the elements.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` and `dest` must not be `NULL`.
 * - **Error Conditions**: Returns 0 if element sizes do not match.
 * - **Undefined Behavior**: Passing iterators from the wrong list or in the wrong order.
 * - **Memory Allocation Expectations**: May trigger node pool expansion in `dest`.
 * - **Return-value Semantics**: Returns the total number of elements successfully copied.
 *
 * \param src source list pointer
 * \param begin iterator to start the copy at, or VL_LIST_ITER_INVALID for the
 * beginning of the list.
 * \param end iterator to end the copy after, or VL_LIST_ITER_INVALID for the
 * end of the list.
 * \param dest destination buffer pointer
 * \param after the iterator to insert elements after, or VL_LIST_ITER_INVALID
 * for the end of the destination.
 * \par Complexity of O(n) linear.
 * \return number of elements copied
 */
VL_API int vlListCopy(vl_linked_list* src, vl_list_iter begin, vl_list_iter end, vl_linked_list* dest,
                      vl_list_iter after);

/**
 * \brief Sorts the specified list in-place using the given comparator.
 *
 * This function implements an iterative merge sort. Elements are not copied in
 * this operation, but rather the links between them are modified.
 *
 * The list is split into a "forest" in the underlying node pool, briefly
 * holding a variety of sub-lists which are later merged back together.
 *
 * ## Contract
 * - **Ownership**: Unchanged. Nodes are rearranged but stay in the same list and pool.
 * - **Lifetime**: All existing iterators remain valid but their logical order in the list may change.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `list` and `cmp` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing a `NULL` comparator or an uninitialized list.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param src source list pointer
 * \param cmp comparator function
 * \par Complexity of O(n log(n)).
 */
VL_API void vlListSort(vl_linked_list* src, vl_compare_function cmp);

/**
 * \brief Performs an iterative search on the specified list.
 *
 * This will return an iterator to the first instance of the specified element
 * that was found using the default comparator.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe for concurrent reads if no other thread is writing.
 * - **Nullability**: `src` must not be `NULL`. `element` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_LIST_ITER_INVALID` if the element is not found.
 * - **Undefined Behavior**: Passing an uninitialized list.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a `vl_list_iter` to the found element, or `VL_LIST_ITER_INVALID`.
 *
 * \param src source list pointer
 * \param element pointer to the element to compare against
 * \par Complexity of O(n) linear.
 * \return iterator to found element, or VL_LIST_ITER_INVALID on failure.
 */
VL_API vl_list_iter vlListFind(vl_linked_list* src, const void* element);

/**
 * \brief Removes the specified element from the list.
 *
 * The underlying node is returned to the underlying node pool for reuse,
 * without modifying its discarded data.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the node back to the pool.
 * - **Lifetime**: The passed iterator becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `list` must not be `NULL`. `iter` should not be `VL_LIST_ITER_INVALID`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator or an iterator from a different list.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param list pointer
 * \param iter node iterator.
 * \par Complexity of O(1) constant.
 */
VL_API void vlListRemove(vl_linked_list* list, vl_list_iter iter);

/**
 * \brief Returns next adjacent iterator, or VL_LIST_ITER_INVALID if no such
 * element exists.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: The returned iterator is valid as long as the element is not removed.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `list` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_LIST_ITER_INVALID` if no next element.
 * - **Undefined Behavior**: Passing an invalid iterator.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the next `vl_list_iter` in the sequence.
 *
 * \sa vlListPrev
 * \param list pointer
 * \param iter node iterator
 * \par Complexity of O(1) constant.
 * \return next iterator, or VL_LIST_ITER_INVALID.
 */
VL_API vl_list_iter vlListNext(vl_linked_list* list, vl_list_iter iter);

/**
 * \brief Returns the previous adjacent iterator, or VL_LIST_ITER_INVALID if no
 * such element exists.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Same as `vlListNext`.
 * - **Thread Safety**: Same as `vlListNext`.
 * - **Nullability**: Same as `vlListNext`.
 * - **Error Conditions**: Returns `VL_LIST_ITER_INVALID` if no previous element.
 * - **Undefined Behavior**: Same as `vlListNext`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the previous `vl_list_iter` in the sequence.
 *
 * \sa vlListPrev
 * \param list pointer
 * \param iter node iterator
 * \par Complexity of O(1) constant.
 * \return previous iterator, or VL_LIST_ITER_INVALID.
 */
VL_API vl_list_iter vlListPrev(vl_linked_list* list, vl_list_iter iter);

/**
 * \brief Returns a pointer to the element data for the specified iterator.
 *
 * ## Contract
 * - **Ownership**: Ownership remains with the list. The caller must not free the returned pointer.
 * - **Lifetime**: The returned pointer is valid until the element is removed or the list is reallocated/destroyed.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: Returns `NULL` if `iter` is `VL_LIST_ITER_INVALID`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a pointer to the raw element data.
 *
 * \param list pointer
 * \param iter node iterator
 * \par Complexity of O(1) constant.
 * \return pointer to element data.
 */
VL_API void* vlListSample(vl_linked_list* list, vl_list_iter iter);

#endif // VL_LIST_H
