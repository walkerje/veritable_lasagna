#include <gtest/gtest.h>

extern "C" {
#include "linked/atomic.h"
}

//TODO: Real tests. Need threading.

TEST(atomic, counter){
    EXPECT_TRUE(vlTestAtomicCounter());
}