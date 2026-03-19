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

#ifndef VL_NIBBLE_H
#define VL_NIBBLE_H

#include <vl/vl_numtypes.h>

/**
 * \brief Nibble type.
 *
 * Nibbles are 4-bit values encoded in half of an unsigned 8-bit integer.
 * Their values range from 0 to 15, or 0x0 to 0xF.
 *
 * Nibbles are an esoteric data type, but are useful for compressing data,
 * bitfield manipulation, and other operations.
 */
typedef vl_uint8_t vl_nibble_t;

#define VL_NIBBLE_MASK_HIGH 0xF0
#define VL_NIBBLE_MASK_LOW 0x0F
#define VL_NIBBLE_MAX 0xF
#define VL_NIBBLE_MIN 0x0

/**
 * \brief Creates a nibble from the lower 4 bits of a byte.
 * 0xAB -> 0x0B
 */
static inline vl_nibble_t vlNibbleLow(vl_uint8_t x) { return x & 0x0F; }

/**
 * \brief Extracts the high nibble from a byte.
 * 0xAB -> 0x0A
 */
static inline vl_nibble_t vlNibbleHigh(vl_uint8_t x) { return (x >> 4) & 0x0F; }

/**
 * \brief Packs two nibbles into a byte.
 * high=0xA, low=0xB -> 0xAB
 */
static inline vl_uint8_t vlNibblePack(vl_nibble_t high, vl_nibble_t low)
{
    return (vl_uint8_t)((high << 4) | (low & 0x0F));
}

/**
 * \brief Adds two nibbles with wrapping (Modulo 16).
 * 0xF + 0x1 = 0x0
 */
static inline vl_nibble_t vlNibbleAdd(vl_nibble_t a, vl_nibble_t b) { return (vl_nibble_t)((a + b) & 0x0F); }

/**
 * \brief Adds two nibbles with saturation (Clamping).
 * 0xF + 0x1 = 0xF
 */
static inline vl_nibble_t vlNibbleAddSat(vl_nibble_t a, vl_nibble_t b)
{
    vl_uint8_t sum = a + b;
    return (sum > 0x0F) ? 0x0F : (vl_nibble_t)sum;
}

/**
 * \brief Subtracts nibble b from a with wrapping (Modulo 16).
 * 0x3 - 0x5 = 0xE
 */
static inline vl_nibble_t vlNibbleSub(vl_nibble_t a, vl_nibble_t b) { return (vl_nibble_t)((a - b) & 0x0F); }

/**
 * \brief Subtracts nibble b from a with saturation (Clamping).
 * 0x3 - 0x5 = 0x0
 */
static inline vl_nibble_t vlNibbleSubSat(vl_nibble_t a, vl_nibble_t b) { return (b > a) ? 0 : (vl_nibble_t)(a - b); }

/**
 * \brief Writes a nibble to the high nibble of a byte.
 * (Write 0xC to 0xAB) -> 0xCB
 */
static inline void vlNibbleSetHigh(vl_uint8_t* dst, vl_nibble_t v) { *dst = (*dst & 0x0F) | (v << 4); }

/**
 * \brief Writes a nibble to the low nibble of a byte.
 * (Write 0xC to 0xAB) -> 0xAC
 */
static inline void vlNibbleSetLow(vl_uint8_t* dst, vl_nibble_t v) { *dst = (*dst & 0xF0) | (v & 0x0F); }

#endif // VL_NIBBLE_H
