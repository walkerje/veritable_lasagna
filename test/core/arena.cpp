#include <gtest/gtest.h>

extern "C" {
#include "linked/arena.h"
}

TEST(arena, growth) {
    EXPECT_TRUE(vlTestArenaGrowth());
}

TEST(arena, coalesce) {
    EXPECT_TRUE(vlTestArenaCoalesce());
}