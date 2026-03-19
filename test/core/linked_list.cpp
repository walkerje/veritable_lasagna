#include <gtest/gtest.h>

extern "C" {
#include "linked/linked_list.h"
}

TEST(linked_list, growth) {
    EXPECT_TRUE(vlTestListGrowth());
}

TEST(linked_list, sort) {
    EXPECT_TRUE(vlTestListSort());
}

class LinkedListIterateTest : public testing::TestWithParam<vl_bool_t> {};

TEST_P(LinkedListIterateTest, iterate) {
    EXPECT_TRUE(vlTestListIterate(GetParam()));
}

INSTANTIATE_TEST_SUITE_P(
    linked_list, LinkedListIterateTest,
    testing::Values(VL_FALSE, VL_TRUE)
);

TEST(linked_list, inline_insert) {
    EXPECT_TRUE(vlTestListInlineInsert());
}