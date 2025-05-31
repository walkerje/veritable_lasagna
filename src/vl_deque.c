#include "vl_deque.h"
#include <stdlib.h>
#include <memory.h>

/**
 * \brief Deque node header.
 * Analogous to a linked list element header.
 * Node data is stored immediately after.
 * \private
 */
typedef struct {
    vl_pool_idx prev;
    vl_pool_idx next;
} vl_deque_node;

void vlDequeInit(vl_deque *deq, vl_uint16_t elementSize) {
    vlPoolInit(&deq->nodes, elementSize + sizeof(vl_deque_node));
    deq->elementSize = elementSize;
    deq->head = VL_POOL_INVALID_IDX;
    deq->tail = VL_POOL_INVALID_IDX;
    deq->totalElements = 0;
}

void vlDequeFree(vl_deque *deq) {
    vlPoolFree(&deq->nodes);
    deq->head = VL_POOL_INVALID_IDX;
    deq->tail = VL_POOL_INVALID_IDX;
    deq->elementSize = 0;
    deq->totalElements = 0;
}

vl_deque *vlDequeNew(vl_uint16_t elementSize) {
    vl_deque *result = (vl_deque *) malloc(sizeof(vl_deque));
    vlDequeInit(result, elementSize);
    return result;
}

void vlDequeDelete(vl_deque *deq) {
    vlDequeFree(deq);
    free(deq);
}

void vlDequeClear(vl_deque *deq) {
    deq->head = VL_POOL_INVALID_IDX;
    deq->tail = VL_POOL_INVALID_IDX;
    deq->totalElements = 0;
    vlPoolClear(&deq->nodes);
}

void vlDequeReserve(vl_deque *deque, vl_dsidx_t numElems) {
    vlPoolReserve(&deque->nodes, numElems);
}

vl_deque *vlDequeClone(const vl_deque *src, vl_deque *dest) {
    if (dest == NULL)
        dest = vlDequeNew(src->elementSize);

    vlPoolClone(&src->nodes, &dest->nodes);
    dest->head = src->head;
    dest->tail = src->tail;
    dest->elementSize = src->elementSize;
    dest->totalElements = src->totalElements;

    return dest;
}

void vlDequePushFront(vl_deque *deq, const void *val) {
    const vl_pool_idx oldHead = deq->head;
    const vl_pool_idx newHead = vlPoolTake(&deq->nodes);

    void *dest = vlPoolSample(&deq->nodes, newHead);
    vl_deque_node *newNode = dest;
    dest = newNode + 1;

    memcpy(dest, val, deq->elementSize);

    newNode->prev = VL_POOL_INVALID_IDX;
    newNode->next = oldHead;

    if (oldHead != VL_POOL_INVALID_IDX) {
        vl_deque_node *oldNode = (vl_deque_node *) vlPoolSample(&deq->nodes, oldHead);
        oldNode->prev = newHead;
    }

    if (deq->tail == VL_POOL_INVALID_IDX) {
        deq->tail = newHead;
    }

    deq->head = newHead;
    deq->totalElements++;
}

int vlDequePopFront(vl_deque *deq, void *val) {
    if (deq->head == VL_POOL_INVALID_IDX)
        return 0;
    else if (deq->head == deq->tail) {
        vl_deque_node *oldHead = (vl_deque_node *) vlPoolSample(&deq->nodes, deq->head);
        memcpy(val, oldHead + 1, deq->elementSize);
        vlPoolReturn(&deq->nodes, deq->head);
        deq->head = deq->tail = VL_POOL_INVALID_IDX;
        deq->totalElements--;
        return 1;
    }

    vl_deque_node *oldNode = (vl_deque_node *) vlPoolSample(&deq->nodes, deq->head);

    if (val)
        memcpy(val, oldNode + 1, deq->elementSize);
    const vl_pool_idx rightIter = oldNode->next;

    vl_deque_node *rightNode = (vl_deque_node *) vlPoolSample(&deq->nodes, rightIter);
    rightNode->prev = VL_POOL_INVALID_IDX;

    deq->totalElements--;
    vlPoolReturn(&deq->nodes, deq->head);
    deq->head = rightIter;

    return 1;
}

void vlDequePushBack(vl_deque *deq, const void *val) {
    const vl_pool_idx oldTail = deq->tail;
    const vl_pool_idx newTail = vlPoolTake(&deq->nodes);

    void *dest = vlPoolSample(&deq->nodes, newTail);
    vl_deque_node *newNode = dest;
    dest = newNode + 1;

    memcpy(dest, val, deq->elementSize);
    newNode->next = VL_POOL_INVALID_IDX;
    newNode->prev = oldTail;

    if (oldTail != VL_POOL_INVALID_IDX) {
        vl_deque_node *oldNode = (vl_deque_node *) vlPoolSample(&deq->nodes, oldTail);
        oldNode->next = newTail;
    }

    if (deq->head == VL_POOL_INVALID_IDX)
        deq->head = newTail;

    deq->tail = newTail;
    deq->totalElements++;
}

int vlDequePopBack(vl_deque *deq, void *val) {
    if (deq->tail == VL_POOL_INVALID_IDX)
        return 0;
    else if (deq->head == deq->tail) {
        vl_deque_node *oldHead = (vl_deque_node *) vlPoolSample(&deq->nodes, deq->tail);
        memcpy(val, oldHead + 1, deq->elementSize);
        vlPoolReturn(&deq->nodes, deq->tail);
        deq->head = deq->tail = VL_POOL_INVALID_IDX;
        deq->totalElements--;
        return 1;
    }

    vl_deque_node *oldNode = (vl_deque_node *) vlPoolSample(&deq->nodes, deq->tail);

    if (val)
        memcpy(val, oldNode + 1, deq->elementSize);

    const vl_pool_idx leftIter = oldNode->prev;
    vl_deque_node *leftNode = (vl_deque_node *) vlPoolSample(&deq->nodes, leftIter);
    leftNode->next = VL_POOL_INVALID_IDX;

    vlPoolReturn(&deq->nodes, deq->tail);
    deq->tail = leftIter;
    deq->totalElements--;
    return 1;
}