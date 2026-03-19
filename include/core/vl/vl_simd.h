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

#ifndef VL_SIMD_H
#define VL_SIMD_H

/**
 * \file vl_simd.h
 * \brief Transparent runtime-selected SIMD abstraction layer.
 *
 * Provides a unified, architecture-agnostic interface to SIMD operations with
 * automatic CPU capability detection and backend selection at initialization
 * time.
 *
 * ## Overview
 *
 * This module abstracts away platform-specific SIMD intrinsics (SSE2, AVX2,
 * NEON, etc.) behind a portable C API. The best available implementation is
 * selected once at application startup via vlSIMDInit(), with graceful fallback
 * to portable C implementations on all platforms.
 *
 * Key design principles:
 * - **Zero runtime cost after init**: All backend selection happens once; after
 * that, function pointers are just table lookups.
 * - **No global state pollution**: Selection is stored locally in
 * vlSIMDFunctions.
 * - **Transparent API**: Users call simple macros; backend is invisible.
 * - **Portable C always available**: The abstraction itself is proven correct
 * before optimizations layer on top.
 *
 * ## Supported Architectures & Backends
 *
 * ### x86 / x86-64
 * - **Portable C**: Always available, unoptimized scalar fallback.
 * - **SSE2**: Available on all x86-64 CPUs and most modern x86. Provides
 * 128-bit operations on 4 float or 4 int32. Integer 32-bit multiply falls back
 * to scalar.
 * - **AVX2**: Intel Haswell (2013+), AMD Excavator (2015+). Extends SSE with
 * 256-bit operations (8 float or 8 int32), true FMA, and better integer
 * operations.
 *
 * ### ARM / ARM64
 * - **NEON (ARMv7)**: 128-bit operations on 4 float or mixed-width integers.
 *   Division uses reciprocal approximation with one Newton-Raphson iteration
 *   (~11-12 bits accuracy).
 * - **NEON64 (ARMv8+)**: Enhanced NEON with true FMA and improved precision.
 *   Handles 8-wide operations via two 128-bit registers.
 *
 * Selection priority (checked in order):
 * - x86: AVX2 > SSE2 > Portable C
 * - ARM64: NEON64 > Portable C
 * - ARM32: NEON > Portable C
 * - Other: Portable C
 *
 * ## Usage Pattern
 *
 * ```c
 * #include <vl/vl_simd.h>
 *
 * int main() {
 *     // Initialize once at startup
 *     const char* backend = vlSIMDInit();
 *     printf("Using SIMD backend: %s\n", backend);  // e.g., "AVX2", "NEON64"
 *
 *     // Use transparently anywhere
 *     vl_simd_vec4_f32 a = vlSIMDLoadVec4F32(data_ptr);
 *     vl_simd_vec4_f32 b = vlSIMDLoadVec4F32(data_ptr2);
 *     vl_simd_vec4_f32 result = vlSIMDAddVec4F32(a, b);
 *     vlSIMDStoreVec4F32(output_ptr, result);
 *
 *     // Horizontal reductions
 *     float sum = vlSIMDHsumVec4F32(result);
 *     float max_val = vlSIMDHmaxVec4F32(result);
 *
 *     return 0;
 * }
 * ```
 *
 * ## Vector Types
 *
 * All vector types are struct-based with element arrays, enabling transparent
 * interchange between backends. Alignment hints are provided for cache
 * efficiency.
 *
 * - **4-wide float (F32)**: 4 × 32-bit floats, 16-byte aligned
 * - **8-wide float (F32)**: 8 × 32-bit floats, 32-byte aligned (for AVX)
 * - **4-wide int32 (I32)**: 4 × 32-bit signed integers, 16-byte aligned
 * - **8-wide int16 (I16)**: 8 × 16-bit signed integers, 32-byte aligned
 * - **32-wide uint8 (U8)**: 32 × 8-bit unsigned integers, 32-byte aligned
 *
 * ## Operation Categories
 *
 * ### Arithmetic (4-wide F32)
 * - **Basic**: vlSIMDAddVec4F32, vlSIMDSubVec4F32, vlSIMDMulVec4F32,
 * vlSIMDDivVec4F32
 * - **Advanced**: vlSIMDFmaVec4F32 (fused multiply-add, hardware native when
 * available)
 *
 * ### Arithmetic (8-wide F32)
 * - **Basic**: vlSIMDAddVec8F32, vlSIMDMulVec8F32
 * - **Advanced**: vlSIMDFmaVec8F32
 *
 * ### Horizontal Reductions (4-wide F32)
 * Reduce a vector to a single scalar by combining all lanes:
 * - **vlSIMDHsumVec4F32**: Sum all 4 elements
 * - **vlSIMDHmaxVec4F32**: Maximum element
 * - **vlSIMDHminVec4F32**: Minimum element
 * - **vlSIMDHprodVec4F32**: Product of all elements
 *
 * ### Comparisons (4-wide F32)
 * Return element-wise masks (0xFFFFFFFF for true, 0x00000000 for false):
 * - **vlSIMDLtVec4F32**: Less-than
 * - **vlSIMDGtVec4F32**: Greater-than
 * - **vlSIMDEqVec4F32**: Equality
 *
 * ### Bitwise Operations (4-wide F32)
 * Treat float bits as integers:
 * - **vlSIMDAndVec4F32, vlSIMDOrVec4F32, vlSIMDXorVec4F32, vlSIMDNotVec4F32**
 *
 * ### Lane Operations (4-wide F32)
 * - **vlSIMDExtractLaneVec4F32**: Extract single lane to scalar
 * - **vlSIMDBroadcastLaneVec4F32**: Replicate single lane to all lanes
 *
 * ### Integer Operations
 * - **I32**: Load, store, add, multiply (4-wide)
 * - **I16**: Load, store, add (8-wide)
 * - **U8**: Load, store (32-wide)
 *
 * ## Important Notes on Precision & Behavior
 *
 * ### Division on NEON (ARMv7/ARMv8)
 * NEON does not have native division. The implementation uses reciprocal
 * approximation with Newton-Raphson refinement:
 * ```
 * recip = vrecpeq_f32(b)
 * recip = recip * vrecpsq_f32(b, recip)  // One iteration
 * result = a * recip
 * ```
 * This achieves ~11-12 bits of accuracy, sufficient for graphics but not
 * for high-precision numerical work. Use portable C or compute higher-precision
 * divisions on CPU if needed.
 *
 * ### Integer Multiply on SSE2
 * SSE2 lacks 32-bit integer multiply, so vlSIMDMulVec4I32 falls back to scalar
 * operations on this backend.
 *
 * ### Comparison Results as Float Bits
 * Comparison operations (lt, gt, eq) return masks stored as float bit patterns:
 * - True: 0xFFFFFFFF (all bits set)
 * - False: 0x00000000
 *
 * These can be used in bitwise operations or with FMA for blending.
 *
 * ### Memory Alignment
 * Load/store operations use unaligned variants (_mm_loadu_ps, _mm256_loadu_ps,
 * etc.) to accept arbitrary pointers. If data is guaranteed aligned, consider
 * manual optimization to aligned variants for performance.
 *
 * ## Thread Safety
 *
 * **Initialization**: vlSIMDInit() is thread-safe. Subsequent calls return
 * immediately and are safe to call from multiple threads.
 *
 * **Runtime use**: vlSIMDFunctions is read-only after initialization. All
 * threads can safely call SIMD operations without synchronization.
 *
 * ## Performance Considerations
 *
 * ### Load/Store Overhead
 * The struct-based design requires loading vectors from components arrays and
 * storing back after operations. Modern compilers optimize these to single
 * instructions when possible, but be aware of this pattern for hot loops.
 *
 * ### Macro API
 * All operations are exposed as macros (e.g., vlSIMDAddVec4F32) that defer to
 * function pointers. This incurs one indirect call per operation. For very
 * tight inner loops, consider caching frequently used functions:
 * ```c
 * vl_simd_add_vec4f32_fn add_fn = vlSIMDFunctions.add_vec4f32;
 * for (...) {
 *     result = add_fn(a, b);  // May be vaguely faster than macro
 * }
 * ```
 *
 * ### 8-Wide Operations
 * 8-wide operations on NEON/SSE2 are synthesized from two 128-bit registers.
 *
 * \sa vlSIMDInit
 * \sa vl_simd_functions_t
 */

#include <vl/vl_memory.h>
#include <vl/vl_numtypes.h>

/* ============================================================================
 * Vector Type Definitions
 * ============================================================================
 */

/**
 * \brief 4-element 32-bit float vector.
 *
 * Represents a 128-bit SIMD vector on most architectures. Stored as a simple
 * component array to enable transparent backend selection.
 *
 * Alignment: 16 bytes (cache line friendly on modern CPUs).
 *
 * \sa vlSIMDLoadVec4F32, vlSIMDStoreVec4F32
 */
typedef struct VL_ALIGN_HINT(16) vl_simd_vec4_f32_
{
    vl_float32_t components[4];
} vl_simd_vec4_f32;

/**
 * \brief 8-element 32-bit float vector.
 *
 * Represents a 256-bit SIMD vector (or two 128-bit vectors on ARM).
 * Alignment: 32 bytes (AVX-friendly).
 *
 * \sa vlSIMDLoadVec8F32, vlSIMDStoreVec8F32
 */
typedef struct VL_ALIGN_HINT(32) vl_simd_vec8_f32_
{
    vl_float32_t components[8];
} vl_simd_vec8_f32;

/**
 * \brief 4-element 32-bit signed integer vector.
 *
 * Used for integer math, bit manipulation, and fixed-point operations.
 * Alignment: 16 bytes.
 *
 * **Behavior Note**: Integer multiply (vlSIMDMulVec4I32) may be scalar on SSE2.
 *
 * \sa vlSIMDLoadVec4I32, vlSIMDStoreVec4I32, vlSIMDMulVec4I32
 */
typedef struct VL_ALIGN_HINT(16) vl_simd_vec4_i32_
{
    vl_int32_t components[4];
} vl_simd_vec4_i32;

/**
 * \brief 8-element 32-bit signed integer vector.
 *
 * Alignment: 32 bytes.
 */
typedef struct VL_ALIGN_HINT(32) vl_simd_vec8_i32_
{
    vl_int32_t components[8];
} vl_simd_vec8_i32;

/**
 * \brief 8-element 16-bit signed integer vector.
 *
 * Used for compact integer storage (e.g., normals, audio samples).
 * Alignment: 32 bytes (to support wider SIMD where applicable).
 */
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4324)
#endif
typedef struct VL_ALIGN_HINT(32) vl_simd_vec8_i16_
{
    vl_int16_t components[8];
} vl_simd_vec8_i16;
#ifdef _MSC_VER
#pragma warning(pop)
#endif

/**
 * \brief 16-element 8-bit unsigned integer vector.
 *
 * Alignment: 16 bytes.
 */
typedef struct VL_ALIGN_HINT(16) vl_simd_vec16_u8_
{
    vl_uint8_t components[16];
} vl_simd_vec16_u8;

/**
 * \brief 32-element 8-bit unsigned integer vector.
 *
 * Used for bulk byte operations (e.g., image processing).
 * Alignment: 32 bytes (AVX-friendly).
 */
typedef struct VL_ALIGN_HINT(32) vl_simd_vec32_u8_
{
    vl_uint8_t components[32];
} vl_simd_vec32_u8;

/* ============================================================================
 * Function Pointer Types for Runtime Dispatch
 *
 * Each operation is defined as a function pointer type. The global
 * vlSIMDFunctions table is populated with concrete implementations at init
 * time.
 * ============================================================================
 */

typedef vl_simd_vec4_f32 (*vl_simd_load_vec4f32_fn)(const vl_float32_t*);
typedef void (*vl_simd_store_vec4f32_fn)(vl_float32_t*, vl_simd_vec4_f32);
typedef vl_simd_vec4_f32 (*vl_simd_splat_vec4f32_fn)(vl_float32_t);
typedef vl_simd_vec4_f32 (*vl_simd_add_vec4f32_fn)(vl_simd_vec4_f32, vl_simd_vec4_f32);
typedef vl_simd_vec4_f32 (*vl_simd_sub_vec4f32_fn)(vl_simd_vec4_f32, vl_simd_vec4_f32);
typedef vl_simd_vec4_f32 (*vl_simd_mul_vec4f32_fn)(vl_simd_vec4_f32, vl_simd_vec4_f32);
typedef vl_simd_vec4_f32 (*vl_simd_div_vec4f32_fn)(vl_simd_vec4_f32, vl_simd_vec4_f32);
typedef vl_simd_vec4_f32 (*vl_simd_fma_vec4f32_fn)(vl_simd_vec4_f32, vl_simd_vec4_f32, vl_simd_vec4_f32);
typedef vl_float32_t (*vl_simd_hsum_vec4f32_fn)(vl_simd_vec4_f32);
typedef vl_simd_vec8_f32 (*vl_simd_load_vec8f32_fn)(const vl_float32_t*);
typedef void (*vl_simd_store_vec8f32_fn)(vl_float32_t*, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_add_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_mul_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_fma_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_splat_vec8f32_fn)(vl_float32_t);
typedef vl_simd_vec8_f32 (*vl_simd_sub_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_lt_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_gt_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_eq_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_and_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_or_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_xor_vec8f32_fn)(vl_simd_vec8_f32, vl_simd_vec8_f32);
typedef vl_simd_vec8_f32 (*vl_simd_not_vec8f32_fn)(vl_simd_vec8_f32);
typedef vl_simd_vec4_f32 (*vl_simd_cmp_vec4f32_fn)(vl_simd_vec4_f32, vl_simd_vec4_f32);
typedef vl_simd_vec4_f32 (*vl_simd_bitwise_vec4f32_fn)(vl_simd_vec4_f32, vl_simd_vec4_f32);
typedef vl_simd_vec4_f32 (*vl_simd_not_vec4f32_fn)(vl_simd_vec4_f32);
typedef vl_float32_t (*vl_simd_hmax_vec4f32_fn)(vl_simd_vec4_f32);
typedef vl_float32_t (*vl_simd_hmin_vec4f32_fn)(vl_simd_vec4_f32);
typedef vl_float32_t (*vl_simd_hprod_vec4f32_fn)(vl_simd_vec4_f32);
typedef vl_float32_t (*vl_simd_extract_lane_vec4f32_fn)(vl_simd_vec4_f32, int);
typedef vl_simd_vec4_f32 (*vl_simd_broadcast_lane_vec4f32_fn)(vl_simd_vec4_f32, int);
typedef vl_simd_vec4_i32 (*vl_simd_load_vec4i32_fn)(const vl_int32_t*);
typedef void (*vl_simd_store_vec4i32_fn)(vl_int32_t*, vl_simd_vec4_i32);
typedef vl_simd_vec4_i32 (*vl_simd_add_vec4i32_fn)(vl_simd_vec4_i32, vl_simd_vec4_i32);
typedef vl_simd_vec4_i32 (*vl_simd_mul_vec4i32_fn)(vl_simd_vec4_i32, vl_simd_vec4_i32);
typedef vl_simd_vec8_i16 (*vl_simd_load_vec8i16_fn)(const vl_int16_t*);
typedef void (*vl_simd_store_vec8i16_fn)(vl_int16_t*, vl_simd_vec8_i16);
typedef vl_simd_vec8_i16 (*vl_simd_add_vec8i16_fn)(vl_simd_vec8_i16, vl_simd_vec8_i16);
typedef vl_simd_vec32_u8 (*vl_simd_load_vec32u8_fn)(const vl_uint8_t*);
typedef void (*vl_simd_store_vec32u8_fn)(vl_uint8_t*, vl_simd_vec32_u8);

/* ============================================================================
 * Global Function Pointer Table
 *
 * This table is set once by vlSIMDInit() based on detected CPU capabilities.
 * All threads read from it safely after initialization.
 * ============================================================================
 */

/**
 * \brief Master SIMD function dispatch table.
 *
 * Initialized by vlSIMDInit() to point to the best available backend for the
 * target architecture. Thread-safe to read after initialization; do not modify.
 *
 * Contains function pointers for:
 * - Load/store operations (memory I/O)
 * - Arithmetic (add, subtract, multiply, divide, FMA)
 * - Comparisons (lt, gt, eq)
 * - Bitwise operations (and, or, xor, not)
 * - Horizontal reductions (sum, max, min, product)
 * - Lane operations (extract, broadcast)
 * - Integer operations (I32, I16, U8)
 *
 * \note Read-only after vlSIMDInit(). Modifying this after initialization
 *       will cause undefined behavior.
 *
 * \sa vlSIMDInit
 */
typedef struct
{
    vl_simd_load_vec4f32_fn load_vec4f32;
    vl_simd_store_vec4f32_fn store_vec4f32;
    vl_simd_splat_vec4f32_fn splat_vec4f32;
    vl_simd_add_vec4f32_fn add_vec4f32;
    vl_simd_sub_vec4f32_fn sub_vec4f32;
    vl_simd_mul_vec4f32_fn mul_vec4f32;
    vl_simd_div_vec4f32_fn div_vec4f32;
    vl_simd_fma_vec4f32_fn fma_vec4f32;
    vl_simd_hsum_vec4f32_fn hsum_vec4f32;
    vl_simd_load_vec8f32_fn load_vec8f32;
    vl_simd_store_vec8f32_fn store_vec8f32;
    vl_simd_add_vec8f32_fn add_vec8f32;
    vl_simd_mul_vec8f32_fn mul_vec8f32;
    vl_simd_fma_vec8f32_fn fma_vec8f32;
    vl_simd_splat_vec8f32_fn splat_vec8f32;
    vl_simd_sub_vec8f32_fn sub_vec8f32;
    vl_simd_lt_vec8f32_fn lt_vec8f32;
    vl_simd_gt_vec8f32_fn gt_vec8f32;
    vl_simd_eq_vec8f32_fn eq_vec8f32;
    vl_simd_and_vec8f32_fn and_vec8f32;
    vl_simd_or_vec8f32_fn or_vec8f32;
    vl_simd_xor_vec8f32_fn xor_vec8f32;
    vl_simd_not_vec8f32_fn not_vec8f32;
    vl_simd_cmp_vec4f32_fn lt_vec4f32;
    vl_simd_cmp_vec4f32_fn gt_vec4f32;
    vl_simd_cmp_vec4f32_fn eq_vec4f32;
    vl_simd_bitwise_vec4f32_fn and_vec4f32;
    vl_simd_bitwise_vec4f32_fn or_vec4f32;
    vl_simd_bitwise_vec4f32_fn xor_vec4f32;
    vl_simd_not_vec4f32_fn not_vec4f32;
    vl_simd_hmax_vec4f32_fn hmax_vec4f32;
    vl_simd_hmin_vec4f32_fn hmin_vec4f32;
    vl_simd_hprod_vec4f32_fn hprod_vec4f32;
    vl_simd_extract_lane_vec4f32_fn extract_lane_vec4f32;
    vl_simd_broadcast_lane_vec4f32_fn broadcast_lane_vec4f32;
    vl_simd_load_vec4i32_fn load_vec4i32;
    vl_simd_store_vec4i32_fn store_vec4i32;
    vl_simd_add_vec4i32_fn add_vec4i32;
    vl_simd_mul_vec4i32_fn mul_vec4i32;
    vl_simd_load_vec8i16_fn load_vec8i16;
    vl_simd_store_vec8i16_fn store_vec8i16;
    vl_simd_add_vec8i16_fn add_vec8i16;
    vl_simd_load_vec32u8_fn load_vec32u8;
    vl_simd_store_vec32u8_fn store_vec32u8;

    /** \brief Backend name string for logging/debugging (e.g., "AVX2", "NEON64").
     */
    const char* backend_name;
} vl_simd_functions_t;

/**
 * \brief Global SIMD function table.
 *
 * Set by vlSIMDInit(). Thread-safe to read after initialization.
 * Do not modify after initialization.
 *
 * \sa vlSIMDInit, vl_simd_functions_t
 */
VL_API extern vl_simd_functions_t vlSIMDFunctions;

/* ============================================================================
 * Initialization
 * ============================================================================
 */

/**
 * \brief Initializes the SIMD subsystem and selects the best available backend.
 *
 * Must be called once at application startup, before any SIMD operations.
 * Thread-safe; subsequent calls return immediately and are safe from any
 * thread.
 *
 * ## Backend Selection Algorithm
 *
 * 1. **x86/x86-64**: Check for AVX2 → SSE2 → fallback to Portable C
 * 2. **ARM64**: Use NEON64 (guaranteed available) → fallback to Portable C
 * 3. **ARM32**: Use NEON (if available) → fallback to Portable C
 * 4. **Other**: Use Portable C
 *
 * CPU capability detection uses:
 * - CPUID instruction (x86/MSVC and GCC/Clang)
 * - Compile-time guarantees (ARM with -mfpu=neon)
 *
 * ## Example
 *
 * ```c
 * int main() {
 *     const char* backend = vlSIMDInit();
 *     printf("SIMD backend: %s\n", backend);
 *
 *     // SIMD operations now use best available backend
 *     vl_simd_vec4_f32 v = vlSIMDLoadVec4F32(data);
 *     // ...
 *
 *     return 0;
 * }
 * ```
 *
 * \return Pointer to a static string naming the selected backend.
 *         Examples: "SSE2", "AVX2", "NEON64", "NEON (ARMv7)", "Portable C".
 *         Pointer is valid for the lifetime of the program.
 *
 * \note Safe to call from any thread. Repeated calls are safe and return
 *       immediately on subsequent invocations.
 *
 * \sa vlSIMDFunctions, vl_simd_functions_t
 */
VL_API const char* vlSIMDInit(void);

/* ============================================================================
 * Inline API (User-Facing)
 *
 * All operations are exposed as inline functions that dereference
 * vlSIMDFunctions. This provides a transparent, backend-agnostic interface.
 * ============================================================================
 */

/* --- 4-Wide Float (F32) Load/Store --- */

/**
 * \brief Loads 4 floats from memory into a vector.
 *
 * \param ptr Pointer to array of 4 floats. Pointer need not be aligned;
 *            unaligned loads are handled transparently.
 * \return 4-element float vector.
 *
 * \sa vlSIMDStoreVec4F32
 */
static inline vl_simd_vec4_f32 vlSIMDLoadVec4F32(const vl_float32_t* ptr) { return vlSIMDFunctions.load_vec4f32(ptr); }

/**
 * \brief Stores a 4-float vector to memory.
 *
 * \param ptr Pointer to memory for 4 floats. Need not be aligned.
 * \param v Vector to store.
 *
 * \sa vlSIMDLoadVec4F32
 */
static inline void vlSIMDStoreVec4F32(vl_float32_t* ptr, vl_simd_vec4_f32 v) { vlSIMDFunctions.store_vec4f32(ptr, v); }

/**
 * \brief Broadcasts a scalar into all 4 lanes.
 *
 * \param scalar Float value to replicate.
 * \return Vector with all 4 elements set to scalar.
 *
 * Example: vlSIMDSplatVec4F32(1.0f) → [1.0, 1.0, 1.0, 1.0]
 */
static inline vl_simd_vec4_f32 vlSIMDSplatVec4F32(vl_float32_t scalar) { return vlSIMDFunctions.splat_vec4f32(scalar); }

/* --- 4-Wide Float (F32) Arithmetic --- */

/**
 * \brief Element-wise addition of two 4-float vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result vector [a[0]+b[0], a[1]+b[1], a[2]+b[2], a[3]+b[3]].
 *
 * \sa vlSIMDSubVec4F32, vlSIMDMulVec4F32
 */
static inline vl_simd_vec4_f32 vlSIMDAddVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.add_vec4f32(a, b);
}

/**
 * \brief Element-wise subtraction of two 4-float vectors.
 *
 * \param a Minuend vector.
 * \param b Subtrahend vector.
 * \return Result vector [a[0]-b[0], a[1]-b[1], a[2]-b[2], a[3]-b[3]].
 */
static inline vl_simd_vec4_f32 vlSIMDSubVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.sub_vec4f32(a, b);
}

/**
 * \brief Element-wise multiplication of two 4-float vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result vector [a[0]*b[0], a[1]*b[1], a[2]*b[2], a[3]*b[3]].
 */
static inline vl_simd_vec4_f32 vlSIMDMulVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.mul_vec4f32(a, b);
}

/**
 * \brief Element-wise division of two 4-float vectors.
 *
 * \param a Dividend vector.
 * \param b Divisor vector.
 * \return Result vector [a[0]/b[0], a[1]/b[1], a[2]/b[2], a[3]/b[3]].
 *
 * **Warning (ARM NEON)**: Uses reciprocal approximation with one Newton-Raphson
 * iteration (~11-12 bits accuracy). Not suitable for high-precision work.
 *
 * \sa vlSIMDFmaVec4F32
 */
static inline vl_simd_vec4_f32 vlSIMDDivVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.div_vec4f32(a, b);
}

/**
 * \brief Fused multiply-add: (a * b) + c.
 *
 * Computes the operation in one step with only one rounding, improving
 * both accuracy and performance on processors with FMA support.
 *
 * \param a Multiplier vector.
 * \param b Multiplicand vector.
 * \param c Addend vector.
 * \return Result vector [(a[i]*b[i])+c[i] for i in 0..3].
 *
 * On backends without hardware FMA (SSE2), this is emulated as (a * b) + c.
 * Accuracy is hardware-dependent but generally better than separate
 * multiply/add.
 */
static inline vl_simd_vec4_f32 vlSIMDFmaVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b, vl_simd_vec4_f32 c)
{
    return vlSIMDFunctions.fma_vec4f32(a, b, c);
}

/* --- 4-Wide Float (F32) Reductions --- */

/**
 * \brief Horizontal sum: adds all 4 elements.
 *
 * \param v Input vector.
 * \return Scalar sum: v[0] + v[1] + v[2] + v[3].
 *
 * Example: vlSIMDHsumVec4F32([1, 2, 3, 4]) → 10.0
 *
 * \sa vlSIMDHmaxVec4F32, vlSIMDHminVec4F32
 */
static inline vl_float32_t vlSIMDHsumVec4F32(vl_simd_vec4_f32 v) { return vlSIMDFunctions.hsum_vec4f32(v); }

/**
 * \brief Horizontal maximum: finds largest element.
 *
 * \param v Input vector.
 * \return Scalar maximum: max(v[0], v[1], v[2], v[3]).
 */
static inline vl_float32_t vlSIMDHmaxVec4F32(vl_simd_vec4_f32 v) { return vlSIMDFunctions.hmax_vec4f32(v); }

/**
 * \brief Horizontal minimum: finds smallest element.
 *
 * \param v Input vector.
 * \return Scalar minimum: min(v[0], v[1], v[2], v[3]).
 */
static inline vl_float32_t vlSIMDHminVec4F32(vl_simd_vec4_f32 v) { return vlSIMDFunctions.hmin_vec4f32(v); }

/**
 * \brief Horizontal product: multiplies all 4 elements.
 *
 * \param v Input vector.
 * \return Scalar product: v[0] * v[1] * v[2] * v[3].
 */
static inline vl_float32_t vlSIMDHprodVec4F32(vl_simd_vec4_f32 v) { return vlSIMDFunctions.hprod_vec4f32(v); }

/* --- 4-Wide Float (F32) Comparisons --- */

/**
 * \brief Element-wise less-than comparison.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Mask vector: [a[i] < b[i] ? 0xFFFFFFFF : 0x00000000 for i in 0..3].
 *
 * Result bits can be used in subsequent bitwise operations or FMA for blending.
 *
 * Example:
 * ```c
 * vl_simd_vec4_f32 cmp = vlSIMDLtVec4F32([1,2,3,4], [2,2,2,2]);
 * // cmp = [0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000]  // [true, false,
 * false, false]
 * ```
 *
 * \sa vlSIMDGtVec4F32, vlSIMDEqVec4F32
 */
static inline vl_simd_vec4_f32 vlSIMDLtVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.lt_vec4f32(a, b);
}

/**
 * \brief Element-wise greater-than comparison.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Mask vector: [a[i] > b[i] ? 0xFFFFFFFF : 0x00000000 for i in 0..3].
 */
static inline vl_simd_vec4_f32 vlSIMDGtVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.gt_vec4f32(a, b);
}

/**
 * \brief Element-wise equality comparison.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Mask vector: [a[i] == b[i] ? 0xFFFFFFFF : 0x00000000 for i in 0..3].
 *
 * **Warning**: Comparing floats for exact equality is generally unsafe due to
 * rounding errors. Consider using an epsilon-based comparison instead.
 */
static inline vl_simd_vec4_f32 vlSIMDEqVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.eq_vec4f32(a, b);
}

/* --- 4-Wide Float (F32) Bitwise Operations --- */

/**
 * \brief Bitwise AND on 4-float vector bit patterns.
 *
 * Useful for masking or combining comparison results.
 *
 * \param a First vector (interpreted as 4 × 32-bit unsigned integers).
 * \param b Second vector.
 * \return Result with bitwise AND applied to each lane.
 *
 * \sa vlSIMDOrVec4F32, vlSIMDXorVec4F32, vlSIMDNotVec4F32
 */
static inline vl_simd_vec4_f32 vlSIMDAndVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.and_vec4f32(a, b);
}

/**
 * \brief Bitwise OR on 4-float vector bit patterns.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result with bitwise OR applied to each lane.
 */
static inline vl_simd_vec4_f32 vlSIMDOrVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.or_vec4f32(a, b);
}

/**
 * \brief Bitwise XOR on 4-float vector bit patterns.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result with bitwise XOR applied to each lane.
 */
static inline vl_simd_vec4_f32 vlSIMDXorVec4F32(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDFunctions.xor_vec4f32(a, b);
}

/**
 * \brief Bitwise NOT on 4-float vector bit pattern.
 *
 * \param a Input vector.
 * \return Result with all bits inverted in each lane.
 */
static inline vl_simd_vec4_f32 vlSIMDNotVec4F32(vl_simd_vec4_f32 a) { return vlSIMDFunctions.not_vec4f32(a); }

/* --- 4-Wide Float (F32) Lane Operations --- */

/**
 * \brief Extracts a single lane from a vector as a scalar.
 *
 * \param v Input vector.
 * \param lane Lane index (0-3). Clamped to valid range.
 * \return Scalar value from v[lane].
 *
 * Example: vlSIMDExtractLaneVec4F32([1, 2, 3, 4], 2) → 3.0
 */
static inline vl_float32_t vlSIMDExtractLaneVec4F32(vl_simd_vec4_f32 v, int lane)
{
    return vlSIMDFunctions.extract_lane_vec4f32(v, lane);
}

/**
 * \brief Replicates a single lane to all lanes.
 *
 * \param v Input vector.
 * \param lane Lane index (0-3). Clamped to valid range.
 * \return Vector with all lanes set to v[lane].
 *
 * Example: vlSIMDBroadcastLaneVec4F32([1, 2, 3, 4], 1) → [2, 2, 2, 2]
 */
static inline vl_simd_vec4_f32 vlSIMDBroadcastLaneVec4F32(vl_simd_vec4_f32 v, int lane)
{
    return vlSIMDFunctions.broadcast_lane_vec4f32(v, lane);
}

/* --- 8-Wide Float (F32) Load/Store --- */

/**
 * \brief Loads 8 floats from memory into a vector.
 *
 * \param ptr Pointer to array of 8 floats.
 * \return 8-element float vector.
 *
 * \note On ARM NEON, synthesized from two 128-bit registers.
 *
 * \sa vlSIMDStoreVec8F32
 */
static inline vl_simd_vec8_f32 vlSIMDLoadVec8F32(const vl_float32_t* ptr) { return vlSIMDFunctions.load_vec8f32(ptr); }

/**
 * \brief Stores an 8-float vector to memory.
 *
 * \param ptr Pointer to memory for 8 floats.
 * \param v Vector to store.
 */
static inline void vlSIMDStoreVec8F32(vl_float32_t* ptr, vl_simd_vec8_f32 v) { vlSIMDFunctions.store_vec8f32(ptr, v); }

/* --- 8-Wide Float (F32) Arithmetic --- */

/**
 * \brief Element-wise addition of two 8-float vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result vector with element-wise sum.
 */
static inline vl_simd_vec8_f32 vlSIMDAddVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    return vlSIMDFunctions.add_vec8f32(a, b);
}

/**
 * \brief Element-wise multiplication of two 8-float vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result vector with element-wise product.
 */
static inline vl_simd_vec8_f32 vlSIMDMulVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    return vlSIMDFunctions.mul_vec8f32(a, b);
}

/**
 * \brief 8-wide fused multiply-add: (a * b) + c.
 *
 * \param a Multiplier vector (8 elements).
 * \param b Multiplicand vector (8 elements).
 * \param c Addend vector (8 elements).
 * \return Result vector with FMA applied to each lane.
 */
static inline vl_simd_vec8_f32 vlSIMDFmaVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b, vl_simd_vec8_f32 c)
{
    return vlSIMDFunctions.fma_vec8f32(a, b, c);
}

/* --- Integer Operations --- */

/**
 * \brief Loads 4 signed 32-bit integers from memory.
 *
 * \param ptr Pointer to array of 4 int32 values.
 * \return 4-element integer vector.
 *
 * \warning Integer multiply (vlSIMDMulVec4I32) may be scalar on SSE2.
 *
 * \sa vlSIMDStoreVec4I32
 */
static inline vl_simd_vec4_i32 vlSIMDLoadVec4I32(const vl_int32_t* ptr) { return vlSIMDFunctions.load_vec4i32(ptr); }

/**
 * \brief Stores a 4-int32 vector to memory.
 *
 * \param ptr Pointer to memory for 4 int32 values.
 * \param v Vector to store.
 */
static inline void vlSIMDStoreVec4I32(vl_int32_t* ptr, vl_simd_vec4_i32 v) { vlSIMDFunctions.store_vec4i32(ptr, v); }

/**
 * \brief Element-wise addition of two 4-int32 vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result vector with element-wise sum (wrapping on overflow).
 */
static inline vl_simd_vec4_i32 vlSIMDAddVec4I32(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    return vlSIMDFunctions.add_vec4i32(a, b);
}

/**
 * \brief Element-wise multiplication of two 4-int32 vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result vector with element-wise product (wrapping on overflow).
 *
 * **Performance Note**: On SSE2, this falls back to scalar operations.
 * Use AVX2 or NEON64 for vectorized int32 multiplication.
 */
static inline vl_simd_vec4_i32 vlSIMDMulVec4I32(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    return vlSIMDFunctions.mul_vec4i32(a, b);
}

/**
 * \brief Loads 8 signed 16-bit integers from memory.
 *
 * \param ptr Pointer to array of 8 int16 values.
 * \return 8-element 16-bit integer vector.
 *
 * \sa vlSIMDStoreVec8I16
 */
static inline vl_simd_vec8_i16 vlSIMDLoadVec8I16(const vl_int16_t* ptr) { return vlSIMDFunctions.load_vec8i16(ptr); }

/**
 * \brief Stores an 8-int16 vector to memory.
 *
 * \param ptr Pointer to memory for 8 int16 values.
 * \param v Vector to store.
 */
static inline void vlSIMDStoreVec8I16(vl_int16_t* ptr, vl_simd_vec8_i16 v) { vlSIMDFunctions.store_vec8i16(ptr, v); }

/**
 * \brief Element-wise addition of two 8-int16 vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result vector with element-wise sum (wrapping on overflow).
 */
static inline vl_simd_vec8_i16 vlSIMDAddVec8I16(vl_simd_vec8_i16 a, vl_simd_vec8_i16 b)
{
    return vlSIMDFunctions.add_vec8i16(a, b);
}

/**
 * \brief Loads 32 unsigned 8-bit integers from memory.
 *
 * \param ptr Pointer to array of 32 uint8 values.
 * \return 32-element 8-bit unsigned integer vector.
 *
 * \sa vlSIMDStoreVec32U8
 */
static inline vl_simd_vec32_u8 vlSIMDLoadVec32U8(const vl_uint8_t* ptr) { return vlSIMDFunctions.load_vec32u8(ptr); }

/**
 * \brief Stores a 32-uint8 vector to memory.
 *
 * \param ptr Pointer to memory for 32 uint8 values.
 * \param v Vector to store.
 */
static inline void vlSIMDStoreVec32U8(vl_uint8_t* ptr, vl_simd_vec32_u8 v) { vlSIMDFunctions.store_vec32u8(ptr, v); }

/**
 * \brief Broadcasts a scalar into all 8 lanes.
 *
 * \param scalar Float value to replicate.
 * \return Vector with all 8 elements set to scalar.
 */
static inline vl_simd_vec8_f32 vlSIMDSplatVec8F32(vl_float32_t scalar) { return vlSIMDFunctions.splat_vec8f32(scalar); }

/**
 * \brief Element-wise subtraction of two 8-float vectors.
 *
 * \param a Minuend vector.
 * \param b Subtrahend vector.
 * \return Result vector with element-wise difference.
 */
static inline vl_simd_vec8_f32 vlSIMDSubVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    return vlSIMDFunctions.sub_vec8f32(a, b);
}

/* --- 8-Wide Float (F32) Comparisons --- */

/**
 * \brief Element-wise less-than comparison for 8-float vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Mask vector: [a[i] < b[i] ? 0xFFFFFFFF : 0x00000000 for i in 0..7].
 */
static inline vl_simd_vec8_f32 vlSIMDLtVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    return vlSIMDFunctions.lt_vec8f32(a, b);
}

/**
 * \brief Element-wise greater-than comparison for 8-float vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Mask vector: [a[i] > b[i] ? 0xFFFFFFFF : 0x00000000 for i in 0..7].
 */
static inline vl_simd_vec8_f32 vlSIMDGtVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    return vlSIMDFunctions.gt_vec8f32(a, b);
}

/**
 * \brief Element-wise equality comparison for 8-float vectors.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Mask vector: [a[i] == b[i] ? 0xFFFFFFFF : 0x00000000 for i in 0..7].
 */
static inline vl_simd_vec8_f32 vlSIMDEqVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    return vlSIMDFunctions.eq_vec8f32(a, b);
}

/* --- 8-Wide Float (F32) Bitwise Operations --- */

/**
 * \brief Bitwise AND on 8-float vector bit patterns.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result with bitwise AND applied to each lane.
 */
static inline vl_simd_vec8_f32 vlSIMDAndVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    return vlSIMDFunctions.and_vec8f32(a, b);
}

/**
 * \brief Bitwise OR on 8-float vector bit patterns.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result with bitwise OR applied to each lane.
 */
static inline vl_simd_vec8_f32 vlSIMDOrVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    return vlSIMDFunctions.or_vec8f32(a, b);
}

/**
 * \brief Bitwise XOR on 8-float vector bit patterns.
 *
 * \param a First vector.
 * \param b Second vector.
 * \return Result with bitwise XOR applied to each lane.
 */
static inline vl_simd_vec8_f32 vlSIMDXorVec8F32(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    return vlSIMDFunctions.xor_vec8f32(a, b);
}

/**
 * \brief Bitwise NOT on 8-float vector bit patterns.
 *
 * \param a Input vector.
 * \return Result with all bits inverted in each lane.
 */
static inline vl_simd_vec8_f32 vlSIMDNotVec8F32(vl_simd_vec8_f32 a) { return vlSIMDFunctions.not_vec8f32(a); }

#endif
