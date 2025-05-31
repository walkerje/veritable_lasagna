#include "vl_compare.h"

#ifndef VL_DEREF
#define VL_DEREF(x, y) *((const x*)y)
#endif

#define VL_COMPARE_FUNC(funcName, type)                             \
vl_int_t funcName (const void* dataA, const void* dataB){           \
    return  (VL_DEREF(type, dataA) > VL_DEREF(type, dataB)) -       \
            (VL_DEREF(type, dataA) < VL_DEREF(type, dataB));        \
}                                                                   \
vl_int_t funcName##Reverse (const void* dataA, const void* dataB){  \
    return  (VL_DEREF(type, dataB) > VL_DEREF(type, dataA)) -       \
            (VL_DEREF(type, dataB) < VL_DEREF(type, dataA));        \
}

#ifdef VL_I8_T

VL_COMPARE_FUNC(vlCompareInt8, vl_int8_t);

VL_COMPARE_FUNC(vlCompareUInt8, vl_uint8_t);
#endif

#ifdef VL_I16_T

VL_COMPARE_FUNC(vlCompareInt16, vl_int16_t);

VL_COMPARE_FUNC(vlCompareUInt16, vl_uint16_t);
#endif

#ifdef VL_I32_T

VL_COMPARE_FUNC(vlCompareInt32, vl_int32_t);

VL_COMPARE_FUNC(vlCompareUInt32, vl_uint32_t);
#endif

#ifdef VL_I64_T

VL_COMPARE_FUNC(vlCompareInt64, vl_int64_t);

VL_COMPARE_FUNC(vlCompareUInt64, vl_uint64_t);
#endif

#ifdef VL_INT_T

VL_COMPARE_FUNC(vlCompareInt, vl_int_t);

VL_COMPARE_FUNC(vlCompareUInt, vl_uint_t);
#endif

#ifdef VL_F32_T

VL_COMPARE_FUNC(vlCompareFloat32, vl_float32_t);
#endif

#ifdef VL_F64_T

VL_COMPARE_FUNC(vlCompareFloat64, vl_float64_t);
#endif

