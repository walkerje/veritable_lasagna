#ifndef VL_NUMTYPES_H
#define VL_NUMTYPES_H
#include <vl/vl_libconfig.h>

/**
 * \brief Byte offset type for data structures.
 */
typedef VL_STRUCTURE_OFFSET_T   vl_dsoffs_t;

/**
 * \brief Index type for data structures.
 */
typedef VL_STRUCTURE_INDEX_T    vl_dsidx_t;

#ifdef VL_U8_T
/**
 * \brief Unsigned 8-bit integer type.
 */
typedef VL_U8_T             vl_uint8_t;
#endif

#ifdef VL_U16_T
/**
 * \brief Unsigned 16-bit integer type.
 */
typedef VL_U16_T            vl_uint16_t;
#endif

#ifdef VL_U32_T
/**
 * \brief Unsigned 32-bit integer type.
 */
typedef VL_U32_T            vl_uint32_t;
#endif

#ifdef VL_U64_T
/**
 * \brief Unsigned 64-bit integer type.
 */
typedef VL_U64_T            vl_uint64_t;
#endif

#ifdef VL_I8_T
/**
 * \brief Signed 16-bit integer type.
 */
typedef VL_I8_T             vl_int8_t;
#endif

#ifdef VL_I16_T
/**
 * \brief Signed 16-bit integer type.
 */
typedef VL_I16_T            vl_int16_t;
#endif

#ifdef VL_I32_T
/**
 * \brief Signed 32-bit integer type.
 */
typedef VL_I32_T            vl_int32_t;
#endif

#ifdef VL_I64_T
/**
 * \brief Signed 64-bit integer type.
 */
typedef VL_I64_T            vl_int64_t;
#endif

/**
 * \brief Largest available unsigned integer type.
 */
typedef VL_ULARGE_T         vl_ularge_t;
/**
 * \brief Largest available signed integer type.
 */
typedef VL_ILARGE_T         vl_ilarge_t;
/**
 * \brief Smallest available unsigned integer type.
 */
typedef VL_USMALL_T         vl_usmall_t;
/**
 * \brief Smallest available signed integer type.
 */
typedef VL_ISMALL_T         vl_ismall_t;

/**
 * \brief Standard signed integer type.
 */
typedef VL_INT_T            vl_int_t;

/**
 * \brief Standard unsigned integer type.
 */
typedef VL_UINT_T           vl_uint_t;

/**
 * \brief Unsigned integer type suitable for expressing memory addresses.
 */
typedef VL_UPTR_T           vl_uintptr_t;

/**
 * \brief Signed integer type suitable for expressing memory addresses.
 */
typedef VL_IPTR_T           vl_intptr_t;

/**
 * \brief 32-bit floating point number type.
 */
typedef VL_F32_T            vl_float32_t;

#ifdef VL_F64_T
/**
 * \brief 64-bit floating point number type.
 */
typedef VL_F64_T            vl_float64_t;
#endif

/**
 * \brief Highest precision floating point number type.
 */
typedef VL_FHIGHP_T         vl_float_highp_t;

#define VL_TRUE     1
#define VL_FALSE    0
#define VL_BOOL_T   VL_USMALL_T
typedef VL_BOOL_T   vl_bool_t;

#endif //VL_NUMTYPES_H