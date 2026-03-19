#include <gtest/gtest.h>

extern "C" {
#include "linked/algo.h"
}

TEST(algo, bit_operations) {
    EXPECT_TRUE(vlTestAlgoBitOperations());
}

TEST(algo, power_of_two) {
    EXPECT_TRUE(vlTestAlgoPowerOfTwo());
}

TEST(algo, gcd_lcm) {
    EXPECT_TRUE(vlTestAlgoGCDLCM());
}

TEST(algo, overflow_detection) {
    EXPECT_TRUE(vlTestAlgoOverflowDetection());
}

TEST(algo, macro_utilities) {
    EXPECT_TRUE(vlTestAlgoMacroUtilities());
}

TEST(algo, comprehensive_bit_test) {
    EXPECT_TRUE(vlTestAlgoComprehensiveBit());
}