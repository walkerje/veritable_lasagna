#include <gtest/gtest.h>

extern "C" {
#include "linked/set.h"
}

TEST(set, growth) {
    EXPECT_TRUE(vlTestSetGrowth());
}

TEST(set, order) {
    EXPECT_TRUE(vlTestSetOrder());
}

class SetIterateTest : public testing::TestWithParam<vl_bool_t> {};

TEST_P(SetIterateTest, iterate) {
    EXPECT_TRUE(vlTestSetIterate(GetParam()));
}

INSTANTIATE_TEST_SUITE_P(
    set, SetIterateTest,
    testing::Values(VL_FALSE, VL_TRUE)
);