/**
 * \file vl_simd_portable.c
 * \brief Portable C SIMD implementations (fallback).
 */

#include <string.h>
#include <vl/vl_simd.h>

static vl_simd_vec4_f32 vlSIMDLoadVec4F32Portable(const vl_float32_t* ptr)
{
    vl_simd_vec4_f32 result;
    memcpy(result.components, ptr, sizeof(result.components));
    return result;
}

static void vlSIMDStoreVec4F32Portable(vl_float32_t* ptr, vl_simd_vec4_f32 v)
{
    memcpy(ptr, v.components, sizeof(v.components));
}

static vl_simd_vec4_f32 vlSIMDSplatVec4F32Portable(vl_float32_t scalar)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
        result.components[i] = scalar;
    return result;
}

static vl_simd_vec4_f32 vlSIMDAddVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
        result.components[i] = a.components[i] + b.components[i];
    return result;
}

static vl_simd_vec4_f32 vlSIMDSubVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
        result.components[i] = a.components[i] - b.components[i];
    return result;
}

static vl_simd_vec4_f32 vlSIMDMulVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
        result.components[i] = a.components[i] * b.components[i];
    return result;
}

static vl_simd_vec4_f32 vlSIMDDivVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
        result.components[i] = a.components[i] / b.components[i];
    return result;
}

static vl_simd_vec4_f32 vlSIMDFmaVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b, vl_simd_vec4_f32 c)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
        result.components[i] = a.components[i] * b.components[i] + c.components[i];
    return result;
}

static vl_float32_t vlSIMDHsumVec4F32Portable(vl_simd_vec4_f32 v)
{
    return v.components[0] + v.components[1] + v.components[2] + v.components[3];
}

static vl_simd_vec8_f32 vlSIMDLoadVec8F32Portable(const vl_float32_t* ptr)
{
    vl_simd_vec8_f32 result;
    memcpy(result.components, ptr, sizeof(result.components));
    return result;
}

static void vlSIMDStoreVec8F32Portable(vl_float32_t* ptr, vl_simd_vec8_f32 v)
{
    memcpy(ptr, v.components, sizeof(v.components));
}

static vl_simd_vec8_f32 vlSIMDAddVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
        result.components[i] = a.components[i] + b.components[i];
    return result;
}

static vl_simd_vec8_f32 vlSIMDMulVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
        result.components[i] = a.components[i] * b.components[i];
    return result;
}

static vl_simd_vec8_f32 vlSIMDFmaVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b, vl_simd_vec8_f32 c)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
        result.components[i] = a.components[i] * b.components[i] + c.components[i];
    return result;
}

static vl_simd_vec4_f32 vlSIMDCmpPortable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b,
                                          int (*cmp_fn)(vl_float32_t, vl_float32_t))
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
    {
        vl_uint32_t mask = cmp_fn(a.components[i], b.components[i]) ? 0xFFFFFFFFu : 0u;
        memcpy(&result.components[i], &mask, sizeof(vl_float32_t));
    }
    return result;
}

static int vlSIMDLtCmp(vl_float32_t a, vl_float32_t b) { return a < b; }
static int vlSIMDGtCmp(vl_float32_t a, vl_float32_t b) { return a > b; }
static int vlSIMDEqCmp(vl_float32_t a, vl_float32_t b) { return a == b; }

static vl_simd_vec4_f32 vlSIMDLtVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDCmpPortable(a, b, vlSIMDLtCmp);
}

static vl_simd_vec4_f32 vlSIMDGtVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDCmpPortable(a, b, vlSIMDGtCmp);
}

static vl_simd_vec4_f32 vlSIMDEqVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    return vlSIMDCmpPortable(a, b, vlSIMDEqCmp);
}

static vl_simd_vec4_f32 vlSIMDAndVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
    {
        vl_uint32_t ua, ub, ur;
        memcpy(&ua, &a.components[i], sizeof(ua));
        memcpy(&ub, &b.components[i], sizeof(ub));
        ur = ua & ub;
        memcpy(&result.components[i], &ur, sizeof(result.components[i]));
    }
    return result;
}

static vl_simd_vec4_f32 vlSIMDOrVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
    {
        vl_uint32_t ua, ub, ur;
        memcpy(&ua, &a.components[i], sizeof(ua));
        memcpy(&ub, &b.components[i], sizeof(ub));
        ur = ua | ub;
        memcpy(&result.components[i], &ur, sizeof(result.components[i]));
    }
    return result;
}

static vl_simd_vec4_f32 vlSIMDXorVec4F32Portable(vl_simd_vec4_f32 a, vl_simd_vec4_f32 b)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
    {
        vl_uint32_t ua, ub, ur;
        memcpy(&ua, &a.components[i], sizeof(ua));
        memcpy(&ub, &b.components[i], sizeof(ub));
        ur = ua ^ ub;
        memcpy(&result.components[i], &ur, sizeof(result.components[i]));
    }
    return result;
}

static vl_simd_vec4_f32 vlSIMDNotVec4F32Portable(vl_simd_vec4_f32 a)
{
    vl_simd_vec4_f32 result;
    for (int i = 0; i < 4; i++)
    {
        vl_uint32_t ua, ur;
        memcpy(&ua, &a.components[i], sizeof(ua));
        ur = ~ua;
        memcpy(&result.components[i], &ur, sizeof(result.components[i]));
    }
    return result;
}

/* Horizontal reductions */
static vl_float32_t vlSIMDHmaxVec4F32Portable(vl_simd_vec4_f32 v)
{
    vl_float32_t max_val = v.components[0];
    for (int i = 1; i < 4; i++)
    {
        if (v.components[i] > max_val)
            max_val = v.components[i];
    }
    return max_val;
}

static vl_float32_t vlSIMDHminVec4F32Portable(vl_simd_vec4_f32 v)
{
    vl_float32_t min_val = v.components[0];
    for (int i = 1; i < 4; i++)
    {
        if (v.components[i] < min_val)
            min_val = v.components[i];
    }
    return min_val;
}

static vl_float32_t vlSIMDHprodVec4F32Portable(vl_simd_vec4_f32 v)
{
    return v.components[0] * v.components[1] * v.components[2] * v.components[3];
}

/* Lane operations */
static vl_float32_t vlSIMDExtractLaneVec4F32Portable(vl_simd_vec4_f32 v, int lane)
{
    return v.components[lane & 3]; // Clamp to 0-3
}

static vl_simd_vec4_f32 vlSIMDBroadcastLaneVec4F32Portable(vl_simd_vec4_f32 v, int lane)
{
    vl_float32_t val = v.components[lane & 3];
    return vlSIMDSplatVec4F32Portable(val);
}

/* Integer 32-bit */
static vl_simd_vec4_i32 vlSIMDLoadVec4I32Portable(const vl_int32_t* ptr)
{
    vl_simd_vec4_i32 result;
    result.components[0] = ptr[0];
    result.components[1] = ptr[1];
    result.components[2] = ptr[2];
    result.components[3] = ptr[3];
    return result;
}

static void vlSIMDStoreVec4I32Portable(vl_int32_t* ptr, vl_simd_vec4_i32 v)
{
    ptr[0] = v.components[0];
    ptr[1] = v.components[1];
    ptr[2] = v.components[2];
    ptr[3] = v.components[3];
}

static vl_simd_vec4_i32 vlSIMDAddVec4I32Portable(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    vl_simd_vec4_i32 result;
    for (int i = 0; i < 4; i++)
    {
        result.components[i] = a.components[i] + b.components[i];
    }
    return result;
}

static vl_simd_vec4_i32 vlSIMDMulVec4I32Portable(vl_simd_vec4_i32 a, vl_simd_vec4_i32 b)
{
    vl_simd_vec4_i32 result;
    for (int i = 0; i < 4; i++)
    {
        result.components[i] = a.components[i] * b.components[i];
    }
    return result;
}

static vl_simd_vec8_f32 vlSIMDSplatVec8F32Portable(vl_float32_t scalar)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
        result.components[i] = scalar;
    return result;
}

static vl_simd_vec8_f32 vlSIMDSubVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
        result.components[i] = a.components[i] - b.components[i];
    return result;
}

static vl_simd_vec8_f32 vlSIMDLtVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
    {
        vl_uint32_t mask = a.components[i] < b.components[i] ? 0xFFFFFFFFu : 0u;
        memcpy(&result.components[i], &mask, sizeof(vl_float32_t));
    }
    return result;
}

static vl_simd_vec8_f32 vlSIMDGtVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
    {
        vl_uint32_t mask = a.components[i] > b.components[i] ? 0xFFFFFFFFu : 0u;
        memcpy(&result.components[i], &mask, sizeof(vl_float32_t));
    }
    return result;
}

static vl_simd_vec8_f32 vlSIMDEqVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
    {
        vl_uint32_t mask = a.components[i] == b.components[i] ? 0xFFFFFFFFu : 0u;
        memcpy(&result.components[i], &mask, sizeof(vl_float32_t));
    }
    return result;
}

static vl_simd_vec8_f32 vlSIMDAndVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
    {
        vl_uint32_t ua, ub, ur;
        memcpy(&ua, &a.components[i], sizeof(ua));
        memcpy(&ub, &b.components[i], sizeof(ub));
        ur = ua & ub;
        memcpy(&result.components[i], &ur, sizeof(result.components[i]));
    }
    return result;
}

static vl_simd_vec8_f32 vlSIMDOrVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
    {
        vl_uint32_t ua, ub, ur;
        memcpy(&ua, &a.components[i], sizeof(ua));
        memcpy(&ub, &b.components[i], sizeof(ub));
        ur = ua | ub;
        memcpy(&result.components[i], &ur, sizeof(result.components[i]));
    }
    return result;
}

static vl_simd_vec8_f32 vlSIMDXorVec8F32Portable(vl_simd_vec8_f32 a, vl_simd_vec8_f32 b)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
    {
        vl_uint32_t ua, ub, ur;
        memcpy(&ua, &a.components[i], sizeof(ua));
        memcpy(&ub, &b.components[i], sizeof(ub));
        ur = ua ^ ub;
        memcpy(&result.components[i], &ur, sizeof(result.components[i]));
    }
    return result;
}

static vl_simd_vec8_f32 vlSIMDNotVec8F32Portable(vl_simd_vec8_f32 a)
{
    vl_simd_vec8_f32 result;
    for (int i = 0; i < 8; i++)
    {
        vl_uint32_t ua, ur;
        memcpy(&ua, &a.components[i], sizeof(ua));
        ur = ~ua;
        memcpy(&result.components[i], &ur, sizeof(result.components[i]));
    }
    return result;
}

static vl_simd_vec8_i16 vlSIMDLoadVec8I16Portable(const vl_int16_t* ptr)
{
    vl_simd_vec8_i16 result;
    for (int i = 0; i < 8; i++)
    {
        result.components[i] = ptr[i];
    }
    return result;
}

static void vlSIMDStoreVec8I16Portable(vl_int16_t* ptr, vl_simd_vec8_i16 v)
{
    for (int i = 0; i < 8; i++)
    {
        ptr[i] = v.components[i];
    }
}

static vl_simd_vec8_i16 vlSIMDAddVec8I16Portable(vl_simd_vec8_i16 a, vl_simd_vec8_i16 b)
{
    vl_simd_vec8_i16 result;
    for (int i = 0; i < 8; i++)
    {
        result.components[i] = a.components[i] + b.components[i];
    }
    return result;
}

/* Integer 8-bit */
static vl_simd_vec32_u8 vlSIMDLoadVec32U8Portable(const vl_uint8_t* ptr)
{
    vl_simd_vec32_u8 result;
    for (int i = 0; i < 32; i++)
    {
        result.components[i] = ptr[i];
    }
    return result;
}

static void vlSIMDStoreVec32U8Portable(vl_uint8_t* ptr, vl_simd_vec32_u8 v)
{
    for (int i = 0; i < 32; i++)
    {
        ptr[i] = v.components[i];
    }
}

static void vlSIMDInitPortable(void)
{
    vlSIMDFunctions.load_vec4f32 = vlSIMDLoadVec4F32Portable;
    vlSIMDFunctions.store_vec4f32 = vlSIMDStoreVec4F32Portable;
    vlSIMDFunctions.splat_vec4f32 = vlSIMDSplatVec4F32Portable;
    vlSIMDFunctions.add_vec4f32 = vlSIMDAddVec4F32Portable;
    vlSIMDFunctions.sub_vec4f32 = vlSIMDSubVec4F32Portable;
    vlSIMDFunctions.mul_vec4f32 = vlSIMDMulVec4F32Portable;
    vlSIMDFunctions.div_vec4f32 = vlSIMDDivVec4F32Portable;
    vlSIMDFunctions.fma_vec4f32 = vlSIMDFmaVec4F32Portable;
    vlSIMDFunctions.hsum_vec4f32 = vlSIMDHsumVec4F32Portable;
    vlSIMDFunctions.load_vec8f32 = vlSIMDLoadVec8F32Portable;
    vlSIMDFunctions.store_vec8f32 = vlSIMDStoreVec8F32Portable;
    vlSIMDFunctions.add_vec8f32 = vlSIMDAddVec8F32Portable;
    vlSIMDFunctions.mul_vec8f32 = vlSIMDMulVec8F32Portable;
    vlSIMDFunctions.fma_vec8f32 = vlSIMDFmaVec8F32Portable;
    vlSIMDFunctions.splat_vec8f32 = vlSIMDSplatVec8F32Portable;
    vlSIMDFunctions.sub_vec8f32 = vlSIMDSubVec8F32Portable;
    vlSIMDFunctions.lt_vec8f32 = vlSIMDLtVec8F32Portable;
    vlSIMDFunctions.gt_vec8f32 = vlSIMDGtVec8F32Portable;
    vlSIMDFunctions.eq_vec8f32 = vlSIMDEqVec8F32Portable;
    vlSIMDFunctions.and_vec8f32 = vlSIMDAndVec8F32Portable;
    vlSIMDFunctions.or_vec8f32 = vlSIMDOrVec8F32Portable;
    vlSIMDFunctions.xor_vec8f32 = vlSIMDXorVec8F32Portable;
    vlSIMDFunctions.not_vec8f32 = vlSIMDNotVec8F32Portable;
    vlSIMDFunctions.lt_vec4f32 = vlSIMDLtVec4F32Portable;
    vlSIMDFunctions.gt_vec4f32 = vlSIMDGtVec4F32Portable;
    vlSIMDFunctions.eq_vec4f32 = vlSIMDEqVec4F32Portable;
    vlSIMDFunctions.and_vec4f32 = vlSIMDAndVec4F32Portable;
    vlSIMDFunctions.or_vec4f32 = vlSIMDOrVec4F32Portable;
    vlSIMDFunctions.xor_vec4f32 = vlSIMDXorVec4F32Portable;
    vlSIMDFunctions.not_vec4f32 = vlSIMDNotVec4F32Portable;
    vlSIMDFunctions.hmax_vec4f32 = vlSIMDHmaxVec4F32Portable;
    vlSIMDFunctions.hmin_vec4f32 = vlSIMDHminVec4F32Portable;
    vlSIMDFunctions.hprod_vec4f32 = vlSIMDHprodVec4F32Portable;
    vlSIMDFunctions.extract_lane_vec4f32 = vlSIMDExtractLaneVec4F32Portable;
    vlSIMDFunctions.broadcast_lane_vec4f32 = vlSIMDBroadcastLaneVec4F32Portable;
    vlSIMDFunctions.load_vec4i32 = vlSIMDLoadVec4I32Portable;
    vlSIMDFunctions.store_vec4i32 = vlSIMDStoreVec4I32Portable;
    vlSIMDFunctions.add_vec4i32 = vlSIMDAddVec4I32Portable;
    vlSIMDFunctions.mul_vec4i32 = vlSIMDMulVec4I32Portable;
    vlSIMDFunctions.load_vec8i16 = vlSIMDLoadVec8I16Portable;
    vlSIMDFunctions.store_vec8i16 = vlSIMDStoreVec8I16Portable;
    vlSIMDFunctions.add_vec8i16 = vlSIMDAddVec8I16Portable;
    vlSIMDFunctions.load_vec32u8 = vlSIMDLoadVec32U8Portable;
    vlSIMDFunctions.store_vec32u8 = vlSIMDStoreVec32U8Portable;
    vlSIMDFunctions.backend_name = "Portable C";
}
