#include <gtest/gtest.h>

extern "C" {
#include "linked/buffer.h"
}

TEST(buffer, clear) {
    EXPECT_TRUE(vlTestBufferClear());
}

TEST(buffer, shrink_to_fit) {
    EXPECT_TRUE(vlTestBufferShrinkToFit());
}

TEST(buffer, clone) {
    EXPECT_TRUE(vlTestBufferClone());
}

TEST(buffer, copy) {
    EXPECT_TRUE(vlTestBufferCopy());
}