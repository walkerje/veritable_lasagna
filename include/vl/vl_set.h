#ifndef VL_SET_H
#define VL_SET_H

#include "vl_linear_pool.h"
#include "vl_compare.h"

#define VL_SET_ITER_INVALID VL_STRUCTURE_INDEX_MAX

/**
 * Convenience macro for forward iterating over the entirety of a set.
 * Implements a for-loop code structure with some of the boilerplate hidden away.
 * \param set pointer
 * \param trackVar name of the tracking variable used as the set iterator
 */
#define VL_SET_FOREACH(set, trackVar) for(vl_set_iter trackVar = vlSetFront(set); (trackVar) != VL_SET_ITER_INVALID; (trackVar) = vlSetNext(set, trackVar))

/**
 * Convenience macro for reverse iterating over the entirety of a set.
 * Implements a for-loop code structure with some of the boilerplate hidden away.
 * \param set pointer
 * \param trackVar name of the tracking variable used as the set iterator
 */
#define VL_SET_FOREACH_REVERSE(set, trackVar) for(vl_set_iter trackVar = vlSetBack(set); (trackVar) != VL_SET_ITER_INVALID; (trackVar) = vlSetPrev(set, trackVar))

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

typedef vl_linearpool_idx vl_set_iter;

/**
 * \brief An ordered set.
 *
 * The vl_set structure represents an ordered, unique set of values sorted according to
 * a supplied comparator. This is implemented using a red/black binary tree, a separate
 * self-balancing structure which guarantees worse-case search, removal, and insertion
 * complexity of O(log(n)).
 *
 * This is another abstraction of a vl_linearpool instance, which maintains an underlying buffer
 * containing all data for the structure.
 *
 * Elements of the set are sorted according to a key, but can also hold supplementary data
 * for each element that is stored in the same block of memory.
 */
typedef struct{
    vl_linearpool                 nodePool;     //node pool which holds all the data in the set.
    vl_memsize_t            elementSize;        //size of each set element, in bytes.
    vl_compare_function     comparator;         //comparator function pointer. see vl_compare.

    vl_set_iter             root;               //root iterator. may change upon insert/remove operations.
} vl_set;

/**
 * Initializes the specified set pointer to hold elements of the specified size.
 * This set should be freed via vlSetFree.
 * \param set set pointer
 * \param elementSize element size, in bytes.
 * \param compFunc comparator function; 0 = same, >0 = greater, <0 = lesser.
 * \sa vlSetFree
 * \par Complexity O(1) constant.
 */
void            vlSetInit(vl_set* set, vl_memsize_t elementSize, vl_compare_function compFunc);

/**
 * Frees the underlying storage buffers for the specified set.
 * Should only be used on sets initialized via vlSetInit.
 * \param set set pointer
 * \sa vlSetInit
 * \par Complexity O(1) constant.
 */
void            vlSetFree(vl_set* set);

/**
 * Allocates a new set on the heap.
 * Initializes the specified set pointer to hold elements of the specified size.
 * This set should be freed via vlSetDelete.
 * \param elementSize element size, in bytes
 * \param compFunc comparator function; 0 = same, >0 = greater, <0 = lesser.
 * \return set pointer
 * \sa vlSetDelete
 * \par Complexity O(1) constant.
 */
vl_set*         vlSetNew(vl_memsize_t elementSize, vl_compare_function compFunc);


/**
 * Deletes the specified set from the heap.
 * Frees the underlying storage buffers for the specified set.
 * Should only be used by sets initialized via vlSetNew.
 * \param set set pointer
 * \sa vlSetNew
 * \par Complexity O(1) constant.
 */
void            vlSetDelete(vl_set* set);

/**
 * Returns an iterator to the last element in the set.
 * \param set set pointer
 * \par Complexity O(log(n)).
 * \return iterator of last element in the set.
 */
vl_set_iter     vlSetFront(vl_set* set);

/**
 * Returns an iterator to the last element in the set.
 * \param set set pointer
 * \par Complexity O(log(n)).
 * \return iterator of last element in the set.
 */
vl_set_iter     vlSetBack(vl_set* set);

/**
 * Inserts the specified element into the set.
 * If the element already exists in the set, the existing iterator is returned.
 * Insertion does not invalidate existing iterators in the set.
 * \param set set pointer
 * \param elem element pointer
 * \par Complexity of O(log(n)).
 * \return iterator to inserted or existing element
 */
vl_set_iter     vlSetInsert(vl_set* set, const void* elem);

/**
 * Returns the pointer to the element at the specified iterator in the set.
 * Undefined behavior occurs when iterator does not exist in the set, or is VL_SET_ITER_INVALID.
 *
 * The pointer is returned as non-const based on the trust that the key by which the set
 * is ordered is not modified. Any supplemental information in the element may be freely modified.
 * \param set set pointer
 * \param iter element iterator
 * \par Complexity O(1) constant.
 * \return pointer to element data
 */
vl_transient*     vlSetSample(vl_set* set, vl_set_iter iter);

/**
 * Returns the iterator to the previous element in the set
 * relative to the specified iterator, as if the set tree is
 * being iterated by inorder traversal.
 * If there is no next element, (e.g, vlSetNext(&set, vlSetBack(&set)),
 * this will return VL_SET_ITER_INVALID.
 *
 * \param set set pointer
 * \param iter set iterator
 * \par Complexity O(log(n)).
 * \return iterator to next element relative to input iter.
 */
vl_set_iter     vlSetNext(vl_set* set, vl_set_iter iter);

/**
 * Returns the iterator to the previous element in the set
 * relative to the specified iterator, as if the set tree is
 * being iterated by reverse inorder traversal.
 * If there is no previous element, (e.g, vlSetPrev(&set, vlSetFront(&set)),
 * this will return VL_SET_ITER_INVALID.
 *
 * \param set set pointer
 * \param iter set iterator
 * \par Complexity O(log(n)).
 * \return iterator to previous element relative to input iter.
 */
vl_set_iter     vlSetPrev(vl_set* set, vl_set_iter iter);

/**
 * Removes the specified iterator from the set.
 * Undefined behavior if element does not exist in the set.
 * Removal of an element does not invalidate other iterators in the set.
 * \param set pointer to set
 * \param iter set iterator
 * \par Complexity O(log(n)).
 */
void            vlSetRemove(vl_set* set, vl_set_iter iter);

/**
 * Removes the specified element from the set.
 * Undefined behavior if element does not exist in the set.
 * Removal of an element does not invalidate other iterators in the set.
 * \param set pointer to set
 * \param elem pointer to element.
 * \par Complexity O(log(n)).
 */
void            vlSetRemoveElem(vl_set* set, const void* elem);

/**
 * Clears the specified set.
 * Does not modify buffer memory associated with the set, but rather resets some variables used to track state.
 * \param set pointer to set
 * \par Complexity O(1).
 */
void            vlSetClear(vl_set* set);

/**
 * \brief Clones the specified set to another.
 *
 * Clones the entirety of the src set to the dest set.
 *
 * The 'src' set pointer must be non-null and initialized.
 * The 'dest' set pointer may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' set pointer is null, a new list is created via vlSetNew.
 * Otherwise, its element size is set to the source's and all of its existing data is replaced.
 *
 * \param src pointer
 * \param dest pointer
 * \return pointer to set that was copied to or created.
 */
vl_set*         vlSetClone(const vl_set* src, vl_set* dest);

/**
 * \brief Copies a range of elements from one set to another.
 *
 * Both the src set and the dest set must have equivalent element sizes, otherwise this is a no-op.
 * Similarly, both sets must also have matching comparators.
 *
 * This is an inclusive range, and as such, the elements referred to by the begin and end iterators
 * are also copied to the target set.
 *
 * The begin and end iterators are expected to be in logical iterative order, meaning that if iterating through
 * the entire src set, begin would be found before end.
 *
 * \param src source buffer pointer
 * \param begin iterator to start the copy at, or VL_SET_ITER_INVALID for the beginning of the list.
 * \param end iterator to end the copy after, or VL_SET_ITER_INVALID for the end of the list.
 * \param dest destination buffer pointer
 * \par Complexity of O(nlog(n)) log-linear.
 * \return number of elements copied
 */
int             vlSetCopy(vl_set* src, vl_set_iter begin, vl_set_iter end, vl_set* dest);

/**
 * Searches for the specified element in the set.
 * Returns VL_SET_ITER_INVALID if element is not found.
 * \param set pointer to set
 * \param elem pointer to element
 * \par Complexity O(log(n)).
 * \return iterator of found element, or VL_SET_ITER_INVALID.
 */
vl_set_iter     vlSetFind(vl_set* set, const void* elem);

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
 * The 'dest' set pointer may be null, but if it is not null it must be initialized.
 * If the 'dest' set pointer is null, a new list is created via vlSetNew.
 *
 * The set notation for this operation is: A u B
 *
 * \param a first set
 * \param b second set
 * \param dest destination set
 * \par Complexity of O(nlog(n)) log-linear.
 * \return pointer to set that was copied to or created, or NULL on mismatched sets.
 */
vl_set*         vlSetUnion(vl_set* a, vl_set* b, vl_set* dest);

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
 * The 'dest' set pointer may be null, but if it is not null it must be initialized.
 * If the 'dest' set pointer is null, a new list is created via vlListNew.
 *
 * The set notation for this operation is: A n B
 *
 * \param a first set
 * \param b second set
 * \param dest destination set
 * \par Complexity of O(nlog(n)) log-linear.
 * \return pointer to set that was copied to or created, or NULL on mismatched sets.
 */
vl_set*         vlSetIntersection(vl_set* a, vl_set* b, vl_set* dest);

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
 * The 'dest' set pointer may be null, but if it is not null it must be initialized.
 * If the 'dest' set pointer is null, a new list is created via vlListNew.
 *
 * The set notation for this operation is: A - B
 *
 * \param a first set
 * \param b second set
 * \param dest destination set
 * \par Complexity of O(nlog(n)) log-linear.
 * \return pointer to set that was copied to or created, or NULL on mismatched sets.
 */
vl_set*         vlSetDifference(vl_set* a, vl_set* b, vl_set* dest);

#endif //VL_SET_H
