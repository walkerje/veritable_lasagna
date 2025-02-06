#include "vl_rand.h"
#include <time.h>
#include <limits.h>

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

//bitmask for lower 23 bits
#define VL_IEEE754_FLOAT32_LOW23        0x007FFFFFu
//bit equivalent of 1.0f
#define VL_IEEE754_FLOAT32_ONE          0x3F800000u

//these are the same as above, but are two copies of it over 8 bytes.
//useful for generating two 32-bit floats at the same time.
#define VL_IEEE754_FLOAT32_LOW23_DW     0x007FFFFF007FFFFFull
#define VL_IEEE754_FLOAT32_ONE_DW       0x3F8000003F800000ull

//bitmask for lower 52 bits
#define VL_IEEE754_FLOAT64_LOW52       0x000FFFFFFFFFFFFFull
//bit equivalent of 1.0
#define VL_IEEE754_FLOAT64_ONE         0x3FF0000000000000ull

vl_rand vlRandInit(){
    vl_rand x = time(NULL);
    vlRandNext(&x);
    return x;
}

vl_rand    vlRandNext(vl_rand* rand){
#ifdef VL_U64_T
    //splitmix64
    vl_rand z = (*rand += 0x9E3779B97F4A7C15ull);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
#else
    //splitmix32
    vl_rand z = (*rand += 0x9E3779B9u);
    z = (z ^ (z >> 16)) * 0x85EBCA6Bu;
    z = (z ^ (z >> 13)) * 0xC2B2AE35u;
    return z ^ (z >> 16);
#endif
}

void vlRandFill(vl_rand* rand, void* mem, vl_memsize_t len){
    const vl_uint_t     rem     = len % sizeof(vl_rand);
    const vl_uint_t     blocks  = (len - rem) / sizeof(vl_rand);
    vl_usmall_t* bytes = mem;
    vl_uint_t i;

    //operate on blocks of 8 bytes at a time...
    for(i = 0; i < blocks; i++){
        *((vl_rand*)bytes) = vlRandNext(rand);
        bytes += sizeof(vl_uint64_t);
    }

    //fill in the last few bytes
    vl_rand last = vlRandNext(rand);
    for(i = 0; i < rem; i++){
        //assign after masking off other bytes
        *bytes = (vl_usmall_t)(last & 0xFF);
        //then, bitshift it off the end of the value.
        last >>= CHAR_BIT;
        bytes++;
    }
}

//these four must be reinterpreted in-definition for return by value
//the random split macro used in the header is also a bitwise reinterpretation

#ifdef VL_I8_T
vl_int8_t vlRandInt8(vl_rand* rand){
    const vl_rand val = vlRandNext(rand);
    return *((const vl_int8_t*)(&val));
}
#endif

#ifdef VL_I16_T
vl_int16_t vlRandInt16(vl_rand* rand){
    const vl_rand val = vlRandNext(rand);
    return *((const vl_int16_t*)(&val));
}
#endif

#ifdef VL_I32_T
vl_int32_t vlRandInt32(vl_rand* rand){
    const vl_rand val = vlRandNext(rand);
    return *((const vl_int32_t*)(&val));
}
#endif

#ifdef VL_I64_T
vl_int64_t vlRandInt64(vl_rand* rand){
    const vl_rand val = vlRandNext(rand);
    return *((const vl_int64_t*)(&val));
}
#endif

vl_float32_t vlRandF(vl_rand* rand){
    //extract 23 bits from the random integer, then use bitwise OR to insert the exponent and sign bits.
    const vl_uint32_t randFloatVal = ((vl_uint32_t)vlRandNext(rand) & VL_IEEE754_FLOAT32_LOW23) | VL_IEEE754_FLOAT32_ONE;
    return (*(float*)(&randFloatVal)) - 1.0f;//subtract 1 because we OR'd in the 1.0 bitfield
}

void vlRandFx2(vl_rand* rand, vl_float32_t* result){
#ifdef VL_U64_T
    //this is the same method as above, but it uses DW (double-wide) equivalent bitfields.
    //doing this bitwise makes it trivial to parallelize with the vl_uint64_t type.
    *((vl_uint64_t*)result) = ((vl_uint64_t) vlRandNext(rand) & VL_IEEE754_FLOAT32_LOW23_DW) | VL_IEEE754_FLOAT32_ONE_DW;
    result[0] -= 1.0f; result[1] -= 1.0f;
#else
    vlRandF(rand, result);
    vlRandF(rand, result + 1);
#endif
}

#ifdef VL_F64_T
vl_float64_t vlRandD(vl_rand* rand){
    //modified version of vlRandF to use extra bits
    const vl_uint64_t randFloatVal = ((vl_uint64_t)vlRandNext(rand) & VL_IEEE754_FLOAT64_LOW52) | VL_IEEE754_FLOAT64_ONE;
    return (*(vl_float64_t*)(&randFloatVal)) - (vl_float64_t)1.0;
}
#endif