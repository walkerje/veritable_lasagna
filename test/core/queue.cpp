#include <gtest/gtest.h>

extern "C" {
#include "linked/queue.h"
}

TEST(queue, growth) {
    EXPECT_TRUE(vlTestQueueGrowth());
}

TEST(queue, fifo) {
    EXPECT_TRUE(vlTestQueueFIFO());
}

TEST(queue, clone) {
    EXPECT_TRUE(vlTestQueueClone());
}