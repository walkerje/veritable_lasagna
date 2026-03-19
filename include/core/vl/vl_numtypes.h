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

#ifndef VL_NUMTYPES_H
#define VL_NUMTYPES_H

#include <vl/vl_libconfig.h>

typedef enum vl_numtype_
{
    VL_NUMTYPE_DSOFFS,
    VL_NUMTYPE_DSIDX,

#ifdef VL_U8_T
    VL_NUMTYPE_UINT8,
#endif

#ifdef VL_U16_T
    VL_NUMTYPE_UINT16,
#endif

#ifdef VL_U32_T
    VL_NUMTYPE_UINT32,
#endif

#ifdef VL_U64_T
    VL_NUMTYPE_UINT64,
#endif

#ifdef VL_I8_T
    VL_NUMTYPE_INT8,
#endif

#ifdef VL_I16_T
    VL_NUMTYPE_INT16,
#endif

#ifdef VL_I32_T
    VL_NUMTYPE_INT32,
#endif

#ifdef VL_I64_T
    VL_NUMTYPE_INT64,
#endif

#ifdef VL_F32_T
    VL_NUMTYPE_FLOAT32,
#endif

#ifdef VL_F64_T
    VL_NUMTYPE_FLOAT64,
#endif

    VL_NUMTYPE_MAX
} vl_numtype;

/**
 * \brief Byte offset type for data structures.
 */
typedef VL_STRUCTURE_OFFSET_T vl_dsoffs_t;

/**
 * \brief Index type for data structures.
 */
typedef VL_STRUCTURE_INDEX_T vl_dsidx_t;

#ifdef VL_U8_T
/**
 * \brief Unsigned 8-bit integer type.
 */
typedef VL_U8_T vl_uint8_t;
#endif

#ifdef VL_U16_T
/**
 * \brief Unsigned 16-bit integer type.
 */
typedef VL_U16_T vl_uint16_t;
#endif

#ifdef VL_U32_T
/**
 * \brief Unsigned 32-bit integer type.
 */
typedef VL_U32_T vl_uint32_t;
#endif

#ifdef VL_U64_T
/**
 * \brief Unsigned 64-bit integer type.
 */
typedef VL_U64_T vl_uint64_t;
#endif

#ifdef VL_I8_T
/**
 * \brief Signed 8-bit integer type.
 */
typedef VL_I8_T vl_int8_t;
#endif

#ifdef VL_I16_T
/**
 * \brief Signed 16-bit integer type.
 */
typedef VL_I16_T vl_int16_t;
#endif

#ifdef VL_I32_T
/**
 * \brief Signed 32-bit integer type.
 */
typedef VL_I32_T vl_int32_t;
#endif

#ifdef VL_I64_T
/**
 * \brief Signed 64-bit integer type.
 */
typedef VL_I64_T vl_int64_t;
#endif

/**
 * \brief Largest available unsigned integer type.
 */
typedef VL_ULARGE_T vl_ularge_t;
/**
 * \brief Largest available signed integer type.
 */
typedef VL_ILARGE_T vl_ilarge_t;
/**
 * \brief Smallest available unsigned integer type.
 */
typedef VL_USMALL_T vl_usmall_t;
/**
 * \brief Smallest available signed integer type.
 */
typedef VL_ISMALL_T vl_ismall_t;

/**
 * \brief Standard signed integer type.
 */
typedef VL_INT_T vl_int_t;

/**
 * \brief Standard unsigned integer type.
 */
typedef VL_UINT_T vl_uint_t;

/**
 * \brief Unsigned integer type suitable for expressing memory addresses.
 */
typedef VL_UPTR_T vl_uintptr_t;

/**
 * \brief Signed integer type suitable for expressing memory addresses.
 */
typedef VL_IPTR_T vl_intptr_t;

/**
 * \brief 32-bit floating point number type.
 */
typedef VL_F32_T vl_float32_t;

#ifdef VL_F64_T
/**
 * \brief 64-bit floating point number type.
 */
typedef VL_F64_T vl_float64_t;
#endif

/**
 * \brief Highest precision floating point number type.
 */
typedef VL_FHIGHP_T vl_float_highp_t;

#define VL_TRUE 1
#define VL_FALSE 0

#define VL_BOOL_T VL_USMALL_T
typedef VL_BOOL_T vl_bool_t;

/**
 * \brief Function pointer type for converting between numeric types.
 *
 * Converts a source value (as void*) to destination (void*).
 * Both pointers must reference valid memory for their respective types.
 *
 * \param src Pointer to source value
 * \param dst Pointer to destination value
 */
typedef void (*vl_numtype_converter)(const void* src, void* dst);

/**
 * \brief Function pointer type for converting between numeric types.
 *
 * Converts a source value (as void*) to destination (void*).
 * Both pointers must reference valid memory for their respective types.
 *
 * \param src Pointer to source value
 * \param dst Pointer to destination value
 */
typedef void (*vl_numtype_normalizer)(const void* src, void* dst);

typedef struct vl_numtype_info_
{
    vl_numtype type;
    vl_uint16_t size;
    vl_uint16_t alignment;
    vl_bool_t isSigned;
    vl_bool_t isInteger;
    vl_bool_t isFloating;

    vl_numtype_converter typeConverters[VL_NUMTYPE_MAX];
} vl_numtype_info;

/**
 * \brief Provides runtime numeric type introspection.
 */
VL_API extern const vl_numtype_info VL_NUMTYPE_INFO[VL_NUMTYPE_MAX];

/**
 * \brief Converts a value from one numeric type to another.
 *
 * Provided pointers must be properly aligned for their respective types.
 *
 * \param src Pointer to source value
 * \param srcType Source numeric type
 * \param dst Pointer to destination value
 * \param dstType Destination numeric type
 */
static inline void vlNumTypeCast(const void* src, vl_numtype srcType, void* dst, vl_numtype dstType)
{
    VL_NUMTYPE_INFO[srcType].typeConverters[dstType](src, dst);
}

static inline vl_uint16_t vlNumTypeSizeof(vl_numtype type) { return VL_NUMTYPE_INFO[type].size; }

#endif // VL_NUMTYPES_H
