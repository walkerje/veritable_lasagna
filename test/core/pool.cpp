#include <gtest/gtest.h>

extern "C" {
#include "linked/pool.h"
}

TEST(pool, clear) {
    EXPECT_TRUE(vlTestPoolClear());
}

TEST(pool, clone) {
    EXPECT_TRUE(vlTestPoolClone());
}

TEST(pool, elem_return) {
    EXPECT_TRUE(vlTestPoolElemReturn());
}

TEST(pool, reserve) {
    EXPECT_TRUE(vlTestPoolReserve());
}

TEST(pool, align) {
    EXPECT_TRUE(vlTestPoolAlign());
}