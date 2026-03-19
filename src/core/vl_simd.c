#include <string.h>
#include <vl/vl_libconfig.h>
#include <vl/vl_simd.h>

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif

/* ============================================================================
 * Include all backend implementations
 * ============================================================================
 */

#include "platform/vl_simd_portable.c"

#ifdef VL_SIMD_SSE2_AVAILABLE
extern void vlSIMDInitSSE2(void);
#endif
#ifdef VL_SIMD_AVX2_AVAILABLE
extern void vlSIMDInitAVX2(void);
#endif
#ifdef VL_SIMD_NEON_AVAILABLE
extern void vlSIMDInitNEON(void);
#endif
#ifdef VL_SIMD_NEON64_AVAILABLE
extern void vlSIMDInitNEON64(void);
#endif

/* Global function table (initialized by vlSIMDInit)
 * Initialize to portable C implementation.
 */
vl_simd_functions_t vlSIMDFunctions = {
    /* 4-wide float operations */
    .load_vec4f32 = vlSIMDLoadVec4F32Portable,
    .store_vec4f32 = vlSIMDStoreVec4F32Portable,
    .splat_vec4f32 = vlSIMDSplatVec4F32Portable,
    .add_vec4f32 = vlSIMDAddVec4F32Portable,
    .sub_vec4f32 = vlSIMDSubVec4F32Portable,
    .mul_vec4f32 = vlSIMDMulVec4F32Portable,
    .div_vec4f32 = vlSIMDDivVec4F32Portable,
    .fma_vec4f32 = vlSIMDFmaVec4F32Portable,
    .hsum_vec4f32 = vlSIMDHsumVec4F32Portable,

    /* 8-wide float operations */
    .load_vec8f32 = vlSIMDLoadVec8F32Portable,
    .store_vec8f32 = vlSIMDStoreVec8F32Portable,
    .add_vec8f32 = vlSIMDAddVec8F32Portable,
    .mul_vec8f32 = vlSIMDMulVec8F32Portable,
    .fma_vec8f32 = vlSIMDFmaVec8F32Portable,

    /* Comparison operations */
    .lt_vec4f32 = vlSIMDLtVec4F32Portable,
    .gt_vec4f32 = vlSIMDGtVec4F32Portable,
    .eq_vec4f32 = vlSIMDEqVec4F32Portable,

    /* Bitwise operations */
    .and_vec4f32 = vlSIMDAndVec4F32Portable,
    .or_vec4f32 = vlSIMDOrVec4F32Portable,
    .xor_vec4f32 = vlSIMDXorVec4F32Portable,
    .not_vec4f32 = vlSIMDNotVec4F32Portable,

    .hmax_vec4f32 = vlSIMDHmaxVec4F32Portable,
    .hmin_vec4f32 = vlSIMDHminVec4F32Portable,
    .hprod_vec4f32 = vlSIMDHprodVec4F32Portable,
    .extract_lane_vec4f32 = vlSIMDExtractLaneVec4F32Portable,
    .broadcast_lane_vec4f32 = vlSIMDBroadcastLaneVec4F32Portable,

    .load_vec4i32 = vlSIMDLoadVec4I32Portable,
    .store_vec4i32 = vlSIMDStoreVec4I32Portable,
    .add_vec4i32 = vlSIMDAddVec4I32Portable,
    .mul_vec4i32 = vlSIMDMulVec4I32Portable,

    /* Metadata */
    .backend_name = "Portable C (Uninitialized)"};

/* ============================================================================
 * CPU Detection
 * ============================================================================
 */

// CPU feature detection (x86)
#if defined(VL_SIMD_SSE2_AVAILABLE) || defined(VL_SIMD_AVX2_AVAILABLE)

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
static inline void vlCPUID(int leaf, int subleaf, int* eax, int* ebx, int* ecx, int* edx)
{
#ifdef _MSC_VER
    int cpuinfo[4] = {0};
    __cpuidex(cpuinfo, leaf, subleaf);
    *eax = cpuinfo[0];
    *ebx = cpuinfo[1];
    *ecx = cpuinfo[2];
    *edx = cpuinfo[3];
#else
    __cpuid_count(leaf, subleaf, *eax, *ebx, *ecx, *edx);
#endif
}
#endif

#ifdef VL_SIMD_AVX2_AVAILABLE
static vl_bool_t vlCPUSupportsAVX2(void)
{
    int eax, ebx, ecx, edx;
    vlCPUID(7, 0, &eax, &ebx, &ecx, &edx);
    return (ebx & (1 << 5)) != 0; // Bit 5 of EBX is AVX2
}
#endif

#ifdef VL_SIMD_SSE2_AVAILABLE
static vl_bool_t vlCPUSupportsSSE2(void)
{
    int eax, ebx, ecx, edx;
    vlCPUID(1, 0, &eax, &ebx, &ecx, &edx);
    return (ecx & (1 << 26)) != 0; // Bit 26 of ECX is SSE2
}
#endif

#endif

// ARM NEON detection
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
static vl_bool_t vlCPUSupportsNEON(void)
{
    return VL_TRUE; // Compile-time guarantee
}
#endif

/* ============================================================================
 * Runtime Initialization
 * ============================================================================
 */

const char* vlSIMDInit(void)
{
    static vl_bool_t initialized = VL_FALSE;

    if (initialized)
    {
        return vlSIMDFunctions.backend_name;
    }

    initialized = VL_TRUE;

    // Try to initialize best available implementation
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#ifdef VL_SIMD_AVX2_AVAILABLE
    if (vlCPUSupportsAVX2())
    {
        vlSIMDInitAVX2();
        return vlSIMDFunctions.backend_name;
    }
#endif

#ifdef VL_SIMD_SSE2_AVAILABLE
    if (vlCPUSupportsSSE2())
    {
        vlSIMDInitSSE2();
        return vlSIMDFunctions.backend_name;
    }
#endif
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#ifdef VL_SIMD_NEON64_AVAILABLE
    vlSIMDInitNEON64();
    return vlSIMDFunctions.backend_name;
#endif
#elif defined(__arm__) || defined(_M_ARM)
#ifdef VL_SIMD_NEON_AVAILABLE
    vlSIMDInitNEON();
    return vlSIMDFunctions.backend_name;
#endif
#endif

    // Fallback to portable C
    vlSIMDInitPortable();
    return vlSIMDFunctions.backend_name;
}
