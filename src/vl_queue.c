#include "vl_queue.h"
#include <stdlib.h>
#include <memory.h>
#include "vl_memory.h"

/**
 * Queues have a very simple header, which is just a single index to the next element.
 * As such, they don't really have a more advanced structure for their management.
 * \private
 */
typedef vl_pool_idx vl_queue_header;

void vlQueueInit(vl_queue *queue, vl_uint16_t elementSize) {
    vlPoolInit(&queue->nodes, elementSize + sizeof(vl_queue_header));
    queue->elementSize = elementSize;
    queue->head = VL_POOL_INVALID_IDX;
    queue->tail = VL_POOL_INVALID_IDX;
}

void vlQueueFree(vl_queue *queue) {
    vlPoolFree(&queue->nodes);
}

vl_queue *vlQueueNew(vl_uint16_t elementSize) {
    vl_queue *queue = malloc(sizeof(vl_queue));
    vlQueueInit(queue, elementSize);
    return queue;
}

void vlQueueDelete(vl_queue *queue) {
    vlQueueFree(queue);
    free(queue);
}

void vlQueueClear(vl_queue *queue) {
    vlPoolClear(&queue->nodes);
    queue->head = queue->tail = VL_POOL_INVALID_IDX;
}

vl_queue *vlQueueClone(const vl_queue *src, vl_queue *dest) {
    if (dest == NULL)
        dest = vlQueueNew(src->elementSize);

    vlPoolClone(&src->nodes, &dest->nodes);

    dest->head = src->head;
    dest->tail = src->tail;
    dest->elementSize = src->elementSize;

    return dest;
}

void vlQueueReserve(vl_queue *queue, vl_dsidx_t numElems) {
    vlPoolReserve(&queue->nodes, numElems);
}

void vlQueuePushBack(vl_queue *queue, const void *element) {
    const vl_pool_idx oldTail = queue->tail;
    const vl_pool_idx newTail = vlPoolTake(&queue->nodes);
    vl_transient *const newMem = vlPoolSample(&queue->nodes, newTail);
    *((vl_queue_header *const) newMem) = newTail;
    memcpy(newMem + sizeof(vl_queue_header), element, queue->elementSize);

    if (oldTail != VL_POOL_INVALID_IDX)
        *((vl_queue_header *) vlPoolSample(&queue->nodes, oldTail)) = newTail;

    if (queue->head == VL_POOL_INVALID_IDX)
        queue->head = newTail;

    queue->tail = newTail;
}

int vlQueuePopFront(vl_queue *queue, void *element) {
    if (queue->head == VL_POOL_INVALID_IDX)
        return 0;

    const vl_pool_idx target = queue->head;
    vl_queue_header *const header = (vl_queue_header *) (vlPoolSample(&queue->nodes, target));
    vl_transient *const data = (vl_transient *) (header + 1);

    if (queue->head == queue->tail) {
        queue->head = queue->tail = VL_POOL_INVALID_IDX;
    } else queue->head = *header;

    if (element)
        memcpy(element, data, queue->elementSize);

    vlPoolReturn(&queue->nodes, target);
    return 1;
}