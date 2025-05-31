#include "vl_async_queue.h"
#include <malloc.h>
#include <string.h>

/**
 * Async queue node header.
 * \private
 */
typedef struct {
    vl_atomic_ptr next;
} vl_async_queue_node;

/**
 * \brief Sets the queue to an "initial" state with a dummy node.
 * \param queue pointer
 * \private
 */
static inline void vl_AsyncQueueSetDummy(vl_async_queue *queue) {
    //We can take and forget the pointer to the dummy node.
    //The async pool is capable of safely freeing all of its own memory.
    vl_async_queue_node *dummy = vlAsyncPoolTake(&queue->elements);
    vlAtomicInit(&dummy->next, VL_TAGPTR_NULL);

    vl_tagged_ptr init = {.ptr = (vl_uintptr_t) dummy, .tag = 0};
    vlAtomicInit(&queue->head, init);
    vlAtomicInit(&queue->tail, init);
}

void vlAsyncQueueInit(vl_async_queue *queue, vl_uint16_t elementSize) {
    queue->elementSize = elementSize;
    vlAsyncPoolInitAligned(&queue->elements, sizeof(vl_async_queue_node) + elementSize, VL_ATOMIC_PTR_ALIGN);
    vl_AsyncQueueSetDummy(queue);
    vlAtomicInit(&queue->size, 0);
}

void vlAsyncQueueFree(vl_async_queue *queue) {
    vlAsyncPoolFree(&queue->elements);
}

vl_async_queue *vlAsyncQueueNew(vl_uint16_t elementSize) {
    vl_async_queue *queue = malloc(sizeof(vl_async_queue));
    vlAsyncQueueInit(queue, elementSize);
    return queue;
}

void vlAsyncQueueDelete(vl_async_queue *queue) {
    vlAsyncQueueFree(queue);
    free(queue);
}

void vlAsyncQueueClear(vl_async_queue *queue) {
    vlAsyncPoolClear(&queue->elements);
    vl_AsyncQueueSetDummy(queue);
    vlAtomicStore(&queue->size, 0);
}

void vlAsyncQueueReset(vl_async_queue *queue) {
    vlAsyncPoolReset(&queue->elements);
    vl_AsyncQueueSetDummy(queue);
    vlAtomicStore(&queue->size, 0);
}

void vlAsyncQueuePushBack(vl_async_queue *queue, const void *value) {
    vl_async_queue_node *node = vlAsyncPoolTake(&queue->elements);
    memcpy(node + 1, value, queue->elementSize);
    vlAtomicInit(&node->next, VL_TAGPTR_NULL);

    while (VL_TRUE) {
        vl_tagged_ptr tail = vlAtomicLoad(&queue->tail);
        vl_async_queue_node *tailNode = (vl_async_queue_node *) tail.ptr;

        vl_tagged_ptr next = vlAtomicLoad(&tailNode->next);

        if (next.ptr == 0) {
            // Try to link the new node at the end
            vl_tagged_ptr expectedNext = next;
            if (vlAtomicPtrCompareExchangeWeak(&tailNode->next, &expectedNext, node)) {
                // Advance tail pointer to new node (optional)
                vlAtomicPtrCompareExchangeWeak(&queue->tail, &tail, node);
                vlAtomicFetchAdd(&queue->size, 1);
                return;
            }
        } else {
            // Tail was behind, advance it
            vlAtomicPtrCompareExchangeWeak(&queue->tail, &tail, (void *) next.ptr);
        }
    }
}

vl_bool_t vlAsyncQueuePopFront(vl_async_queue *queue, void *outValue) {
    while (VL_TRUE) {
        vl_tagged_ptr head = vlAtomicLoad(&queue->head);
        vl_tagged_ptr tail = vlAtomicLoad(&queue->tail);
        vl_async_queue_node *headNode = (vl_async_queue_node *) head.ptr;

        vl_tagged_ptr next = vlAtomicLoad(&headNode->next);
        vl_async_queue_node *nextNode = (vl_async_queue_node *) next.ptr;

        if (nextNode == NULL) {
            // Queue is empty
            return VL_FALSE;
        }

        // If head == tail, tail may be behind, so help move it forward
        if (head.ptr == tail.ptr) {
            vlAtomicPtrCompareExchangeWeak(&queue->tail, &tail, (void *) next.ptr);
        }

        // Try to swing head forward
        vl_tagged_ptr expectedHead = head;
        if (vlAtomicPtrCompareExchangeWeak(&queue->head, &expectedHead, (void *) next.ptr)) {
            memcpy(outValue, nextNode + 1, queue->elementSize);
            vlAsyncPoolReturn(&queue->elements, headNode);
            vlAtomicFetchSub(&queue->size, 1);
            return VL_TRUE;
        }
    }
}