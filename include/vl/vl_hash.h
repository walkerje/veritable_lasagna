#ifndef VL_HASH_H
#define VL_HASH_H

#include "vl_memory.h"

/**
 * \brief Hash function return type.
 */
typedef vl_ularge_t vl_hash;

/**
 * \brief Hash function typedef.
 *
 * This function definition
 *
 * \param data      read-only pointer to the data that will be hashed
 * \param dataSize  length of the data, in bytes. Usually this can be ignored unless the data is variable in length.
 */
typedef vl_hash (*vl_hash_function)(const void* data, vl_memsize_t dataSize);

#define VL_HASH_STRING  vlHashString
#define VL_HASH_BYTES   vlHashString
#define VL_HASH_UINT8   vlHash8
#define VL_HASH_UINT16  vlHash16
#define VL_HASH_UINT32  vlHash32
#define VL_HASH_UINT64  vlHash64
#define VL_HASH_INT8    vlHash8
#define VL_HASH_INT16   vlHash16
#define VL_HASH_INT32   vlHash32
#define VL_HASH_INT64   vlHash64
#define VL_HASH_FLOAT   vlHashFloat
#define VL_HASH_DOUBLE  vlHashDouble

#define vlHashChar          vlHash8
#define vlHashShort         vlHash16
#define vlHashInt           vlHash32
#define vlHashLong          vlHash64
#define vlHashUChar         vlHash8
#define vlHashUShort        vlHash16
#define vlHashUInt          vlHash32
#define vlHashULong         vlHash64

/**
 * \brief Hashes the specified string.
 *
 * Uses the FNV-1A-64 string hashing algorithm.
 * See here: https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 *
 * \param data      read-only pointer to the data that will be hashed
 * \param dataSize  length of the data, in bytes. Usually this can be ignored unless the data is variable in length.
 * \return corresponding hash code.
 */
vl_hash vlHashString                        (const void* data, vl_memsize_t dataSize);

/**
 * \brief Generates a hash code for the 8-bit sequence at the specified address.
 *
 * This is a direct bitcast of an 8-bit sequence to a 64-bit sequence.
 * The returned value will equal the value of the original sequence.
 *
 * \param data pointer
 * \return hash code
 */
vl_hash vlHash8     (const void* data, vl_memsize_t);

/**
 * \brief Generates a hash code for the 16-bit sequence at the specified address.
 * This is a direct bitcast of an 8-bit sequence to a 64-bit sequence.
 * The returned value will equal the value of the original sequence.
 * \param data pointer
 * \return hash code
 */
vl_hash vlHash16    (const void* data, vl_memsize_t);

/**
 * \brief Generates a hash code for the 32-bit sequence at the specified address.
 *
 * This is a direct bitcast of an 32-bit sequence to a 64-bit sequence.
 * The returned value will equal the value of the original sequence.
 *
 * \param data pointer
 * \return hash code
 */
vl_hash vlHash32    (const void* data, vl_memsize_t);

/**
 * \brief Generates a hash code for the 64-bit sequence at the specified address.
 *
 * The returned value will equal the value of the original sequence.
 *
 * \param data pointer
 * \return hash code
 */
vl_hash vlHash64    (const void* data, vl_memsize_t);

#ifndef vlHashCombine
/**
 * \brief Combine two instances of vl_hash.
 *
 * Based on the algorithm in the Boost C++ libraries by the same name.
 * The magic number in this macro is intended to act as random bits,
 * where each is equally likely to be a 1 or 0 with no relationships.
 *
 * \param a vl_hash value
 * \param b vl_hash value
 * \return merged hash
 */
#define vlHashCombine(a, b) (vl_hash)(((a) ^ (b)) + 0b11101111100100101101101011001011 + ((a) << 6) + ((a) >> 2))
#endif

#endif //VL_HASH_H
