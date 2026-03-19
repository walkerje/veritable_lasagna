#include <gtest/gtest.h>

extern "C" {
#include "linked/hashtable.h"
}

TEST(hashtable, collision) {
    EXPECT_TRUE(vlTestHashTableCollision());
}

TEST(hashtable, removal) {
    EXPECT_TRUE(vlTestHashTableRemove());
}

class HashTableInsertTest : public testing::TestWithParam<std::tuple<vl_uint32_t, vl_bool_t>> {};

TEST_P(HashTableInsertTest, insert) {
    auto [size, reserved] = GetParam();
    EXPECT_TRUE(vlTestHashTableInsert(size, reserved));
}

INSTANTIATE_TEST_SUITE_P(
    hashtable, HashTableInsertTest,
    testing::Values(
        std::make_tuple(10000000, VL_FALSE),
        std::make_tuple(10000000, VL_TRUE)
    )
);

class HashTableIterateTest : public testing::TestWithParam<std::tuple<vl_int_t, vl_int_t, vl_bool_t>> {};

TEST_P(HashTableIterateTest, iterate) {
    auto [size, rounds, reserved] = GetParam();
    EXPECT_TRUE(vlTestHashTableIterate(size, rounds, reserved));
}

INSTANTIATE_TEST_SUITE_P(
    hashtable, HashTableIterateTest,
    testing::Values(
        std::make_tuple(1000000, 10, VL_FALSE),
        std::make_tuple(1000000, 10, VL_TRUE)
    )
);

TEST(hashtable, real_world_case) {
    EXPECT_TRUE(vlTestHashTableRealWorld());
}