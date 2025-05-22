#include <gtest/gtest.h>

extern "C" {
#include "linked/async_queue.h"
}

TEST(async_queue, basic){
    EXPECT_TRUE(vlAsyncQueueTest());
}

TEST(async_queue, MPMC){
    EXPECT_TRUE(vlAsyncQueueTestMPMC());
}