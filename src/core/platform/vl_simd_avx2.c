
/**
 * \file vl_simd_avx2.c
 * \brief AVX2 SIMD implementation
 *
 * Provides optimized SIMD operations using AVX2 instructions.
 * Requires CPU support for AVX2 (Intel Haswell or later, AMD Excavator or
 * later).
 */

#include <immintrin.h>
#include <vl/vl_simd.h>

/* ============================================================================
 * 4-Wide Float Operations (F32)
 * ============================================================================
 */

static vl_simd_vec4_f32 vlSIMDLoadVec4F32AVX2(const vl_float32_t* ptr)
{
    vl_simd_vec4_f32 result;
    _mm_storeu_ps(result.components, _mm_loadu_ps(ptr));
    return result;
}

static void vlSIMDStoreVec4F32AVX2(vl_float32_t* ptr, vl_simd_vec4_f32 v)
{
    _mm_storeu_ps(ptr, _mm_loadu_ps(v.components));
}

static vl_simd_vec4_f32 vlSIMDSplatVec4F32AVX2(vl_float32_t scalar)
{
    vl_simd_vec4_f32 result;
    __m128 splat = _mm_set1_ps(scalar);
    _mm_storeu_ps(result.components, splat);
    return result;
}

static vl_simd_vec4_f32 vlSIMDAddVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 sum = _mm_add_ps(va, vb);
    _mm_storeu_ps(result.components, sum);
    return result;
}

static vl_simd_vec4_f32 vlSIMDSubVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 diff = _mm_sub_ps(va, vb);
    _mm_storeu_ps(result.components, diff);
    return result;
}

static vl_simd_vec4_f32 vlSIMDMulVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 prod = _mm_mul_ps(va, vb);
    _mm_storeu_ps(result.components, prod);
    return result;
}

static vl_simd_vec4_f32 vlSIMDDivVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 quot = _mm_div_ps(va, vb);
    _mm_storeu_ps(result.components, quot);
    return result;
}

static vl_simd_vec4_f32 vlSIMDFmaVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b, vl_simd_vec4_f32 c)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 vc = _mm_loadu_ps(c.components);
    __m128 fma = _mm_fmadd_ps(va, vb, vc);
    _mm_storeu_ps(result.components, fma);
    return result;
}

static vl_float32_t vlSIMDHsumVec4F32AVX2(vl_simd_vec4_f32 v)
{
    __m128 vec = _mm_loadu_ps(v.components);
    __m128 shuf = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(vec, shuf);
    shuf = _mm_shuffle_ps(sums, sums, _MM_SHUFFLE(1, 0, 3, 2));
    sums = _mm_add_ps(sums, shuf);
    return _mm_cvtss_f32(sums);
}

static vl_float32_t vlSIMDHmaxVec4F32AVX2(vl_simd_vec4_f32 v)
{
    __m128 vec = _mm_loadu_ps(v.components);
    __m128 shuf = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 maxs = _mm_max_ps(vec, shuf);
    shuf = _mm_shuffle_ps(maxs, maxs, _MM_SHUFFLE(1, 0, 3, 2));
    maxs = _mm_max_ps(maxs, shuf);
    return _mm_cvtss_f32(maxs);
}

static vl_float32_t vlSIMDHminVec4F32AVX2(vl_simd_vec4_f32 v)
{
    __m128 vec = _mm_loadu_ps(v.components);
    __m128 shuf = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 mins = _mm_min_ps(vec, shuf);
    shuf = _mm_shuffle_ps(mins, mins, _MM_SHUFFLE(1, 0, 3, 2));
    mins = _mm_min_ps(mins, shuf);
    return _mm_cvtss_f32(mins);
}

static vl_float32_t vlSIMDHprodVec4F32AVX2(vl_simd_vec4_f32 v)
{
    __m128 vec = _mm_loadu_ps(v.components);
    __m128 shuf = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 prods = _mm_mul_ps(vec, shuf);
    shuf = _mm_shuffle_ps(prods, prods, _MM_SHUFFLE(1, 0, 3, 2));
    prods = _mm_mul_ps(prods, shuf);
    return _mm_cvtss_f32(prods);
}

static vl_float32_t vlSIMDExtractLaneVec4F32AVX2(vl_simd_vec4_f32 v, int lane)
{
    lane &= 3; // Clamp to 0-3
    return v.components[lane];
}

static vl_simd_vec4_f32 vlSIMDBroadcastLaneVec4F32AVX2(vl_simd_vec4_f32 v, int lane)
{
    lane &= 3; // Clamp to 0-3
    return vlSIMDSplatVec4F32AVX2(v.components[lane]);
}

/* ============================================================================
 * 8-Wide Float Operations (F32)
 * ============================================================================
 */

static vl_simd_vec8_f32 vlSIMDLoadVec8F32AVX2(const vl_float32_t* ptr)
{
    vl_simd_vec8_f32 result;
    __m256 loaded = _mm256_loadu_ps(ptr);
    _mm256_storeu_ps(result.components, loaded);
    return result;
}

static void vlSIMDStoreVec8F32AVX2(vl_float32_t* ptr, vl_simd_vec8_f32 v)
{
    __m256 vec = _mm256_loadu_ps(v.components);
    _mm256_storeu_ps(ptr, vec);
}

static vl_simd_vec8_f32 vlSIMDAddVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 sum = _mm256_add_ps(va, vb);
    _mm256_storeu_ps(result.components, sum);
    return result;
}

static vl_simd_vec8_f32 vlSIMDMulVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 prod = _mm256_mul_ps(va, vb);
    _mm256_storeu_ps(result.components, prod);
    return result;
}

static vl_simd_vec8_f32 vlSIMDFmaVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b, vl_simd_vec8_f32 c)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 vc = _mm256_loadu_ps(c.components);
    __m256 fma = _mm256_fmadd_ps(va, vb, vc);
    _mm256_storeu_ps(result.components, fma);
    return result;
}

/* ============================================================================
 * Comparison Operations (F32)
 * ============================================================================
 */

static vl_simd_vec4_f32 vlSIMDLtVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 cmp = _mm_cmplt_ps(va, vb);
    _mm_storeu_ps(result.components, cmp);
    return result;
}

static vl_simd_vec4_f32 vlSIMDGtVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 cmp = _mm_cmpgt_ps(va, vb);
    _mm_storeu_ps(result.components, cmp);
    return result;
}

static vl_simd_vec4_f32 vlSIMDEqVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 cmp = _mm_cmpeq_ps(va, vb);
    _mm_storeu_ps(result.components, cmp);
    return result;
}

/* ============================================================================
 * Bitwise Operations (F32)
 * ============================================================================
 */

static vl_simd_vec4_f32 vlSIMDAndVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 res = _mm_and_ps(va, vb);
    _mm_storeu_ps(result.components, res);
    return result;
}

static vl_simd_vec4_f32 vlSIMDOrVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 res = _mm_or_ps(va, vb);
    _mm_storeu_ps(result.components, res);
    return result;
}

static vl_simd_vec4_f32 vlSIMDXorVec4F32AVX2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 vb = _mm_loadu_ps(b.components);
    __m128 res = _mm_xor_ps(va, vb);
    _mm_storeu_ps(result.components, res);
    return result;
}

static vl_simd_vec4_f32 vlSIMDNotVec4F32AVX2(vl_simd_vec4_f32 a)
{
    vl_simd_vec4_f32 result;
    __m128 va = _mm_loadu_ps(a.components);
    __m128 all_ones = _mm_castsi128_ps(_mm_set1_epi32(-1));
    __m128 res = _mm_xor_ps(va, all_ones);
    _mm_storeu_ps(result.components, res);
    return result;
}

/* ============================================================================
 * Integer 32-bit Operations (I32)
 * ============================================================================
 */

static vl_simd_vec4_i32 vlSIMDLoadVec4I32AVX2(const vl_int32_t* ptr)
{
    vl_simd_vec4_i32 result;
    __m128i loaded = _mm_loadu_si128((const __m128i*)ptr);
    _mm_storeu_si128((__m128i*)result.components, loaded);
    return result;
}

static void vlSIMDStoreVec4I32AVX2(vl_int32_t* ptr, vl_simd_vec4_i32 v)
{
    __m128i vec = _mm_loadu_si128((const __m128i*)v.components);
    _mm_storeu_si128((__m128i*)ptr, vec);
}

static vl_simd_vec4_i32 vlSIMDAddVec4I32AVX2(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    vl_simd_vec4_i32 result;
    __m128i va = _mm_loadu_si128((const __m128i*)a.components);
    __m128i vb = _mm_loadu_si128((const __m128i*)b.components);
    __m128i sum = _mm_add_epi32(va, vb);
    _mm_storeu_si128((__m128i*)result.components, sum);
    return result;
}

static vl_simd_vec4_i32 vlSIMDMulVec4I32AVX2(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    vl_simd_vec4_i32 result;
    __m128i va = _mm_loadu_si128((const __m128i*)a.components);
    __m128i vb = _mm_loadu_si128((const __m128i*)b.components);
    __m128i prod = _mm_mullo_epi32(va, vb);
    _mm_storeu_si128((__m128i*)result.components, prod);
    return result;
}

/* ============================================================================
 * Integer 16-bit Operations (I16)
 * ============================================================================
 */

static vl_simd_vec8_i16 vlSIMDLoadVec8I16AVX2(const vl_int16_t* ptr)
{
    vl_simd_vec8_i16 result;
    __m128i loaded = _mm_loadu_si128((const __m128i*)ptr);
    _mm_storeu_si128((__m128i*)result.components, loaded);
    return result;
}

static void vlSIMDStoreVec8I16AVX2(vl_int16_t* ptr, vl_simd_vec8_i16 v)
{
    __m128i vec = _mm_loadu_si128((const __m128i*)v.components);
    _mm_storeu_si128((__m128i*)ptr, vec);
}

static vl_simd_vec8_i16 vlSIMDAddVec8I16AVX2(vl_simd_vec8_i16 a, vl_simd_vec8_i16 b)
{
    vl_simd_vec8_i16 result;
    __m128i va = _mm_loadu_si128((const __m128i*)a.components);
    __m128i vb = _mm_loadu_si128((const __m128i*)b.components);
    __m128i sum = _mm_add_epi16(va, vb);
    _mm_storeu_si128((__m128i*)result.components, sum);
    return result;
}

/* ============================================================================
 * Integer 8-bit Operations (U8)
 * ============================================================================
 */

static vl_simd_vec32_u8 vlSIMDLoadVec32U8AVX2(const vl_uint8_t* ptr)
{
    vl_simd_vec32_u8 result;
    __m256i loaded = _mm256_loadu_si256((const __m256i*)ptr);
    _mm256_storeu_si256((__m256i*)result.components, loaded);
    return result;
}

static void vlSIMDStoreVec32U8AVX2(vl_uint8_t* ptr, vl_simd_vec32_u8 v)
{
    __m256i vec = _mm256_loadu_si256((const __m256i*)v.components);
    _mm256_storeu_si256((__m256i*)ptr, vec);
}

static vl_simd_vec8_f32 vlSIMDSplatVec8F32AVX2(vl_float32_t scalar)
{
    vl_simd_vec8_f32 result;
    __m256 splat = _mm256_set1_ps(scalar);
    _mm256_storeu_ps(result.components, splat);
    return result;
}

static vl_simd_vec8_f32 vlSIMDSubVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 diff = _mm256_sub_ps(va, vb);
    _mm256_storeu_ps(result.components, diff);
    return result;
}

static vl_simd_vec8_f32 vlSIMDLtVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 cmp = _mm256_cmp_ps(va, vb, _CMP_LT_OQ);
    _mm256_storeu_ps(result.components, cmp);
    return result;
}

static vl_simd_vec8_f32 vlSIMDGtVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 cmp = _mm256_cmp_ps(va, vb, _CMP_GT_OQ);
    _mm256_storeu_ps(result.components, cmp);
    return result;
}

static vl_simd_vec8_f32 vlSIMDEqVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 cmp = _mm256_cmp_ps(va, vb, _CMP_EQ_OQ);
    _mm256_storeu_ps(result.components, cmp);
    return result;
}

static vl_simd_vec8_f32 vlSIMDAndVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 res = _mm256_and_ps(va, vb);
    _mm256_storeu_ps(result.components, res);
    return result;
}

static vl_simd_vec8_f32 vlSIMDOrVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 res = _mm256_or_ps(va, vb);
    _mm256_storeu_ps(result.components, res);
    return result;
}

static vl_simd_vec8_f32 vlSIMDXorVec8F32AVX2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 vb = _mm256_loadu_ps(b.components);
    __m256 res = _mm256_xor_ps(va, vb);
    _mm256_storeu_ps(result.components, res);
    return result;
}

static vl_simd_vec8_f32 vlSIMDNotVec8F32AVX2(vl_simd_vec8_f32 a)
{
    vl_simd_vec8_f32 result;
    __m256 va = _mm256_loadu_ps(a.components);
    __m256 all_ones = _mm256_castsi256_ps(_mm256_set1_epi32(-1));
    __m256 res = _mm256_xor_ps(va, all_ones);
    _mm256_storeu_ps(result.components, res);
    return result;
}

/* ============================================================================
 * Initialization
 * ============================================================================
 */

void vlSIMDInit_AVX2(void)
{
    vlSIMDFunctions.load_vec4f32 = vlSIMDLoadVec4F32AVX2;
    vlSIMDFunctions.store_vec4f32 = vlSIMDStoreVec4F32AVX2;
    vlSIMDFunctions.splat_vec4f32 = vlSIMDSplatVec4F32AVX2;
    vlSIMDFunctions.add_vec4f32 = vlSIMDAddVec4F32AVX2;
    vlSIMDFunctions.sub_vec4f32 = vlSIMDSubVec4F32AVX2;
    vlSIMDFunctions.mul_vec4f32 = vlSIMDMulVec4F32AVX2;
    vlSIMDFunctions.div_vec4f32 = vlSIMDDivVec4F32AVX2;
    vlSIMDFunctions.fma_vec4f32 = vlSIMDFmaVec4F32AVX2;
    vlSIMDFunctions.hsum_vec4f32 = vlSIMDHsumVec4F32AVX2;
    vlSIMDFunctions.hmax_vec4f32 = vlSIMDHmaxVec4F32AVX2;
    vlSIMDFunctions.hmin_vec4f32 = vlSIMDHminVec4F32AVX2;
    vlSIMDFunctions.hprod_vec4f32 = vlSIMDHprodVec4F32AVX2;
    vlSIMDFunctions.extract_lane_vec4f32 = vlSIMDExtractLaneVec4F32AVX2;
    vlSIMDFunctions.broadcast_lane_vec4f32 = vlSIMDBroadcastLaneVec4F32AVX2;
    vlSIMDFunctions.load_vec8f32 = vlSIMDLoadVec8F32AVX2;
    vlSIMDFunctions.store_vec8f32 = vlSIMDStoreVec8F32AVX2;
    vlSIMDFunctions.add_vec8f32 = vlSIMDAddVec8F32AVX2;
    vlSIMDFunctions.mul_vec8f32 = vlSIMDMulVec8F32AVX2;
    vlSIMDFunctions.fma_vec8f32 = vlSIMDFmaVec8F32AVX2;
    vlSIMDFunctions.splat_vec8f32 = vlSIMDSplatVec8F32AVX2;
    vlSIMDFunctions.sub_vec8f32 = vlSIMDSubVec8F32AVX2;
    vlSIMDFunctions.lt_vec8f32 = vlSIMDLtVec8F32AVX2;
    vlSIMDFunctions.gt_vec8f32 = vlSIMDGtVec8F32AVX2;
    vlSIMDFunctions.eq_vec8f32 = vlSIMDEqVec8F32AVX2;
    vlSIMDFunctions.and_vec8f32 = vlSIMDAndVec8F32AVX2;
    vlSIMDFunctions.or_vec8f32 = vlSIMDOrVec8F32AVX2;
    vlSIMDFunctions.xor_vec8f32 = vlSIMDXorVec8F32AVX2;
    vlSIMDFunctions.not_vec8f32 = vlSIMDNotVec8F32AVX2;
    vlSIMDFunctions.lt_vec4f32 = vlSIMDLtVec4F32AVX2;
    vlSIMDFunctions.gt_vec4f32 = vlSIMDGtVec4F32AVX2;
    vlSIMDFunctions.eq_vec4f32 = vlSIMDEqVec4F32AVX2;
    vlSIMDFunctions.and_vec4f32 = vlSIMDAndVec4F32AVX2;
    vlSIMDFunctions.or_vec4f32 = vlSIMDOrVec4F32AVX2;
    vlSIMDFunctions.xor_vec4f32 = vlSIMDXorVec4F32AVX2;
    vlSIMDFunctions.not_vec4f32 = vlSIMDNotVec4F32AVX2;
    vlSIMDFunctions.load_vec4i32 = vlSIMDLoadVec4I32AVX2;
    vlSIMDFunctions.store_vec4i32 = vlSIMDStoreVec4I32AVX2;
    vlSIMDFunctions.add_vec4i32 = vlSIMDAddVec4I32AVX2;
    vlSIMDFunctions.mul_vec4i32 = vlSIMDMulVec4I32AVX2;
    vlSIMDFunctions.load_vec8i16 = vlSIMDLoadVec8I16AVX2;
    vlSIMDFunctions.store_vec8i16 = vlSIMDStoreVec8I16AVX2;
    vlSIMDFunctions.add_vec8i16 = vlSIMDAddVec8I16AVX2;
    vlSIMDFunctions.load_vec32u8 = vlSIMDLoadVec32U8AVX2;
    vlSIMDFunctions.store_vec32u8 = vlSIMDStoreVec32U8AVX2;

    vlSIMDFunctions.backend_name = "AVX2";
}
