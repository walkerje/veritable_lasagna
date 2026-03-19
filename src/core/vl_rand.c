#include "vl_rand.h"

#include <limits.h>
#include <string.h>
#include <time.h>

#ifdef VL_IEEE754_FLOAT32_LOW23
#undef VL_IEEE754_FLOAT32_LOW23
#endif

#ifdef VL_IEEE754_FLOAT32_ONE
#undef VL_IEEE754_FLOAT32_ONE
#endif

#ifdef VL_IEEE754_FLOAT32_LOW23_DW
#undef VL_IEEE754_FLOAT32_LOW23_DW
#endif

#ifdef VL_IEEE754_FLOAT32_ONE_DW
#undef VL_IEEE754_FLOAT32_ONE_DW
#endif

#ifdef VL_IEEE754_FLOAT64_LOW52
#undef VL_IEEE754_FLOAT64_LOW52
#endif

#ifdef VL_IEEE754_FLOAT64_ONE
#undef VL_IEEE754_FLOAT64_ONE
#endif

// bitmask for lower 23 bits
#define VL_IEEE754_FLOAT32_LOW23 0x007FFFFFu
// bit equivalent of 1.0f
#define VL_IEEE754_FLOAT32_ONE 0x3F800000u

// these are the same as above, but are two copies of it over 8 bytes.
// useful for generating two 32-bit floats at the same time.
#define VL_IEEE754_FLOAT32_LOW23_DW 0x007FFFFF007FFFFFull
#define VL_IEEE754_FLOAT32_ONE_DW 0x3F8000003F800000ull

// bitmask for lower 52 bits
#define VL_IEEE754_FLOAT64_LOW52 0x000FFFFFFFFFFFFFull
// bit equivalent of 1.0
#define VL_IEEE754_FLOAT64_ONE 0x3FF0000000000000ull

vl_rand vlRandInit(void)
{
    vl_rand x = time(NULL);
    vlRandNext(&x);
    return x;
}

vl_rand vlRandNext(vl_rand* rand)
{
#ifdef VL_U64_T
    // splitmix64
    vl_rand z = (*rand += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
#else
    // splitmix32
    vl_rand z = (*rand += 0x9E3779B9u);
    z = (z ^ (z >> 16)) * 0x85EBCA6Bu;
    z = (z ^ (z >> 13)) * 0xC2B2AE35u;
    return z ^ (z >> 16);
#endif
}

void vlRandFill(vl_rand* rand, void* mem, vl_memsize_t len)
{
    vl_usmall_t* bytes = (vl_usmall_t*)mem;
    vl_memsize_t remaining = len;

    // Align to 8-byte boundary if necessary
    const vl_memsize_t alignment = sizeof(vl_rand);
    const vl_memsize_t misalignment = (vl_uintptr_t)bytes % alignment;

    if (misalignment != 0)
    {
        // Fill leading unaligned bytes
        const vl_memsize_t align_bytes = alignment - misalignment;
        const vl_memsize_t to_fill = (align_bytes < remaining) ? align_bytes : remaining;

        vl_rand last = vlRandNext(rand);
        for (vl_memsize_t i = 0; i < to_fill; i++)
        {
            *bytes++ = (vl_usmall_t)(last & 0xFF);
            last >>= CHAR_BIT;
        }
        remaining -= to_fill;
    }

    // Process aligned blocks (use memcpy to avoid alignment/aliasing UB)
    const vl_memsize_t blocks = remaining / sizeof(vl_rand);
    for (vl_memsize_t i = 0; i < blocks; i++)
    {
        const vl_rand next = vlRandNext(rand);
        memcpy(bytes, &next, sizeof(next));
        bytes += sizeof(vl_rand);
    }
    remaining -= blocks * sizeof(vl_rand);

    // Fill remaining trailing bytes
    if (remaining > 0)
    {
        vl_rand last = vlRandNext(rand);
        for (vl_memsize_t i = 0; i < remaining; i++)
        {
            *bytes++ = (vl_usmall_t)(last & 0xFF);
            last >>= CHAR_BIT;
        }
    }
}

#ifdef VL_I8_T

vl_int8_t vlRandInt8(vl_rand* rand)
{
    const vl_rand val = vlRandNext(rand);
    vl_int8_t out = 0;
    memcpy(&out, &val, sizeof(out));
    return out;
}

#endif

#ifdef VL_I16_T

vl_int16_t vlRandInt16(vl_rand* rand)
{
    const vl_rand val = vlRandNext(rand);
    vl_int16_t out = 0;
    memcpy(&out, &val, sizeof(out));
    return out;
}

#endif

#ifdef VL_I32_T

vl_int32_t vlRandInt32(vl_rand* rand)
{
    const vl_rand val = vlRandNext(rand);
    vl_int32_t out = 0;
    memcpy(&out, &val, sizeof(out));
    return out;
}

#endif

#ifdef VL_I64_T

vl_int64_t vlRandInt64(vl_rand* rand)
{
    const vl_rand val = vlRandNext(rand);
    vl_int64_t out = 0;
    memcpy(&out, &val, sizeof(out));
    return out;
}

#endif

vl_float32_t vlRandF(vl_rand* rand)
{
    // extract 23 bits, set exponent to 127 to produce [1,2), then subtract 1 =>
    // [0,1)
    const vl_uint32_t bits = ((vl_uint32_t)vlRandNext(rand) & VL_IEEE754_FLOAT32_LOW23) | VL_IEEE754_FLOAT32_ONE;

    vl_float32_t out = 0.0f;
    memcpy(&out, &bits, sizeof(out));
    return out - 1.0f;
}

void vlRandFx2(vl_rand* rand, vl_float32_t* result)
{
#ifdef VL_U64_T
    const vl_uint64_t bits = ((vl_uint64_t)vlRandNext(rand) & VL_IEEE754_FLOAT32_LOW23_DW) | VL_IEEE754_FLOAT32_ONE_DW;

    memcpy(result, &bits, sizeof(bits));
    result[0] -= 1.0f;
    result[1] -= 1.0f;
#else
    result[0] = vlRandF(rand);
    result[1] = vlRandF(rand);
#endif
}

#ifdef VL_F64_T

vl_float64_t vlRandD(vl_rand* rand)
{
    const vl_uint64_t bits = ((vl_uint64_t)vlRandNext(rand) & VL_IEEE754_FLOAT64_LOW52) | VL_IEEE754_FLOAT64_ONE;

    vl_float64_t out = 0.0;
    memcpy(&out, &bits, sizeof(out));
    return out - (vl_float64_t)1.0;
}

#endif
