#include "vl_queue.h"
#include <stdlib.h>
#include <memory.h>

/**
 * Queues have a very simple header, which is just a single index to the next element.
 * As such, they don't really have a more advanced structure for their management.
 * \private
 */
typedef vl_linearpool_idx vl_queue_header;

void vlQueueInit(vl_queue* queue, vl_memsize_t elementSize){
    vlLinearPoolInit(&queue->nodes, elementSize + sizeof(vl_queue_header));
    queue->elementSize = elementSize;
    queue->head = VL_POOL_INVALID_IDX;
    queue->tail = VL_POOL_INVALID_IDX;
}

void vlQueueFree(vl_queue* queue){
    vlLinearPoolFree(&queue->nodes);
}

vl_queue* vlQueueNew(vl_memsize_t elementSize){
    vl_queue* queue = malloc(sizeof(vl_queue));
    vlQueueInit(queue, elementSize);
    return queue;
}

void vlQueueDelete(vl_queue* queue){
    vlQueueFree(queue);
    free(queue);
}

void vlQueueClear(vl_queue* queue){
    vlLinearPoolClear(&queue->nodes);
    queue->head = queue->tail = VL_POOL_INVALID_IDX;
}

vl_queue* vlQueueClone(const vl_queue* src, vl_queue* dest){
    if(dest == NULL)
        dest = vlQueueNew(src->elementSize);

    vlLinearPoolClone(&src->nodes, &dest->nodes);

    dest->head = src->head;
    dest->tail = src->tail;
    dest->elementSize = src->elementSize;

    return dest;
}

void vlQueueReserve(vl_queue* queue, vl_memsize_t numElems){
    vlLinearPoolReserve(&queue->nodes, numElems);
}

void vlQueuePushBack(vl_queue* queue, const void* element){
    const vl_linearpool_idx       oldTail     =   queue->tail;
    const vl_linearpool_idx       newTail     = vlLinearPoolTake(&queue->nodes);
    vl_transient* const     newMem      =   vlLinearPoolSample(&queue->nodes, newTail);
    *((vl_queue_header* const)    newMem)     =   newTail;
    memcpy(newMem + sizeof(vl_queue_header), element, queue->elementSize);

    if(oldTail != VL_POOL_INVALID_IDX)
        *((vl_queue_header*) vlLinearPoolSample(&queue->nodes, oldTail)) = newTail;

    if(queue->head == VL_POOL_INVALID_IDX)
        queue->head = newTail;

    queue->tail = newTail;
}

int vlQueuePopFront(vl_queue* queue, void* element){
    if(queue->head == VL_POOL_INVALID_IDX)
        return 0;

    const vl_linearpool_idx       target      = queue->head;
    vl_queue_header* const  header      = (vl_queue_header*)(vlLinearPoolSample(&queue->nodes, target));
    vl_transient*    const  data        = (vl_transient*)(header + 1);

    if(queue->head == queue->tail){
        queue->head = queue->tail = VL_POOL_INVALID_IDX;
    }else queue->head = *header;

    if(element)
        memcpy(element, data, queue->elementSize);

    vlLinearPoolReturn(&queue->nodes, target);
    return 1;
}

vl_dsidx_t vlQueueSize(vl_queue* queue){
    return queue->nodes.totalTaken;
}