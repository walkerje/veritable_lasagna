#include <gtest/gtest.h>

extern "C" {
#include "linked/memory.h"
}

class MemoryAlignTest : public testing::TestWithParam<vl_int_t> {};

TEST_P(MemoryAlignTest, align) {
    ASSERT_TRUE(vlTestMemAlign(GetParam()));
}

INSTANTIATE_TEST_SUITE_P(
    memory, MemoryAlignTest,
    testing::Values(16, 32, 64, 128, 256, 512)
);

class MemorySortTest : public testing::TestWithParam<vl_int_t> {};

TEST_P(MemorySortTest, sort) {
    ASSERT_TRUE(vlTestMemSort(GetParam()));
}

INSTANTIATE_TEST_SUITE_P(
    memory, MemorySortTest,
    testing::Values(10, 100, 1000, 10000, 100000, 1000000)
);

TEST(memory, reverse) {
    ASSERT_TRUE(vlTestMemReverse());
}