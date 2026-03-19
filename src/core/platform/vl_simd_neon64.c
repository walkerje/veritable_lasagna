/**
 * \file vl_simd_neon64.c
 * \brief SIMD implementation using ARM NEON (64-bit/ARMv8+)
 *
 * Provides optimized SIMD operations for ARMv8+ processors using NEON.
 * All operations are 128-bit wide (matching vl_simd_vec4_f32 layout).
 */

#include <arm_neon.h>
#include <string.h>
#include <vl/vl_simd.h>

/* ============================================================================
 * 4-Wide Float32 Operations
 * ============================================================================
 */

static vl_simd_vec4_f32 vlSIMDLoadVec4F32NEON64(const vl_float32_t* ptr)
{
    vl_simd_vec4_f32 result;
    float32x4_t v = vld1q_f32(ptr);
    vst1q_f32(result.components, v);
    return result;
}

static void vlSIMDStoreVec4F32NEON64(vl_float32_t* ptr, vl_simd_vec4_f32 v)
{
    float32x4_t nv = vld1q_f32(v.components);
    vst1q_f32(ptr, nv);
}

static vl_simd_vec4_f32 vlSIMDSplatVec4F32NEON64(vl_float32_t scalar)
{
    vl_simd_vec4_f32 result;
    float32x4_t v = vdupq_n_f32(scalar);
    vst1q_f32(result.components, v);
    return result;
}

static vl_simd_vec4_f32 vlSIMDAddVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    float32x4_t sum = vaddq_f32(va, vb);
    vst1q_f32(result.components, sum);
    return result;
}

static vl_simd_vec4_f32 vlSIMDSubVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    float32x4_t diff = vsubq_f32(va, vb);
    vst1q_f32(result.components, diff);
    return result;
}

static vl_simd_vec4_f32 vlSIMDMulVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    float32x4_t prod = vmulq_f32(va, vb);
    vst1q_f32(result.components, prod);
    return result;
}

static vl_simd_vec4_f32 vlSIMDDivVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    // NEON doesn't have direct division, use reciprocal approximation +
    // Newton-Raphson
    float32x4_t reciprocal = vrecpeq_f32(vb);
    reciprocal = vmulq_f32(reciprocal, vrecpsq_f32(vb, reciprocal));
    float32x4_t quot = vmulq_f32(va, reciprocal);
    vst1q_f32(result.components, quot);
    return result;
}

static vl_simd_vec4_f32 vlSIMDFmaVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b, vl_simd_vec4_f32 c)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    float32x4_t vc = vld1q_f32(c.components);
    float32x4_t fma = vfmaq_f32(vc, va, vb);
    vst1q_f32(result.components, fma);
    return result;
}

static vl_float32_t vlSIMDHsumVec4F32NEON64(vl_simd_vec4_f32 v)
{
    float32x4_t nv = vld1q_f32(v.components);
    float32x2_t sum_64 = vadd_f32(vget_high_f32(nv), vget_low_f32(nv));
    float32x2_t sum_32 = vpadd_f32(sum_64, sum_64);
    return vget_lane_f32(sum_32, 0);
}

static vl_float32_t vlSIMDHmaxVec4F32NEON64(vl_simd_vec4_f32 v)
{
    float32x4_t nv = vld1q_f32(v.components);
    float32x4_t max_64 = vpmax_f32(vget_high_f32(nv), vget_low_f32(nv));
    float32x4_t max_32 = vpmax_f32(max_64, max_64);
    return vget_lane_f32(max_32, 0);
}

static vl_float32_t vlSIMDHminVec4F32NEON64(vl_simd_vec4_f32 v)
{
    float32x4_t nv = vld1q_f32(v.components);
    float32x4_t min_64 = vpmin_f32(vget_high_f32(nv), vget_low_f32(nv));
    float32x4_t min_32 = vpmin_f32(min_64, min_64);
    return vget_lane_f32(min_32, 0);
}

static vl_float32_t vlSIMDHprodVec4F32NEON64(vl_simd_vec4_f32 v)
{
    vl_simd_vec4_f32 mul1 = vlSIMDMulVec4F32NEON64(v,
                                                   vlSIMDSplatVec4F32NEON64(1.0f) // Dummy, will be replaced
    );
    // Manual product: multiply all elements
    return v.components[0] * v.components[1] * v.components[2] * v.components[3];
}

static vl_float32_t vlSIMDExtractLaneVec4F32NEON64(vl_simd_vec4_f32 v, int lane) { return v.components[lane & 3]; }

static vl_simd_vec4_f32 vlSIMDBroadcastLaneVec4F32NEON64(vl_simd_vec4_f32 v, int lane)
{
    return vlSIMDSplatVec4F32NEON64(v.components[lane & 3]);
}

/* ============================================================================
 * Comparison Operations
 * ============================================================================
 */

static vl_simd_vec4_f32 vlSIMDLtVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    uint32x4_t cmp = vcltq_f32(va, vb);
    vst1q_f32(result.components, vreinterpretq_f32_u32(cmp));
    return result;
}

static vl_simd_vec4_f32 vlSIMDGtVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    uint32x4_t cmp = vcgtq_f32(va, vb);
    vst1q_f32(result.components, vreinterpretq_f32_u32(cmp));
    return result;
}

static vl_simd_vec4_f32 vlSIMDEqVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    float32x4_t va = vld1q_f32(a.components);
    float32x4_t vb = vld1q_f32(b.components);
    uint32x4_t cmp = vceqq_f32(va, vb);
    vst1q_f32(result.components, vreinterpretq_f32_u32(cmp));
    return result;
}

/* ============================================================================
 * Bitwise Operations
 * ============================================================================
 */

static vl_simd_vec4_f32 vlSIMDAndVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    uint32x4_t va = vld1q_u32((uint32_t*)a.components);
    uint32x4_t vb = vld1q_u32((uint32_t*)b.components);
    uint32x4_t res = vandq_u32(va, vb);
    vst1q_u32((uint32_t*)result.components, res);
    return result;
}

static vl_simd_vec4_f32 vlSIMDOrVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    uint32x4_t va = vld1q_u32((uint32_t*)a.components);
    uint32x4_t vb = vld1q_u32((uint32_t*)b.components);
    uint32x4_t res = vorrq_u32(va, vb);
    vst1q_u32((uint32_t*)result.components, res);
    return result;
}

static vl_simd_vec4_f32 vlSIMDXorVec4F32NEON64(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    uint32x4_t va = vld1q_u32((uint32_t*)a.components);
    uint32x4_t vb = vld1q_u32((uint32_t*)b.components);
    uint32x4_t res = veorq_u32(va, vb);
    vst1q_u32((uint32_t*)result.components, res);
    return result;
}

static vl_simd_vec4_f32 vlSIMDNotVec4F32NEON64(vl_simd_vec4_f32 a)
{
    vl_simd_vec4_f32 result;
    uint32x4_t va = vld1q_u32((uint32_t*)a.components);
    uint32x4_t res = vmvnq_u32(va);
    vst1q_u32((uint32_t*)result.components, res);
    return result;
}

/* ============================================================================
 * 8-Wide Float32 Operations (using two NEON registers)
 * ============================================================================
 */

static vl_simd_vec8_f32 vlSIMDLoadVec8F32NEON64(const vl_float32_t* ptr)
{
    vl_simd_vec8_f32 result;
    vst1q_f32(&result.components[0], vld1q_f32(&ptr[0]));
    vst1q_f32(&result.components[4], vld1q_f32(&ptr[4]));
    return result;
}

static void vlSIMDStoreVec8F32NEON64(vl_float32_t* ptr, vl_simd_vec8_f32 v)
{
    vst1q_f32(&ptr[0], vld1q_f32(&v.components[0]));
    vst1q_f32(&ptr[4], vld1q_f32(&v.components[4]));
}

static vl_simd_vec8_f32 vlSIMDAddVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
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

static vl_simd_vec8_f32 vlSIMDMulVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
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

static vl_simd_vec8_f32 vlSIMDFmaVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b, vl_simd_vec8_f32 c)
{
    vl_simd_vec8_f32 result;
    float32x4_t a_low = vld1q_f32(&a.components[0]);
    float32x4_t a_high = vld1q_f32(&a.components[4]);
    float32x4_t b_low = vld1q_f32(&b.components[0]);
    float32x4_t b_high = vld1q_f32(&b.components[4]);
    float32x4_t c_low = vld1q_f32(&c.components[0]);
    float32x4_t c_high = vld1q_f32(&c.components[4]);
    vst1q_f32(&result.components[0], vfmaq_f32(c_low, a_low, b_low));
    vst1q_f32(&result.components[4], vfmaq_f32(c_high, a_high, b_high));
    return result;
}

/* ============================================================================
 * Integer 32-bit Operations
 * ============================================================================
 */

static vl_simd_vec4_i32 vlSIMDLoadVec4I32NEON64(const vl_int32_t* ptr)
{
    vl_simd_vec4_i32 result;
    int32x4_t v = vld1q_s32(ptr);
    vst1q_s32(result.components, v);
    return result;
}

static void vlSIMDStoreVec4I32NEON64(vl_int32_t* ptr, vl_simd_vec4_i32 v)
{
    int32x4_t nv = vld1q_s32(v.components);
    vst1q_s32(ptr, nv);
}

static vl_simd_vec4_i32 vlSIMDAddVec4I32NEON64(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    vl_simd_vec4_i32 result;
    int32x4_t va = vld1q_s32(a.components);
    int32x4_t vb = vld1q_s32(b.components);
    int32x4_t sum = vaddq_s32(va, vb);
    vst1q_s32(result.components, sum);
    return result;
}

static vl_simd_vec4_i32 vlSIMDMulVec4I32NEON64(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    vl_simd_vec4_i32 result;
    int32x4_t va = vld1q_s32(a.components);
    int32x4_t vb = vld1q_s32(b.components);
    int32x4_t prod = vmulq_s32(va, vb);
    vst1q_s32(result.components, prod);
    return result;
}

/* ============================================================================
 * Integer 16-bit Operations
 * ============================================================================
 */

static vl_simd_vec8_i16 vlSIMDLoadVec8I16NEON64(const vl_int16_t* ptr)
{
    vl_simd_vec8_i16 result;
    int16x8_t v = vld1q_s16(ptr);
    vst1q_s16(result.components, v);
    return result;
}

static void vlSIMDStoreVec8I16NEON64(vl_int16_t* ptr, vl_simd_vec8_i16 v)
{
    int16x8_t nv = vld1q_s16(v.components);
    vst1q_s16(ptr, nv);
}

static vl_simd_vec8_i16 vlSIMDAddVec8I16NEON64(vl_simd_vec8_i16 a, vl_simd_vec8_i16 b)
{
    vl_simd_vec8_i16 result;
    int16x8_t va = vld1q_s16(a.components);
    int16x8_t vb = vld1q_s16(b.components);
    int16x8_t sum = vaddq_s16(va, vb);
    vst1q_s16(result.components, sum);
    return result;
}

/* ============================================================================
 * Integer 8-bit Operations
 * ============================================================================
 */

static vl_simd_vec32_u8 vlSIMDLoadVec32U8NEON64(const vl_uint8_t* ptr)
{
    vl_simd_vec32_u8 result;
    // Load two 128-bit registers (16 bytes each)
    vst1q_u8(&result.components[0], vld1q_u8(&ptr[0]));
    vst1q_u8(&result.components[16], vld1q_u8(&ptr[16]));
    return result;
}

static void vlSIMDStoreVec32U8NEON64(vl_uint8_t* ptr, vl_simd_vec32_u8 v)
{
    vst1q_u8(&ptr[0], vld1q_u8(&v.components[0]));
    vst1q_u8(&ptr[16], vld1q_u8(&v.components[16]));
}

/* ============================================================================
 * 8-Wide Float32 Comparison and Bitwise Operations
 * ============================================================================
 */

static vl_simd_vec8_f32 vlSIMDLtVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
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

static vl_simd_vec8_f32 vlSIMDGtVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
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

static vl_simd_vec8_f32 vlSIMDEqVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
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

static vl_simd_vec8_f32 vlSIMDAndVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    uint32x4_t a_low = vld1q_u32((uint32_t*)&a.components[0]);
    uint32x4_t a_high = vld1q_u32((uint32_t*)&a.components[4]);
    uint32x4_t b_low = vld1q_u32((uint32_t*)&b.components[0]);
    uint32x4_t b_high = vld1q_u32((uint32_t*)&b.components[4]);
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(vandq_u32(a_low, b_low)));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(vandq_u32(a_high, b_high)));
    return result;
}

static vl_simd_vec8_f32 vlSIMDOrVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    uint32x4_t a_low = vld1q_u32((uint32_t*)&a.components[0]);
    uint32x4_t a_high = vld1q_u32((uint32_t*)&a.components[4]);
    uint32x4_t b_low = vld1q_u32((uint32_t*)&b.components[0]);
    uint32x4_t b_high = vld1q_u32((uint32_t*)&b.components[4]);
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(vorrq_u32(a_low, b_low)));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(vorrq_u32(a_high, b_high)));
    return result;
}

static vl_simd_vec8_f32 vlSIMDXorVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    uint32x4_t a_low = vld1q_u32((uint32_t*)&a.components[0]);
    uint32x4_t a_high = vld1q_u32((uint32_t*)&a.components[4]);
    uint32x4_t b_low = vld1q_u32((uint32_t*)&b.components[0]);
    uint32x4_t b_high = vld1q_u32((uint32_t*)&b.components[4]);
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(veorq_u32(a_low, b_low)));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(veorq_u32(a_high, b_high)));
    return result;
}

static vl_simd_vec8_f32 vlSIMDNotVec8F32NEON64(vl_simd_vec8_f32 a)
{
    vl_simd_vec8_f32 result;
    uint32x4_t a_low = vld1q_u32((uint32_t*)&a.components[0]);
    uint32x4_t a_high = vld1q_u32((uint32_t*)&a.components[4]);
    vst1q_f32(&result.components[0], vreinterpretq_f32_u32(vmvnq_u32(a_low)));
    vst1q_f32(&result.components[4], vreinterpretq_f32_u32(vmvnq_u32(a_high)));
    return result;
}

static vl_simd_vec8_f32 vlSIMDSplatVec8F32NEON64(vl_float32_t scalar)
{
    vl_simd_vec8_f32 result;
    float32x4_t v = vdupq_n_f32(scalar);
    vst1q_f32(&result.components[0], v);
    vst1q_f32(&result.components[4], v);
    return result;
}

static vl_simd_vec8_f32 vlSIMDSubVec8F32NEON64(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
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

/* ============================================================================
 * Initialization
 * ============================================================================
 */

void vlSIMDInitNEON64(void)
{
    /* 4-wide float operations */
    vlSIMDFunctions.load_vec4f32 = vlSIMDLoadVec4F32NEON64;
    vlSIMDFunctions.store_vec4f32 = vlSIMDStoreVec4F32NEON64;
    vlSIMDFunctions.splat_vec4f32 = vlSIMDSplatVec4F32NEON64;
    vlSIMDFunctions.add_vec4f32 = vlSIMDAddVec4F32NEON64;
    vlSIMDFunctions.sub_vec4f32 = vlSIMDSubVec4F32NEON64;
    vlSIMDFunctions.mul_vec4f32 = vlSIMDMulVec4F32NEON64;
    vlSIMDFunctions.div_vec4f32 = vlSIMDDivVec4F32NEON64;
    vlSIMDFunctions.fma_vec4f32 = vlSIMDFmaVec4F32NEON64;
    vlSIMDFunctions.hsum_vec4f32 = vlSIMDHsumVec4F32NEON64;
    vlSIMDFunctions.hmax_vec4f32 = vlSIMDHmaxVec4F32NEON64;
    vlSIMDFunctions.hmin_vec4f32 = vlSIMDHminVec4F32NEON64;
    vlSIMDFunctions.hprod_vec4f32 = vlSIMDHprodVec4F32NEON64;
    vlSIMDFunctions.extract_lane_vec4f32 = vlSIMDExtractLaneVec4F32NEON64;
    vlSIMDFunctions.broadcast_lane_vec4f32 = vlSIMDBroadcastLaneVec4F32NEON64;

    /* 8-wide float operations */
    vlSIMDFunctions.load_vec8f32 = vlSIMDLoadVec8F32NEON64;
    vlSIMDFunctions.store_vec8f32 = vlSIMDStoreVec8F32NEON64;
    vlSIMDFunctions.add_vec8f32 = vlSIMDAddVec8F32NEON64;
    vlSIMDFunctions.mul_vec8f32 = vlSIMDMulVec8F32NEON64;
    vlSIMDFunctions.fma_vec8f32 = vlSIMDFmaVec8F32NEON64;
    vlSIMDFunctions.splat_vec8f32 = vlSIMDSplatVec8F32NEON64;
    vlSIMDFunctions.sub_vec8f32 = vlSIMDSubVec8F32NEON64;
    vlSIMDFunctions.lt_vec8f32 = vlSIMDLtVec8F32NEON64;
    vlSIMDFunctions.gt_vec8f32 = vlSIMDGtVec8F32NEON64;
    vlSIMDFunctions.eq_vec8f32 = vlSIMDEqVec8F32NEON64;
    vlSIMDFunctions.and_vec8f32 = vlSIMDAndVec8F32NEON64;
    vlSIMDFunctions.or_vec8f32 = vlSIMDOrVec8F32NEON64;
    vlSIMDFunctions.xor_vec8f32 = vlSIMDXorVec8F32NEON64;
    vlSIMDFunctions.not_vec8f32 = vlSIMDNotVec8F32NEON64;

    /* Comparison operations */
    vlSIMDFunctions.lt_vec4f32 = vlSIMDLtVec4F32NEON64;
    vlSIMDFunctions.gt_vec4f32 = vlSIMDGtVec4F32NEON64;
    vlSIMDFunctions.eq_vec4f32 = vlSIMDEqVec4F32NEON64;

    /* Bitwise operations */
    vlSIMDFunctions.and_vec4f32 = vlSIMDAndVec4F32NEON64;
    vlSIMDFunctions.or_vec4f32 = vlSIMDOrVec4F32NEON64;
    vlSIMDFunctions.xor_vec4f32 = vlSIMDXorVec4F32NEON64;
    vlSIMDFunctions.not_vec4f32 = vlSIMDNotVec4F32NEON64;

    /* Integer 32-bit operations */
    vlSIMDFunctions.load_vec4i32 = vlSIMDLoadVec4I32NEON64;
    vlSIMDFunctions.store_vec4i32 = vlSIMDStoreVec4I32NEON64;
    vlSIMDFunctions.add_vec4i32 = vlSIMDAddVec4I32NEON64;
    vlSIMDFunctions.mul_vec4i32 = vlSIMDMulVec4I32NEON64;

    /* Integer 16-bit operations */
    vlSIMDFunctions.load_vec8i16 = vlSIMDLoadVec8I16NEON64;
    vlSIMDFunctions.store_vec8i16 = vlSIMDStoreVec8I16NEON64;
    vlSIMDFunctions.add_vec8i16 = vlSIMDAddVec8I16NEON64;

    /* Integer 8-bit operations */
    vlSIMDFunctions.load_vec32u8 = vlSIMDLoadVec32U8NEON64;
    vlSIMDFunctions.store_vec32u8 = vlSIMDStoreVec32U8NEON64;

    vlSIMDFunctions.backend_name = "NEON64";
}
