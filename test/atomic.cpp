#include <gtest/gtest.h>

extern "C" {
#include "linked/atomic.h"
}

//TODO: Real tests. Need threading.

TEST(atomic, compile){
    EXPECT_TRUE(vlTESTAtomicCompile());
}