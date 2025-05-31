#include <gtest/gtest.h>

extern "C" {
#include "linked/atomic.h"
}

TEST(atomic, counter) {
    EXPECT_TRUE(vlTestAtomicCounter());
}