/**
 * ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
 * ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
 * ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
 *  ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
 *   ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
 * ====---: A Data Structure and Algorithms library for C11.  :---====
 *
 * Copyright 2026 Jesse Walker, released under the MIT license.
 * Git Repository:  https://github.com/walkerje/veritable_lasagna
 * \private
 */

#ifndef VL_ALGO_H
#define VL_ALGO_H

#include "vl_compare.h"
#include "vl_numtypes.h"

/**
 * \brief Returns the minimum of two values.
 * \param a first value
 * \param b second value
 * \return the smaller of the two values
 */
#define VL_MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * \brief Returns the maximum of two values.
 * \param a first value
 * \param b second value
 * \return the larger of the two values
 */
#define VL_MAX(a, b) ((a) > (b) ? (a) : (b))

/**
 * \brief Returns the absolute value of a number.
 * \param x input value
 * \return absolute value of x
 */
#define VL_ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * \brief Clamps a value between a minimum and maximum range.
 * \param x value to clamp
 * \param min minimum allowed value
 * \param max maximum allowed value
 * \return x clamped to the range [min, max]
 */
#define VL_CLAMP(x, min, max) VL_MAX(min, VL_MIN(x, max))

/**
 * \brief Counts the number of set bits (population count) in an 8-bit value.
 *
 * Uses compiler intrinsics when available for optimal performance.
 * Falls back to portable bit manipulation algorithms otherwise.
 *
 * \param value 8-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of bits set to 1 in the value
 */
VL_API vl_uint_t vlAlgoPopCount8(vl_uint8_t value);

/**
 * \brief Counts the number of set bits (population count) in a 16-bit value.
 *
 * Uses compiler intrinsics when available for optimal performance.
 * Falls back to portable bit manipulation algorithms otherwise.
 *
 * \param value 16-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of bits set to 1 in the value
 */
VL_API vl_uint_t vlAlgoPopCount16(vl_uint16_t value);

/**
 * \brief Counts the number of set bits (population count) in a 32-bit value.
 *
 * Uses compiler intrinsics when available for optimal performance.
 * Falls back to portable bit manipulation algorithms otherwise.
 *
 * \param value 32-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of bits set to 1 in the value
 */
VL_API vl_uint_t vlAlgoPopCount32(vl_uint32_t value);

/**
 * \brief Counts the number of set bits (population count) in a 64-bit value.
 *
 * Uses compiler intrinsics when available for optimal performance.
 * Falls back to portable bit manipulation algorithms otherwise.
 *
 * \param value 64-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of bits set to 1 in the value
 */
VL_API vl_uint_t vlAlgoPopCount64(vl_uint64_t value);

/**
 * \brief Counts the number of leading zero bits in an 8-bit value.
 *
 * Returns 8 if the input value is 0. Uses compiler intrinsics when available
 * for optimal performance, with portable fallback implementations.
 *
 * \param value 8-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of leading zero bits (0-8)
 */
VL_API vl_uint_t vlAlgoCLZ8(vl_uint8_t value);

/**
 * \brief Counts the number of leading zero bits in a 16-bit value.
 *
 * Returns 16 if the input value is 0. Uses compiler intrinsics when available
 * for optimal performance, with portable fallback implementations.
 *
 * \param value 16-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of leading zero bits (0-16)
 */
VL_API vl_uint_t vlAlgoCLZ16(vl_uint16_t value);

/**
 * \brief Counts the number of leading zero bits in a 32-bit value.
 *
 * Returns 32 if the input value is 0. Uses compiler intrinsics when available
 * for optimal performance, with portable fallback implementations.
 *
 * \param value 32-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of leading zero bits (0-32)
 */
VL_API vl_uint_t vlAlgoCLZ32(vl_uint32_t value);

/**
 * \brief Counts the number of leading zero bits in a 64-bit value.
 *
 * Returns 64 if the input value is 0. Uses compiler intrinsics when available
 * for optimal performance, with portable fallback implementations.
 *
 * \param value 64-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of leading zero bits (0-64)
 */
VL_API vl_uint_t vlAlgoCLZ64(vl_uint64_t value);

/**
 * \brief Counts the number of trailing zero bits in an 8-bit value.
 *
 * Returns 8 if the input value is 0. Uses compiler intrinsics when available
 * for optimal performance, with portable fallback implementations.
 *
 * \param value 8-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of trailing zero bits (0-8)
 */
VL_API vl_uint_t vlAlgoCTZ8(vl_uint8_t value);

/**
 * \brief Counts the number of trailing zero bits in a 16-bit value.
 *
 * Returns 16 if the input value is 0. Uses compiler intrinsics when available
 * for optimal performance, with portable fallback implementations.
 *
 * \param value 16-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of trailing zero bits (0-16)
 */
VL_API vl_uint_t vlAlgoCTZ16(vl_uint16_t value);

/**
 * \brief Counts the number of trailing zero bits in a 32-bit value.
 *
 * Returns 32 if the input value is 0. Uses compiler intrinsics when available
 * for optimal performance, with portable fallback implementations.
 *
 * \param value 32-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of trailing zero bits (0-32)
 */
VL_API vl_uint_t vlAlgoCTZ32(vl_uint32_t value);

/**
 * \brief Counts the number of trailing zero bits in a 64-bit value.
 *
 * Returns 64 if the input value is 0. Uses compiler intrinsics when available
 * for optimal performance, with portable fallback implementations.
 *
 * \param value 64-bit unsigned integer
 * \par Complexity O(1) constant
 * \return number of trailing zero bits (0-64)
 */
VL_API vl_uint_t vlAlgoCTZ64(vl_uint64_t value);

/**
 * \brief Computes the next power of 2 greater than or equal to the given value.
 *
 * If the input value is already a power of 2, it returns the same value.
 * Returns 1 for input value 0. Returns 0 if the result would overflow.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: N/A.
 * - **Thread Safety**: Thread-safe (stateless).
 * - **Nullability**: N/A.
 * - **Error Conditions**: Returns 0 if the next power of 2 would exceed the range of `vl_uint_t`.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the computed power of 2, or 0 on overflow.
 *
 * \param value input unsigned integer
 * \par Complexity O(1) constant
 * \return next power of 2, or 0 on overflow
 */
VL_API vl_uint_t vlAlgoNextPO2(vl_ularge_t value);

/**
 * \brief Tests whether a value is a power of 2.
 *
 * Returns VL_FALSE for input value 0, as 0 is not considered a power of 2.
 * Uses efficient bit manipulation to perform the test.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: N/A.
 * - **Thread Safety**: Thread-safe (stateless).
 * - **Nullability**: N/A.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the value is a power of 2, `VL_FALSE` otherwise.
 *
 * \param value input unsigned integer
 * \par Complexity O(1) constant
 * \return VL_TRUE if value is a power of 2, VL_FALSE otherwise
 */
VL_API vl_bool_t vlAlgoIsPO2(vl_ularge_t value);

/**
 * \brief Computes the Greatest Common Divisor (GCD) of two unsigned integers.
 *
 * Uses the Euclidean algorithm for efficient computation. Handles edge cases
 * where one or both inputs are zero appropriately.
 *
 * \param a first unsigned integer
 * \param b second unsigned integer
 * \par Complexity O(log(min(a,b))) logarithmic
 * \return greatest common divisor of a and b
 */
VL_API vl_ularge_t vlAlgoGCD(vl_ularge_t a, vl_ularge_t b);

/**
 * \brief Computes the Least Common Multiple (LCM) of two unsigned integers.
 *
 * Uses the relationship LCM(a,b) = |a*b| / GCD(a,b) with overflow protection.
 * Returns 0 if either input is 0 or if the result would overflow.
 *
 * \param a first unsigned integer
 * \param b second unsigned integer
 * \par Complexity O(log(min(a,b))) logarithmic
 * \return least common multiple of a and b, or 0 on overflow
 */
VL_API vl_ularge_t vlAlgoLCM(vl_ularge_t a, vl_ularge_t b);

/**
 * \brief Computes the Greatest Common Divisor (GCD) of two signed integers.
 *
 * Converts inputs to absolute values and delegates to the unsigned version.
 * Handles the edge case of the most negative integer value safely.
 *
 * \param a first signed integer
 * \param b second signed integer
 * \par Complexity O(log(min(|a|,|b|))) logarithmic
 * \return greatest common divisor of |a| and |b|
 */
VL_API vl_ilarge_t vlAlgoGCDSigned(vl_ilarge_t a, vl_ilarge_t b);

/**
 * \brief Computes the Least Common Multiple (LCM) of two signed integers.
 *
 * The result is always positive by mathematical definition. Converts inputs
 * to absolute values and delegates to the unsigned version with overflow
 * checking. Returns 0 if either input is 0 or if the result would overflow.
 *
 * \param a first signed integer
 * \param b second signed integer
 * \par Complexity O(log(min(|a|,|b|))) logarithmic
 * \return least common multiple of |a| and |b|, or 0 on overflow
 */
VL_API vl_ilarge_t vlAlgoLCMSigned(vl_ilarge_t a, vl_ilarge_t b);

/**
 * \brief Tests whether adding two unsigned integers would cause overflow.
 *
 * Uses compiler intrinsics when available for optimal performance and accuracy.
 * Falls back to portable overflow detection otherwise.
 *
 * \param a first unsigned integer
 * \param b second unsigned integer
 * \par Complexity O(1) constant
 * \return VL_TRUE if a + b would overflow, VL_FALSE otherwise
 */
VL_API vl_bool_t vlAlgoAddOverflow(vl_ularge_t a, vl_ularge_t b);

/**
 * \brief Tests whether multiplying two unsigned integers would cause overflow.
 *
 * Uses compiler intrinsics when available for optimal performance and accuracy.
 * Falls back to portable overflow detection otherwise.
 *
 * \param a first unsigned integer
 * \param b second unsigned integer
 * \par Complexity O(1) constant
 * \return VL_TRUE if a * b would overflow, VL_FALSE otherwise
 */
VL_API vl_bool_t vlAlgoMulOverflow(vl_ularge_t a, vl_ularge_t b);

/**
 * \brief Tests whether subtracting two unsigned integers would cause underflow.
 *
 * For unsigned integers, underflow occurs when the first operand is smaller
 * than the second operand. Uses efficient comparison for detection.
 *
 * \param a first unsigned integer
 * \param b second unsigned integer
 * \par Complexity O(1) constant
 * \return VL_TRUE if a - b would underflow, VL_FALSE otherwise
 */
VL_API vl_bool_t vlAlgoSubUnderflow(vl_ularge_t a, vl_ularge_t b);

#endif // VL_ALGO_H
