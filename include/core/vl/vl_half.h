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

#ifndef VL_HALF_H
#define VL_HALF_H

#include <string.h> /* memcpy */
#include <vl/vl_numtypes.h>

/**
 * \file vl_half.h
 * \brief IEEE 754-2008 binary16 (half-precision) floating-point helpers.
 *
 * This header defines a portable, bit-exact representation of half-precision
 * floating-point values (binary16) and a small set of inline helper functions
 * for inspecting, classifying, packing, and converting them.
 *
 * Design goals:
 *  - No reliance on compiler-specific half types (e.g. _Float16, __fp16)
 *  - No bitfields (avoids layout and endianness ambiguities)
 *  - No type-punning or strict-aliasing violations
 *  - Explicit, predictable semantics
 *  - Suitable for use in low-level systems code
 *
 * The \ref vl_half type represents the raw 16-bit IEEE-754 bit pattern.
 * It is NOT a native C floating-point type.
 */

/**
 * \brief Raw half-precision floating-point bit pattern.
 *
 * This is a 16-bit unsigned integer holding an IEEE 754-2008 binary16 value:
 *
 *   sign : 1 bit
 *   exp  : 5 bits (bias = 15)
 *   frac : 10 bits
 */
typedef vl_uint16_t vl_half_t;

/*=============================================================================
 * Constants
 *============================================================================*/

/** \brief Sign bit mask. */
#define VL_HALF_SIGN_MASK 0x8000
/** \brief Exponent field mask. */
#define VL_HALF_EXP_MASK 0x7C00
/** \brief Fraction (mantissa) mask. */
#define VL_HALF_FRAC_MASK 0x03FF

/** \brief Exponent field shift. */
#define VL_HALF_EXP_SHIFT 10
/** \brief Exponent bias for binary16. */
#define VL_HALF_EXP_BIAS 15

/* Signed zeros */
#define VL_HALF_POS_ZERO ((vl_half)0x0000)
#define VL_HALF_NEG_ZERO ((vl_half)0x8000)

/* Infinities */
#define VL_HALF_POS_INF ((vl_half)0x7C00)
#define VL_HALF_NEG_INF ((vl_half)0xFC00)

/* NaNs */
#define VL_HALF_QNAN ((vl_half)0x7E00) /* canonical quiet NaN */
#define VL_HALF_SNAN ((vl_half)0x7D00) /* signaling NaN (payload = 0) */

/* Ones */
#define VL_HALF_ONE ((vl_half)0x3C00) /*  1.0 */
#define VL_HALF_NEG_ONE ((vl_half)0xBC00) /* -1.0 */

/* Largest finite values */
#define VL_HALF_MAX ((vl_half)0x7BFF) /* +65504 */
#define VL_HALF_MIN ((vl_half)0xFBFF) /* -65504 */

/* Smallest normal values */
#define VL_HALF_MIN_POS ((vl_half)0x0400) /*  2^-14 */
#define VL_HALF_MIN_NEG ((vl_half)0x8400)

/* Smallest subnormal values */
#define VL_HALF_TRUE_MIN_POS ((vl_half)0x0001) /* 2^-24 */
#define VL_HALF_TRUE_MIN_NEG ((vl_half)0x8001)

/* Difference between 1.0 and the next representable value */
#define VL_HALF_EPSILON ((vl_half)0x1400) /* 2^-10 */

/*=============================================================================
 * Bit extraction helpers
 *============================================================================*/

/**
 * \brief Extracts the sign bit.
 * \return 0 for positive, 1 for negative.
 */
static inline vl_uint16_t vlHalfSign(vl_half_t h) { return (h & VL_HALF_SIGN_MASK) >> 15; }

/**
 * \brief Extracts the biased exponent field.
 * \return Exponent value in range [0, 31].
 */
static inline vl_uint16_t vlHalfExp(vl_half_t h) { return (h & VL_HALF_EXP_MASK) >> VL_HALF_EXP_SHIFT; }

/**
 * \brief Extracts the fraction (mantissa) field.
 * \return Fraction value in range [0, 1023].
 */
static inline vl_uint16_t vlHalfFrac(vl_half_t h) { return h & VL_HALF_FRAC_MASK; }

/*=============================================================================
 * Classification helpers
 *============================================================================*/

/**
 * \brief Tests whether the value is +0 or -0.
 */
static inline int vlHalfIsZero(vl_half_t h) { return (h & ~VL_HALF_SIGN_MASK) == 0; }

/**
 * \brief Tests whether the value is a subnormal number.
 */
static inline int vlHalfIsSubnormal(vl_half_t h) { return (vlHalfExp(h) == 0) && (vlHalfFrac(h) != 0); }

/**
 * \brief Tests whether the value is positive or negative infinity.
 */
static inline int vlHalfIsInf(vl_half_t h) { return (h & ~VL_HALF_SIGN_MASK) == VL_HALF_EXP_MASK; }

/**
 * \brief Tests whether the value is a NaN.
 */
static inline int vlHalfIsNaN(vl_half_t h) { return (vlHalfExp(h) == 31) && (vlHalfFrac(h) != 0); }

/**
 * \brief Tests the sign bit.
 */
static inline int vlHalfSignBit(vl_half_t h) { return (h & VL_HALF_SIGN_MASK) != 0; }

/*=============================================================================
 * Packing helper
 *============================================================================*/

/**
 * \brief Packs sign, exponent, and fraction fields into a half value.
 *
 * The inputs are masked to their valid bit widths.
 */
static inline vl_half_t vlHalfPack(vl_uint16_t sign, vl_uint16_t exp, vl_uint16_t frac)
{
    return (vl_half_t)(((sign & 1) << 15) | ((exp & 0x1F) << VL_HALF_EXP_SHIFT) | (frac & VL_HALF_FRAC_MASK));
}

/*=============================================================================
 * Bit-cast helpers
 *============================================================================*/

/*=============================================================================
 * Conversion: float -> half
 *============================================================================*/

/**
 * \brief Converts a 32-bit float to half precision.
 *
 * Rounding mode: round-to-nearest, ties-to-even.
 *
 * Special cases:
 *  - NaN propagates as a quiet NaN
 *  - Overflow produces infinity
 *  - Underflow produces zero or a subnormal
 */
static inline vl_half_t vlHalfFromFloat(float x)
{
    vl_uint32_t f;
    memcpy(&f, &x, sizeof f);

    vl_uint32_t sign = (f >> 31) & 1;
    vl_int32_t exp = (f >> 23) & 0xFF;
    vl_uint32_t frac = f & 0x7FFFFF;

    if (exp == 255)
    {
        if (frac == 0)
            return vlHalfPack(sign, 31, 0);
        return vlHalfPack(sign, 31, 1);
    }

    vl_int32_t e = exp - 127;

    if (e > 15)
        return vlHalfPack(sign, 31, 0);

    if (e < -14)
    {
        if (e < -24)
            return vlHalfPack(sign, 0, 0);

        vl_uint32_t mant = frac | 0x800000;
        vl_uint32_t shift = (vl_uint32_t)(-e - 1);
        vl_uint32_t rshift = shift + 13;

        vl_uint32_t frac16 = mant >> rshift;
        vl_uint32_t rem = mant & ((1u << rshift) - 1);
        vl_uint32_t half = 1u << (rshift - 1);

        if (rem > half || (rem == half && (frac16 & 1)))
            frac16++;

        return vlHalfPack(sign, 0, frac16);
    }

    vl_uint32_t frac16 = frac >> 13;
    vl_uint32_t rem = frac & 0x1FFF;

    if (rem > 0x1000 || (rem == 0x1000 && (frac16 & 1)))
        frac16++;

    if (frac16 == 0x400)
    {
        frac16 = 0;
        e++;
        if (e > 15)
            return vlHalfPack(sign, 31, 0);
    }

    return vlHalfPack(sign, (vl_uint16_t)(e + VL_HALF_EXP_BIAS), frac16);
}

/*=============================================================================
 * Conversion: half -> float
 *============================================================================*/

/**
 * \brief Converts a half-precision value to 32-bit float.
 */
static inline float vlHalfToFloat(vl_half_t h)
{
    const vl_uint32_t sign = vlHalfSign(h);
    const vl_uint32_t exp = vlHalfExp(h);
    vl_uint32_t frac = vlHalfFrac(h);

    vl_uint32_t value;

    if (exp == 0)
    {
        if (frac == 0)
        {
            value = sign << 31;
        }
        else
        {
            vl_int32_t e = -14;
            while ((frac & 0x400) == 0)
            {
                frac <<= 1;
                e--;
            }
            frac &= 0x3FF;
            value = (sign << 31) | ((e + 127) << 23) | (frac << 13);
        }
    }
    else if (exp == 31)
    {
        value = (sign << 31) | (255 << 23) | (frac << 13);
    }
    else
    {
        value = (sign << 31) | ((exp - VL_HALF_EXP_BIAS + 127) << 23) | (frac << 13);
    }

    vl_float32_t result = 0;
    memcpy(&result, &value, sizeof result);
    return result;
}

/*=============================================================================
 * Arithmetic helpers (via widening)
 *============================================================================*/

/** \brief Adds two half values. */
static inline vl_half_t vlHalfAdd(vl_half_t a, vl_half_t b)
{
    return vlHalfFromFloat(vlHalfToFloat(a) + vlHalfToFloat(b));
}

/** \brief Subtracts two half values. */
static inline vl_half_t vlHalfSub(vl_half_t a, vl_half_t b)
{
    return vlHalfFromFloat(vlHalfToFloat(a) - vlHalfToFloat(b));
}

/** \brief Multiplies two half values. */
static inline vl_half_t vlHalfMul(vl_half_t a, vl_half_t b)
{
    return vlHalfFromFloat(vlHalfToFloat(a) * vlHalfToFloat(b));
}

/** \brief Divides two half values. */
static inline vl_half_t vlHalfDiv(vl_half_t a, vl_half_t b)
{
    return vlHalfFromFloat(vlHalfToFloat(a) / vlHalfToFloat(b));
}

#endif /* VL_HALF_H */
