//
// Created by silas on 3/1/2025.
//
#include <gtest/gtest.h>

extern "C" {
#include "linked/async_pool.h"
}

TEST(async_pool, SPSC){
    EXPECT_TRUE(vlTestAsyncPoolBasic());
}

TEST(async_pool, MPMC){
EXPECT_TRUE(vlTestAsyncPoolMPMC());
}

TEST(async_pool, clear_and_reuse){
    EXPECT_TRUE(vlTestAsyncPoolClearAndReuse());
}