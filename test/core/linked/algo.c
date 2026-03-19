#include "algo.h"
#include <vl/vl_algo.h>
#include <stdio.h>

vl_bool_t vlTestAlgoBitOperations() {
    // Test population count functions
    vl_uint8_t test8 = 0xFF;  // All bits set
    vl_uint16_t test16 = 0xF0F0;  // Alternating pattern
    vl_uint32_t test32 = 0x12345678;  // Mixed pattern
    vl_uint64_t test64 = 0x123456789ABCDEF0ULL;  // Large mixed pattern

    // Population count tests
    vl_uint_t pop8 = vlAlgoPopCount8(test8);
    vl_uint_t pop16 = vlAlgoPopCount16(test16);
    vl_uint_t pop32 = vlAlgoPopCount32(test32);
    vl_uint_t pop64 = vlAlgoPopCount64(test64);

    // Expected results: 8, 8, 13, 32 respectively
    if (pop8 != 8 || pop16 != 8 || pop32 != 13 || pop64 != 32) {
        return VL_FALSE;
    }

    // Test leading zero count
    vl_uint_t clz32 = vlAlgoCLZ32(0x00000001);  // Should be 31
    vl_uint_t clz16 = vlAlgoCLZ16(0x0001);      // Should be 15

    if (clz32 != 31 || clz16 != 15) {
        return VL_FALSE;
    }

    // Test trailing zero count
    vl_uint_t ctz32 = vlAlgoCTZ32(0x00000008);  // Should be 3
    vl_uint_t ctz64 = vlAlgoCTZ64(0x0000000000000010ULL);  // Should be 4

    if (ctz32 != 3 || ctz64 != 4) {
        return VL_FALSE;
    }

    return VL_TRUE;
}

vl_bool_t vlTestAlgoPowerOfTwo() {
    // Test power of 2 operations
    vl_bool_t is_po2_true = vlAlgoIsPO2(16);     // Should be true
    vl_bool_t is_po2_false = vlAlgoIsPO2(15);    // Should be false
    vl_bool_t is_po2_zero = vlAlgoIsPO2(0);      // Should be false

    if (!is_po2_true || is_po2_false || is_po2_zero) {
        return VL_FALSE;
    }

    // Test next power of 2
    vl_uint_t next_po2_1 = vlAlgoNextPO2(5);    // Should be 8
    vl_uint_t next_po2_2 = vlAlgoNextPO2(16);   // Should be 16 (already power of 2)
    vl_uint_t next_po2_3 = vlAlgoNextPO2(0);    // Should be 1

    if (next_po2_1 != 8 || next_po2_2 != 16 || next_po2_3 != 1) {
        return VL_FALSE;
    }

    return VL_TRUE;
}

vl_bool_t vlTestAlgoGCDLCM() {
    // Test Greatest Common Divisor
    vl_ularge_t gcd1 = vlAlgoGCD(48, 18);        // Should be 6
    vl_ularge_t gcd2 = vlAlgoGCD(17, 13);        // Should be 1 (coprime)
    vl_ularge_t gcd3 = vlAlgoGCD(100, 75);       // Should be 25

    if (gcd1 != 6 || gcd2 != 1 || gcd3 != 25) {
        return VL_FALSE;
    }

    // Test Least Common Multiple
    vl_ularge_t lcm1 = vlAlgoLCM(12, 18);        // Should be 36
    vl_ularge_t lcm2 = vlAlgoLCM(7, 5);          // Should be 35 (coprime)
    vl_ularge_t lcm3 = vlAlgoLCM(4, 6);          // Should be 12

    if (lcm1 != 36 || lcm2 != 35 || lcm3 != 12) {
        return VL_FALSE;
    }

    // Test signed versions
    vl_ilarge_t gcd_signed = vlAlgoGCDSigned(-12, 18);  // Should be 6
    vl_ilarge_t lcm_signed = vlAlgoLCMSigned(-4, 6);    // Should be 12

    if (gcd_signed != 6 || lcm_signed != 12) {
        return VL_FALSE;
    }

    return VL_TRUE;
}

vl_bool_t vlTestAlgoOverflowDetection() {
    // Test addition overflow detection
    vl_ularge_t max_value = (vl_ularge_t)-1;  // Maximum possible value
    vl_bool_t add_overflow_true = vlAlgoAddOverflow(max_value, 1);     // Should overflow
    vl_bool_t add_overflow_false = vlAlgoAddOverflow(100, 200);        // Should not overflow

    if (!add_overflow_true || add_overflow_false) {
        return VL_FALSE;
    }

    // Test multiplication overflow detection
    vl_bool_t mul_overflow_true = vlAlgoMulOverflow(max_value, 2);     // Should overflow
    vl_bool_t mul_overflow_false = vlAlgoMulOverflow(100, 200);        // Should not overflow

    if (!mul_overflow_true || mul_overflow_false) {
        return VL_FALSE;
    }

    // Test subtraction underflow detection
    vl_bool_t sub_underflow_true = vlAlgoSubUnderflow(5, 10);          // Should underflow
    vl_bool_t sub_underflow_false = vlAlgoSubUnderflow(10, 5);         // Should not underflow

    if (!sub_underflow_true || sub_underflow_false) {
        return VL_FALSE;
    }

    return VL_TRUE;
}

vl_bool_t vlTestAlgoMacroUtilities() {
    // Test utility macros
    int min_result = VL_MIN(10, 5);        // Should be 5
    int max_result = VL_MAX(10, 5);        // Should be 10
    int abs_positive = VL_ABS(7);          // Should be 7
    int abs_negative = VL_ABS(-7);         // Should be 7
    int clamp_low = VL_CLAMP(2, 5, 10);    // Should be 5 (clamped to min)
    int clamp_high = VL_CLAMP(15, 5, 10);  // Should be 10 (clamped to max)
    int clamp_normal = VL_CLAMP(7, 5, 10); // Should be 7 (within range)

    if (min_result != 5 || max_result != 10 ||
        abs_positive != 7 || abs_negative != 7 ||
        clamp_low != 5 || clamp_high != 10 || clamp_normal != 7) {
        return VL_FALSE;
    }

    return VL_TRUE;
}

vl_bool_t vlTestAlgoComprehensiveBit() {
    // Test edge cases for bit operations
    vl_uint_t clz_zero_32 = vlAlgoCLZ32(0);    // Should be 32
    vl_uint_t ctz_zero_32 = vlAlgoCTZ32(0);    // Should be 32
    vl_uint_t pop_zero = vlAlgoPopCount32(0);   // Should be 0

    if (clz_zero_32 != 32 || ctz_zero_32 != 32 || pop_zero != 0) {
        return VL_FALSE;
    }

    // Test all different bit sizes
    vl_uint_t clz8_result = vlAlgoCLZ8(0x01);   // Should be 7
    vl_uint_t ctz8_result = vlAlgoCTZ8(0x80);   // Should be 7
    vl_uint_t clz64_result = vlAlgoCLZ64(0x0000000000000001ULL);  // Should be 63

    if (clz8_result != 7 || ctz8_result != 7 || clz64_result != 63) {
        return VL_FALSE;
    }

    return VL_TRUE;
}
