
/**
 * \file vl_simd_sse2.c
 * \brief SSE2 SIMD implementation
 *
 * Provides optimized SSE2 implementations for 4-wide float and integer
 * operations. SSE2 is available on all x86-64 processors and most modern x86
 * processors.
 */

#include <emmintrin.h>
#include <string.h>
#include <vl/vl_simd.h>

/* ============================================================================
 * 4-Wide Float32 Operations
 * ============================================================================
 */

static vl_simd_vec4_f32 vlSIMDLoadVec4F32SSE2(const vl_float32_t* ptr)
{
    vl_simd_vec4_f32 result;
    _mm_storeu_ps(result.components, _mm_loadu_ps(ptr));
    return result;
}

static void vlSIMDStoreVec4F32SSE2(vl_float32_t* ptr, vl_simd_vec4_f32 v)
{
    _mm_storeu_ps(ptr, _mm_loadu_ps(v.components));
}

static vl_simd_vec4_f32 vlSIMDSplatVec4F32SSE2(vl_float32_t scalar)
{
    vl_simd_vec4_f32 result;
    _mm_storeu_ps(result.components, _mm_set1_ps(scalar));
    return result;
}

static vl_simd_vec4_f32 vlSIMDAddVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_add_ps(ma, mb));
    return result;
}

static vl_simd_vec4_f32 vlSIMDSubVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_sub_ps(ma, mb));
    return result;
}

static vl_simd_vec4_f32 vlSIMDMulVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_mul_ps(ma, mb));
    return result;
}

static vl_simd_vec4_f32 vlSIMDDivVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_div_ps(ma, mb));
    return result;
}

static vl_simd_vec4_f32 vlSIMDFmaVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b, vl_simd_vec4_f32 c)
{
    // SSE2 doesn't have FMA, so compute (a * b) + c
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    __m128 mc = _mm_loadu_ps(c.components);
    __m128 prod = _mm_mul_ps(ma, mb);
    _mm_storeu_ps(result.components, _mm_add_ps(prod, mc));
    return result;
}

static vl_float32_t vlSIMDHsumVec4F32SSE2(vl_simd_vec4_f32 v)
{
    __m128 m = _mm_loadu_ps(v.components);

    // Horizontal sum using shuffles
    __m128 shuf = _mm_shuffle_ps(m, m, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(m, shuf);

    shuf = _mm_shuffle_ps(sums, sums, _MM_SHUFFLE(1, 0, 3, 2));
    sums = _mm_add_ps(sums, shuf);

    return _mm_cvtss_f32(sums);
}

static vl_float32_t vlSIMDHmaxVec4F32SSE2(vl_simd_vec4_f32 v)
{
    __m128 m = _mm_loadu_ps(v.components);

    __m128 shuf = _mm_shuffle_ps(m, m, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 maxs = _mm_max_ps(m, shuf);

    shuf = _mm_shuffle_ps(maxs, maxs, _MM_SHUFFLE(1, 0, 3, 2));
    maxs = _mm_max_ps(maxs, shuf);

    return _mm_cvtss_f32(maxs);
}

static vl_float32_t vlSIMDHminVec4F32SSE2(vl_simd_vec4_f32 v)
{
    __m128 m = _mm_loadu_ps(v.components);

    __m128 shuf = _mm_shuffle_ps(m, m, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 mins = _mm_min_ps(m, shuf);

    shuf = _mm_shuffle_ps(mins, mins, _MM_SHUFFLE(1, 0, 3, 2));
    mins = _mm_min_ps(mins, shuf);

    return _mm_cvtss_f32(mins);
}

static vl_float32_t vlSIMDHprodVec4F32SSE2(vl_simd_vec4_f32 v)
{
    __m128 m = _mm_loadu_ps(v.components);

    __m128 shuf = _mm_shuffle_ps(m, m, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 prods = _mm_mul_ps(m, shuf);

    shuf = _mm_shuffle_ps(prods, prods, _MM_SHUFFLE(1, 0, 3, 2));
    prods = _mm_mul_ps(prods, shuf);

    return _mm_cvtss_f32(prods);
}

static vl_float32_t vlSIMDExtractLaneVec4F32SSE2(vl_simd_vec4_f32 v, int lane) { return v.components[lane & 3]; }

static vl_simd_vec4_f32 vlSIMDBroadcastLaneVec4F32SSE2(vl_simd_vec4_f32 v, int lane)
{
    vl_float32_t val = v.components[lane & 3];
    return vlSIMDSplatVec4F32SSE2(val);
}

/* Comparison operations */
static vl_simd_vec4_f32 vlSIMDLtVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_cmplt_ps(ma, mb));
    return result;
}

static vl_simd_vec4_f32 vlSIMDGtVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_cmpgt_ps(ma, mb));
    return result;
}

static vl_simd_vec4_f32 vlSIMDEqVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_cmpeq_ps(ma, mb));
    return result;
}

/* Bitwise operations */
static vl_simd_vec4_f32 vlSIMDAndVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_and_ps(ma, mb));
    return result;
}

static vl_simd_vec4_f32 vlSIMDOrVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_or_ps(ma, mb));
    return result;
}

static vl_simd_vec4_f32 vlSIMDXorVec4F32SSE2(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 mb = _mm_loadu_ps(b.components);
    _mm_storeu_ps(result.components, _mm_xor_ps(ma, mb));
    return result;
}

static vl_simd_vec4_f32 vlSIMDNotVec4F32SSE2(vl_simd_vec4_f32 a)
{
    vl_simd_vec4_f32 result;
    __m128 ma = _mm_loadu_ps(a.components);
    __m128 all_bits = _mm_castsi128_ps(_mm_set1_epi32(0xFFFFFFFF));
    _mm_storeu_ps(result.components, _mm_xor_ps(ma, all_bits));
    return result;
}

/* ============================================================================
 * 4-Wide Integer32 Operations
 * ============================================================================
 */

static vl_simd_vec4_i32 vlSIMDLoadVec4I32SSE2(const vl_int32_t* ptr)
{
    vl_simd_vec4_i32 result;
    __m128i v = _mm_loadu_si128((const __m128i*)ptr);
    _mm_storeu_si128((__m128i*)result.components, v);
    return result;
}

static void vlSIMDStoreVec4I32SSE2(vl_int32_t* ptr, vl_simd_vec4_i32 v)
{
    _mm_storeu_si128((__m128i*)ptr, _mm_loadu_si128((const __m128i*)v.components));
}

static vl_simd_vec4_i32 vlSIMDAddVec4I32SSE2(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    vl_simd_vec4_i32 result;
    __m128i va = _mm_loadu_si128((const __m128i*)a.components);
    __m128i vb = _mm_loadu_si128((const __m128i*)b.components);
    _mm_storeu_si128((__m128i*)result.components, _mm_add_epi32(va, vb));
    return result;
}

static vl_simd_vec4_i32 vlSIMDMulVec4I32SSE2(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    // SSE2 doesn't have 32-bit multiply, need to use epu32_mul32 workaround
    vl_simd_vec4_i32 result;
    for (int i = 0; i < 4; i++)
    {
        result.components[i] = a.components[i] * b.components[i];
    }
    return result;
}

/* ============================================================================
 * 8-Wide Integer16 Operations
 * ============================================================================
 */

static vl_simd_vec8_i16 vlSIMDLoadVec8I16SSE2(const vl_int16_t* ptr)
{
    vl_simd_vec8_i16 result;
    __m128i v = _mm_loadu_si128((const __m128i*)ptr);
    _mm_storeu_si128((__m128i*)result.components, v);
    return result;
}

static void vlSIMDStoreVec8I16SSE2(vl_int16_t* ptr, vl_simd_vec8_i16 v)
{
    _mm_storeu_si128((__m128i*)ptr, _mm_loadu_si128((const __m128i*)v.components));
}

static vl_simd_vec8_i16 vlSIMDAddVec8I16SSE2(vl_simd_vec8_i16 a, vl_simd_vec8_i16 b)
{
    vl_simd_vec8_i16 result;
    __m128i va = _mm_loadu_si128((const __m128i*)a.components);
    __m128i vb = _mm_loadu_si128((const __m128i*)b.components);
    _mm_storeu_si128((__m128i*)result.components, _mm_add_epi16(va, vb));
    return result;
}

/* ============================================================================
 * 32-Wide Unsigned8 Operations
 * ============================================================================
 */

static vl_simd_vec32_u8 vlSIMDLoadVec32U8SSE2(const vl_uint8_t* ptr)
{
    vl_simd_vec32_u8 result;
    // SSE2 can only load 16 bytes, so use two loads
    __m128i v0 = _mm_loadu_si128((const __m128i*)(ptr + 0));
    __m128i v1 = _mm_loadu_si128((const __m128i*)(ptr + 16));
    _mm_storeu_si128((__m128i*)(result.components + 0), v0);
    _mm_storeu_si128((__m128i*)(result.components + 16), v1);
    return result;
}

static void vlSIMDStoreVec32U8SSE2(vl_uint8_t* ptr, vl_simd_vec32_u8 v)
{
    _mm_storeu_si128((__m128i*)(ptr + 0), _mm_loadu_si128((const __m128i*)(v.components + 0)));
    _mm_storeu_si128((__m128i*)(ptr + 16), _mm_loadu_si128((const __m128i*)(v.components + 16)));
}

static vl_simd_vec8_f32 vlSIMDSplatVec8F32SSE2(vl_float32_t scalar)
{
    vl_simd_vec8_f32 result;
    __m128 splat = _mm_set1_ps(scalar);
    _mm_storeu_ps(&result.components[0], splat);
    _mm_storeu_ps(&result.components[4], splat);
    return result;
}

static vl_simd_vec8_f32 vlSIMDSubVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    _mm_storeu_ps(&result.components[0], _mm_sub_ps(va_low, vb_low));
    _mm_storeu_ps(&result.components[4], _mm_sub_ps(va_high, vb_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDLtVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    _mm_storeu_ps(&result.components[0], _mm_cmplt_ps(va_low, vb_low));
    _mm_storeu_ps(&result.components[4], _mm_cmplt_ps(va_high, vb_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDGtVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    _mm_storeu_ps(&result.components[0], _mm_cmpgt_ps(va_low, vb_low));
    _mm_storeu_ps(&result.components[4], _mm_cmpgt_ps(va_high, vb_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDEqVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    _mm_storeu_ps(&result.components[0], _mm_cmpeq_ps(va_low, vb_low));
    _mm_storeu_ps(&result.components[4], _mm_cmpeq_ps(va_high, vb_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDAndVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    _mm_storeu_ps(&result.components[0], _mm_and_ps(va_low, vb_low));
    _mm_storeu_ps(&result.components[4], _mm_and_ps(va_high, vb_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDOrVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    _mm_storeu_ps(&result.components[0], _mm_or_ps(va_low, vb_low));
    _mm_storeu_ps(&result.components[4], _mm_or_ps(va_high, vb_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDXorVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    _mm_storeu_ps(&result.components[0], _mm_xor_ps(va_low, vb_low));
    _mm_storeu_ps(&result.components[4], _mm_xor_ps(va_high, vb_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDNotVec8F32SSE2(vl_simd_vec8_f32 a)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 all_bits = _mm_castsi128_ps(_mm_set1_epi32(0xFFFFFFFF));
    _mm_storeu_ps(&result.components[0], _mm_xor_ps(va_low, all_bits));
    _mm_storeu_ps(&result.components[4], _mm_xor_ps(va_high, all_bits));
    return result;
}

/* ============================================================================
 * 8-Wide Float32 Operations
 * ============================================================================
 */

static vl_simd_vec8_f32 vlSIMDLoadVec8F32SSE2(const vl_float32_t* ptr)
{
    vl_simd_vec8_f32 result;
    __m128 v0 = _mm_loadu_ps(&ptr[0]);
    __m128 v1 = _mm_loadu_ps(&ptr[4]);
    _mm_storeu_ps(&result.components[0], v0);
    _mm_storeu_ps(&result.components[4], v1);
    return result;
}

static void vlSIMDStoreVec8F32SSE2(vl_float32_t* ptr, vl_simd_vec8_f32 v)
{
    __m128 v0 = _mm_loadu_ps(&v.components[0]);
    __m128 v1 = _mm_loadu_ps(&v.components[4]);
    _mm_storeu_ps(&ptr[0], v0);
    _mm_storeu_ps(&ptr[4], v1);
}

static vl_simd_vec8_f32 vlSIMDAddVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    _mm_storeu_ps(&result.components[0], _mm_add_ps(va_low, vb_low));
    _mm_storeu_ps(&result.components[4], _mm_add_ps(va_high, vb_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDMulVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    _mm_storeu_ps(&result.components[0], _mm_mul_ps(va_low, vb_low));
    _mm_storeu_ps(&result.components[4], _mm_mul_ps(va_high, vb_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDFmaVec8F32SSE2(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b, vl_simd_vec8_f32 c)
{
    vl_simd_vec8_f32 result;
    __m128 va_low = _mm_loadu_ps(&a.components[0]);
    __m128 va_high = _mm_loadu_ps(&a.components[4]);
    __m128 vb_low = _mm_loadu_ps(&b.components[0]);
    __m128 vb_high = _mm_loadu_ps(&b.components[4]);
    __m128 vc_low = _mm_loadu_ps(&c.components[0]);
    __m128 vc_high = _mm_loadu_ps(&c.components[4]);

    __m128 prod_low = _mm_mul_ps(va_low, vb_low);
    __m128 prod_high = _mm_mul_ps(va_high, vb_high);

    _mm_storeu_ps(&result.components[0], _mm_add_ps(prod_low, vc_low));
    _mm_storeu_ps(&result.components[4], _mm_add_ps(prod_high, vc_high));
    return result;
}

/* ============================================================================
 * Initialization
 * ============================================================================
 */

void vlSIMDInit_SSE2(void)
{
    vlSIMDFunctions.load_vec4f32 = vlSIMDLoadVec4F32SSE2;
    vlSIMDFunctions.store_vec4f32 = vlSIMDStoreVec4F32SSE2;
    vlSIMDFunctions.splat_vec4f32 = vlSIMDSplatVec4F32SSE2;
    vlSIMDFunctions.add_vec4f32 = vlSIMDAddVec4F32SSE2;
    vlSIMDFunctions.sub_vec4f32 = vlSIMDSubVec4F32SSE2;
    vlSIMDFunctions.mul_vec4f32 = vlSIMDMulVec4F32SSE2;
    vlSIMDFunctions.div_vec4f32 = vlSIMDDivVec4F32SSE2;
    vlSIMDFunctions.fma_vec4f32 = vlSIMDFmaVec4F32SSE2;
    vlSIMDFunctions.hsum_vec4f32 = vlSIMDHsumVec4F32SSE2;
    vlSIMDFunctions.hmax_vec4f32 = vlSIMDHmaxVec4F32SSE2;
    vlSIMDFunctions.hmin_vec4f32 = vlSIMDHminVec4F32SSE2;
    vlSIMDFunctions.hprod_vec4f32 = vlSIMDHprodVec4F32SSE2;
    vlSIMDFunctions.extract_lane_vec4f32 = vlSIMDExtractLaneVec4F32SSE2;
    vlSIMDFunctions.broadcast_lane_vec4f32 = vlSIMDBroadcastLaneVec4F32SSE2;
    vlSIMDFunctions.splat_vec8f32 = vlSIMDSplatVec8F32SSE2;
    vlSIMDFunctions.sub_vec8f32 = vlSIMDSubVec8F32SSE2;
    vlSIMDFunctions.lt_vec8f32 = vlSIMDLtVec8F32SSE2;
    vlSIMDFunctions.gt_vec8f32 = vlSIMDGtVec8F32SSE2;
    vlSIMDFunctions.eq_vec8f32 = vlSIMDEqVec8F32SSE2;
    vlSIMDFunctions.and_vec8f32 = vlSIMDAndVec8F32SSE2;
    vlSIMDFunctions.or_vec8f32 = vlSIMDOrVec8F32SSE2;
    vlSIMDFunctions.xor_vec8f32 = vlSIMDXorVec8F32SSE2;
    vlSIMDFunctions.not_vec8f32 = vlSIMDNotVec8F32SSE2;
    vlSIMDFunctions.lt_vec4f32 = vlSIMDLtVec4F32SSE2;
    vlSIMDFunctions.gt_vec4f32 = vlSIMDGtVec4F32SSE2;
    vlSIMDFunctions.eq_vec4f32 = vlSIMDEqVec4F32SSE2;
    vlSIMDFunctions.and_vec4f32 = vlSIMDAndVec4F32SSE2;
    vlSIMDFunctions.or_vec4f32 = vlSIMDOrVec4F32SSE2;
    vlSIMDFunctions.xor_vec4f32 = vlSIMDXorVec4F32SSE2;
    vlSIMDFunctions.not_vec4f32 = vlSIMDNotVec4F32SSE2;
    vlSIMDFunctions.load_vec4i32 = vlSIMDLoadVec4I32SSE2;
    vlSIMDFunctions.store_vec4i32 = vlSIMDStoreVec4I32SSE2;
    vlSIMDFunctions.add_vec4i32 = vlSIMDAddVec4I32SSE2;
    vlSIMDFunctions.mul_vec4i32 = vlSIMDMulVec4I32SSE2;
    vlSIMDFunctions.load_vec8i16 = vlSIMDLoadVec8I16SSE2;
    vlSIMDFunctions.store_vec8i16 = vlSIMDStoreVec8I16SSE2;
    vlSIMDFunctions.add_vec8i16 = vlSIMDAddVec8I16SSE2;
    vlSIMDFunctions.load_vec32u8 = vlSIMDLoadVec32U8SSE2;
    vlSIMDFunctions.store_vec32u8 = vlSIMDStoreVec32U8SSE2;
    vlSIMDFunctions.load_vec8f32 = vlSIMDLoadVec8F32SSE2;
    vlSIMDFunctions.store_vec8f32 = vlSIMDStoreVec8F32SSE2;
    vlSIMDFunctions.add_vec8f32 = vlSIMDAddVec8F32SSE2;
    vlSIMDFunctions.mul_vec8f32 = vlSIMDMulVec8F32SSE2;
    vlSIMDFunctions.fma_vec8f32 = vlSIMDFmaVec8F32SSE2;
    vlSIMDFunctions.backend_name = "SSE2";
}
