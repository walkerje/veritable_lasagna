#include "queue.h"
#include <vl/vl_queue.h>

vl_bool_t vlTestQueueGrowth() {
    int queueSize = 10000;
    vl_queue *queue = vlQueueNew(sizeof(int));
    int sum = 0;

    for (int i = 0; i < queueSize; i++) {
        //Enqueue 0....(queueSize - 1).
        vlQueuePushBack(queue, &i);
        sum += i;
    }

    int curVal = 0;
    while (vlQueuePopFront(queue, &curVal))
        sum -= curVal;

    vlQueueDelete(queue);

    return (vl_bool_t)(sum == 0);
}

vl_bool_t vlTestQueueFIFO() {
    int queueSize = 10;
    int values[10] = {23, -47, 89, -16, 72, -88, 34, -5, 56, -33};
    vl_queue *queue = vlQueueNew(sizeof(int));

    for (int i = 0; i < queueSize; i++) {
        vlQueuePushBack(queue, values + i);
    }

    int i = 0;
    vl_bool_t result = VL_TRUE;
    int curValue;
    while (vlQueuePopFront(queue, &curValue)) {
        if (i >= queueSize) {
            result = VL_FALSE;
            break;
        }

        result = result && (curValue == values[i]);
        i++;
    }

    vlQueueDelete(queue);

    return result;
}

vl_bool_t vlTestQueueClone() {
    int queueSize = 10;
    int values[10] = {23, -47, 89, -16, 72, -88, 34, -5, 56, -33};
    vl_queue *queue = vlQueueNew(sizeof(int));

    for (int i = 0; i < queueSize; i++) {
        vlQueuePushBack(queue, values + i);
    }

    vl_queue *clone = vlQueueClone(queue, NULL);
    vlQueueDelete(queue);

    int i = 0;
    vl_bool_t result = VL_TRUE;
    int curValue;
    while (vlQueuePopFront(clone, &curValue)) {
        if (i >= queueSize) {
            result = VL_FALSE;
            break;
        }

        result = result && (curValue == values[i]);
        i++;
    }

    vlQueueDelete(clone);

    return result;
}
