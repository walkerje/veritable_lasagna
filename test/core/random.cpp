#include <gtest/gtest.h>

extern "C" {
#include "linked/random.h"
#include <vl/vl_memory.h>
}

TEST(random, rand_vec4f) {
    EXPECT_TRUE(vlTestRandomVec4f());
}

class RandomFillTest : public testing::TestWithParam<vl_memsize_t> {};

TEST_P(RandomFillTest, fill) {
    EXPECT_TRUE(vlTestRandomFill(GetParam()));
}

INSTANTIATE_TEST_SUITE_P(
    random, RandomFillTest,
    testing::Values(128, VL_KB(5), VL_MB(5))
);