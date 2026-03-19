#include "msgpack.h"
#include <vl/vl_msgpack.h>
#include <vl/vl_msgpack_io.h>
#include <vl/vl_rand.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

static void vl_MsgPackEncodeTestMessage(vl_msgpack_encoder *enc) {
    vlMsgPackIOEncodeMapBegin(enc);
    {
        vlMsgPackIOEncodeString(enc, "int");
        vlMsgPackIOEncodeInt(enc, 1);

        vlMsgPackIOEncodeString(enc, "float");
        vlMsgPackIOEncodeFloat64(enc, 0.5f);

        vlMsgPackIOEncodeString(enc, "boolean");
        vlMsgPackIOEncodeBool(enc, VL_TRUE);

        vlMsgPackIOEncodeString(enc, "null");
        vlMsgPackIOEncodeNil(enc);

        vlMsgPackIOEncodeString(enc, "string");
        vlMsgPackIOEncodeString(enc, "foo bar");

        vlMsgPackIOEncodeString(enc, "array");
        vlMsgPackIOEncodeArrayBegin(enc);
        {
            vlMsgPackIOEncodeString(enc, "foo");
            vlMsgPackIOEncodeString(enc, "bar");
        }
        vlMsgPackIOEncodeArrayEnd(enc);

        vlMsgPackIOEncodeString(enc, "object");

        vlMsgPackIOEncodeMapBegin(enc);
        {
            vlMsgPackIOEncodeString(enc, "foo");
            vlMsgPackIOEncodeInt(enc, 1);

            vlMsgPackIOEncodeString(enc, "bar");
            vlMsgPackIOEncodeFloat32(enc, 0.5f);
        }
        vlMsgPackIOEncodeMapEnd(enc);
    }
    vlMsgPackIOEncodeMapEnd(enc);
}

static void vl_MsgPackPrint(vl_msgpack *pack, vl_msgpack_iter iter, vl_dsidx_t depth) {
    (void)pack; (void)iter; (void)depth;
    /* Print logic removed for tests to keep output clean, or could be kept if needed for debugging */
}

vl_bool_t vlTestMsgPackPartialEncode() {
    float expectedSum = 0.0f;
    float finalSum = -1.0f;

    vl_msgpack_encoder *enc = vlMsgPackIOEncoderNew();
    {
        vl_rand rand = vlRandInit();
        vlMsgPackIOEncodeArrayBegin(enc);
        {
            for (int i = 0; i < 32; i++) {
                float randValue = vlRandF(&rand) * 10;
                expectedSum += randValue;
                vlMsgPackIOEncodeFloat32(enc, randValue);
            }
        }
        vlMsgPackIOEncodeArrayEnd(enc);
    }
    vl_msgpack *pack = vlMsgPackNew();

    vl_msgpack_decoder dec;
    vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.size);

    const char *arrayKey = "partial-encode";
    const vl_msgpack_iter arrayIter = vlMsgPackIODecodeToDOM(&dec, pack, vlMsgPackRoot(pack), arrayKey,
                                                             (vl_memsize_t)strlen(arrayKey));

    if (arrayIter != VL_MSGPACK_ITER_INVALID) {
        finalSum = 0.0f;
        VL_MSGPACK_FOREACH_CHILD(pack, arrayIter, curIter)finalSum += vlMsgPackGetFloat32(pack, curIter, 0.0f);
    }

    vlMsgPackIOEncoderDelete(enc);
    vlMsgPackDelete(pack);

    return expectedSum == finalSum;
}

vl_bool_t vlTestMsgPackRoundTrip() {
    vl_msgpack_encoder *enc = vlMsgPackIOEncoderNew();
    vl_MsgPackEncodeTestMessage(enc);

    if (enc->error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vl_msgpack *pack = vlMsgPackNew();
    vl_msgpack_decoder dec;
    vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.offset);

    const char *encKey = "round-trip";
    vl_msgpack_iter decodeIter = vlMsgPackIODecodeToDOM(&dec, pack, vlMsgPackRoot(pack), encKey, (vl_memsize_t)strlen(encKey));

    if (dec.error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        vlMsgPackDelete(pack);
        return VL_FALSE;
    }

    vl_MsgPackPrint(pack, decodeIter, 0);

    vl_msgpack_encoder *secondEnc = vlMsgPackIOEncoderNew();
    vlMsgPackIOEncodeFromDOM(secondEnc, pack, decodeIter);

    if (secondEnc->error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        vlMsgPackIOEncoderDelete(secondEnc);
        vlMsgPackDelete(pack);
        return VL_FALSE;
    }

    vl_bool_t result = enc->buffer.offset == secondEnc->buffer.offset;
    if (result)
        result = (memcmp(enc->buffer.data, secondEnc->buffer.data, enc->buffer.offset) == 0);

    vlMsgPackIOEncoderDelete(enc);
    vlMsgPackIOEncoderDelete(secondEnc);
    vlMsgPackDelete(pack);

    return result;
}

vl_bool_t vlTestMsgPackNegativeIntegers() {
    vl_msgpack_encoder *enc = vlMsgPackIOEncoderNew();

    vlMsgPackIOEncodeInt(enc, -1);
    vlMsgPackIOEncodeInt(enc, -32);
    vlMsgPackIOEncodeInt(enc, -33);
    vlMsgPackIOEncodeInt(enc, -128);
    vlMsgPackIOEncodeInt(enc, -129);
    vlMsgPackIOEncodeInt(enc, -32768);
    vlMsgPackIOEncodeInt(enc, -32769);
    vlMsgPackIOEncodeInt(enc, INT64_MIN);

    if (enc->error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vl_msgpack_decoder dec;
    vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.offset);

    vl_msgpack_io_token tk;
    vl_ilarge_t expected[] = {-1, -32, -33, -128, -129, -32768, -32769, INT64_MIN};

    for (int i = 0; i < 8; i++) {
        if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_INT || tk.integer.value != expected[i]) {
            vlMsgPackIOEncoderDelete(enc);
            return VL_FALSE;
        }
    }

    vlMsgPackIOEncoderDelete(enc);
    return VL_TRUE;
}

vl_bool_t vlTestMsgPackPositiveIntegers() {
    vl_msgpack_encoder *enc = vlMsgPackIOEncoderNew();

    vlMsgPackIOEncodeInt(enc, 0);
    vlMsgPackIOEncodeInt(enc, 127);
    vlMsgPackIOEncodeUInt(enc, 128);
    vlMsgPackIOEncodeUInt(enc, 255);
    vlMsgPackIOEncodeUInt(enc, 256);
    vlMsgPackIOEncodeUInt(enc, 65535);
    vlMsgPackIOEncodeUInt(enc, 65536);
    vlMsgPackIOEncodeUInt(enc, UINT64_MAX);

    if (enc->error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vl_msgpack_decoder dec;
    vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.offset);

    vl_msgpack_io_token tk;
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_INT || tk.integer.value != 0) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_INT || tk.integer.value != 127) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_UINT || tk.uinteger.value != 128) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_UINT || tk.uinteger.value != 255) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_UINT || tk.uinteger.value != 256) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_UINT || tk.uinteger.value != 65535) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_UINT || tk.uinteger.value != 65536) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_UINT || tk.uinteger.value != UINT64_MAX) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }

    vlMsgPackIOEncoderDelete(enc);
    return VL_TRUE;
}

vl_bool_t vlTestMsgPackStringBoundaries() {
    vl_msgpack_encoder *enc = vlMsgPackIOEncoderNew();

    vlMsgPackIOEncodeStringLen(enc, "", 0);
    vlMsgPackIOEncodeStringLen(enc, "12345678901234567890123456789012", 31);

    char str32[32];
    memset(str32, 'a', 32);
    vlMsgPackIOEncodeStringLen(enc, str32, 32);

    char str255[255];
    memset(str255, 'b', 255);
    vlMsgPackIOEncodeStringLen(enc, str255, 255);

    char str256[256];
    memset(str256, 'c', 256);
    vlMsgPackIOEncodeStringLen(enc, str256, 256);

    if (enc->error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vl_msgpack_decoder dec;
    vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.offset);

    vl_msgpack_io_token tk;
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_STRING || tk.string.length != 0) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_STRING || tk.string.length != 31) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_STRING || tk.string.length != 32) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_STRING || tk.string.length != 255) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_STRING || tk.string.length != 256) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }

    vlMsgPackIOEncoderDelete(enc);
    return VL_TRUE;
}

vl_bool_t vlTestMsgPackFloatPrecision() {
    vl_msgpack_encoder *enc = vlMsgPackIOEncoderNew();

    vl_float32_t f32_vals[] = {0.0f, -0.0f, 3.14159f, 1e-10f, 1e10f, -123.456f};
    for (int i = 0; i < 6; i++) {
        vlMsgPackIOEncodeFloat32(enc, f32_vals[i]);
    }

    vl_float64_t f64_vals[] = {0.0, -0.0, 3.14159265358979, 1e-100, 1e100, -123.456789012345};
    for (int i = 0; i < 6; i++) {
        vlMsgPackIOEncodeFloat64(enc, f64_vals[i]);
    }

    if (enc->error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vl_msgpack_decoder dec;
    vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.offset);

    vl_msgpack_io_token tk;
    for (int i = 0; i < 6; i++) {
        vlMsgPackIODecoderNext(&dec, &tk);
        if (tk.type != VL_MSGPACK_FLOAT32 || tk.float32.value != f32_vals[i]) {
            vlMsgPackIOEncoderDelete(enc);
            return VL_FALSE;
        }
    }

    for (int i = 0; i < 6; i++) {
        vlMsgPackIODecoderNext(&dec, &tk);
        if (tk.type != VL_MSGPACK_FLOAT64 || tk.float64.value != f64_vals[i]) {
            vlMsgPackIOEncoderDelete(enc);
            return VL_FALSE;
        }
    }

    vlMsgPackIOEncoderDelete(enc);
    return VL_TRUE;
}

vl_bool_t vlTestMsgPackBinaryAndExt() {
    vl_msgpack_encoder *enc = vlMsgPackIOEncoderNew();

    vl_uint8_t bindata[] = {0xDE, 0xAD, 0xBE, 0xEF};
    vlMsgPackIOEncodeBinary(enc, bindata, 4);

    vl_uint8_t extdata[] = {0x01, 0x02, 0x03};
    vlMsgPackIOEncodeExt(enc, 42, extdata, 3);

    vlMsgPackIOEncodeExt(enc, -1, NULL, 0);

    if (enc->error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vl_msgpack_decoder dec;
    vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.offset);

    vl_msgpack_io_token tk;

    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_BINARY || tk.binary.length != 4 || memcmp(tk.binary.ptr, bindata, 4) != 0) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_EXT || tk.ext.extType != 42 || tk.ext.length != 4 || memcmp(tk.ext.ptr, extdata, 3) != 0) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_EXT || tk.ext.extType != -1 || tk.ext.length != 1) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vlMsgPackIOEncoderDelete(enc);
    return VL_TRUE;
}

vl_bool_t vlTestMsgPackEncoderErrors() {
    vl_msgpack_encoder *enc1 = vlMsgPackIOEncoderNew();
    vlMsgPackIOEncodeMapBegin(enc1);
    vlMsgPackIOEncodeString(enc1, "key");
    vlMsgPackIOEncodeInt(enc1, 42);
    vlMsgPackIOEncodeString(enc1, "orphan");
    vlMsgPackIOEncodeMapEnd(enc1);

    if (enc1->error != VL_MSGPACK_IO_ERR_UNBALANCED_MAP) {
        vlMsgPackIOEncoderDelete(enc1);
        return VL_FALSE;
    }
    vlMsgPackIOEncoderDelete(enc1);

    vl_msgpack_encoder *enc2 = vlMsgPackIOEncoderNew();
    vlMsgPackIOEncodeMapEnd(enc2);
    if (enc2->error != VL_MSGPACK_IO_ERR_STACK_UNDERFLOW) {
        vlMsgPackIOEncoderDelete(enc2);
        return VL_FALSE;
    }
    vlMsgPackIOEncoderDelete(enc2);

    vl_msgpack_encoder *enc3 = vlMsgPackIOEncoderNew();
    vlMsgPackIOEncodeArrayEnd(enc3);
    if (enc3->error != VL_MSGPACK_IO_ERR_STACK_UNDERFLOW) {
        vlMsgPackIOEncoderDelete(enc3);
        return VL_FALSE;
    }
    vlMsgPackIOEncoderDelete(enc3);

    return VL_TRUE;
}

vl_bool_t vlTestMsgPackDecoderEOF() {
    vl_uint8_t truncated_int64[] = {0xD3, 0x00, 0x00, 0x00};
    vl_msgpack_decoder dec1;
    vlMsgPackIODecoderStart(&dec1, truncated_int64, sizeof(truncated_int64));

    vl_msgpack_io_token tk;
    if (vlMsgPackIODecoderNext(&dec1, &tk) || dec1.error != VL_MSGPACK_IO_ERR_UNEXPECTED_EOF) {
        return VL_FALSE;
    }

    vl_uint8_t truncated_str[] = {0xDA, 0x00, 0x10};
    vl_msgpack_decoder dec2;
    vlMsgPackIODecoderStart(&dec2, truncated_str, sizeof(truncated_str));
    if (vlMsgPackIODecoderNext(&dec2, &tk) || dec2.error != VL_MSGPACK_IO_ERR_UNEXPECTED_EOF) {
        return VL_FALSE;
    }

    return VL_TRUE;
}

vl_bool_t vlTestMsgPackEmptyContainers() {
    vl_msgpack_encoder *enc = vlMsgPackIOEncoderNew();
    vlMsgPackIOEncodeMapBegin(enc);
    vlMsgPackIOEncodeMapEnd(enc);
    vlMsgPackIOEncodeArrayBegin(enc);
    vlMsgPackIOEncodeArrayEnd(enc);

    if (enc->error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vl_msgpack_decoder dec;
    vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.offset);

    vl_msgpack_io_token tk;
    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_MAP || tk.map.keyValuePairs != 0) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vlMsgPackIODecoderNext(&dec, &tk);
    if (tk.type != VL_MSGPACK_ARRAY || tk.array.elements != 0) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vlMsgPackIOEncoderDelete(enc);
    return VL_TRUE;
}

vl_bool_t vlTestMsgPackAllTypes() {
    vl_msgpack_encoder *enc = vlMsgPackIOEncoderNew();
    vlMsgPackIOEncodeNil(enc);
    vlMsgPackIOEncodeBool(enc, VL_TRUE);
    vlMsgPackIOEncodeBool(enc, VL_FALSE);
    vlMsgPackIOEncodeInt(enc, -42);
    vlMsgPackIOEncodeUInt(enc, 100);
    vlMsgPackIOEncodeFloat32(enc, 1.5f);
    vlMsgPackIOEncodeFloat64(enc, 2.718);
    vlMsgPackIOEncodeString(enc, "hello");
    vl_uint8_t data[] = {0x42, 0x43};
    vlMsgPackIOEncodeBinary(enc, data, 2);
    vl_uint8_t ext_data[] = {0xAA};
    vlMsgPackIOEncodeExt(enc, 7, ext_data, 1);

    if (enc->error != VL_MSGPACK_IO_ERR_NONE) {
        vlMsgPackIOEncoderDelete(enc);
        return VL_FALSE;
    }

    vl_msgpack_decoder dec;
    vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.offset);

    vl_msgpack_io_token tk;
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_NIL) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_BOOL || !tk.boolean.value) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_BOOL || tk.boolean.value) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_INT || tk.integer.value != -42) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_UINT || tk.uinteger.value != 100) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_FLOAT32) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_FLOAT64) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_STRING || tk.string.length != 5) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_BINARY || tk.binary.length != 2) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }
    if (!vlMsgPackIODecoderNext(&dec, &tk) || tk.type != VL_MSGPACK_EXT || tk.ext.extType != 7 || tk.ext.length != 1) { vlMsgPackIOEncoderDelete(enc); return VL_FALSE; }

    vlMsgPackIOEncoderDelete(enc);
    return VL_TRUE;
}
