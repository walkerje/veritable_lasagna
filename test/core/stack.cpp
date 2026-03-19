#include <gtest/gtest.h>

extern "C" {
#include "linked/stack.h"
}

TEST(stack, hanoi) {
    EXPECT_TRUE(vlTestStackHanoi());
}