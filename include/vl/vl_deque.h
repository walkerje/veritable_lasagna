#ifndef VL_DEQUE_H
#define VL_DEQUE_H

#include "vl_linear_pool.h"

/**
 * \brief Double-ended queue.
 *
 * The Deque data structure is a doubly-linked list.
 * It is implemented on top of a pool allocator, and thus requires all elements in the queue to be the same size.
 *
 * Items may be added or removed from either end, but iteration is inherently disallowed.
 * Just like the vl_queue data structure, direct sampling is also disallowed. Thus, all IO requires a copy.
 * If you're using "weighty" (or, large in byte size) elements and are worried about the efficiency of copying,
 * storing pointers, iterators, or offsets in the queue is a perfectly valid approach.
 * \sa vl_queue
 */
typedef struct{
    vl_linearpool         nodes;          //pool nodes
    vl_memsize_t    elementSize;    //size of a single element, in bytes.
    vl_linearpool_idx     head;           //first element
    vl_linearpool_idx     tail;           //last element
} vl_deque;

/**
 * \brief Initializes the specified instance of vl_deque.
 *
 * The deque should later be freed via vlDequeFree.
 *
 * \sa vlDequeFree
 * \param deq pointer
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 */
void vlDequeInit(vl_deque* deq, vl_memsize_t elementSize);

/**
 * \brief Frees the specified instance of vl_deque.
 *
 * The deque should have been initialized via vlDequeInit.
 *
 * \sa vlDequeInit
 * \param deq pointer
 */
void vlDequeFree(vl_deque* deq);

/**
 * \brief Allocates, initializes, and returns an instance of vl_deque.
 *
 * The deque should later be freed via vlDequeFree.
 *
 * \sa vlDequeFree
 * \param deq pointer
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 * \return deque pointer
 */
vl_deque* vlDequeNew(vl_memsize_t elementSize);

/**
 * \brief De-initializes and deletes the specified instance of vl_deque.
 *
 * The deque should have been initialized via vlDequeNew.
 * \sa vlDequeNew
 * \param deq pointer
 * \par Complexity O(1) constant.
 */
void vlDequeDelete(vl_deque* deq);

/**
 * \brief Clears the specified deque.
 *
 * This does not actually touch any underlying element data, but does
 * reset some bookkeeping values.
 *
 * \param deq pointer
 * \par Complexity O(1) constant.
 */
void vlDequeClear(vl_deque* deq);

/**
 * \brief Reserves space for n-many elements in the underlying buffer of the specified deque.
 *
 * This is done by doubling the size until the requested growth is met or exceeded.
 * This function will always result in the reallocation of the underlying memory.
 *
 * \param deque pointer
 * \param n total number of elements to reserve space for.
 */
void vlDequeReserve(vl_deque* deque, vl_memsize_t n);

/**
 * \brief Clones the specified deque to another.
 *
 * Clones the entirety of the src deque to the dest deque.
 *
 * The 'src' deque pointer must be non-null and initialized.
 * The 'dest' deque pointer may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' deque pointer is null, a new list is created via vlListNew.
 * Otherwise, its element size is set to the source's and all of its existing data is replaced.
 *
 * \param src pointer
 * \param dest pointer
 * \return pointer to deque that was copied to or created.
 */
vl_deque* vlDequeClone(const vl_deque* src, vl_deque* dest);

/**
 * \brief Returns the total number of elements
 * \param deq pointer
 * \par Complexity O(1) constant.
 * \return total number of elements in the deque.
 */
vl_memsize_t vlDequeSize(vl_deque* deq);

/**
 * \brief Adds and copies an element to the front of the deque.
 * \param deq pointer
 * \param val element data pointer
 * \par Complexity O(1) constant.
 */
void vlDequePushFront(vl_deque* deq, const void* val);

/**
 * \brief Copies and removes an element from the front of the deque.
 *
 * If specified pointer val is NULL, the element is not copied, but the element is still removed.
 *
 * \param deq pointer
 * \param val pointer where the element will be copied to.
 * \par Complexity O(1) constant.
 * \return 1 if success, 0 if failed
 */
int vlDequePopFront(vl_deque* deq, void* val);

/**
 * \brief Adds and copies an en element to the end of the deque.
 * \param deq pointer
 * \param val element data pointer
 * \par Complexity O(1) constant.
 */
void vlDequePushBack(vl_deque* deq, const void* val);

/**
 * \brief Copies and removes an element from the end of the deque.
 *
 * If specified pointer val is NULL, the element is not copied, but the element is still removed.
 *
 * \param deq pointer
 * \param val pointer where the element will be copied to.
 * \return 1 if success, 0 if failed.
 */
int vlDequePopBack(vl_deque* deq, void* val);

#endif //VL_DEQUE_H