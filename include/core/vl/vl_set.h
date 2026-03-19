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

#ifndef VL_SET_H
#define VL_SET_H

#include "vl_compare.h"
#include "vl_memory.h"
#include "vl_pool.h"

#define VL_SET_ITER_INVALID VL_STRUCTURE_INDEX_MAX

/**
 * Convenience macro for forward iterating over the entirety of a set.
 * Implements a for-loop code structure with some of the boilerplate hidden
 * away.
 * \param set pointer
 * \param trackVar name of the tracking variable used as the set iterator
 */
#define VL_SET_FOREACH(set, trackVar)                                                                                  \
    for (vl_set_iter trackVar = vlSetFront(set); (trackVar) != VL_SET_ITER_INVALID;                                    \
         (trackVar) = vlSetNext(set, trackVar))

/**
 * Convenience macro for reverse iterating over the entirety of a set.
 * Implements a for-loop code structure with some of the boilerplate hidden
 * away.
 * \param set pointer
 * \param trackVar name of the tracking variable used as the set iterator
 */
#define VL_SET_FOREACH_REVERSE(set, trackVar)                                                                          \
    for (vl_set_iter trackVar = vlSetBack(set); (trackVar) != VL_SET_ITER_INVALID;                                     \
         (trackVar) = vlSetPrev(set, trackVar))

#ifndef vlSetSize
/**
 * \brief Returns the size of the specified set.
 *
 * This macro looks at total taken from the internal node pool.
 *
 * \param set pointer
 * \return total number of elements in the set.
 */
#define vlSetSize(set) (size_t)((set)->nodePool.totalTaken)
#endif

typedef vl_pool_idx vl_set_iter;

/**
 * \brief An ordered set.
 *
 * The vl_set structure represents an ordered, unique set of values sorted
 * according to a supplied comparator. This is implemented using a red/black
 * binary tree, a separate self-balancing structure which guarantees worse-case
 * search, removal, and insertion complexity of O(log(n)).
 *
 * Elements of the set are sorted according to a key, but can also hold
 * supplementary data for each element that is stored in the same block of
 * memory.
 */
typedef struct
{
    vl_pool nodePool; // node pool which holds all the data in the set.
    vl_uint16_t elementSize; // size of each set element, in bytes.
    vl_dsidx_t totalElements; // total number of elements in the set.
    vl_compare_function comparator; // comparator function pointer. see vl_compare.

    vl_set_iter root; // root iterator. may change upon insert/remove operations.
} vl_set;

/**
 * Initializes the specified set pointer to hold elements of the specified size.
 * This set should be freed via vlSetFree.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the `set` struct. The function initializes the internal node pool.
 * - **Lifetime**: The set is valid until `vlSetFree` or `vlSetDelete`.
 * - **Thread Safety**: Not thread-safe. Concurrent access must be synchronized.
 * - **Nullability**: `set` must not be `NULL`. `compFunc` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an already initialized set without first calling `vlSetFree` (causes memory leak).
 * - **Memory Allocation Expectations**: Initializes an internal `vl_pool` which allocates management structures.
 * - **Return-value Semantics**: None (void).
 *
 * \param set set pointer
 * \param elementSize element size, in bytes.
 * \param compFunc comparator function; 0 = same, >0 = greater, <0 = lesser.
 * \sa vlSetFree
 * \par Complexity O(1) constant.
 */
VL_API void vlSetInit(vl_set* set, vl_memsize_t elementSize, vl_compare_function compFunc);

/**
 * Frees the underlying storage buffers for the specified set.
 * Should only be used on sets initialized via vlSetInit.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal node pool. Does NOT release the `set` struct itself.
 * - **Lifetime**: The set and all its iterators become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `set` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double free.
 * - **Memory Allocation Expectations**: Deallocates internal pool structures.
 * - **Return-value Semantics**: None (void).
 *
 * \param set set pointer
 * \sa vlSetInit
 * \par Complexity O(1) constant.
 */
VL_API void vlSetFree(vl_set* set);

/**
 * Allocates a new set on the heap.
 * Initializes the specified set pointer to hold elements of the specified size.
 * This set should be freed via vlSetDelete.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_set` pointer and is responsible for calling `vlSetDelete`.
 * - **Lifetime**: The set is valid until `vlSetDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `NULL` if heap allocation for the set struct fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_set` struct and its internal node pool.
 * - **Return-value Semantics**: Returns a pointer to the newly allocated and initialized set, or `NULL`.
 *
 * \param elementSize element size, in bytes
 * \param compFunc comparator function; 0 = same, >0 = greater, <0 = lesser.
 * \return set pointer
 * \sa vlSetDelete
 * \par Complexity O(1) constant.
 */
VL_API vl_set* vlSetNew(vl_memsize_t elementSize, vl_compare_function compFunc);

/**
 * Deletes the specified set from the heap.
 * Frees the underlying storage buffers for the specified set.
 * Should only be used by sets initialized via vlSetNew.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal node pool and the `vl_set` struct.
 * - **Lifetime**: The set pointer and all its iterators become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Safe to call if `set` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Deallocates internal resources and the set struct.
 * - **Return-value Semantics**: None (void).
 *
 * \param set set pointer
 * \sa vlSetNew
 * \par Complexity O(1) constant.
 */
VL_API void vlSetDelete(vl_set* set);

/**
 * Returns an iterator to the first element in the set (the one with the minimum key).
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Valid until the element is removed or the set is reallocated/destroyed.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: Returns `VL_SET_ITER_INVALID` if the set is empty.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized set.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a `vl_set_iter` handle to the first element.
 *
 * \param set set pointer
 * \par Complexity O(log(n)).
 * \return iterator of first element in the set.
 */
VL_API vl_set_iter vlSetFront(vl_set* set);

/**
 * Returns an iterator to the last element in the set (the one with the maximum key).
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Valid until the element is removed or the set is reallocated/destroyed.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: Returns `VL_SET_ITER_INVALID` if the set is empty.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized set.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a `vl_set_iter` handle to the last element.
 *
 * \param set set pointer
 * \par Complexity O(log(n)).
 * \return iterator of last element in the set.
 */
VL_API vl_set_iter vlSetBack(vl_set* set);

/**
 * Inserts the specified element into the set.
 * If the element already exists in the set, the existing iterator is returned.
 * Insertion does not invalidate existing iterators in the set.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The set maintains its own copy.
 * - **Lifetime**: Valid until removed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `set` must not be `NULL`. `elem` should not be `NULL`.
 * - **Error Conditions**: Returns `VL_SET_ITER_INVALID` if node allocation fails.
 * - **Undefined Behavior**: Passing an uninitialized set.
 * - **Memory Allocation Expectations**: May trigger expansion of the underlying node pool.
 * - **Return-value Semantics**: Returns a `vl_set_iter` handle to the element (either the newly inserted or the
 * existing equivalent one), or `VL_SET_ITER_INVALID`.
 *
 * \param set set pointer
 * \param elem element pointer
 * \par Complexity of O(log(n)).
 * \return iterator to inserted or existing element
 */
VL_API vl_set_iter vlSetInsert(vl_set* set, const void* elem);

/**
 * Returns the pointer to the element at the specified iterator in the set.
 * Undefined behavior occurs when iterator does not exist in the set, or is
 * VL_SET_ITER_INVALID.
 *
 * The pointer is returned as non-const based on the trust that the key by which
 * the set is ordered is not modified. Any supplemental information in the
 * element may be freely modified.
 *
 * ## Contract
 * - **Ownership**: Ownership remains with the set. The caller can modify supplemental data but must not modify the
 * ordering key or free the pointer.
 * - **Lifetime**: Valid until the element is removed or the set is reallocated/destroyed.
 * - **Thread Safety**: Safe for concurrent reads if no thread is writing. Not thread-safe for concurrent writes to the
 * same element.
 * - **Nullability**: Returns `NULL` if `iter` is `VL_SET_ITER_INVALID`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a pointer to the raw element data.
 *
 * \param set set pointer
 * \param iter element iterator
 * \par Complexity O(1) constant.
 * \return pointer to element data
 */
VL_API void* vlSetSample(vl_set* set, vl_set_iter iter);

/**
 * Returns the iterator to the next element in the set
 * relative to the specified iterator, as if the set tree is
 * being iterated by inorder traversal.
 * If there is no next element, this will return VL_SET_ITER_INVALID.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Valid until the element is removed.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: Returns `VL_SET_ITER_INVALID` if no more elements exist.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the next `vl_set_iter` in the sequence.
 *
 * \param set set pointer
 * \param iter set iterator
 * \par Complexity O(log(n)).
 * \return iterator to next element relative to input iter.
 */
VL_API vl_set_iter vlSetNext(vl_set* set, vl_set_iter iter);

/**
 * Returns the iterator to the previous element in the set
 * relative to the specified iterator, as if the set tree is
 * being iterated by reverse inorder traversal.
 * If there is no previous element, this will return VL_SET_ITER_INVALID.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Valid until the element is removed.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: Returns `VL_SET_ITER_INVALID` if no previous element exists.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the previous `vl_set_iter` in the sequence.
 *
 * \param set set pointer
 * \param iter set iterator
 * \par Complexity O(log(n)).
 * \return iterator to previous element relative to input iter.
 */
VL_API vl_set_iter vlSetPrev(vl_set* set, vl_set_iter iter);

/**
 * Removes the specified iterator from the set.
 * Undefined behavior if element does not exist in the set.
 * Removal of an element does not invalidate other iterators in the set.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the node back to the internal pool.
 * - **Lifetime**: The passed iterator becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `set` must not be `NULL`. `iter` should not be `VL_SET_ITER_INVALID`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator or an iterator from a different set.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param set pointer to set
 * \param iter set iterator
 * \par Complexity O(log(n)).
 */
VL_API void vlSetRemove(vl_set* set, vl_set_iter iter);

/**
 * Removes the specified element from the set.
 * Undefined behavior if element does not exist in the set.
 * Removal of an element does not invalidate other iterators in the set.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the node back to the internal pool.
 * - **Lifetime**: Iterators to the removed element become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `set` must not be `NULL`. `elem` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized set or an element not in the set.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param set pointer to set
 * \param elem pointer to element.
 * \par Complexity O(log(n)).
 */
VL_API void vlSetRemoveElem(vl_set* set, const void* elem);

/**
 * Clears the specified set.
 * Does not modify buffer memory associated with the set, but rather resets some
 * variables used to track state.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: All previously returned iterators and sampled pointers become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `set` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized set.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param set pointer to set
 * \par Complexity O(1).
 */
VL_API void vlSetClear(vl_set* set);

/**
 * \brief Clones the specified set to another.
 *
 * Clones the entirety of the src set to the dest set, including all elements and their order.
 *
 * The 'src' set pointer must be non-null and initialized.
 * The 'dest' set pointer may be null, but if it is not null it must be
 * initialized.
 *
 * If the 'dest' set pointer is null, a new set is created via vlSetNew.
 * Otherwise, its element size is set to the source's and all of its existing
 * data is replaced.
 *
 * ## Contract
 * - **Ownership**: If `dest` is `NULL`, the caller owns the returned `vl_set`. If `dest` is provided, ownership remains
 * with the caller.
 * - **Lifetime**: Valid until deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` must not be `NULL`. `dest` can be `NULL`.
 * - **Error Conditions**: Returns `NULL` if allocation fails.
 * - **Undefined Behavior**: Passing an uninitialized set.
 * - **Memory Allocation Expectations**: May allocate a new set struct and multiple nodes.
 * - **Return-value Semantics**: Returns the pointer to the cloned set, or `NULL` on failure.
 *
 * \param src pointer
 * \param dest pointer
 * \return pointer to set that was copied to or created.
 */
VL_API vl_set* vlSetClone(const vl_set* src, vl_set* dest);

/**
 * \brief Copies a range of elements from one set to another.
 *
 * Both the src set and the dest set must have equivalent element sizes,
 * otherwise this is a no-op. Similarly, both sets must also have matching
 * comparators.
 *
 * This is an inclusive range, and as such, the elements referred to by the
 * begin and end iterators are also copied to the target set.
 *
 * The begin and end iterators are expected to be in logical iterative order,
 * meaning that if iterating through the entire src set, begin would be found
 * before end.
 *
 * ## Contract
 * - **Ownership**: Unchanged. `dest` maintains copies of the elements.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` and `dest` must not be `NULL`.
 * - **Error Conditions**: Returns 0 if element sizes or comparators do not match.
 * - **Undefined Behavior**: Passing iterators from the wrong set or in the wrong order.
 * - **Memory Allocation Expectations**: May trigger node pool expansion in `dest`.
 * - **Return-value Semantics**: Returns the total number of elements successfully copied.
 *
 * \param src source buffer pointer
 * \param begin iterator to start the copy at, or VL_SET_ITER_INVALID for the
 * beginning of the list.
 * \param end iterator to end the copy after, or VL_SET_ITER_INVALID for the end
 * of the list.
 * \param dest destination buffer pointer
 * \par Complexity of O(nlog(n)) log-linear.
 * \return number of elements copied
 */
VL_API int vlSetCopy(vl_set* src, vl_set_iter begin, vl_set_iter end, vl_set* dest);

/**
 * Searches for the specified element in the set.
 * Returns VL_SET_ITER_INVALID if element is not found.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `set` must not be `NULL`. `elem` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_SET_ITER_INVALID` if not found.
 * - **Undefined Behavior**: Passing an uninitialized set.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a `vl_set_iter` handle to the element, or `VL_SET_ITER_INVALID`.
 *
 * \param set pointer to set
 * \param elem pointer to element
 * \par Complexity O(log(n)).
 * \return iterator of found element, or VL_SET_ITER_INVALID.
 */
VL_API vl_set_iter vlSetFind(vl_set* set, const void* elem);

/**
 * \brief Computes the union between sets A and B, stored in set dest.
 *
 * Unions consist of all elements that reside in either A and B.
 * Consider this equivalent to bitwise OR.
 *
 * This function has computational relevance to set theory.
 *
 * Both A and B sets must have identical element sizes and comparators.
 * Otherwise, this is a no-op and will return null.
 *
 * The 'dest' set pointer may be null, but if it is not null it must be
 * initialized. If the 'dest' set pointer is null, a new set is created via
 * vlSetNew.
 *
 * The set notation for this operation is: A u B
 *
 * ## Contract
 * - **Ownership**: Same as `vlSetClone` for `dest`.
 * - **Lifetime**: Valid until deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `a` and `b` must not be `NULL`. `dest` can be `NULL`.
 * - **Error Conditions**: Returns `NULL` if element sizes or comparators do not match, or if allocation fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: May allocate a new set struct and multiple nodes.
 * - **Return-value Semantics**: Returns the pointer to the union set (`dest` or a new instance), or `NULL`.
 *
 * \param a first set
 * \param b second set
 * \param dest destination set
 * \par Complexity of O(nlog(n)) log-linear.
 * \return pointer to set that was copied to or created, or NULL on mismatched
 * sets.
 */
VL_API vl_set* vlSetUnion(vl_set* a, vl_set* b, vl_set* dest);

/**
 * \brief Computes the intersection between sets A and B, stored in set dest.
 *
 * Intersections consist of all elements that reside in both A and B,
 * and ONLY elements that exist in both A and B.
 * Consider this equivalent to bitwise AND.
 *
 * This function has computational relevance to set theory.
 *
 * Both A and B sets must have identical element sizes and comparators.
 * Otherwise, this is a no-op and will return null.
 *
 * The 'dest' set pointer may be null, but if it is not null it must be
 * initialized. If the 'dest' set pointer is null, a new set is created via
 * vlSetNew.
 *
 * The set notation for this operation is: A n B
 *
 * ## Contract
 * - **Ownership**: Same as `vlSetUnion`.
 * - **Lifetime**: Valid until deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Same as `vlSetUnion`.
 * - **Error Conditions**: Same as `vlSetUnion`.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Same as `vlSetUnion`.
 * - **Return-value Semantics**: Returns the pointer to the intersection set, or `NULL`.
 *
 * \param a first set
 * \param b second set
 * \param dest destination set
 * \par Complexity of O(nlog(n)) log-linear.
 * \return pointer to set that was copied to or created, or NULL on mismatched
 * sets.
 */
VL_API vl_set* vlSetIntersection(vl_set* a, vl_set* b, vl_set* dest);

/**
 * \brief Compute the difference between sets A and B, stored in set dest.
 *
 * Differences consist of all elements that reside in either A and B,
 * but NOT elements that exist in both A and B.
 * Consider this equivalent to bitwise XOR.
 *
 * This function has computational relevance to set theory.
 *
 * Both A and B sets must have identical element sizes and comparators.
 * Otherwise, this is a no-op and will return null.
 *
 * The 'dest' set pointer may be null, but if it is not null it must be
 * initialized. If the 'dest' set pointer is null, a new set is created via
 * vlSetNew.
 *
 * The set notation for this operation is: A - B
 *
 * ## Contract
 * - **Ownership**: Same as `vlSetUnion`.
 * - **Lifetime**: Valid until deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Same as `vlSetUnion`.
 * - **Error Conditions**: Same as `vlSetUnion`.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Same as `vlSetUnion`.
 * - **Return-value Semantics**: Returns the pointer to the difference set, or `NULL`.
 *
 * \param a first set
 * \param b second set
 * \param dest destination set
 * \par Complexity of O(nlog(n)) log-linear.
 * \return pointer to set that was copied to or created, or NULL on mismatched
 * sets.
 */
VL_API vl_set* vlSetDifference(vl_set* a, vl_set* b, vl_set* dest);

#endif // VL_SET_H
