#include <stddef.h>
#include <vl/vl_numtypes.h>

#include "vl_memory.h"

//===== Converter Functions =====

#define GEN_CONVERTER(src_type, src_name, dst_type, dst_name)                                                          \
    static void vlConv_##src_name##_to_##dst_name(const void* src, void* dst)                                          \
    {                                                                                                                  \
        *(dst_type*)dst = (dst_type)(*(const src_type*)src);                                                           \
    }

// DSOFFS converters
static void vlConv_DSOFFS_to_DSIDX(const void* src, void* dst)
{
    *(vl_dsidx_t*)dst = (vl_dsidx_t)(*(const vl_dsoffs_t*)src);
}

#ifdef VL_U8_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_dsoffs_t, DSOFFS, vl_float64_t, F64)
#endif

// DSIDX converters
static void vlConv_DSIDX_to_DSOFFS(const void* src, void* dst)
{
    *(vl_dsoffs_t*)dst = (vl_dsoffs_t)(*(const vl_dsidx_t*)src);
}

#ifdef VL_U8_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_dsidx_t, DSIDX, vl_float64_t, F64)
#endif

#ifdef VL_U8_T
// UINT8 converters
GEN_CONVERTER(vl_uint8_t, U8, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_uint8_t, U8, vl_dsidx_t, DSIDX)
#ifdef VL_U16_T
GEN_CONVERTER(vl_uint8_t, U8, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_uint8_t, U8, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_uint8_t, U8, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_uint8_t, U8, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_uint8_t, U8, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_uint8_t, U8, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_uint8_t, U8, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_uint8_t, U8, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_uint8_t, U8, vl_float64_t, F64)
#endif
#endif

#ifdef VL_U16_T
// UINT16 converters
GEN_CONVERTER(vl_uint16_t, U16, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_uint16_t, U16, vl_dsidx_t, DSIDX)
#ifdef VL_U8_T
GEN_CONVERTER(vl_uint16_t, U16, vl_uint8_t, U8)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_uint16_t, U16, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_uint16_t, U16, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_uint16_t, U16, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_uint16_t, U16, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_uint16_t, U16, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_uint16_t, U16, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_uint16_t, U16, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_uint16_t, U16, vl_float64_t, F64)
#endif
#endif

#ifdef VL_U32_T
// UINT32 converters
GEN_CONVERTER(vl_uint32_t, U32, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_uint32_t, U32, vl_dsidx_t, DSIDX)
#ifdef VL_U8_T
GEN_CONVERTER(vl_uint32_t, U32, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_uint32_t, U32, vl_uint16_t, U16)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_uint32_t, U32, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_uint32_t, U32, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_uint32_t, U32, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_uint32_t, U32, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_uint32_t, U32, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_uint32_t, U32, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_uint32_t, U32, vl_float64_t, F64)
#endif
#endif

#ifdef VL_U64_T
// UINT64 converters
GEN_CONVERTER(vl_uint64_t, U64, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_uint64_t, U64, vl_dsidx_t, DSIDX)
#ifdef VL_U8_T
GEN_CONVERTER(vl_uint64_t, U64, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_uint64_t, U64, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_uint64_t, U64, vl_uint32_t, U32)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_uint64_t, U64, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_uint64_t, U64, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_uint64_t, U64, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_uint64_t, U64, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_uint64_t, U64, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_uint64_t, U64, vl_float64_t, F64)
#endif
#endif

#ifdef VL_I8_T
// INT8 converters
GEN_CONVERTER(vl_int8_t, I8, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_int8_t, I8, vl_dsidx_t, DSIDX)
#ifdef VL_U8_T
GEN_CONVERTER(vl_int8_t, I8, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_int8_t, I8, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_int8_t, I8, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_int8_t, I8, vl_uint64_t, U64)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_int8_t, I8, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_int8_t, I8, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_int8_t, I8, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_int8_t, I8, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_int8_t, I8, vl_float64_t, F64)
#endif
#endif

#ifdef VL_I16_T
// INT16 converters
GEN_CONVERTER(vl_int16_t, I16, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_int16_t, I16, vl_dsidx_t, DSIDX)
#ifdef VL_U8_T
GEN_CONVERTER(vl_int16_t, I16, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_int16_t, I16, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_int16_t, I16, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_int16_t, I16, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_int16_t, I16, vl_int8_t, I8)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_int16_t, I16, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_int16_t, I16, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_int16_t, I16, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_int16_t, I16, vl_float64_t, F64)
#endif
#endif

#ifdef VL_I32_T
// INT32 converters
GEN_CONVERTER(vl_int32_t, I32, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_int32_t, I32, vl_dsidx_t, DSIDX)
#ifdef VL_U8_T
GEN_CONVERTER(vl_int32_t, I32, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_int32_t, I32, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_int32_t, I32, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_int32_t, I32, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_int32_t, I32, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_int32_t, I32, vl_int16_t, I16)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_int32_t, I32, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_int32_t, I32, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_int32_t, I32, vl_float64_t, F64)
#endif
#endif

#ifdef VL_I64_T
// INT64 converters
GEN_CONVERTER(vl_int64_t, I64, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_int64_t, I64, vl_dsidx_t, DSIDX)
#ifdef VL_U8_T
GEN_CONVERTER(vl_int64_t, I64, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_int64_t, I64, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_int64_t, I64, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_int64_t, I64, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_int64_t, I64, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_int64_t, I64, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_int64_t, I64, vl_int32_t, I32)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_int64_t, I64, vl_float32_t, F32)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_int64_t, I64, vl_float64_t, F64)
#endif
#endif

#ifdef VL_F32_T
// FLOAT32 converters
GEN_CONVERTER(vl_float32_t, F32, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_float32_t, F32, vl_dsidx_t, DSIDX)
#ifdef VL_U8_T
GEN_CONVERTER(vl_float32_t, F32, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_float32_t, F32, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_float32_t, F32, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_float32_t, F32, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_float32_t, F32, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_float32_t, F32, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_float32_t, F32, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_float32_t, F32, vl_int64_t, I64)
#endif
#ifdef VL_F64_T
GEN_CONVERTER(vl_float32_t, F32, vl_float64_t, F64)
#endif
#endif

#ifdef VL_F64_T
// FLOAT64 converters
GEN_CONVERTER(vl_float64_t, F64, vl_dsoffs_t, DSOFFS)
GEN_CONVERTER(vl_float64_t, F64, vl_dsidx_t, DSIDX)
#ifdef VL_U8_T
GEN_CONVERTER(vl_float64_t, F64, vl_uint8_t, U8)
#endif
#ifdef VL_U16_T
GEN_CONVERTER(vl_float64_t, F64, vl_uint16_t, U16)
#endif
#ifdef VL_U32_T
GEN_CONVERTER(vl_float64_t, F64, vl_uint32_t, U32)
#endif
#ifdef VL_U64_T
GEN_CONVERTER(vl_float64_t, F64, vl_uint64_t, U64)
#endif
#ifdef VL_I8_T
GEN_CONVERTER(vl_float64_t, F64, vl_int8_t, I8)
#endif
#ifdef VL_I16_T
GEN_CONVERTER(vl_float64_t, F64, vl_int16_t, I16)
#endif
#ifdef VL_I32_T
GEN_CONVERTER(vl_float64_t, F64, vl_int32_t, I32)
#endif
#ifdef VL_I64_T
GEN_CONVERTER(vl_float64_t, F64, vl_int64_t, I64)
#endif
#ifdef VL_F32_T
GEN_CONVERTER(vl_float64_t, F64, vl_float32_t, F32)
#endif
#endif

//===== Metadata Array =====

const vl_numtype_info VL_NUMTYPE_INFO[VL_NUMTYPE_MAX] = {
    // VL_NUMTYPE_DSOFFS
    {.type = VL_NUMTYPE_DSOFFS,
     .size = sizeof(vl_dsoffs_t),
     .alignment = VL_ALIGNOF(vl_dsoffs_t),
     .isSigned = VL_FALSE,
     .isInteger = VL_TRUE,
     .isFloating = VL_FALSE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = NULL,
             [VL_NUMTYPE_DSIDX] = vlConv_DSOFFS_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_DSOFFS_to_U8,
#endif
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_DSOFFS_to_U16,
#endif
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_DSOFFS_to_U32,
#endif
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_DSOFFS_to_U64,
#endif
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_DSOFFS_to_I8,
#endif
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_DSOFFS_to_I16,
#endif
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_DSOFFS_to_I32,
#endif
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_DSOFFS_to_I64,
#endif
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_DSOFFS_to_F32,
#endif
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_DSOFFS_to_F64,
#endif
         }},
    // VL_NUMTYPE_DSIDX
    {
        .type = VL_NUMTYPE_DSIDX,
        .size = sizeof(vl_dsidx_t),
        .alignment = VL_ALIGNOF(vl_dsidx_t),
        .isSigned = VL_FALSE,
        .isInteger = VL_TRUE,
        .isFloating = VL_FALSE,
        .typeConverters =
            {
                [VL_NUMTYPE_DSOFFS] = vlConv_DSIDX_to_DSOFFS,
                [VL_NUMTYPE_DSIDX] = NULL,
#ifdef VL_U8_T
                [VL_NUMTYPE_UINT8] = vlConv_DSIDX_to_U8,
#endif
#ifdef VL_U16_T
                [VL_NUMTYPE_UINT16] = vlConv_DSIDX_to_U16,
#endif
#ifdef VL_U32_T
                [VL_NUMTYPE_UINT32] = vlConv_DSIDX_to_U32,
#endif
#ifdef VL_U64_T
                [VL_NUMTYPE_UINT64] = vlConv_DSIDX_to_U64,
#endif
#ifdef VL_I8_T
                [VL_NUMTYPE_INT8] = vlConv_DSIDX_to_I8,
#endif
#ifdef VL_I16_T
                [VL_NUMTYPE_INT16] = vlConv_DSIDX_to_I16,
#endif
#ifdef VL_I32_T
                [VL_NUMTYPE_INT32] = vlConv_DSIDX_to_I32,
#endif
#ifdef VL_I64_T
                [VL_NUMTYPE_INT64] = vlConv_DSIDX_to_I64,
#endif
#ifdef VL_F32_T
                [VL_NUMTYPE_FLOAT32] = vlConv_DSIDX_to_F32,
#endif
#ifdef VL_F64_T
                [VL_NUMTYPE_FLOAT64] = vlConv_DSIDX_to_F64,
#endif
            },
    },

#ifdef VL_U8_T
    // VL_NUMTYPE_UINT8
    {.type = VL_NUMTYPE_UINT8,
     .size = sizeof(vl_uint8_t),
     .alignment = VL_ALIGNOF(vl_uint8_t),
     .isSigned = VL_FALSE,
     .isInteger = VL_TRUE,
     .isFloating = VL_FALSE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_U8_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_U8_to_DSIDX,
             [VL_NUMTYPE_UINT8] = NULL,
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_U8_to_U16,
#endif
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_U8_to_U32,
#endif
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_U8_to_U64,
#endif
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_U8_to_I8,
#endif
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_U8_to_I16,
#endif
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_U8_to_I32,
#endif
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_U8_to_I64,
#endif
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_U8_to_F32,
#endif
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_U8_to_F64,
#endif
         }},
#endif

#ifdef VL_U16_T
    // VL_NUMTYPE_UINT16
    {.type = VL_NUMTYPE_UINT16,
     .size = sizeof(vl_uint16_t),
     .alignment = VL_ALIGNOF(vl_uint16_t),
     .isSigned = VL_FALSE,
     .isInteger = VL_TRUE,
     .isFloating = VL_FALSE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_U16_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_U16_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_U16_to_U8,
#endif
             [VL_NUMTYPE_UINT16] = NULL,
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_U16_to_U32,
#endif
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_U16_to_U64,
#endif
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_U16_to_I8,
#endif
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_U16_to_I16,
#endif
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_U16_to_I32,
#endif
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_U16_to_I64,
#endif
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_U16_to_F32,
#endif
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_U16_to_F64,
#endif
         }},
#endif

#ifdef VL_U32_T
    // VL_NUMTYPE_UINT32
    {.type = VL_NUMTYPE_UINT32,
     .size = sizeof(vl_uint32_t),
     .alignment = VL_ALIGNOF(vl_uint32_t),
     .isSigned = VL_FALSE,
     .isInteger = VL_TRUE,
     .isFloating = VL_FALSE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_U32_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_U32_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_U32_to_U8,
#endif
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_U32_to_U16,
#endif
             [VL_NUMTYPE_UINT32] = NULL,
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_U32_to_U64,
#endif
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_U32_to_I8,
#endif
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_U32_to_I16,
#endif
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_U32_to_I32,
#endif
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_U32_to_I64,
#endif
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_U32_to_F32,
#endif
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_U32_to_F64,
#endif
         }},
#endif

#ifdef VL_U64_T
    // VL_NUMTYPE_UINT64
    {.type = VL_NUMTYPE_UINT64,
     .size = sizeof(vl_uint64_t),
     .alignment = VL_ALIGNOF(vl_uint64_t),
     .isSigned = VL_FALSE,
     .isInteger = VL_TRUE,
     .isFloating = VL_FALSE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_U64_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_U64_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_U64_to_U8,
#endif
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_U64_to_U16,
#endif
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_U64_to_U32,
#endif
             [VL_NUMTYPE_UINT64] = NULL,
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_U64_to_I8,
#endif
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_U64_to_I16,
#endif
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_U64_to_I32,
#endif
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_U64_to_I64,
#endif
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_U64_to_F32,
#endif
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_U64_to_F64,
#endif
         }},
#endif

#ifdef VL_I8_T
    // VL_NUMTYPE_INT8
    {.type = VL_NUMTYPE_INT8,
     .size = sizeof(vl_int8_t),
     .alignment = VL_ALIGNOF(vl_int8_t),
     .isSigned = VL_TRUE,
     .isInteger = VL_TRUE,
     .isFloating = VL_FALSE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_I8_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_I8_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_I8_to_U8,
#endif
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_I8_to_U16,
#endif
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_I8_to_U32,
#endif
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_I8_to_U64,
#endif
             [VL_NUMTYPE_INT8] = NULL,
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_I8_to_I16,
#endif
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_I8_to_I32,
#endif
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_I8_to_I64,
#endif
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_I8_to_F32,
#endif
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_I8_to_F64,
#endif
         }},
#endif

#ifdef VL_I16_T
    // VL_NUMTYPE_INT16
    {.type = VL_NUMTYPE_INT16,
     .size = sizeof(vl_int16_t),
     .alignment = VL_ALIGNOF(vl_int16_t),
     .isSigned = VL_TRUE,
     .isInteger = VL_TRUE,
     .isFloating = VL_FALSE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_I16_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_I16_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_I16_to_U8,
#endif
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_I16_to_U16,
#endif
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_I16_to_U32,
#endif
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_I16_to_U64,
#endif
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_I16_to_I8,
#endif
             [VL_NUMTYPE_INT16] = NULL,
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_I16_to_I32,
#endif
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_I16_to_I64,
#endif
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_I16_to_F32,
#endif
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_I16_to_F64,
#endif
         }},
#endif

#ifdef VL_I32_T
    // VL_NUMTYPE_INT32
    {.type = VL_NUMTYPE_INT32,
     .size = sizeof(vl_int32_t),
     .alignment = VL_ALIGNOF(vl_int32_t),
     .isSigned = VL_TRUE,
     .isInteger = VL_TRUE,
     .isFloating = VL_FALSE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_I32_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_I32_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_I32_to_U8,
#endif
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_I32_to_U16,
#endif
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_I32_to_U32,
#endif
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_I32_to_U64,
#endif
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_I32_to_I8,
#endif
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_I32_to_I16,
#endif
             [VL_NUMTYPE_INT32] = NULL,
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_I32_to_I64,
#endif
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_I32_to_F32,
#endif
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_I32_to_F64,
#endif
         }},
#endif

#ifdef VL_I64_T
    // VL_NUMTYPE_INT64
    {.type = VL_NUMTYPE_INT64,
     .size = sizeof(vl_int64_t),
     .alignment = VL_ALIGNOF(vl_int64_t),
     .isSigned = VL_TRUE,
     .isInteger = VL_TRUE,
     .isFloating = VL_FALSE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_I64_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_I64_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_I64_to_U8,
#endif
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_I64_to_U16,
#endif
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_I64_to_U32,
#endif
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_I64_to_U64,
#endif
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_I64_to_I8,
#endif
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_I64_to_I16,
#endif
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_I64_to_I32,
#endif
             [VL_NUMTYPE_INT64] = NULL,
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_I64_to_F32,
#endif
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_I64_to_F64,
#endif
         }},
#endif

#ifdef VL_F32_T
    // VL_NUMTYPE_FLOAT32
    {.type = VL_NUMTYPE_FLOAT32,
     .size = sizeof(vl_float32_t),
     .alignment = VL_ALIGNOF(vl_float32_t),
     .isSigned = VL_TRUE,
     .isInteger = VL_FALSE,
     .isFloating = VL_TRUE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_F32_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_F32_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_F32_to_U8,
#endif
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_F32_to_U16,
#endif
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_F32_to_U32,
#endif
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_F32_to_U64,
#endif
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_F32_to_I8,
#endif
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_F32_to_I16,
#endif
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_F32_to_I32,
#endif
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_F32_to_I64,
#endif
             [VL_NUMTYPE_FLOAT32] = NULL,
#ifdef VL_F64_T
             [VL_NUMTYPE_FLOAT64] = vlConv_F32_to_F64,
#endif
         }},
#endif

#ifdef VL_F64_T
    // VL_NUMTYPE_FLOAT64
    {.type = VL_NUMTYPE_FLOAT64,
     .size = sizeof(vl_float64_t),
     .alignment = VL_ALIGNOF(vl_float64_t),
     .isSigned = VL_TRUE,
     .isInteger = VL_FALSE,
     .isFloating = VL_TRUE,
     .typeConverters =
         {
             [VL_NUMTYPE_DSOFFS] = vlConv_F64_to_DSOFFS,
             [VL_NUMTYPE_DSIDX] = vlConv_F64_to_DSIDX,
#ifdef VL_U8_T
             [VL_NUMTYPE_UINT8] = vlConv_F64_to_U8,
#endif
#ifdef VL_U16_T
             [VL_NUMTYPE_UINT16] = vlConv_F64_to_U16,
#endif
#ifdef VL_U32_T
             [VL_NUMTYPE_UINT32] = vlConv_F64_to_U32,
#endif
#ifdef VL_U64_T
             [VL_NUMTYPE_UINT64] = vlConv_F64_to_U64,
#endif
#ifdef VL_I8_T
             [VL_NUMTYPE_INT8] = vlConv_F64_to_I8,
#endif
#ifdef VL_I16_T
             [VL_NUMTYPE_INT16] = vlConv_F64_to_I16,
#endif
#ifdef VL_I32_T
             [VL_NUMTYPE_INT32] = vlConv_F64_to_I32,
#endif
#ifdef VL_I64_T
             [VL_NUMTYPE_INT64] = vlConv_F64_to_I64,
#endif
#ifdef VL_F32_T
             [VL_NUMTYPE_FLOAT32] = vlConv_F64_to_F32,
#endif
             [VL_NUMTYPE_FLOAT64] = NULL,
         }},
#endif
};
