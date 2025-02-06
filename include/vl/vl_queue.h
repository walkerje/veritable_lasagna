#ifndef VL_QUEUE_H
#define VL_QUEUE_H

#include "vl_linear_pool.h"

/**
 * \brief First in, first out queue.
 *
 * The Queue data structure is a simplified forward-facing linked list.
 * It is implemented on top of a pool allocator, and thus requires all elements in the queue to be the same size.
 *
 * Items may be added to the end and removed from the beginning.
 * Just like the vl_deque data structure, direct sampling is also disallowed. Thus, all IO requires a copy.
 * \sa vl_deque
 */
typedef struct{
    vl_linearpool         nodes;        //pool nodes
    vl_memsize_t    elementSize;        //size of a single element, in bytes.
    vl_linearpool_idx     head;         //first element
    vl_linearpool_idx     tail;         //last element
} vl_queue;

/**
 * \brief Initializes the specified queue.
 *
 * The queue should then later be freed via vlQueueFree.
 *
 * \sa vlQueueFree
 * \param queue pointer
 * \param elementSize size of a single queue element, in bytes
 * \par Complexity of O(1) constant.
 */
void        vlQueueInit(vl_queue* queue, vl_memsize_t elementSize);

/**
 * \brief Frees the specified queue.
 *
 * The queue should have been initialized via vlQueueInit.
 *
 * \sa vlQueueFree
 * \param queue pointer
 * \par Complexity of O(1) constant.
 */
void        vlQueueFree(vl_queue* queue);

/**
 * \brief Allocates on the heap, initializes, and returns a queue instance.
 *
 * The queue should then later be deleted via vlQueueDelete.
 *
 * \sa vlQueueDelete
 * \param elementSize size of a single queue element, in bytes
 * \par Complexity of O(1) constant.
 * \return pointer to queue
 */
vl_queue*   vlQueueNew(vl_memsize_t elementSize);

/**
 * \brief Deletes the specified queue.
 *
 * The queue should have been initialized via vlQueueNew.
 *
 * \sa vlQueueNew
 * \param queue pointer
 * \par Complexity of O(1) constant.
 */
void        vlQueueDelete(vl_queue* queue);

/**
 * \brief Clones the specified queue to another.
 *
 * Clones the entirety of the src queue to the dest queue.
 *
 * The 'src' queue pointer must be non-null and initialized.
 * The 'dest' queue pointer may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' queue pointer is null, a new queue is created via vlQueueNew.
 * Otherwise, its element size is set to the source's and all of its existing data is replaced.
 *
 * \param src pointer
 * \param dest pointer
 * \return pointer to queue that was copied to or created.
 */
vl_queue*        vlQueueClone(const vl_queue* src, vl_queue* dest);

/**
 * \brief Reserves space for n-many elements in the underlying buffer of the specified queue.
 *
 * This is done by doubling the size until the requested growth is met or exceeded.
 * This function will always result in the reallocation of the underlying memory.
 *
 * \param queue pointer
 * \param n total number of elements to reserve space for.
 */
void vlQueueReserve(vl_queue* queue, vl_memsize_t n);

/**
 * \brief Clears the specified queue.
 *
 * The underlying data in the queue is untouched, but rather some book-keeping
 * variables are reset.
 *
 * \param queue
 * \par Complexity of O(1) constant.
 */
void        vlQueueClear(vl_queue* queue);

/**
 * \brief Adds a new element to the end of the queue.
 *
 * The element data is copied to the queue.
 *
 * \param queue pointer
 * \param element data pointer
 * \par Complexity of O(1) constant.
 */
void        vlQueuePushBack(vl_queue* queue, const void* element);

/**
 * \brief Copies the first element in the queue, and removes it from the queue.
 *
 * This is a no-op if the queue is empty.
 *
 * \param queue pointer
 * \param element data pointer
 * \par Complexity of O(1) constant.
 * \return 1 if an element was copied and removed, 0 otherwise.
 */
int         vlQueuePopFront(vl_queue* queue, void* element);

/**
 * \brief Returns the total number of elements in the specified queue.
 * \param queue pointer
 * \par Complexity of O(1) constant.
 * \return size of the queue
 */
vl_dsidx_t      vlQueueSize(vl_queue* queue);

#endif //VL_QUEUE_H
