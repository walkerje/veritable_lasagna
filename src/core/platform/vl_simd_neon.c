
/**
 * \file vl_simd_neon.c
 * \brief SIMD implementation using ARM NEON (ARMv7 with NEON)
 *
 * Provides optimized SIMD operations using 128-bit NEON vectors.
 * All operations target the NEON extension available on ARMv7.
 */

#include <arm_neon.h>
#include <vl/vl_simd.h>

/* ============================================================================
 * 4-wide F32 Operations
 * ============================================================================
 */

static vl_simd_vec4_f32 vlSIMDLoadVec4F32NEON(const vl_float32_t* ptr)
{
    vl_simd_vec4_f32 result;
    float32x4_t v = vld1q_f32(ptr);
    vst1q_f32(result.components, v);
    return result;
}

static void vlSIMDStoreVec4F32NEON(vl_float32_t* ptr, vl_simd_vec4_f32 v)
{
    float32x4_t vec = vld1q_f32(v.components);
    vst1q_f32(ptr, vec);
}

static vl_simd_vec4_f32 vlSIMDSplatVec4F32NEON(vl_float32_t scalar)
{
    vl_simd_vec4_f32 result;
    float32x4_t v = vdupq_n_f32(scalar);
    vst1q_f32(result.components, v);
    return result;
}

static vl_simd_vec4_f32 vlSIMDAddVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    float32x4_t sum = vaddq_f32(va, vb);
    vst1q_f32(result.components, sum);
    return result;
}

static vl_simd_vec4_f32 vlSIMDSubVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    float32x4_t diff = vsubq_f32(va, vb);
    vst1q_f32(result.components, diff);
    return result;
}

static vl_simd_vec4_f32 vlSIMDMulVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    float32x4_t prod = vmulq_f32(va, vb);
    vst1q_f32(result.components, prod);
    return result;
}

static vl_simd_vec4_f32 vlSIMDDivVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);

    // NEON doesn't have direct division, use reciprocal approximation
    float32x4_t reciprocal = vrecpeq_f32(vb);
    // Refine reciprocal (Newton-Raphson iteration)
    reciprocal = vmulq_f32(reciprocal, vrecpsq_f32(vb, reciprocal));
    reciprocal = vmulq_f32(reciprocal, vrecpsq_f32(vb, reciprocal));

    float32x4_t quotient = vmulq_f32(va, reciprocal);
    vst1q_f32(result.components, quotient);
    return result;
}

static vl_simd_vec4_f32 vlSIMDFmaVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b, vl_simd_vec4_f32 c)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    float32x4_t vc = vld1q_f32(c.components);

    // FMA: a * b + c
    float32x4_t fma_result = vmlaq_f32(vc, va, vb);
    vst1q_f32(result.components, fma_result);
    return result;
}

static vl_float32_t vlSIMDHsumVec4F32NEON(vl_simd_vec4_f32 v)
{
    float32x4_t vec = vld1q_f32(v.components);
    float32x2_t sum = vadd_f32(vget_high_f32(vec), vget_low_f32(vec));
    sum = vpadd_f32(sum, sum);
    return vget_lane_f32(sum, 0);
}

static vl_float32_t vlSIMDHmaxVec4F32NEON(vl_simd_vec4_f32 v)
{
    float32x4_t vec = vld1q_f32(v.components);
    float32x4_t max_h = vpmaxq_f32(vec, vec);
    max_h = vpmaxq_f32(max_h, max_h);
    return vgetq_lane_f32(max_h, 0);
}

static vl_float32_t vlSIMDHminVec4F32NEON(vl_simd_vec4_f32 v)
{
    float32x4_t vec = vld1q_f32(v.components);
    float32x4_t min_h = vpminq_f32(vec, vec);
    min_h = vpminq_f32(min_h, min_h);
    return vgetq_lane_f32(min_h, 0);
}

static vl_float32_t vlSIMDHprodVec4F32NEON(vl_simd_vec4_f32 v)
{
    float32x4_t vec = vld1q_f32(v.components);
    float32x2_t prod = vmul_f32(vget_high_f32(vec), vget_low_f32(vec));
    prod = vmul_f32(prod, vrev64_f32(prod));
    return vget_lane_f32(prod, 0);
}

static vl_float32_t vlSIMDExtractLaneVec4F32NEON(vl_simd_vec4_f32 v, int lane)
{
    float32x4_t vec = vld1q_f32(v.components);
    return vgetq_lane_f32(vec, lane & 3);
}

static vl_simd_vec4_f32 vlSIMDBroadcastLaneVec4F32NEON(vl_simd_vec4_f32 v, int lane)
{
    float32x4_t vec = vld1q_f32(v.components);
    float32_t scalar = vgetq_lane_f32(vec, lane & 3);
    return vlSIMDSplatVec4F32NEON(scalar);
}

/* Comparison operations */
static vl_simd_vec4_f32 vlSIMDLtVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    uint32x4_t cmp = vcltq_f32(va, vb);
    vst1q_f32(result.components, vreinterpretq_f32_u32(cmp));
    return result;
}

static vl_simd_vec4_f32 vlSIMDGtVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    uint32x4_t cmp = vcgtq_f32(va, vb);
    vst1q_f32(result.components, vreinterpretq_f32_u32(cmp));
    return result;
}

static vl_simd_vec4_f32 vlSIMDEqVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    uint32x4_t cmp = vceqq_f32(va, vb);
    vst1q_f32(result.components, vreinterpretq_f32_u32(cmp));
    return result;
}

/* Bitwise operations */
static vl_simd_vec4_f32 vlSIMDAndVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    uint32x4_t va = vreinterpretq_u32_f32(vld1q_f32(a.components));
    uint32x4_t vb = vreinterpretq_u32_f32(vld1q_f32(b.components));
    uint32x4_t res = vandq_u32(va, vb);
    vst1q_f32(result.components, vreinterpretq_f32_u32(res));
    return result;
}

static vl_simd_vec4_f32 vlSIMDOrVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    uint32x4_t va = vreinterpretq_u32_f32(vld1q_f32(a.components));
    uint32x4_t vb = vreinterpretq_u32_f32(vld1q_f32(b.components));
    uint32x4_t res = vorrq_u32(va, vb);
    vst1q_f32(result.components, vreinterpretq_f32_u32(res));
    return result;
}

static vl_simd_vec4_f32 vlSIMDXorVec4F32NEON(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    uint32x4_t va = vreinterpretq_u32_f32(vld1q_f32(a.components));
    uint32x4_t vb = vreinterpretq_u32_f32(vld1q_f32(b.components));
    uint32x4_t res = veorq_u32(va, vb);
    vst1q_f32(result.components, vreinterpretq_f32_u32(res));
    return result;
}

static vl_simd_vec4_f32 vlSIMDNotVec4F32NEON(vl_simd_vec4_f32 a)
{
    vl_simd_vec4_f32 result;
    uint32x4_t va = vreinterpretq_u32_f32(vld1q_f32(a.components));
    uint32x4_t res = vmvnq_u32(va);
    vst1q_f32(result.components, vreinterpretq_f32_u32(res));
    return result;
}

/* ============================================================================
 * 4-wide I32 Operations
 * ============================================================================
 */

static vl_simd_vec4_i32 vlSIMDLoadVec4I32NEON(const vl_int32_t* ptr)
{
    vl_simd_vec4_i32 result;
    int32x4_t v = vld1q_s32(ptr);
    vst1q_s32(result.components, v);
    return result;
}

static void vlSIMDStoreVec4I32NEON(vl_int32_t* ptr, vl_simd_vec4_i32 v)
{
    int32x4_t vec = vld1q_s32(v.components);
    vst1q_s32(ptr, vec);
}

static vl_simd_vec4_i32 vlSIMDAddVec4I32NEON(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    vl_simd_vec4_i32 result;
    int32x4_t va = vld1q_s32(a.components);
    int32x4_t vb = vld1q_s32(b.components);
    int32x4_t sum = vaddq_s32(va, vb);
    vst1q_s32(result.components, sum);
    return result;
}

static vl_simd_vec4_i32 vlSIMDMulVec4I32NEON(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    vl_simd_vec4_i32 result;
    int32x4_t va = vld1q_s32(a.components);
    int32x4_t vb = vld1q_s32(b.components);
    int32x4_t prod = vmulq_s32(va, vb);
    vst1q_s32(result.components, prod);
    return result;
}

/* ============================================================================
 * 8-wide I16 Operations
 * ============================================================================
 */

static vl_simd_vec8_i16 vlSIMDLoadVec8I16NEON(const vl_int16_t* ptr)
{
    vl_simd_vec8_i16 result;
    int16x8_t v = vld1q_s16(ptr);
    vst1q_s16(result.components, v);
    return result;
}

static void vlSIMDStoreVec8I16NEON(vl_int16_t* ptr, vl_simd_vec8_i16 v)
{
    int16x8_t vec = vld1q_s16(v.components);
    vst1q_s16(ptr, vec);
}

static vl_simd_vec8_i16 vlSIMDAddVec8I16NEON(vl_simd_vec8_i16 a, vl_simd_vec8_i16 b)
{
    vl_simd_vec8_i16 result;
    int16x8_t va = vld1q_s16(a.components);
    int16x8_t vb = vld1q_s16(b.components);
    int16x8_t sum = vaddq_s16(va, vb);
    vst1q_s16(result.components, sum);
    return result;
}

/* ============================================================================
 * 32-wide U8 Operations
 * ============================================================================
 */

static vl_simd_vec32_u8 vlSIMDLoadVec32U8NEON(const vl_uint8_t* ptr)
{
    vl_simd_vec32_u8 result;
    // Load in 4x 8-byte chunks since we're on ARMv7
    for (int i = 0; i < 4; i++)
    {
        uint8x8_t v = vld1_u8(&ptr[i * 8]);
        vst1_u8(&result.components[i * 8], v);
    }
    return result;
}

static void vlSIMDStoreVec32U8NEON(vl_uint8_t* ptr, vl_simd_vec32_u8 v)
{
    for (int i = 0; i < 4; i++)
    {
        uint8x8_t vec = vld1_u8(&v.components[i * 8]);
        vst1_u8(&ptr[i * 8], vec);
    }
}

/* ============================================================================
 * 8-wide F32 Operations (using two 128-bit NEON registers)
 * ============================================================================
 */

static vl_simd_vec8_f32 vlSIMDLoadVec8F32NEON(const vl_float32_t* ptr)
{
    vl_simd_vec8_f32 result;
    float32x4_t v0 = vld1q_f32(&ptr[0]);
    float32x4_t v1 = vld1q_f32(&ptr[4]);
    vst1q_f32(&result.components[0], v0);
    vst1q_f32(&result.components[4], v1);
    return result;
}

static void vlSIMDStoreVec8F32NEON(vl_float32_t* ptr, vl_simd_vec8_f32 v)
{
    float32x4_t v0 = vld1q_f32(&v.components[0]);
    float32x4_t v1 = vld1q_f32(&v.components[4]);
    vst1q_f32(&ptr[0], v0);
    vst1q_f32(&ptr[4], v1);
}

static vl_simd_vec8_f32 vlSIMDSplatVec8F32NEON(vl_float32_t scalar)
{
    vl_simd_vec8_f32 result;
    float32x4_t v = vdupq_n_f32(scalar);
    vst1q_f32(&result.components[0], v);
    vst1q_f32(&result.components[4], v);
    return result;
}

static vl_simd_vec8_f32 vlSIMDAddVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    float32x4_t a_low = vld1q_f32(&a.components[0]);
    float32x4_t a_high = vld1q_f32(&a.components[4]);
    float32x4_t b_low = vld1q_f32(&b.components[0]);
    float32x4_t b_high = vld1q_f32(&b.components[4]);
    vst1q_f32(&result.components[0], vaddq_f32(a_low, b_low));
    vst1q_f32(&result.components[4], vaddq_f32(a_high, b_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDSubVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    float32x4_t a_low = vld1q_f32(&a.components[0]);
    float32x4_t a_high = vld1q_f32(&a.components[4]);
    float32x4_t b_low = vld1q_f32(&b.components[0]);
    float32x4_t b_high = vld1q_f32(&b.components[4]);
    vst1q_f32(&result.components[0], vsubq_f32(a_low, b_low));
    vst1q_f32(&result.components[4], vsubq_f32(a_high, b_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDMulVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    float32x4_t a_low = vld1q_f32(&a.components[0]);
    float32x4_t a_high = vld1q_f32(&a.components[4]);
    float32x4_t b_low = vld1q_f32(&b.components[0]);
    float32x4_t b_high = vld1q_f32(&b.components[4]);
    vst1q_f32(&result.components[0], vmulq_f32(a_low, b_low));
    vst1q_f32(&result.components[4], vmulq_f32(a_high, b_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDFmaVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b, vl_simd_vec8_f32 c)
{
    vl_simd_vec8_f32 result;
    float32x4_t a_low = vld1q_f32(&a.components[0]);
    float32x4_t a_high = vld1q_f32(&a.components[4]);
    float32x4_t b_low = vld1q_f32(&b.components[0]);
    float32x4_t b_high = vld1q_f32(&b.components[4]);
    float32x4_t c_low = vld1q_f32(&c.components[0]);
    float32x4_t c_high = vld1q_f32(&c.components[4]);
    vst1q_f32(&result.components[0], vmlaq_f32(c_low, a_low, b_low));
    vst1q_f32(&result.components[4], vmlaq_f32(c_high, a_high, b_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDLtVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    float32x4_t a_low = vld1q_f32(&a.components[0]);
    float32x4_t a_high = vld1q_f32(&a.components[4]);
    float32x4_t b_low = vld1q_f32(&b.components[0]);
    float32x4_t b_high = vld1q_f32(&b.components[4]);
    uint32x4_t cmp_low = vcltq_f32(a_low, b_low);
    uint32x4_t cmp_high = vcltq_f32(a_high, b_high);
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(cmp_low));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(cmp_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDGtVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    float32x4_t a_low = vld1q_f32(&a.components[0]);
    float32x4_t a_high = vld1q_f32(&a.components[4]);
    float32x4_t b_low = vld1q_f32(&b.components[0]);
    float32x4_t b_high = vld1q_f32(&b.components[4]);
    uint32x4_t cmp_low = vcgtq_f32(a_low, b_low);
    uint32x4_t cmp_high = vcgtq_f32(a_high, b_high);
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(cmp_low));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(cmp_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDEqVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    float32x4_t a_low = vld1q_f32(&a.components[0]);
    float32x4_t a_high = vld1q_f32(&a.components[4]);
    float32x4_t b_low = vld1q_f32(&b.components[0]);
    float32x4_t b_high = vld1q_f32(&b.components[4]);
    uint32x4_t cmp_low = vceqq_f32(a_low, b_low);
    uint32x4_t cmp_high = vceqq_f32(a_high, b_high);
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(cmp_low));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(cmp_high));
    return result;
}

static vl_simd_vec8_f32 vlSIMDAndVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    uint32x4_t a_low = vreinterpretq_u32_f32(vld1q_f32(&a.components[0]));
    uint32x4_t a_high = vreinterpretq_u32_f32(vld1q_f32(&a.components[4]));
    uint32x4_t b_low = vreinterpretq_u32_f32(vld1q_f32(&b.components[0]));
    uint32x4_t b_high = vreinterpretq_u32_f32(vld1q_f32(&b.components[4]));
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(vandq_u32(a_low, b_low)));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(vandq_u32(a_high, b_high)));
    return result;
}

static vl_simd_vec8_f32 vlSIMDOrVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    uint32x4_t a_low = vreinterpretq_u32_f32(vld1q_f32(&a.components[0]));
    uint32x4_t a_high = vreinterpretq_u32_f32(vld1q_f32(&a.components[4]));
    uint32x4_t b_low = vreinterpretq_u32_f32(vld1q_f32(&b.components[0]));
    uint32x4_t b_high = vreinterpretq_u32_f32(vld1q_f32(&b.components[4]));
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(vorrq_u32(a_low, b_low)));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(vorrq_u32(a_high, b_high)));
    return result;
}

static vl_simd_vec8_f32 vlSIMDXorVec8F32NEON(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    uint32x4_t a_low = vreinterpretq_u32_f32(vld1q_f32(&a.components[0]));
    uint32x4_t a_high = vreinterpretq_u32_f32(vld1q_f32(&a.components[4]));
    uint32x4_t b_low = vreinterpretq_u32_f32(vld1q_f32(&b.components[0]));
    uint32x4_t b_high = vreinterpretq_u32_f32(vld1q_f32(&b.components[4]));
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(veorq_u32(a_low, b_low)));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(veorq_u32(a_high, b_high)));
    return result;
}

static vl_simd_vec8_f32 vlSIMDNotVec8F32NEON(vl_simd_vec8_f32 a)
{
    vl_simd_vec8_f32 result;
    uint32x4_t a_low = vreinterpretq_u32_f32(vld1q_f32(&a.components[0]));
    uint32x4_t a_high = vreinterpretq_u32_f32(vld1q_f32(&a.components[4]));
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(vmvnq_u32(a_low)));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(vmvnq_u32(a_high)));
    return result;
}

/* ============================================================================
 * Initialization
 * ============================================================================
 */

void vlSIMDInitNEON(void)
{
    /* 4-wide F32 operations */
    vlSIMDFunctions.load_vec4f32 = vlSIMDLoadVec4F32NEON;
    vlSIMDFunctions.store_vec4f32 = vlSIMDStoreVec4F32NEON;
    vlSIMDFunctions.splat_vec4f32 = vlSIMDSplatVec4F32NEON;
    vlSIMDFunctions.add_vec4f32 = vlSIMDAddVec4F32NEON;
    vlSIMDFunctions.sub_vec4f32 = vlSIMDSubVec4F32NEON;
    vlSIMDFunctions.mul_vec4f32 = vlSIMDMulVec4F32NEON;
    vlSIMDFunctions.div_vec4f32 = vlSIMDDivVec4F32NEON;
    vlSIMDFunctions.fma_vec4f32 = vlSIMDFmaVec4F32NEON;
    vlSIMDFunctions.hsum_vec4f32 = vlSIMDHsumVec4F32NEON;
    vlSIMDFunctions.hmax_vec4f32 = vlSIMDHmaxVec4F32NEON;
    vlSIMDFunctions.hmin_vec4f32 = vlSIMDHminVec4F32NEON;
    vlSIMDFunctions.hprod_vec4f32 = vlSIMDHprodVec4F32NEON;
    vlSIMDFunctions.extract_lane_vec4f32 = vlSIMDExtractLaneVec4F32NEON;
    vlSIMDFunctions.broadcast_lane_vec4f32 = vlSIMDBroadcastLaneVec4F32NEON;

    /* 8-wide F32 operations */
    vlSIMDFunctions.load_vec8f32 = vlSIMDLoadVec8F32NEON;
    vlSIMDFunctions.store_vec8f32 = vlSIMDStoreVec8F32NEON;
    vlSIMDFunctions.add_vec8f32 = vlSIMDAddVec8F32NEON;
    vlSIMDFunctions.mul_vec8f32 = vlSIMDMulVec8F32NEON;
    vlSIMDFunctions.fma_vec8f32 = vlSIMDFmaVec8F32NEON;
    vlSIMDFunctions.splat_vec8f32 = vlSIMDSplatVec8F32NEON;
    vlSIMDFunctions.sub_vec8f32 = vlSIMDSubVec8F32NEON;
    vlSIMDFunctions.lt_vec8f32 = vlSIMDLtVec8F32NEON;
    vlSIMDFunctions.gt_vec8f32 = vlSIMDGtVec8F32NEON;
    vlSIMDFunctions.eq_vec8f32 = vlSIMDEqVec8F32NEON;
    vlSIMDFunctions.and_vec8f32 = vlSIMDAndVec8F32NEON;
    vlSIMDFunctions.or_vec8f32 = vlSIMDOrVec8F32NEON;
    vlSIMDFunctions.xor_vec8f32 = vlSIMDXorVec8F32NEON;
    vlSIMDFunctions.not_vec8f32 = vlSIMDNotVec8F32NEON;

    /* Comparison operations */
    vlSIMDFunctions.lt_vec4f32 = vlSIMDLtVec4F32NEON;
    vlSIMDFunctions.gt_vec4f32 = vlSIMDGtVec4F32NEON;
    vlSIMDFunctions.eq_vec4f32 = vlSIMDEqVec4F32NEON;

    /* Bitwise operations */
    vlSIMDFunctions.and_vec4f32 = vlSIMDAndVec4F32NEON;
    vlSIMDFunctions.or_vec4f32 = vlSIMDOrVec4F32NEON;
    vlSIMDFunctions.xor_vec4f32 = vlSIMDXorVec4F32NEON;
    vlSIMDFunctions.not_vec4f32 = vlSIMDNotVec4F32NEON;

    /* Integer operations */
    vlSIMDFunctions.load_vec4i32 = vlSIMDLoadVec4I32NEON;
    vlSIMDFunctions.store_vec4i32 = vlSIMDStoreVec4I32NEON;
    vlSIMDFunctions.add_vec4i32 = vlSIMDAddVec4I32NEON;
    vlSIMDFunctions.mul_vec4i32 = vlSIMDMulVec4I32NEON;

    vlSIMDFunctions.load_vec8i16 = vlSIMDLoadVec8I16NEON;
    vlSIMDFunctions.store_vec8i16 = vlSIMDStoreVec8I16NEON;
    vlSIMDFunctions.add_vec8i16 = vlSIMDAddVec8I16NEON;

    vlSIMDFunctions.load_vec32u8 = vlSIMDLoadVec32U8NEON;
    vlSIMDFunctions.store_vec32u8 = vlSIMDStoreVec32U8NEON;

    vlSIMDFunctions.backend_name = "NEON (ARMv7)";
}
