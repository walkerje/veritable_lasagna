#ifndef VL_COMPARE_H
#define VL_COMPARE_H

#include "vl_numtypes.h"

/**
 * The compare function typedef.
 * Used for sorting.
 *
 * In most cases, the following holds true for sorting in ascending order:
 * Should return < 0 if dataA > dataB.
 * Should return > 0 if dataA < dataB.
 * Should return 0 if dataA == dataB.
 *
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 */
typedef vl_int_t (*vl_compare_function)(const void *dataA, const void *dataB);

#ifdef VL_I8_T

/**
 * \brief Compares two signed 8-bit integers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareInt8(const void *dataA, const void *dataB);

/**
 * \brief Compares two signed 8-bit integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareInt8Reverse(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned 8-bit integers
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareUInt8(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned 8-bit integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareUInt8Reverse(const void *dataA, const void *dataB);

#endif

#ifdef VL_I16_T

/**
 * \brief Compares two signed 16-bit integers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareInt16(const void *dataA, const void *dataB);

/**
 * \brief Compares two signed 16-bit integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareInt16Reverse(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned 16-bit integers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareUInt16(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned 16-bit integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareUInt16Reverse(const void *dataA, const void *dataB);

#endif

#ifdef VL_I32_T

/**
 * \brief Compares two signed 32-bit integers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareInt32(const void *dataA, const void *dataB);

/**
 * \brief Compares two signed 32-bit integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareInt32Reverse(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned 32-bit integers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareUInt32(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned 32-bit integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareUInt32Reverse(const void *dataA, const void *dataB);

#endif

#ifdef VL_I64_T

/**
 * \brief Compares two signed 64-bit integers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareInt64(const void *dataA, const void *dataB);

/**
 * \brief Compares two signed 64-bit integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareInt64Reverse(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned 64-bit integers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareUInt64(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned 32-bit integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareUInt64Reverse(const void *dataA, const void *dataB);

#endif

#ifdef VL_INT_T

/**
 * \brief Compares two integers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareInt(const void *dataA, const void *dataB);

/**
 * \brief Compares two signed integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareIntReverse(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned integers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareUInt(const void *dataA, const void *dataB);

/**
 * \brief Compares two unsigned integers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareUIntReverse(const void *dataA, const void *dataB);

#endif

#ifdef VL_F32_T

/**
 * \brief Compare two 32-bit floating point numbers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareFloat32(const void *dataA, const void *dataB);

/**
 * \brief Compare two 64-bit floating point numbers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareFloat32Reverse(const void *dataA, const void *dataB);

#endif

#ifdef VL_F64_T

/**
 * \brief Compare two 64-bit floating point numbers.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *A - *B
 */
VL_API vl_int_t vlCompareFloat64(const void *dataA, const void *dataB);

/**
 * \brief Compare two 64-bit floating point numbers, in reverse.
 * \param dataA pointer to first datum
 * \param dataB pointer to second datum
 * \sa vl_compare_function
 * \par Complexity of O(1) constant.
 * \return *B - *A
 */
VL_API vl_int_t vlCompareFloat64Reverse(const void *dataA, const void *dataB);

#endif

#endif //VL_COMPARE_H
