#ifndef VL_RAND_H
#define VL_RAND_H

#include "vl_numtypes.h"
#include "vl_memory.h"

#ifndef VL_H_RANDOM_SPLIT
/**
 * \brief This macro distributes random bytes to the specified pointer.
 *
 * This macro results in a reinterpretation suitable for all integer types,
 * but not for floating point types.
 *
 * The total number of bytes distributed is equal to sizeof(vl_ularge_t).
 *
 * \private
 */
#define VL_H_RANDOM_SPLIT(randPtr, resultPtr) (*((vl_ularge_t*)(resultPtr)) = vlRandNext(randPtr))
#endif

/**
 * \brief Random state type.
 *
 * The PRNG algorithm used by the VL library is splitmix64 or splitmix32.
 * It offers fast pseudo-random number generation and passes Big Crush with minimal state.
 * All vlRand* functions are almost entirely branchless.
 */
typedef vl_ularge_t vl_rand;

/**
 * \brief Initializes a random state (aka, seed) based on the current time.
 *
 * This function can be skipped entirely, assuming another seed integer can be provided.
 *
 * \par Complexity of O(1) constant.
 * \return random state/seed.
 */
VL_API vl_rand vlRandInit();

/**
 * \brief Returns the "next" 64-bit integer.
 *
 * This function has a side effect of updating the random state.
 *
 * \par Complexity of O(1) constant.
 * \param rand pointer to random state
 * \return unsigned 64-bit integer
 */
VL_API vl_rand vlRandNext(vl_rand *rand);

/**
 * \brief Fills the specified region of memory with random bytes.
 *
 * This function operates on blocks of 8 bytes at a time, until it
 * fills in the last few remaining bytes.
 *
 * \param rand rand state
 * \param mem destination
 * \param len total number of bytes to fill.
 * \par Complexity of O(n) linear.
 */
VL_API void vlRandFill(vl_rand *rand, void *mem, vl_ularge_t len);

#ifdef VL_U8_T
/**
 * \brief Generate a random unsigned 8-bit integer.
 *
 * Range of returned value is in 0...UINT8_MAX
 *
 * \par randPtr pointer to random state
 * \return random integer
 */
#define vlRandUInt8(randPtr)    (vl_uint8_t)   (vlRandNext(randPtr))

#ifdef VL_U64_T
/**
 * \brief Generate eight random unsigned 8-bit integers.
 *
 * Range of returned value is in 0...UINT8_MAX
 * \par randPtr pointer to random state
 * \par resultPtr pointer to where the result will be stored
 */
#define vlRandUInt8x8(randPtr, resultPtr) VL_H_RANDOM_SPLIT(randPtr, resultPtr)
#elif defined VL_U32_T
/**
 * \brief Generate eight random unsigned 8-bit integers.
 *
 * Range of returned value is in 0...UINT8_MAX.
 * \par randPtr pointer to random state
 * \par resultPtr pointer to where the result will be stored
 */
#define vlRandUInt8x8(randPtr, resultPtr)   VL_H_RANDOM_SPLIT(randPtr, resultPtr); \
                                            VL_H_RANDOM_SPLIT(randPtr, ((VL_U8_T*)(resultPtr)) + 4)
#endif
#endif

#ifdef VL_U16_T
/**
 * \brief Generate a random unsigned 16-bit integer.
 *
 * Range of returned value is in 0...UINT16_MAX
 *
 * \param randPtr pointer to random state
 * \par Complexity of O(1) constant.
 * \return random integer
 */
#define vlRandUInt16(randPtr)   (vl_uint16_t)  (vlRandNext(randPtr))

#ifdef VL_U64_T
/**
 * \brief Generates four random unsigned 16-bit integers.
 *
 * Range of returned value is in 0...UINT16_MAX
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandUInt16x4(randPtr, resultPtr) VL_H_RANDOM_SPLIT(randPtr, resultPtr)
#elif defined VL_U32_T
/**
 * \brief Generates four random unsigned 16-bit integers.
 *
 * Range of returned value is in 0...UINT16_MAX
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandUInt16x4(randPtr, resultPtr)  VL_H_RANDOM_SPLIT(randPtr, resultPtr); \
                                            VL_H_RANDOM_SPLIT(randPtr, ((VL_U16_T*)(resultPtr)) + 2)
#endif
#endif

#ifdef VL_U32_T
/**
 * \brief Generate a random unsigned 32-bit integer.
 *
 * Range of returned value is in 0...UINT32_MAX
 *
 * \param randPtr pointer to random state
 * \par Complexity of O(1) constant.
 * \return random integer
 */
#define vlRandUInt32(randPtr)   (vl_uint32_t)  (vlRandNext(randPtr))

#ifdef VL_U64_T
/**
 * \brief Generates two random unsigned 32-bit integers.
 *
 * Range of returned value is in 0...UINT32_MAX.
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandUInt32x2(randPtr, resultPtr) VL_H_RANDOM_SPLIT(randPtr, resultPtr)
#else
/**
 * \brief Generates two random unsigned 32-bit integers.
 *
 * Range of returned value is in 0...UINT32_MAX.
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandUInt32x2(randPtr, resultPtr)  VL_H_RANDOM_SPLIT(randPtr, resultPtr); \
                                            VL_H_RANDOM_SPLIT(randPtr, ((VL_U32_T*)(resultPtr) + 1))
#endif
#endif

#ifdef VL_U64_T

/**
 * \brief Generate a random unsigned 64-bit integer.
 *
 * Range of returned value is in 0...UINT64_MAX
 *
 * \par randPtr pointer to random state
 * \par Complexity of O(1) constant.
 * \return random integer
 */
#define vlRandUInt64(randPtr)   (vl_uint64_t)  (vlRandNext(randPtr))
#endif

#ifdef VL_I8_T

/**
 * \brief Generate a random 8-bit integer.
 *
 * Range of returned value is in INT8_MIN...INT8_MAX.
 *
 * \par randPtr pointer to random state
 * \par Complexity of O(1) constant.
 * \return random integer
 */
VL_API vl_int8_t vlRandInt8(vl_rand *rand);

#ifdef VL_U64_T
/**
 * \brief Generates eight random 8-bit integers.
 *
 * Range of returned value is in INT8_MIN...INT8_MAX.
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandInt8x8(randPtr, resultPtr)     VL_H_RANDOM_SPLIT(randPtr, resultPtr)
#elif defined VL_U32_T
/**
 * \brief Generates eight random 8-bit integers.
 *
 * Range of returned value is in INT8_MIN...INT8_MAX.
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandInt8x8(randPtr, resultPtr)    VL_H_RANDOM_SPLIT(randPtr, resultPtr); \
                                            VL_H_RANDOM_SPLIT(randPtr, ((VL_I8_T*)(resultPtr) + 4)
#endif
#endif

#ifdef VL_I16_T

/**
 * \brief Generate a random 16-bit integer.
 *
 * Range of returned value is in INT16_MIN...INT16_MAX.
 *
 * \par randPtr pointer to random state
 * \return random integer
 */
VL_API vl_int16_t vlRandInt16(vl_rand *rand);

#ifdef VL_U64_T
/**
 * \brief Generates four random 16-bit integers.
 *
 * Range of returned value is in INT16_MIN...INT16_MAX.
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandInt16x4(randPtr, resultPtr)    VL_H_RANDOM_SPLIT(randPtr, resultPtr)
#elif defined VL_U32_T

/**
 * \brief Generates four random 16-bit integers.
 *
 * Range of returned value is in INT16_MIN...INT16_MAX.
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandInt16x4(randPtr, resultPtr)   VL_H_RANDOM_SPLIT(randPtr, resultPtr);\
                                            VL_H_RANDOM_SPLIT(randPtr, (VL_I16_T)(resultPtr) + 2)
#endif
#endif

#ifdef VL_I32_T

/**
 * \brief Generate a random 32-bit integer.
 *
 * Range of returned value is in INT32_MIN...INT32_MAX.
 *
 * \par randPtr pointer to random state
 * \return random integer
 */
VL_API vl_int32_t vlRandInt32(vl_rand *rand);

#ifdef VL_I64_T
/**
 * \brief Generates two random 32-bit integers.
 *
 * Range of returned value is in INT32_MIN...INT32_MAX.
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandInt32x2(randPtr, resultPtr)    VL_H_RANDOM_SPLIT(randPtr, resultPtr)
#elif defined VL_I32_T
/**
 * \brief Generates two random 32-bit integers.
 *
 * Range of returned value is in INT32_MIN...INT32_MAX.
 *
 * \param randPtr pointer to random state
 * \param result where the resulting integers will be stored.
 * \par Complexity of O(1) constant.
 */
#define vlRandInt32x2(randPtr, resultPtr)  VL_H_RANDOM_SPLIT(randPtr, resultPtr);\
                                           VL_H_RANDOM_SPLIT(randPtr, ((VL_I32_T*)(resultPtr)) + 1)
#endif
#endif

#ifdef VL_I64_T

/**
 * \brief Generate a random 64-bit integer.
 *
 * Range of returned value is in INT64_MIN...INT64_MAX.
 *
 * \param randPtr pointer to random state
 * \par Complexity of O(1) constant.
 * \return random integer
 */
VL_API vl_int64_t vlRandInt64(vl_rand *rand);

#endif

/**
 * \brief Generate a random float.
 *
 * Range of returned value is in 0...1.
 *
 * \param randPtr pointer to random state
 * \par Complexity of O(1) constant.
 * \return random float
 */
VL_API vl_float32_t vlRandF(vl_rand *randPtr);

/**
 * \brief Generates two random floats.
 *
 * Range of returned floats is in range of 0...1.
 *
 * \param randPtr pointer to random state
 * \param resultPtr where the result will be stored
 * \par Complexity of O(1) constant.
 */
VL_API void vlRandFx2(vl_rand *randPtr, vl_float32_t *resultPtr);

/**
 * \brief Generates four random floats.
 *
 * Range of returned floats is in range of 0...1.
 *
 * \param randPtr pointer to random state
 * \param resultPtr where the result will be stored.
 * \par Complexity of O(1) constant.
 * \return void
 */
#define vlRandFx4(randPtr, resultPtr)   vlRandFx2(randPtr, (vl_float32_t*)(resultPtr));\
                                        vlRandFx2(randPtr, ((vl_float32_t*)(resultPtr)) + 2)

/**
 * \brief Generate a random double.
 *
 * Range of returned value is in 0...1.
 *
 * \param randPtr pointer to random state
 * \par Complexity of O(1) constant.
 * \return random double.
 */
VL_API vl_float64_t vlRandD(vl_rand *randPtr);

#endif //VL_RAND_H
