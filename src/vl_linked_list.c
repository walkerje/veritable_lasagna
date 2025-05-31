#include "vl_linked_list.h"

#include <stdlib.h>
#include <memory.h>

/**
 * \brief Linked list node header.
 * Node data is stored immediately after.
 * \private
 */
typedef struct {
    vl_pool_idx prev;
    vl_pool_idx next;
} vl_linked_list_node;

void vlListInit(vl_linked_list *list, vl_uint16_t elementSize) {
    vlPoolInit(&list->nodePool, elementSize + sizeof(vl_linked_list_node));
    list->head = list->tail = VL_LIST_ITER_INVALID;
    list->elementSize = elementSize;
    list->length = 0;
}

vl_linked_list *vlListNew(vl_uint16_t elementSize) {
    vl_linked_list *list = malloc(sizeof(vl_linked_list));
    vlListInit(list, elementSize);
    return list;
}

void vlListDelete(vl_linked_list *list) {
    vlListFree(list);
    free(list);
}

vl_list_iter vlListPushFront(vl_linked_list *list, const void *elem) {
    vl_list_iter oldHead = list->head;
    vl_list_iter newHead = vlPoolTake(&list->nodePool);

    void *dest = vlPoolSample(&list->nodePool, newHead);
    vl_linked_list_node *newNode = dest;
    dest = newNode + 1;

    memcpy(dest, elem, list->elementSize);

    newNode->prev = VL_LIST_ITER_INVALID;
    newNode->next = oldHead;

    if (oldHead != VL_LIST_ITER_INVALID) {
        vl_linked_list_node *oldNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, oldHead);
        oldNode->prev = newHead;
    }

    if (list->tail == VL_LIST_ITER_INVALID)
        list->tail = newHead;

    list->head = newHead;
    list->length++;
    return newHead;
}

void vlListPopFront(vl_linked_list *list) {
    if (list->head == VL_LIST_ITER_INVALID)
        return;
    else if (list->head == list->tail) {
        vlPoolReturn(&list->nodePool, list->head);
        list->head = list->tail = VL_LIST_ITER_INVALID;
        return;
    }

    vl_linked_list_node *oldNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, list->head);
    const vl_list_iter rightIter = oldNode->next;

    vl_linked_list_node *rightNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, rightIter);
    rightNode->prev = VL_LIST_ITER_INVALID;

    vlPoolReturn(&list->nodePool, list->head);
    list->head = rightIter;
    list->length--;
}

vl_list_iter vlListPushBack(vl_linked_list *list, const void *elem) {
    const vl_list_iter oldTail = list->tail;
    const vl_list_iter newTail = vlPoolTake(&list->nodePool);

    void *dest = vlPoolSample(&list->nodePool, newTail);
    vl_linked_list_node *newNode = dest;
    dest = newNode + 1;

    memcpy(dest, elem, list->elementSize);
    newNode->next = VL_LIST_ITER_INVALID;
    newNode->prev = oldTail;

    if (oldTail != VL_LIST_ITER_INVALID) {
        vl_linked_list_node *oldNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, oldTail);
        oldNode->next = newTail;
    }

    if (list->head == VL_LIST_ITER_INVALID)
        list->head = newTail;

    list->tail = newTail;
    list->length++;
    return newTail;
}

void vlListPopBack(vl_linked_list *list) {
    if (list->tail == VL_LIST_ITER_INVALID)
        return;
    else if (list->head == list->tail) {
        vlPoolReturn(&list->nodePool, list->tail);
        list->head = list->tail = VL_LIST_ITER_INVALID;
        return;
    }

    vl_linked_list_node *oldNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, list->tail);
    const vl_list_iter leftIter = oldNode->prev;

    vl_linked_list_node *leftNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, leftIter);
    leftNode->next = VL_LIST_ITER_INVALID;

    vlPoolReturn(&list->nodePool, list->tail);
    list->tail = leftIter;
    list->length--;
}

vl_list_iter vlListInsertAfter(vl_linked_list *list, vl_list_iter target, const void *elem) {
    vl_linked_list_node *leftNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, target);
    if (leftNode->next == VL_LIST_ITER_INVALID)
        return vlListPushBack(list, elem);

    vl_linked_list_node *rightNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, leftNode->next);

    vl_list_iter newIter = vlPoolTake(&list->nodePool);
    vl_linked_list_node *newNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, newIter);

    void *dest = newNode + 1;
    memcpy(dest, elem, list->elementSize);

    newNode->next = leftNode->next;
    leftNode->next = newIter;
    newNode->prev = target;
    rightNode->prev = newIter;
    list->length++;

    return newIter;
}

vl_list_iter vlListInsertBefore(vl_linked_list *list, vl_list_iter target, const void *elem) {
    vl_linked_list_node *rightNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, target);
    if (rightNode->prev == VL_LIST_ITER_INVALID)
        return vlListPushFront(list, elem);

    vl_linked_list_node *leftNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, rightNode->prev);

    vl_list_iter newIter = vlPoolTake(&list->nodePool);
    vl_linked_list_node *newNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, newIter);

    void *dest = newNode + 1;
    memcpy(dest, elem, list->elementSize);

    newNode->next = target;
    leftNode->next = newIter;
    newNode->prev = rightNode->prev;
    rightNode->prev = newIter;
    list->length++;

    return newIter;
}

vl_linked_list *vlListClone(const vl_linked_list *src, vl_linked_list *dest) {
    if (dest == NULL)
        dest = vlListNew(src->elementSize);

    vlPoolClone(&src->nodePool, &dest->nodePool);
    dest->head = src->head;
    dest->tail = src->tail;
    dest->elementSize = src->elementSize;
    dest->length = src->length;

    return dest;
}

int vlListCopy(vl_linked_list *src, vl_list_iter begin, vl_list_iter end, vl_linked_list *dest, vl_list_iter after) {
    if (src->elementSize != dest->elementSize)
        return 0;

    int totalCopied = 0;

    if (begin == VL_LIST_ITER_INVALID)
        begin = src->head;

    if (end == VL_LIST_ITER_INVALID)
        end = src->tail;

    vl_list_iter curIter = begin;
    vl_list_iter insertIter = after;

    while (curIter != VL_LIST_ITER_INVALID) {
        insertIter = vlListInsertAfter(dest, insertIter, vlListSample(src, curIter));
        totalCopied++;

        if (curIter == end)
            break;
        curIter = vlListNext(src, curIter);
    }

    return totalCopied;
}

/**
 * \brief Iteratively traverses a linked list to determine its size and last node.
 * \param list pointer
 * \param begin head of the list
 * \param stopIter (output if non-null) iterator to the last element in the list
 * \private
 * \return length of list segment
 */
vl_dsidx_t vl_ListSortSegmentLength(vl_linked_list *list, vl_list_iter begin, vl_list_iter *stopIter) {
    if (begin == VL_LIST_ITER_INVALID)
        return 0;

    vl_dsidx_t length = 0;
    vl_list_iter current = begin, lastIter = begin;
    vl_linked_list_node *currentHeader;

    while (current != VL_LIST_ITER_INVALID) {
        lastIter = current;
        currentHeader = (vl_linked_list_node *) vlPoolSample(&list->nodePool, current);
        current = currentHeader->next;
        length++;
    }

    if (stopIter != NULL)
        *stopIter = lastIter;

    return length;
}

/**
 * \brief Merges the two specified sub-lists together in sorted order.
 *
 * \param list list pointer
 * \param aIter iterator to head of sub-list A
 * \param bIter iterator to head of sub-list B.
 * \param mergeHead (output) iterator to head of resulting list.
 * \param mergeTail (output) iterator to tail of resulting list.
 * \param compareFunction comparator function
 * \private
 */
void vl_ListSortMerge(vl_linked_list *list, vl_list_iter aIter, vl_list_iter bIter,
                      vl_list_iter *mergeHead, vl_list_iter *mergeTail,
                      vl_compare_function compareFunction) {
    if (aIter == VL_LIST_ITER_INVALID) {
        *mergeHead = bIter;
        vl_ListSortSegmentLength(list, bIter, mergeTail);
        return;
    }

    if (bIter == VL_LIST_ITER_INVALID) {
        *mergeHead = aIter;
        vl_ListSortSegmentLength(list, aIter, mergeTail);
        return;
    }

    vl_linked_list_node *leftHeader = (vl_linked_list_node *) vlPoolSample(&list->nodePool, aIter);
    vl_linked_list_node *rightHeader = (vl_linked_list_node *) vlPoolSample(&list->nodePool, bIter);
    vl_linked_list_node *tailHeader;
    void *leftData = leftHeader + 1;
    void *rightData = rightHeader + 1;

    if (compareFunction(leftData, rightData) <= 0) {
        //left < right
        *mergeHead = aIter;
        aIter = leftHeader->next;
    } else {
        //right < left
        *mergeHead = bIter;
        bIter = rightHeader->next;
    }

    *mergeTail = *mergeHead;

    while (aIter != VL_LIST_ITER_INVALID && bIter != VL_LIST_ITER_INVALID) {
        leftHeader = (vl_linked_list_node *) vlPoolSample(&list->nodePool, aIter);
        rightHeader = (vl_linked_list_node *) vlPoolSample(&list->nodePool, bIter);
        tailHeader = (vl_linked_list_node *) vlPoolSample(&list->nodePool, *mergeTail);

        if (compareFunction(leftHeader + 1, rightHeader + 1) <= 0) {
            tailHeader->next = aIter;
            leftHeader->prev = *mergeTail;
            aIter = leftHeader->next;
        } else {
            tailHeader->next = bIter;
            rightHeader->prev = *mergeTail;
            bIter = rightHeader->next;
        }

        *mergeTail = tailHeader->next;
    }

    tailHeader = (vl_linked_list_node *) vlPoolSample(&list->nodePool, *mergeTail);

    if (aIter != VL_LIST_ITER_INVALID) {
        tailHeader->next = aIter;
        leftHeader->prev = *mergeTail;
        vl_ListSortSegmentLength(list, aIter, mergeTail);
    } else {
        tailHeader->next = bIter;
        rightHeader->prev = *mergeTail;
        vl_ListSortSegmentLength(list, bIter, mergeTail);
    }
}

/**
 * \brief Splits the specified list into two parts, with the first part having the specified length.
 *
 * The second part will have whatever elements remain in the larger list.
 *
 * \param list pointer
 * \param atIter iterator to the beginning of the first split
 * \param length total number of elements in the first split
 * \private
 * \return iterator to where the split occurred (first iterator in the second half of the split)
 */
vl_list_iter vl_ListSortSplit(vl_linked_list *list, vl_list_iter atIter, vl_dsidx_t length) {
    if (atIter == VL_LIST_ITER_INVALID)
        return VL_LIST_ITER_INVALID;
    vl_linked_list_node *atHeader, *splitHeader;
    vl_list_iter splitIter = VL_LIST_ITER_INVALID;

    for (int i = 0; i < length; i++) {
        atHeader = (vl_linked_list_node *) vlPoolSample(&list->nodePool, atIter);

        if (atHeader->next == VL_LIST_ITER_INVALID)
            return VL_LIST_ITER_INVALID;

        atIter = atHeader->next;
    }

    if (atHeader->next != VL_LIST_ITER_INVALID) {
        splitIter = atHeader->next;
        splitHeader = (vl_linked_list_node *) vlPoolSample(&list->nodePool, atHeader->next);
        splitHeader->prev = VL_LIST_ITER_INVALID;
    }

    atHeader->next = VL_LIST_ITER_INVALID;

    return splitIter;
}

void vlListSort(vl_linked_list *list, vl_compare_function compareFunction) {
    if (list->head == list->tail)
        return;

    const vl_dsidx_t length = list->length;
    vl_list_iter leftIter, rightIter, currentIter, tailIter, mergedHead, mergedTail;
    vl_dsidx_t passLength = 1;

    while (passLength < length) {
        currentIter = list->head;
        tailIter = VL_LIST_ITER_INVALID;

        while (currentIter != VL_LIST_ITER_INVALID) {
            leftIter = currentIter;
            rightIter = vl_ListSortSplit(list, leftIter, passLength);
            currentIter = vl_ListSortSplit(list, rightIter, passLength);
            vl_ListSortMerge(list, leftIter, rightIter, &mergedHead, &mergedTail, compareFunction);

            if (tailIter == VL_LIST_ITER_INVALID) {
                list->head = mergedHead;
            } else {
                vl_linked_list_node *const tailNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool,
                                                                                           tailIter);
                vl_linked_list_node *const mergedHeadNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool,
                                                                                                 mergedHead);

                tailNode->next = mergedHead;
                mergedHeadNode->prev = tailIter;
            }
            tailIter = mergedTail;
        }

        passLength *= 2;
    }
}

vl_list_iter vlListFind(vl_linked_list *src, const void *element) {
    VL_LIST_FOREACH(src, curIter)if (memcmp(vlListSample(src, curIter), element, src->elementSize) == 0)
            return curIter;
    return VL_LIST_ITER_INVALID;
}

void vlListRemove(vl_linked_list *list, vl_list_iter iter) {
    vl_linked_list_node *oldNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, iter);
    vl_list_iter leftIter = oldNode->prev;
    vl_list_iter rightIter = oldNode->next;

    if (leftIter != VL_LIST_ITER_INVALID) {
        vl_linked_list_node *leftNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, leftIter);
        leftNode->next = rightIter;
    } else list->head = rightIter;

    if (rightIter != VL_LIST_ITER_INVALID) {
        vl_linked_list_node *rightNode = (vl_linked_list_node *) vlPoolSample(&list->nodePool, rightIter);
        rightNode->prev = leftIter;
    } else list->tail = leftIter;

    if (leftIter == VL_LIST_ITER_INVALID && rightIter == VL_LIST_ITER_INVALID)
        list->head = list->tail = VL_LIST_ITER_INVALID;

    vlPoolReturn(&list->nodePool, iter);
}

vl_list_iter vlListNext(vl_linked_list *list, vl_list_iter iter) {
    vl_linked_list_node *const node = (vl_linked_list_node *) vlPoolSample(&list->nodePool, iter);
    return node->next;
}

vl_list_iter vlListPrev(vl_linked_list *list, vl_list_iter iter) {
    vl_linked_list_node *const node = (vl_linked_list_node *) vlPoolSample(&list->nodePool, iter);
    return node->prev;
}

void *vlListSample(vl_linked_list *list, vl_list_iter iter) {
    vl_linked_list_node *const node = (vl_linked_list_node *) vlPoolSample(&list->nodePool, iter);
    return (void *) (node + 1);
}