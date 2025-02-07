#ifndef VL_LIST_H
#define VL_LIST_H

#include "vl_fixed_pool.h"
#include "vl_compare.h"
#include "vl_memory.h"

#define VL_LIST_ITER_INVALID VL_FIXEDPOOL_INVALID_IDX

/**
 * This is a simple macro for iterating a list.
 * \param list pointer vl_list pointer
 * \param trackVar identifier of the iterator. Always of type vl_list_iter.
 * \sa vl_list_iter
 */
#define VL_LIST_FOREACH(list, trackVar) for(vl_list_iter trackVar = (list)->head; (trackVar) != VL_LIST_ITER_INVALID; (trackVar) = vlListNext(list, trackVar))

/**
* This is a simple macro for iterating a list, in reverse.
* \param list pointer vl_list pointer
* \param trackVar identifier of the iterator. Always of type vl_list_iter.
* \sa vl_list_iter
*/
#define VL_LIST_FOREACH_REVERSE(list, trackVar) for(vl_list_iter trackVar = (list)->tail; (trackVar) != VL_LIST_ITER_INVALID; (trackVar) = vlListPrev(list, trackVar))

/**
 * \brief List iterator type. Represents a location within a vl_linked_list.
 */
typedef vl_fixedpool_idx vl_list_iter;

/**
 * \brief A doubly-linked list.
 *
 * The vl_linked_list structure represents a doubly linked list with elements of a fixed size.
 * This is implemented on top of vl_fixedpool, which is used to manage underlying memory.
 * Due to using a pool, allocation should not occur frequently due to the larger pool
 * of memory which is sliced into smaller pieces for individual nodes.
 *
 * The vl_list_iter are simply vl_fixedpool indices, and thus,
 * surrounding iterators are not invalidated when removing or inserting elements.
 *
 * Pointers to list elements may never be invalidated due to using a fixed pool.
 *
 * \sa vl_fixedpool
 */
typedef struct{
    vl_fixedpool        nodePool;
    vl_memsize_t        elementSize;
    vl_list_iter        head;
    vl_list_iter        tail;
    vl_dsidx_t          length;
} vl_linked_list;

/**
 * \brief Initializes the specified list instance.
 *
 * The initialized list should be freed via vlListFree.
 *
 * \sa vlListFree
 * \param list pointer
 * \param elementSize size of a single list element, in bytes.
 * \par Complexity of O(1) constant.
 */
void            vlListInit(vl_linked_list* list, vl_memsize_t elementSize);

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
#define vlListFree(listPtr) vlFixedPoolFree(&((listPtr)->nodePool))

#endif

/**
 * \brief Allocates on the heap, initializes, and returns a list instance.
 * \param elementSize size of a single list element, in bytes.
 * \par Complexity of O(1) constant.
 * \return pointer to created list
 */
vl_linked_list* vlListNew(vl_memsize_t elementSize);

/**
 * \brief Deletes the specified list instance.
 *
 * The specified list should be created via vlListNew.
 *
 * \sa vlListNew
 * \param list pointer
 * \par Complexity of O(1) constant.
 */
void            vlListDelete(vl_linked_list* list);

/**
 * \brief Adds a new element to the front of the list.
 *
 * Element data is copied to the internal pool allocator.
 *
 * \param list pointer
 * \param elem pointer to element data
 * \par Complexity of O(1) constant.
 * \return iterator referring to added element
 */
vl_list_iter    vlListPushFront(vl_linked_list* list, const void* elem);

/**
 * \brief Removes whatever element is at the front of the list.
 *
 * This is a no-op if the list is empty.
 *
 * \param list pointer
 * \par Complexity of O(1) constant.
 */
void            vlListPopFront(vl_linked_list* list);

/**
 * \brief Adds a new element to the end of the list.
 *
 * Element data is copied to the internal pool allocator.
 *
 * \param list pointer
 * \param elem pointer to element data
 * \par Complexity of O(1) constant.
 * \return iterator referring to added element
 */
vl_list_iter    vlListPushBack(vl_linked_list* list, const void* elem);

/**
 * \brief Removes whatever element is at the end of the list.
 *
 * This is a no-op if the list is empty.
 *
 * \param list pointer
 * \par Complexity of O(1) constant.
 */
void            vlListPopBack(vl_linked_list* list);

/**
 * \brief Inserts an element immediately after the specified target.
 *
 * Element data is copied to the internal pool allocator.
 *
 * \param list pointer
 * \param target iterator to element that will have something inserted after it.
 * \param elem pointer
 * \par Complexity of O(1) constant.
 * \return iterator to inserted element.
 */
vl_list_iter    vlListInsertAfter(vl_linked_list* list, vl_list_iter target, const void* elem);

/**
 * \brief Inserts an element immediately before the specified target
 *
 * Element data is copied to the internal pool allocator.
 *
 * \param list pointer
 * \param target iterator to element that will have something inserted before it.
 * \param elem
 * \par Complexity of O(1) constant.
 * \return iterator to inserted element.
 */
vl_list_iter    vlListInsertBefore(vl_linked_list* list, vl_list_iter target, const void* elem);

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
#define vlListReserve(listPtr, n)   vlFixedPoolReserve(&((listPtr)->nodePool))
#endif

#ifndef vlListClear
/**
 * \brief Clears the specified list so it can be used as if it was just initialized.
 *
 * This function does not touch any information in the underlying buffer, but rather
 * resets some book-keeping variables.
 *
 * \param list pointer
 * \par Complexity of O(1) constant.
 */
#define vlListClear(listPtr)   (((listPtr)->head = (listPtr)->tail = VL_LIST_ITER_INVALID));\
                                    vlFixedPoolClear(&((listPtr)->nodePool))
#endif

/**
 * \brief Clones the specified list to another.
 *
 * Clones the entirety of the src list to the dest list.
 *
 * The 'src' list pointer must be non-null and initialized.
 * The 'dest' list pointer may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' list pointer is null, a new list is created via vlListNew.
 * Otherwise, its element size is set to the source's and all of its existing data is replaced.
 *
 * \param src
 * \param dest
 * \return pointer to list that was copied to or created.
 */
vl_linked_list* vlListClone(const vl_linked_list* src, vl_linked_list* dest);

/**
 * \brief Copies a range of elements from one list to another.
 *
 * Both the src list and the dest list must have equivalent element sizes, otherwise this is a no-op.
 *
 * This is an inclusive range, and as such, the elements referred to by the begin and end iterators
 * are also copied to the target list.
 *
 * The begin and end iterators are expected to be in logical iterative order, meaning that if iterating through
 * the entire src list, begin would be found before end.
 *
 * \param src source list pointer
 * \param begin iterator to start the copy at, or VL_LIST_ITER_INVALID for the beginning of the list.
 * \param end iterator to end the copy after, or VL_LIST_ITER_INVALID for the end of the list.
 * \param dest destination buffer pointer
 * \param after the iterator to insert elements after, or VL_LIST_ITER_INVALID for the end of the destination.
 * \par Complexity of O(n) linear.
 * \return number of elements copied
 */
int             vlListCopy(vl_linked_list* src, vl_list_iter begin, vl_list_iter end, vl_linked_list* dest, vl_list_iter after);

/**
 * \brief Sorts the specified list in-place using the given comparator.
 *
 * This function implements an iterative merge sort. Elements are not copied in this operation,
 * but rather the links between them are modified.
 *
 * The list is split into a "forest" in the underlying node pool, briefly holding a variety
 * of sub-lists which are later merged back together.
 *
 * \param src source list pointer
 * \param cmp comparator function
 * \par Complexity of O(n log(n)).
 */
void            vlListSort(vl_linked_list* src, vl_compare_function cmp);

/**
 * \brief Performs an iterative search on the specified list.
 *
 * This will return an iterator to the first instance of the specified element that was found.
 *
 * Returns VL_LIST_ITER_INVALID if the element could not be found.
 *
 * \param src source list pointer
 * \param element pointer to the element to compare against
 * \par Complexity of O(n) linear.
 * \return iterator to found element, or VL_LIST_ITER_INVALID on failure.
 */
vl_list_iter    vlListFind(vl_linked_list* src, const void* element);

/**
 * \brief Removes the specified element from the list.
 *
 * The underlying node is returned to the underlying node pool for reuse, without modifying its discarded data.
 *
 * \param list pointer
 * \param iter node iterator.
 * \par Complexity of O(1) constant.
 */
void            vlListRemove(vl_linked_list* list, vl_list_iter iter);

/**
 * \brief Returns next adjacent iterator, or VL_LIST_ITER_INVALID if no such element exists.
 *
 * \sa vlListPrev
 * \param list pointer
 * \param iter node iterator
 * \par Complexity of O(1) constant.
 * \return next iterator, or VL_LIST_ITER_INVALID.
 */
vl_list_iter    vlListNext(vl_linked_list* list, vl_list_iter iter);

/**
 * \brief Returns the previous adjacent iterator, or VL_LIST_ITER_INVALID if no such element exists.
 *
 * \sa vlListPrev
 * \param list pointer
 * \param iter node iterator
 * \par Complexity of O(1) constant.
 * \return previous iterator, or VL_LIST_ITER_INVALID.
 */
vl_list_iter    vlListPrev(vl_linked_list* list, vl_list_iter iter);

/**
 * \brief Returns a pointer to the element data for the specified iterator.
 *
 * \param list pointer
 * \param iter node iterator
 * \par Complexity of O(1) constant.
 * \return pointer to element data.
 */
vl_transient*     vlListSample(vl_linked_list* list, vl_list_iter iter);

#endif //VL_LIST_H
