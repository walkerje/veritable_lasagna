#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "vl_algo.h"

#include <limits.h>

#if defined(_MSC_VER)
#define VL_COMPILER_MSVC 1
#include <intrin.h>
#include <shlwapi.h>
#elif defined(__GNUC__) || defined(__clang__)
#define VL_COMPILER_GCC_LIKE 1
#endif

vl_uint_t vlAlgoPopCount8(vl_uint8_t value)
{
#if defined(VL_COMPILER_MSVC)
    return __popcnt16(value);
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_popcount(value);
#else
    // Fallback implementation
    vl_uint_t count = 0;
    while (value)
    {
        count += value & 1;
        value >>= 1;
    }
    return count;
#endif
}

vl_uint_t vlAlgoPopCount16(vl_uint16_t value)
{
#if defined(VL_COMPILER_MSVC)
    return __popcnt16(value);
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_popcount(value);
#else
    // Brian Kernighan's algorithm (faster fallback)
    vl_uint_t count = 0;
    while (value)
    {
        value &= value - 1; // Clear the lowest set bit
        count++;
    }
    return count;
#endif
}

vl_uint_t vlAlgoPopCount32(vl_uint32_t value)
{
#if defined(VL_COMPILER_MSVC)
    return __popcnt(value);
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_popcount(value);
#else
    // Fallback using bit manipulation tricks
    value = value - ((value >> 1) & 0x55555555);
    value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
    return (((value + (value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
#endif
}

vl_uint_t vlAlgoPopCount64(vl_uint64_t value)
{
#if defined(VL_COMPILER_MSVC)
#ifdef _WIN64
    return (vl_uint_t)__popcnt64(value);
#else
    return __popcnt((vl_uint32_t)value) + __popcnt((vl_uint32_t)(value >> 32));
#endif
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_popcountll(value);
#else
    // Fallback: split into two 32-bit operations
    return vlAlgoPopCount32((vl_uint32_t)value) + vlAlgoPopCount32((vl_uint32_t)(value >> 32));
#endif
}
vl_uint_t vlAlgoCLZ8(vl_uint8_t value)
{
    if (value == 0)
        return 8;

#if defined(VL_COMPILER_MSVC)
    unsigned long index;
    _BitScanReverse(&index, value);
    return 7 - index;
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_clz(value) - 24; // Adjust for 32-bit result
#else
    // Fallback
    vl_uint_t count = 0;
    if (value == 0)
        return 8;
    if (!(value & 0xF0))
    {
        count += 4;
        value <<= 4;
    }
    if (!(value & 0xC0))
    {
        count += 2;
        value <<= 2;
    }
    if (!(value & 0x80))
    {
        count += 1;
    }
    return count;
#endif
}

vl_uint_t vlAlgoCLZ16(vl_uint16_t value)
{
    if (value == 0)
        return 16;

#if defined(VL_COMPILER_MSVC)
    unsigned long index;
    _BitScanReverse(&index, value);
    return 15 - index;
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_clz(value) - 16; // Adjust for 32-bit result
#else
    // Fallback
    return vlAlgoCLZ8(value >> 8) ? vlAlgoCLZ8(value >> 8) + 8 : vlAlgoCLZ8(value & 0xFF);
#endif
}

vl_uint_t vlAlgoCLZ32(vl_uint32_t value)
{
    if (value == 0)
        return 32;

#if defined(VL_COMPILER_MSVC)
    unsigned long index;
    _BitScanReverse(&index, value);
    return 31 - index;
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_clz(value);
#else
    // Fallback using binary search
    vl_uint_t count = 0;
    if (!(value & 0xFFFF0000))
    {
        count += 16;
        value <<= 16;
    }
    if (!(value & 0xFF000000))
    {
        count += 8;
        value <<= 8;
    }
    if (!(value & 0xF0000000))
    {
        count += 4;
        value <<= 4;
    }
    if (!(value & 0xC0000000))
    {
        count += 2;
        value <<= 2;
    }
    if (!(value & 0x80000000))
    {
        count += 1;
    }
    return count;
#endif
}

vl_uint_t vlAlgoCLZ64(vl_uint64_t value)
{
    if (value == 0)
        return 64;

#if defined(VL_COMPILER_MSVC)
#ifdef _WIN64
    unsigned long index;
    _BitScanReverse64(&index, value);
    return 63 - index;
#else
    // 32-bit MSVC fallback
    vl_uint32_t high = (vl_uint32_t)(value >> 32);
    if (high != 0)
    {
        return vlAlgoCLZ32(high);
    }
    else
    {
        return 32 + vlAlgoCLZ32((vl_uint32_t)value);
    }
#endif
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_clzll(value);
#else
    // Fallback: split into two 32-bit operations
    vl_uint32_t high = (vl_uint32_t)(value >> 32);
    if (high != 0)
    {
        return vlAlgoCLZ32(high);
    }
    else
    {
        return 32 + vlAlgoCLZ32((vl_uint32_t)value);
    }
#endif
}

vl_uint_t vlAlgoCTZ8(vl_uint8_t value)
{
    if (value == 0)
        return 8;

#if defined(VL_COMPILER_MSVC)
    unsigned long index;
    _BitScanForward(&index, value);
    return index;
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_ctz(value);
#else
    // Fallback using De Bruijn sequence
    static const vl_uint_t table[8] = {0, 1, 2, 4, 7, 3, 6, 5};
    return table[((vl_uint32_t)((value & -value) * 0x17U)) >> 29];
#endif
}

vl_uint_t vlAlgoCTZ16(vl_uint16_t value)
{
    if (value == 0)
        return 16;

#if defined(VL_COMPILER_MSVC)
    unsigned long index;
    _BitScanForward(&index, value);
    return index;
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_ctz(value);
#else
    // Fallback
    if ((value & 0xFF) != 0)
    {
        return vlAlgoCTZ8(value & 0xFF);
    }
    else
    {
        return 8 + vlAlgoCTZ8(value >> 8);
    }
#endif
}

vl_uint_t vlAlgoCTZ32(vl_uint32_t value)
{
    if (value == 0)
        return 32;

#if defined(VL_COMPILER_MSVC)
    unsigned long index;
    _BitScanForward(&index, value);
    return index;
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_ctz(value);
#else
    // Fallback using De Bruijn sequence
    static const vl_uint_t table[32] = {0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
                                        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9};
    return table[((vl_uint32_t)((value & -value) * 0x077CB531U)) >> 27];
#endif
}

vl_uint_t vlAlgoCTZ64(vl_uint64_t value)
{
    if (value == 0)
        return 64;

#if defined(VL_COMPILER_MSVC)
#ifdef _WIN64
    unsigned long index;
    _BitScanForward64(&index, value);
    return index;
#else
    // 32-bit MSVC fallback
    vl_uint32_t low = (vl_uint32_t)value;
    if (low != 0)
    {
        return vlAlgoCTZ32(low);
    }
    else
    {
        return 32 + vlAlgoCTZ32((vl_uint32_t)(value >> 32));
    }
#endif
#elif defined(VL_COMPILER_GCC_LIKE)
    return __builtin_ctzll(value);
#else
    // Fallback: split into two 32-bit operations
    vl_uint32_t low = (vl_uint32_t)value;
    if (low != 0)
    {
        return vlAlgoCTZ32(low);
    }
    else
    {
        return 32 + vlAlgoCTZ32((vl_uint32_t)(value >> 32));
    }
#endif
}
vl_uint_t vlAlgoNextPO2(vl_ularge_t value)
{
    if (value == 0)
        return 1;
    if (value > (ULLONG_MAX >> 1))
        return 0; // Overflow case

    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127)
#endif
    if (sizeof(vl_ularge_t) > 4)
    {
        value |= value >> 32;
    }
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#endif
    return (vl_uint_t)(value + 1);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}

vl_bool_t vlAlgoIsPO2(vl_ularge_t value) { return value != 0 && (value & (value - 1)) == 0; }
vl_bool_t vlAlgoAddOverflow(vl_ularge_t a, vl_ularge_t b)
{
#if defined(VL_COMPILER_GCC_LIKE)
    // GCC/Clang builtin (available since GCC 5.1)
    vl_ularge_t result;
    return __builtin_add_overflow(a, b, &result);

#elif defined(VL_COMPILER_MSVC)
    // MSVC doesn't have direct overflow builtins, use manual check
    return (ULLONG_MAX - a) < b;

#else
    // Portable fallback
    return (ULLONG_MAX - a) < b;
#endif
}

vl_ularge_t vlAlgoGCD(vl_ularge_t a, vl_ularge_t b)
{
    if (a == 0)
        return b;
    if (b == 0)
        return a;

    // Find the greatest power of 2 that divides both a and b
    vl_uint_t shift = 0;
    while (((a | b) & 1) == 0)
    {
        a >>= 1;
        b >>= 1;
        shift++;
    }

    // Remove all factors of 2 from a
    while ((a & 1) == 0)
    {
        a >>= 1;
    }

    do
    {
        // Remove all factors of 2 from b
        while ((b & 1) == 0)
        {
            b >>= 1;
        }

        // Ensure a <= b
        if (a > b)
        {
            vl_ularge_t temp = a;
            a = b;
            b = temp;
        }

        b = b - a;
    }
    while (b != 0);

    return a << shift;
}

vl_ilarge_t vlAlgoGCDSigned(vl_ilarge_t a, vl_ilarge_t b)
{
    // Convert to absolute values for computation
    vl_ularge_t ua = (a < 0) ? (vl_ularge_t)(-a) : (vl_ularge_t)a;
    vl_ularge_t ub = (b < 0) ? (vl_ularge_t)(-b) : (vl_ularge_t)b;

    return (vl_ilarge_t)vlAlgoGCD(ua, ub);
}

vl_ilarge_t vlAlgoLCMSigned(vl_ilarge_t a, vl_ilarge_t b)
{
    // Handle edge cases
    if (a == 0 || b == 0)
        return 0;

    // Convert to absolute values for computation
    vl_ularge_t ua = (a < 0) ? (vl_ularge_t)(-a) : (vl_ularge_t)a;
    vl_ularge_t ub = (b < 0) ? (vl_ularge_t)(-b) : (vl_ularge_t)b;

    // Handle potential overflow from negating minimum signed value
    // If a == LLONG_MIN, then -a would overflow in signed arithmetic
    if (a == LLONG_MIN)
    {
        ua = (vl_ularge_t)LLONG_MIN; // Cast directly to unsigned
    }
    if (b == LLONG_MIN)
    {
        ub = (vl_ularge_t)LLONG_MIN; // Cast directly to unsigned
    }

    // Compute LCM using unsigned function
    vl_ularge_t result = vlAlgoLCM(ua, ub);

    // Check if result fits in signed range
    if (result > (vl_ularge_t)LLONG_MAX)
    {
        // Result would overflow signed integer
        return 0; // or handle error as appropriate for your library
    }

    // LCM is always positive by mathematical definition
    return (vl_ilarge_t)result;
}

vl_ularge_t vlAlgoLCM(vl_ularge_t a, vl_ularge_t b)
{
    // Handle edge cases
    if (a == 0 || b == 0)
        return 0;

    // LCM(a,b) = |a*b| / GCD(a,b)
    // To avoid overflow, compute: a / GCD(a,b) * b
    vl_ularge_t gcd = vlAlgoGCD(a, b);

    // Check for potential overflow in the division result
    vl_ularge_t quotient = a / gcd;
    if (quotient > ULLONG_MAX / b)
    {
        // Overflow would occur
        return 0; // or handle error as appropriate
    }

    return quotient * b;
}

vl_bool_t vlAlgoSubUnderflow(vl_ularge_t a, vl_ularge_t b)
{
#if defined(VL_COMPILER_GCC_LIKE)
    // GCC/Clang builtin
    vl_ularge_t result;
    return __builtin_sub_overflow(a, b, &result);

#else
    // Portable implementation
    return a < b;
#endif
}

vl_bool_t vlAlgoMulOverflow(vl_ularge_t a, vl_ularge_t b)
{
#if defined(VL_COMPILER_GCC_LIKE)
    // GCC/Clang builtin
    vl_ularge_t result;
    return __builtin_mul_overflow(a, b, &result);

#elif defined(VL_COMPILER_MSVC) && defined(_WIN64)
    // MSVC 64-bit: use _umul128 intrinsic
    vl_ularge_t high;
    const vl_ularge_t low = _umul128(a, b, &high);
    (void)low;
    return high != 0;

#else
    // Portable fallback
    if (a == 0 || b == 0)
        return VL_FALSE;
    return a > (ULLONG_MAX / b);
#endif
}
