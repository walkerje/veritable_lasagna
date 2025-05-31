#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_queue.h>

int queue_growth() {
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

    return sum == 0;
}

int queue_fifo(int cloned) {
    int queueSize = 10;
    int values[10] = {23, -47, 89, -16, 72, -88, 34, -5, 56, -33};
    vl_queue *queue = vlQueueNew(sizeof(int));

    for (int i = 0; i < queueSize; i++) {
        vlQueuePushBack(queue, values + i);
    }

    if (cloned) {
        vl_queue *temp = vlQueueClone(queue, NULL);
        vlQueueDelete(queue);
        queue = temp;
    }

    int i = 0, result = 1, curValue;
    while (vlQueuePopFront(queue, &curValue)) {
        if (i >= queueSize) {
            result = 0;
            break;
        }

        result = result && (curValue == values[i]);
        i++;
    }

    vlQueueDelete(queue);

    return result;
}
}

TEST(queue, growth) {
    EXPECT_TRUE(queue_growth());
}

TEST(queue, fifo) {
    EXPECT_TRUE(queue_fifo(false));
}

TEST(queue, clone) {
    EXPECT_TRUE(queue_fifo(true));
}