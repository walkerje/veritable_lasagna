#include "vl_msgpack_io.h"
#include "vl_libconfig.h"

#ifdef VL_SYSTEM_BIG_ENDIAN
#define _VL_FLIP_ENDIAN(symbol)
#else
/**
 * \brief Equivalent to a generic htnohl/htons/ntohl/ntohs, which convert to/from system to network byte order.
 *
 * Flips byte sequences in-place.
 *
 * \private
 */
#define _VL_FLIP_ENDIAN(symbol) vlMemReverse(&(symbol), sizeof(symbol))
#endif

typedef enum vl_msgpack_io_type_ {
    //These types are bitmasked when encoded and must be extracted when decoding.
    VL_MSGPACK_IO_POS_FIXINT = 0x00,
    VL_MSGPACK_IO_FIXMAP = 0x80,
    VL_MSGPACK_IO_FIXARRAY = 0x90,
    VL_MSGPACK_IO_FIXSTR = 0xA0,
    VL_MSGPACK_IO_NEG_FIXINT = 0xE0,

    //These types don't have their bits modified.
    VL_MSGPACK_IO_NIL = 0xC0,
    VL_MSGPACK_IO_FALSE = 0xC2,
    VL_MSGPACK_IO_TRUE = 0xC3,
    VL_MSGPACK_IO_BIN8 = 0xC4,
    VL_MSGPACK_IO_BIN16 = 0xC5,
    VL_MSGPACK_IO_BIN32 = 0xC6,
    VL_MSGPACK_IO_EXT8 = 0xC7,
    VL_MSGPACK_IO_EXT16 = 0xC8,
    VL_MSGPACK_IO_EXT32 = 0xC9,
    VL_MSGPACK_IO_FLOAT32 = 0xCA,
    VL_MSGPACK_IO_FLOAT64 = 0xCB,
    VL_MSGPACK_IO_UINT8 = 0xCC,
    VL_MSGPACK_IO_UINT16 = 0xCD,
    VL_MSGPACK_IO_UINT32 = 0xCE,
    VL_MSGPACK_IO_UINT64 = 0xCF,
    VL_MSGPACK_IO_INT8 = 0xD0,
    VL_MSGPACK_IO_INT16 = 0xD1,
    VL_MSGPACK_IO_INT32 = 0xD2,
    VL_MSGPACK_IO_INT64 = 0xD3,
    VL_MSGPACK_IO_FIXEXT1 = 0xD4,
    VL_MSGPACK_IO_FIXEXT2 = 0xD5,
    VL_MSGPACK_IO_FIXEXT4 = 0xD6,
    VL_MSGPACK_IO_FIXEXT8 = 0xD7,
    VL_MSGPACK_IO_FIXEXT16 = 0xD8,
    VL_MSGPACK_IO_STR8 = 0xD9,
    VL_MSGPACK_IO_STR16 = 0xDA,
    VL_MSGPACK_IO_STR32 = 0xDB,
    VL_MSGPACK_IO_ARRAY16 = 0xDC,
    VL_MSGPACK_IO_ARRAY32 = 0xDD,
    VL_MSGPACK_IO_MAP16 = 0xDE,
    VL_MSGPACK_IO_MAP32 = 0xDF,
} vl_msgpack_io_type;

/**
 * \brief A stack frame used by the encoder. Represents a container element.
 * \private
 */
typedef struct vl_msgpack_encframe_ {
    vl_bool_t isArray;
    vl_dsoffs_t startOffset;
    vl_dsoffs_t sizeOffset;
    vl_dsoffs_t dataOffset;
    vl_dsidx_t totalElements;
} vl_msgpack_encframe;

vl_uint8_t vl_MsgPackEncodeFrame(vl_msgpack_encframe *frame, vl_dsoffs_t *offset) {
    if (!frame->isArray) {
        //Is a map? Map lengths are defined as total number of key-value pairs, so divide by 2.
        frame->totalElements /= 2;
    }

    const vl_bool_t isFixed = frame->totalElements <= 0xF;

    if (isFixed) {
        *offset = sizeof(vl_int32_t);
        return (frame->isArray ? VL_MSGPACK_IO_FIXARRAY : VL_MSGPACK_IO_FIXMAP) | (frame->totalElements & 0xF);
    }

    if (frame->totalElements <= 0xFFFF) {
        *offset = sizeof(vl_int16_t);
        return frame->isArray ? VL_MSGPACK_IO_ARRAY16 : VL_MSGPACK_IO_MAP16;
    }

    *offset = 0;
    return frame->isArray ? VL_MSGPACK_IO_ARRAY32 : VL_MSGPACK_IO_MAP32;
}

void vl_MsgPackEncodePush(vl_msgpack_encoder *enc, vl_bool_t isArray) {
    vlStackPush(&enc->stateStack, sizeof(vl_msgpack_encframe));
    vl_msgpack_encframe *frame = (vl_msgpack_encframe *) vlStackPeek(&enc->stateStack);

    //Claim space for the largest possible encoded container type and size.
    frame->isArray = isArray;
    frame->startOffset = vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), NULL);
    frame->sizeOffset = vlBufferWrite(&enc->buffer, sizeof(vl_uint32_t), NULL);
    frame->dataOffset = enc->buffer.offset;
    frame->totalElements = 0;
}

vl_bool_t vl_MsgPackEncodePop(vl_msgpack_encoder *enc) {
    if (vlStackEmpty(&enc->stateStack))
        return VL_FALSE;

    vl_msgpack_encframe frame = *((vl_msgpack_encframe *) vlStackPeek(&enc->stateStack));
    vlStackPop(&enc->stateStack);

    vl_dsoffs_t offset;
    const vl_uint8_t typeTag = vl_MsgPackEncodeFrame(&frame, &offset);
    const vl_dsoffs_t sizeLength = sizeof(vl_uint32_t) - offset;

    //Write the type tag.
    memcpy(enc->buffer.data + frame.startOffset, &typeTag, sizeof(vl_uint8_t));

    //Write the size offset.
    if (sizeLength == sizeof(vl_uint16_t)) {
        vl_uint16_t encSize = (vl_uint16_t) frame.totalElements;
        _VL_FLIP_ENDIAN(encSize);
        memcpy(enc->buffer.data + frame.sizeOffset, &encSize, sizeLength);
    } else if (sizeLength == sizeof(vl_uint32_t)) {
        vl_uint32_t encSize = (vl_uint32_t) frame.totalElements;
        _VL_FLIP_ENDIAN(encSize);
        memcpy(enc->buffer.data + frame.sizeOffset, &encSize, sizeLength);
    }

    //Offset the child data relative to the resolved type.
    if (offset > 0) {
        //We reserved enough space in the buffer to encode any type.
        //If we end up actually only needing a smaller type, we must memmove
        //the data that follows over a few bytes.
        void *dest = (void *) (((vl_uintptr_t) enc->buffer.data) + (frame.dataOffset - offset));
        const void *src = (const void *) (((vl_uintptr_t) enc->buffer.data) + frame.dataOffset);
        const vl_dsoffs_t blockSize = enc->buffer.offset - frame.dataOffset;
        memmove(dest, src, blockSize);
        enc->buffer.offset -= offset;//Offset the write pointer downwards the number of offset bytes.
    }

    return VL_TRUE;
}

static inline void vl_MsgPackEncodeIncrement(vl_msgpack_encoder *enc) {
    if (vlStackEmpty(&enc->stateStack))
        return;
    ((vl_msgpack_encframe *) vlStackPeek(&enc->stateStack))->totalElements++;
}


void vlMsgPackIOEncoderInit(vl_msgpack_encoder *enc) {
    vlStackInit(&enc->stateStack);
    vlBufferInit(&enc->buffer);

    enc->error = VL_MSGPACK_IO_ERR_NONE;
    enc->errorType = VL_MSGPACK_NIL;
    enc->errorDepth = 0;
}

void vlMsgPackIOEncoderFree(vl_msgpack_encoder *enc) {
    vlBufferFree(&enc->buffer);
    vlStackFree(&enc->stateStack);
}

vl_msgpack_encoder *vlMsgPackIOEncoderNew() {
    vl_msgpack_encoder *enc = malloc(sizeof(vl_msgpack_encoder));
    vlMsgPackIOEncoderInit(enc);
    return enc;
}

void vlMsgPackIOEncoderDelete(vl_msgpack_encoder *enc) {
    vlMsgPackIOEncoderFree(enc);
    free(enc);
}

void vlMsgPackIOEncoderClear(vl_msgpack_encoder *enc) {
    vlStackReset(&enc->stateStack);
    vlBufferReset(&enc->buffer);
    enc->error = VL_MSGPACK_IO_ERR_NONE;
    enc->errorType = VL_MSGPACK_NIL;
    enc->errorDepth = 0;
}

void vlMsgPackIOEncodeMapBegin(vl_msgpack_encoder *enc) {
    vl_MsgPackEncodeIncrement(enc);
    vl_MsgPackEncodePush(enc, VL_FALSE);
}

void vlMsgPackIOEncodeMapEnd(vl_msgpack_encoder *enc) {
    if (vlStackEmpty(&enc->stateStack)) {
        enc->error = VL_MSGPACK_IO_ERR_STACK_UNDERFLOW;
        enc->errorDepth = 0;
        enc->errorType = VL_MSGPACK_MAP;
        return;
    }

    const vl_msgpack_encframe *topPtr = (vl_msgpack_encframe *) vlStackPeek(&enc->stateStack);
    if (topPtr->totalElements % 2 == 1) {
        enc->error = VL_MSGPACK_IO_ERR_UNBALANCED_MAP;
        enc->errorDepth = enc->stateStack.depth;
        enc->errorType = VL_MSGPACK_MAP;
        return;
    }

    vl_MsgPackEncodePop(enc);
}

void vlMsgPackIOEncodeArrayBegin(vl_msgpack_encoder *enc) {
    vl_MsgPackEncodeIncrement(enc);
    vl_MsgPackEncodePush(enc, VL_TRUE);
}

void vlMsgPackIOEncodeArrayEnd(vl_msgpack_encoder *enc) {
    if (vlStackEmpty(&enc->stateStack)) {
        enc->error = VL_MSGPACK_IO_ERR_STACK_UNDERFLOW;
        enc->errorDepth = 0;
        enc->errorType = VL_MSGPACK_ARRAY;
    } else vl_MsgPackEncodePop(enc);
}

void vlMsgPackIOEncodeBool(vl_msgpack_encoder *enc, vl_bool_t value) {
    vl_MsgPackEncodeIncrement(enc);
    const vl_uint8_t encVal = value ? VL_MSGPACK_IO_TRUE : VL_MSGPACK_IO_FALSE;
    vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &encVal);
}

void vlMsgPackIOEncodeStringLen(vl_msgpack_encoder *enc, const char *value, vl_uint32_t len) {
    vl_MsgPackEncodeIncrement(enc);
    if (len <= 0b00011111) {
        //fixstring
        const vl_uint8_t fixLen = (vl_uint8_t) (len) & 0b00011111;
        const vl_uint8_t typeTag = VL_MSGPACK_IO_FIXSTR | fixLen;
        //Length is encoded in type tag.
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
    } else if (len <= 0xFF) {
        vl_uint8_t encLen = (vl_uint8_t) len;
        const vl_uint8_t typeTag = VL_MSGPACK_IO_STR8;
        //No need to flip endian for str8 case.
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &encLen);
    } else if (len <= 0xFFFF) {
        vl_uint16_t encLen = (vl_uint16_t) len;
        const vl_uint8_t typeTag = VL_MSGPACK_IO_STR16;

        _VL_FLIP_ENDIAN(encLen);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint16_t), &encLen);
    } else {
        vl_uint32_t encLen = (vl_uint32_t) len;
        const vl_uint8_t typeTag = VL_MSGPACK_IO_STR32;

        _VL_FLIP_ENDIAN(encLen);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint32_t), &encLen);
    }
    vlBufferWrite(&enc->buffer, len, value);
}

void vlMsgPackIOEncodeFloat32(vl_msgpack_encoder *enc, vl_float32_t value) {
    vl_MsgPackEncodeIncrement(enc);
    const vl_uint8_t typeTag = VL_MSGPACK_IO_FLOAT32;
    _VL_FLIP_ENDIAN(value);
    vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
    vlBufferWrite(&enc->buffer, sizeof(vl_float32_t), &value);
}

void vlMsgPackIOEncodeFloat64(vl_msgpack_encoder *enc, vl_float64_t value) {
    vl_MsgPackEncodeIncrement(enc);
    const vl_uint8_t typeTag = VL_MSGPACK_IO_FLOAT64;
    _VL_FLIP_ENDIAN(value);
    vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
    vlBufferWrite(&enc->buffer, sizeof(vl_float64_t), &value);
}

void vlMsgPackIOEncodeBinary(vl_msgpack_encoder *enc, const void *value, vl_uint32_t len) {
    vl_MsgPackEncodeIncrement(enc);
    if (len <= 0xFF) {
        vl_uint8_t encLen = (vl_uint8_t) len;
        const vl_uint8_t typeTag = VL_MSGPACK_IO_BIN8;

        //No need to flip endian for bin8 case.

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &encLen);
    } else if (len <= 0xFFFF) {
        vl_uint16_t encLen = (vl_uint16_t) len;
        const vl_uint8_t typeTag = VL_MSGPACK_IO_BIN16;

        _VL_FLIP_ENDIAN(encLen);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint16_t), &encLen);
    } else {
        vl_uint32_t encLen = (vl_uint32_t) len;
        const vl_uint8_t typeTag = VL_MSGPACK_IO_BIN32;

        _VL_FLIP_ENDIAN(encLen);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint32_t), &encLen);
    }
    vlBufferWrite(&enc->buffer, len, value);
}

void vlMsgPackIOEncodeExt(vl_msgpack_encoder *enc, vl_int8_t type, const void *value, vl_uint32_t len) {
    vl_MsgPackEncodeIncrement(enc);

    const vl_bool_t isFixed = (len <= 16);
    if (isFixed) {//Fixed EXT types must have a size <= 16.
        vl_int8_t encLen;
        vl_uint8_t encType;
        vl_int8_t tempBuffer[16];
        memset(tempBuffer, 0, sizeof(vl_int8_t) * 16);

        if (len > 0 && value)
            memcpy(tempBuffer, value, len);

        switch (len) {
            case 0: //fallthrough
            case 1:
                encLen = 1;
                encType = VL_MSGPACK_IO_FIXEXT1;
                break;
            case 2:
                encLen = 2;
                encType = VL_MSGPACK_IO_FIXEXT2;
                break;
            case 3: //fallthrough
            case 4:
                encLen = 4;
                encType = VL_MSGPACK_IO_FIXEXT4;
                break;
            case 5: //fallthrough
            case 6: //fallthrough
            case 7: //fallthrough
            case 8: //fallthrough
                encLen = 8;
                encType = VL_MSGPACK_IO_FIXEXT8;
                break;
            case 9:  //fallthrough
            case 10: //fallthrough
            case 11: //fallthrough
            case 12: //fallthrough
            case 13: //fallthrough
            case 14: //fallthrough
            case 15: //fallthrough
            case 16: //fallthrough
                encLen = 16;
                encType = VL_MSGPACK_IO_FIXEXT16;
                break;
        }

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &encType);
        vlBufferWrite(&enc->buffer, sizeof(vl_int8_t), &type);
        vlBufferWrite(&enc->buffer, encLen, tempBuffer);

        return;
    }

    //non-fixed variants.

    if (len <= 0xFF) {
        vl_uint8_t encType = VL_MSGPACK_IO_EXT8;
        vl_uint8_t encLen = (vl_uint8_t) len;

        vlBufferWrite(&enc->buffer, sizeof(vl_int8_t), &encType);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &encLen);
        vlBufferWrite(&enc->buffer, sizeof(vl_int8_t), &type);
        vlBufferWrite(&enc->buffer, len, value);
    } else if (len <= 0xFFFF) {
        vl_uint8_t encType = VL_MSGPACK_IO_EXT16;
        vl_uint16_t encLen = (vl_uint16_t) len;

        _VL_FLIP_ENDIAN(encLen);

        vlBufferWrite(&enc->buffer, sizeof(vl_int8_t), &encType);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint16_t), &encLen);
        vlBufferWrite(&enc->buffer, sizeof(vl_int8_t), &type);
        vlBufferWrite(&enc->buffer, len, value);
    } else {
        vl_uint8_t encType = VL_MSGPACK_IO_EXT32;
        vl_uint32_t encLen = (vl_uint32_t) len;

        _VL_FLIP_ENDIAN(encLen);

        vlBufferWrite(&enc->buffer, sizeof(vl_int8_t), &encType);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint32_t), &encLen);
        vlBufferWrite(&enc->buffer, sizeof(vl_int8_t), &type);
        vlBufferWrite(&enc->buffer, len, value);
    }
}

void vlMsgPackIOEncodeInt(vl_msgpack_encoder *enc, vl_ilarge_t value) {
    vl_MsgPackEncodeIncrement(enc);
    static const vl_int8_t VL_MSGPACK_POS_FIXINT_MAX = +0x7F;            //2^7-1
    static const vl_int8_t VL_MSGPACK_POS_FIXINT_MIN = +0x00;            //0
    static const vl_int8_t VL_MSGPACK_NEG_FIXINT_MAX = -0x1;             //-1
    static const vl_int8_t VL_MSGPACK_NEG_FIXINT_MIN = -0x1F;            //2^5-1
    static const vl_int8_t VL_MSGPACK_INT8_MAX = +0x7F;            //2^7-1
    static const vl_int8_t VL_MSGPACK_INT8_MIN = -0x7F;
    static const vl_int16_t VL_MSGPACK_INT16_MAX = +0x7FFF;          //2^15-1
    static const vl_int16_t VL_MSGPACK_INT16_MIN = -0x7FFF;
    static const vl_int32_t VL_MSGPACK_INT32_MAX = +0x7FFFFFFF;      //2^31-1
    static const vl_int32_t VL_MSGPACK_INT32_MIN = -0x7FFFFFFF;

    if (value <= VL_MSGPACK_POS_FIXINT_MAX && value >= VL_MSGPACK_POS_FIXINT_MIN) {
        //fixint
        const vl_uint8_t typeTag = VL_MSGPACK_IO_POS_FIXINT | ((vl_int8_t) (value & 0xFF));
        //Int is encoded in type tag.
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
    } else if (value <= VL_MSGPACK_NEG_FIXINT_MAX && value >= VL_MSGPACK_NEG_FIXINT_MAX) {
        vl_int8_t encVal = (vl_int8_t) (value & 0xFF);
        const vl_uint8_t typeTag = VL_MSGPACK_IO_NEG_FIXINT | encVal;
        //Negative int is encoded in type tag.
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
    } else if (value <= VL_MSGPACK_INT8_MAX && value >= VL_MSGPACK_INT8_MIN) {
        vl_int8_t encVal = (vl_int8_t) (value);
        const vl_uint8_t typeTag = VL_MSGPACK_IO_INT8;
        //No need to flip int8 case.
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_int8_t), &encVal);
    } else if (value <= VL_MSGPACK_INT16_MAX && value >= VL_MSGPACK_INT16_MIN) {
        vl_int16_t encVal = (vl_int16_t) (value);
        const vl_uint8_t typeTag = VL_MSGPACK_IO_INT16;

        _VL_FLIP_ENDIAN(encVal);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_int16_t), &encVal);
    } else if (value <= VL_MSGPACK_INT32_MAX && value >= VL_MSGPACK_INT32_MIN) {
        vl_int32_t encVal = (vl_int32_t) (value);
        const vl_uint8_t typeTag = VL_MSGPACK_IO_INT32;

        _VL_FLIP_ENDIAN(encVal);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_int32_t), &encVal);
    } else {
        vl_int64_t encVal = (vl_int64_t) (value);
        const vl_uint8_t typeTag = VL_MSGPACK_IO_INT64;

        _VL_FLIP_ENDIAN(encVal);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_int64_t), &encVal);
    }
}

void vlMsgPackIOEncodeUInt(vl_msgpack_encoder *enc, vl_ularge_t value) {
    vl_MsgPackEncodeIncrement(enc);
    if (value <= 0xFF) {
        vl_uint8_t encVal = (vl_int8_t) (value);
        const vl_uint8_t typeTag = VL_MSGPACK_IO_UINT8;
        //No need to flip uint8 case.
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &encVal);
    } else if (value <= 0xFFFF) {
        vl_uint16_t encVal = (vl_int16_t) (value);
        const vl_uint8_t typeTag = VL_MSGPACK_IO_UINT16;

        _VL_FLIP_ENDIAN(encVal);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint16_t), &encVal);
    } else if (value <= 0xFFFFFFFF) {
        vl_uint32_t encVal = (vl_int32_t) (value);
        const vl_uint8_t typeTag = VL_MSGPACK_IO_UINT32;

        _VL_FLIP_ENDIAN(encVal);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint32_t), &encVal);
    } else if (value <= 0xFFFFFFFFFFFFFFFF) {
        vl_uint64_t encVal = (vl_int64_t) (value);
        const vl_uint8_t typeTag = VL_MSGPACK_IO_UINT64;

        _VL_FLIP_ENDIAN(encVal);

        vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &typeTag);
        vlBufferWrite(&enc->buffer, sizeof(vl_uint64_t), &encVal);
    }
}

void vlMsgPackIOEncodeNil(vl_msgpack_encoder *enc) {
    vl_MsgPackEncodeIncrement(enc);
    static const vl_uint8_t encNIL = VL_MSGPACK_IO_NIL;
    vlBufferWrite(&enc->buffer, sizeof(vl_uint8_t), &encNIL);
}

void vlMsgPackIOEncodeFromDOM(vl_msgpack_encoder *enc, vl_msgpack *src, vl_msgpack_iter curIter) {
    const vl_msgpack_type curType = vlMsgPackType(src, curIter);

    switch (curType) {
        //Non-Terminals

        case VL_MSGPACK_ARRAY:
            vlMsgPackIOEncodeArrayBegin(enc);
            VL_MSGPACK_FOREACH_CHILD(src, curIter, childIter)
                //RECURSIVE CALL HERE
                vlMsgPackIOEncodeFromDOM(enc, src, childIter);
            vlMsgPackIOEncodeArrayEnd(enc);
            break;
        case VL_MSGPACK_MAP:
            vlMsgPackIOEncodeMapBegin(enc);
            VL_MSGPACK_FOREACH_CHILD(src, curIter, childIter) {
                vl_memsize_t keyLen;
                const char *keyStr = (const char *) vlMsgPackSampleKey(src, childIter, &keyLen);
                vlMsgPackIOEncodeStringLen(enc, keyStr, keyLen);
                //RECURSIVE CALL HERE
                vlMsgPackIOEncodeFromDOM(enc, src, childIter);
            }
            vlMsgPackIOEncodeMapEnd(enc);
            break;

            //Terminals

        case VL_MSGPACK_NIL:
            vlMsgPackIOEncodeNil(enc);
            break;
        case VL_MSGPACK_BOOL:
            vlMsgPackIOEncodeBool(enc, vlMsgPackGetBool(src, curIter, VL_FALSE));
            break;
        case VL_MSGPACK_INT:
            vlMsgPackIOEncodeInt(enc, vlMsgPackGetInt(src, curIter, 0));
            break;
        case VL_MSGPACK_UINT:
            vlMsgPackIOEncodeUInt(enc, vlMsgPackGetUInt(src, curIter, 0));
            break;
        case VL_MSGPACK_FLOAT32:
            vlMsgPackIOEncodeFloat32(enc, vlMsgPackGetFloat32(src, curIter, 0.0f));
            break;
        case VL_MSGPACK_FLOAT64:
            vlMsgPackIOEncodeFloat64(enc, vlMsgPackGetFloat64(src, curIter, 0.0));
            break;
        case VL_MSGPACK_STRING: {
            vl_memsize_t strSize;
            const vl_transient *strValue = vlMsgPackSampleValue(src, curIter, &strSize);
            vlMsgPackIOEncodeStringLen(enc, (const char *) strValue, strSize);
            break;
        }
        case VL_MSGPACK_BINARY: {
            vl_memsize_t binSize;
            const vl_transient *binValue = vlMsgPackSampleValue(src, curIter, &binSize);
            vlMsgPackIOEncodeBinary(enc, binValue, binSize);
            break;
        }
        case VL_MSGPACK_EXT: {
            const vl_int8_t extType = vlMsgPackExtType(src, curIter);
            vl_memsize_t extSize;
            const vl_transient *extValue = vlMsgPackSampleValue(src, curIter, &extSize);
            vlMsgPackIOEncodeExt(enc, extType, extValue, extSize);
            break;
        }
    };
}

void vlMsgPackIODecoderStart(vl_msgpack_decoder *dec, const void *srcMem, vl_memsize_t srcLen) {
    dec->srcMem = srcMem;
    dec->srcLen = srcLen;
    dec->srcOffset = 0;

    dec->error = VL_MSGPACK_IO_ERR_NONE;
    dec->errorType = VL_MSGPACK_NIL;
    dec->errorOffset = 0;
}

#ifndef _VL_DECODE_EOFCHECK
#define _VL_DECODE_EOFCHECK(dec, size)\
    if((dec->srcOffset + (size)) > dec->srcLen){           \
        dec->error = VL_MSGPACK_IO_ERR_UNEXPECTED_EOF;      \
        dec->errorOffset = dec->srcOffset;                  \
        dec->errorType = tkPtr->type;                       \
        return VL_FALSE;                                    \
    }
#endif

vl_bool_t vlMsgPackIODecoderNext(vl_msgpack_decoder *dec, vl_msgpack_io_token *tkPtr) {
    if (dec->srcOffset + 1 > dec->srcLen)
        return VL_FALSE;//Well-formed EOF will not create an error here.

    const vl_uint8_t typeTag = *(dec->srcMem + (dec->srcOffset++));

    switch (typeTag) {
        case VL_MSGPACK_IO_NIL:
            tkPtr->type = VL_MSGPACK_NIL;
            break;
        case VL_MSGPACK_IO_TRUE://fallthrough
        case VL_MSGPACK_IO_FALSE:
            tkPtr->type = VL_MSGPACK_BOOL;
            tkPtr->boolean.value = (typeTag == VL_MSGPACK_IO_TRUE);
            break;
        case VL_MSGPACK_IO_BIN8:
            tkPtr->type = VL_MSGPACK_BINARY;
            _VL_DECODE_EOFCHECK(dec, 1);
            tkPtr->binary.length = *(dec->srcMem + (dec->srcOffset++));
            _VL_DECODE_EOFCHECK(dec, tkPtr->binary.length);
            tkPtr->binary.ptr = dec->srcMem + dec->srcOffset;
            dec->srcOffset += tkPtr->binary.length;
            break;
        case VL_MSGPACK_IO_BIN16: {
            vl_uint16_t binLength;
            tkPtr->type = VL_MSGPACK_BINARY;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint16_t));
            memcpy(&binLength, dec->srcMem + dec->srcOffset, sizeof(vl_uint16_t));
            dec->srcOffset += sizeof(vl_uint16_t);

            _VL_FLIP_ENDIAN(binLength);
            _VL_DECODE_EOFCHECK(dec, binLength);

            tkPtr->binary.length = binLength;
            tkPtr->binary.ptr = dec->srcMem + dec->srcOffset;
            dec->srcOffset += binLength;
            break;
        }
        case VL_MSGPACK_IO_BIN32: {
            vl_uint32_t binLength;
            tkPtr->type = VL_MSGPACK_BINARY;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint32_t));
            memcpy(&binLength, dec->srcMem + dec->srcOffset, sizeof(vl_uint32_t));
            dec->srcOffset += sizeof(vl_uint32_t);

            _VL_FLIP_ENDIAN(binLength);

            _VL_DECODE_EOFCHECK(dec, binLength);
            tkPtr->binary.length = binLength;
            tkPtr->binary.ptr = dec->srcMem + dec->srcOffset;
            dec->srcOffset += binLength;
            break;
        }
        case VL_MSGPACK_IO_EXT8:
            tkPtr->type = VL_MSGPACK_EXT;
            _VL_DECODE_EOFCHECK(dec, 1);
            tkPtr->ext.length = *(dec->srcMem + (dec->srcOffset++));
            _VL_DECODE_EOFCHECK(dec, tkPtr->ext.length + 1);
            tkPtr->ext.extType = *(vl_int8_t *) (dec->srcMem + (dec->srcOffset++));
            tkPtr->ext.ptr = dec->srcMem + dec->srcOffset;
            dec->srcOffset += tkPtr->binary.length;
            break;
        case VL_MSGPACK_IO_EXT16: {
            vl_int16_t extLen;

            tkPtr->type = VL_MSGPACK_EXT;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint16_t));
            memcpy(&extLen, dec->srcMem + dec->srcOffset, sizeof(vl_uint16_t));
            dec->srcOffset += sizeof(vl_uint16_t);

            _VL_FLIP_ENDIAN(extLen);

            _VL_DECODE_EOFCHECK(dec, extLen + 1);
            tkPtr->ext.length = extLen;
            tkPtr->ext.extType = *(vl_int8_t *) (dec->srcMem + (dec->srcOffset++));
            tkPtr->ext.ptr = dec->srcMem + dec->srcOffset;
            dec->srcOffset += tkPtr->binary.length;
            break;
        }
        case VL_MSGPACK_IO_EXT32: {
            vl_int32_t extLen;

            tkPtr->type = VL_MSGPACK_EXT;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint32_t));
            memcpy(&extLen, dec->srcMem + dec->srcOffset, sizeof(vl_uint32_t));
            dec->srcOffset += sizeof(vl_uint32_t);

            _VL_FLIP_ENDIAN(extLen);

            _VL_DECODE_EOFCHECK(dec, extLen + 1);
            tkPtr->ext.length = extLen;
            tkPtr->ext.extType = *(vl_int8_t *) (dec->srcMem + (dec->srcOffset++));
            tkPtr->ext.ptr = dec->srcMem + dec->srcOffset;
            dec->srcOffset += tkPtr->binary.length;
            break;
        }
        case VL_MSGPACK_IO_FLOAT32: {
            tkPtr->type = VL_MSGPACK_FLOAT32;
            _VL_DECODE_EOFCHECK(dec, sizeof(vl_float32_t));
            memcpy(&tkPtr->float64.value, dec->srcMem + dec->srcOffset, sizeof(vl_float32_t));
            dec->srcOffset += sizeof(vl_float32_t);
            _VL_FLIP_ENDIAN(tkPtr->float32.value);
            break;
        }
        case VL_MSGPACK_IO_FLOAT64: {
            tkPtr->type = VL_MSGPACK_FLOAT64;
            _VL_DECODE_EOFCHECK(dec, sizeof(vl_float64_t));
            memcpy(&tkPtr->float64.value, dec->srcMem + dec->srcOffset, sizeof(vl_float64_t));
            dec->srcOffset += sizeof(vl_float64_t);
            _VL_FLIP_ENDIAN(tkPtr->float64.value);
            break;
        }
        case VL_MSGPACK_IO_UINT8:
            tkPtr->type = VL_MSGPACK_UINT;
            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint8_t));
            tkPtr->uinteger.value = *(dec->srcMem + (dec->srcOffset++));
            break;
        case VL_MSGPACK_IO_UINT16: {
            vl_uint16_t value;
            tkPtr->type = VL_MSGPACK_UINT;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint16_t));
            memcpy(&value, dec->srcMem + dec->srcOffset, sizeof(vl_uint16_t));
            dec->srcOffset += sizeof(vl_uint16_t);

            _VL_FLIP_ENDIAN(value);

            tkPtr->uinteger.value = value;
            break;
        }
        case VL_MSGPACK_IO_UINT32: {
            vl_uint32_t value;
            tkPtr->type = VL_MSGPACK_UINT;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint32_t));
            memcpy(&value, dec->srcMem + dec->srcOffset, sizeof(vl_uint32_t));
            dec->srcOffset += sizeof(vl_uint32_t);

            _VL_FLIP_ENDIAN(value);

            tkPtr->uinteger.value = value;
            break;
        }
        case VL_MSGPACK_IO_UINT64: {
            vl_uint64_t value;
            tkPtr->type = VL_MSGPACK_UINT;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint64_t));
            memcpy(&value, dec->srcMem + dec->srcOffset, sizeof(vl_uint64_t));
            dec->srcOffset += sizeof(vl_uint64_t);

            _VL_FLIP_ENDIAN(value);

            tkPtr->uinteger.value = value;
            break;
        }
        case VL_MSGPACK_IO_INT8:
            tkPtr->type = VL_MSGPACK_INT;
            _VL_DECODE_EOFCHECK(dec, sizeof(vl_int8_t));
            tkPtr->integer.value = *(vl_int8_t *) (dec->srcMem + (dec->srcOffset++));
            break;
        case VL_MSGPACK_IO_INT16: {
            vl_int16_t value;
            tkPtr->type = VL_MSGPACK_INT;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_int16_t));
            memcpy(&value, dec->srcMem + dec->srcOffset, sizeof(vl_int16_t));
            dec->srcOffset += sizeof(vl_int16_t);

            _VL_FLIP_ENDIAN(value);

            tkPtr->integer.value = value;
            break;
        }
        case VL_MSGPACK_IO_INT32: {
            vl_int32_t value;
            tkPtr->type = VL_MSGPACK_INT;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_int32_t));
            memcpy(&value, dec->srcMem + dec->srcOffset, sizeof(vl_int32_t));
            dec->srcOffset += sizeof(vl_int32_t);

            _VL_FLIP_ENDIAN(value);

            tkPtr->integer.value = value;
            break;
        }
        case VL_MSGPACK_IO_INT64: {
            vl_int64_t value;
            tkPtr->type = VL_MSGPACK_INT;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_int64_t));
            memcpy(&value, dec->srcMem + dec->srcOffset, sizeof(vl_int64_t));
            dec->srcOffset += sizeof(vl_int64_t);

            _VL_FLIP_ENDIAN(value);

            tkPtr->integer.value = value;
            break;
        }
        case VL_MSGPACK_IO_FIXEXT1: //fallthrough
        case VL_MSGPACK_IO_FIXEXT2: //fallthrough
        case VL_MSGPACK_IO_FIXEXT4: //fallthrough
        case VL_MSGPACK_IO_FIXEXT8: //fallthrough
        case VL_MSGPACK_IO_FIXEXT16:
            tkPtr->type = VL_MSGPACK_EXT;
            tkPtr->ext.length = 1 << (typeTag - VL_MSGPACK_IO_FIXEXT1);
            _VL_DECODE_EOFCHECK(dec, tkPtr->ext.length + 1);
            tkPtr->ext.extType = *(vl_int8_t *) (dec->srcMem + (dec->srcOffset++));
            tkPtr->ext.ptr = (dec->srcMem + dec->srcOffset);
            dec->srcOffset += tkPtr->ext.length;
            break;
        case VL_MSGPACK_IO_STR8:
            tkPtr->type = VL_MSGPACK_STRING;
            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint8_t));
            tkPtr->string.length = *(dec->srcMem + (dec->srcOffset++));
            _VL_DECODE_EOFCHECK(dec, tkPtr->string.length);
            tkPtr->string.ptr = (const char *) (dec->srcMem + dec->srcOffset);
            dec->srcOffset += tkPtr->binary.length;
            break;
        case VL_MSGPACK_IO_STR16: {
            vl_uint16_t strLength;
            tkPtr->type = VL_MSGPACK_STRING;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint16_t));
            memcpy(&strLength, dec->srcMem + dec->srcOffset, sizeof(vl_uint16_t));
            dec->srcOffset += sizeof(vl_uint16_t);

            _VL_FLIP_ENDIAN(strLength);

            _VL_DECODE_EOFCHECK(dec, strLength);
            tkPtr->string.length = strLength;
            tkPtr->string.ptr = (const char *) (dec->srcMem + dec->srcOffset);
            dec->srcOffset += strLength;
            break;
        }
        case VL_MSGPACK_IO_STR32: {
            vl_uint32_t strLength;
            tkPtr->type = VL_MSGPACK_STRING;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint32_t));
            memcpy(&strLength, dec->srcMem + dec->srcOffset, sizeof(vl_uint32_t));
            dec->srcOffset += sizeof(vl_uint32_t);

            _VL_FLIP_ENDIAN(strLength);

            _VL_DECODE_EOFCHECK(dec, strLength);
            tkPtr->string.length = strLength;
            tkPtr->string.ptr = (const char *) (dec->srcMem + dec->srcOffset);
            dec->srcOffset += strLength;
            break;
        }
        case VL_MSGPACK_IO_ARRAY16: {
            vl_uint16_t arrayLen;
            tkPtr->type = VL_MSGPACK_ARRAY;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint16_t));
            memcpy(&arrayLen, dec->srcMem + dec->srcOffset, sizeof(vl_uint16_t));
            dec->srcOffset += sizeof(vl_uint16_t);

            _VL_FLIP_ENDIAN(arrayLen);

            tkPtr->array.elements = arrayLen;
            break;
        }
        case VL_MSGPACK_IO_ARRAY32: {
            vl_uint32_t arrayLen;
            tkPtr->type = VL_MSGPACK_ARRAY;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint32_t));
            memcpy(&arrayLen, dec->srcMem + dec->srcOffset, sizeof(vl_uint32_t));
            dec->srcOffset += sizeof(vl_uint32_t);

            _VL_FLIP_ENDIAN(arrayLen);

            tkPtr->array.elements = arrayLen;
            break;
        }
        case VL_MSGPACK_IO_MAP16: {
            vl_uint16_t pairs;
            tkPtr->type = VL_MSGPACK_MAP;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint16_t));
            memcpy(&pairs, dec->srcMem + dec->srcOffset, sizeof(vl_uint16_t));
            dec->srcOffset += sizeof(vl_uint16_t);

            _VL_FLIP_ENDIAN(pairs);

            tkPtr->map.keyValuePairs = pairs;
            break;
        }
        case VL_MSGPACK_IO_MAP32: {
            vl_uint32_t pairs;
            tkPtr->type = VL_MSGPACK_MAP;

            _VL_DECODE_EOFCHECK(dec, sizeof(vl_uint32_t));
            memcpy(&pairs, dec->srcMem + dec->srcOffset, sizeof(vl_uint32_t));
            dec->srcOffset += sizeof(vl_uint32_t);

            _VL_FLIP_ENDIAN(pairs);

            tkPtr->map.keyValuePairs = pairs;
            break;
        }

        default:
            //Fixed types that have their data length encoded in the tags themselves.
            if (typeTag <= 0x7F) {
                //Positive Fixint
                tkPtr->type = VL_MSGPACK_INT;
                tkPtr->integer.value = +(vl_ilarge_t) (typeTag & 0b01111111);
            } else if (typeTag >= 0xE0) {
                //Negative Fixint
                tkPtr->type = VL_MSGPACK_INT;
                tkPtr->integer.value = -(vl_ilarge_t) (typeTag & 0x00011111);
            } else if (typeTag >= 0x80 && typeTag <= 0x8F) {
                //Fixmap
                tkPtr->type = VL_MSGPACK_MAP;
                tkPtr->map.keyValuePairs = (vl_dsidx_t) (typeTag & 0b00001111);
            } else if (typeTag >= 0x90 && typeTag <= 0x9F) {
                //Fixarray
                tkPtr->type = VL_MSGPACK_ARRAY;
                tkPtr->map.keyValuePairs = (vl_dsidx_t) (typeTag & 0b00001111);
            } else if (typeTag >= 0xA0 && typeTag <= 0xBF) {
                //Fixstr
                tkPtr->type = VL_MSGPACK_STRING;
                tkPtr->string.length = (vl_uint32_t) (typeTag & 0b00011111);
                _VL_DECODE_EOFCHECK(dec, tkPtr->string.length);
                tkPtr->string.ptr = (const char *) (dec->srcMem + dec->srcOffset);
                dec->srcOffset += tkPtr->string.length;
            }
            break;
    }

    return VL_TRUE;
}

vl_msgpack_iter
vlMsgPackIODecodeToDOM(vl_msgpack_decoder *decoder, vl_msgpack *dest, vl_msgpack_iter parent, const void *key,
                       vl_memsize_t keyLen) {
    vl_msgpack_io_token curToken;
    vl_msgpack_iter childIter;
    const vl_msgpack_type parentType = vlMsgPackType(dest, parent);

    if ((parentType != VL_MSGPACK_ARRAY && parentType != VL_MSGPACK_MAP) || !vlMsgPackIODecoderNext(decoder, &curToken))
        return VL_MSGPACK_ITER_INVALID;

    switch (curToken.type) {
        case VL_MSGPACK_ARRAY:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_ARRAY, parent, key, keyLen, &curToken.array.elements,
                                        sizeof(vl_dsidx_t));
            for (vl_dsidx_t curIndex = 0; curIndex < curToken.array.elements; curIndex++) {
                if (vlMsgPackIODecodeToDOM(decoder, dest, childIter, &curIndex, sizeof(vl_dsidx_t)) ==
                    VL_MSGPACK_ITER_INVALID) {
                    //Collapse on failure.
                    vlMsgPackRemove(dest, childIter);
                    return VL_MSGPACK_ITER_INVALID;
                }
            }
            break;
        case VL_MSGPACK_MAP:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_MAP, parent, key, keyLen, NULL, 0);
            for (vl_dsidx_t curPair = 0; curPair < curToken.map.keyValuePairs; curPair++) {
                //read string token first, then recurse into the value.
                vl_msgpack_io_token keyToken;
                if (!vlMsgPackIODecoderNext(decoder, &keyToken)) {
                    decoder->error = VL_MSGPACK_IO_ERR_UNEXPECTED_EOF;
                    decoder->errorOffset = decoder->srcOffset;
                    vlMsgPackRemove(dest, childIter);
                    return VL_MSGPACK_ITER_INVALID;
                }
                if (keyToken.type != VL_MSGPACK_STRING) {
                    vlMsgPackRemove(dest, childIter);
                    decoder->error = VL_MSGPACK_IO_ERR_UNEXPECTED_TOKEN;
                    decoder->errorOffset = decoder->srcOffset;
                    return VL_MSGPACK_ITER_INVALID;
                }

                if (vlMsgPackIODecodeToDOM(decoder, dest, childIter, keyToken.string.ptr, keyToken.string.length) ==
                    VL_MSGPACK_ITER_INVALID) {
                    //Collapse on failure.
                    vlMsgPackRemove(dest, childIter);
                    return VL_MSGPACK_ITER_INVALID;
                }
            }
            break;
        case VL_MSGPACK_NIL:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_NIL, parent, key, keyLen, NULL, 0);
            break;
        case VL_MSGPACK_BOOL:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_BOOL, parent, key, keyLen, &curToken.boolean.value,
                                        sizeof(vl_bool_t));
            break;
        case VL_MSGPACK_INT:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_INT, parent, key, keyLen, &curToken.integer.value,
                                        sizeof(vl_ilarge_t));
            break;
        case VL_MSGPACK_UINT:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_UINT, parent, key, keyLen, &curToken.uinteger.value,
                                        sizeof(vl_ularge_t));
            break;
        case VL_MSGPACK_FLOAT32:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_FLOAT32, parent, key, keyLen, &curToken.float32.value,
                                        sizeof(vl_float32_t));
            break;
        case VL_MSGPACK_FLOAT64:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_FLOAT64, parent, key, keyLen, &curToken.float64.value,
                                        sizeof(vl_float64_t));
            break;
        case VL_MSGPACK_STRING:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_STRING, parent, key, keyLen, curToken.string.ptr,
                                        curToken.string.length);
            break;
        case VL_MSGPACK_BINARY:
            childIter = vlMsgPackInsert(dest, VL_MSGPACK_BINARY, parent, key, keyLen, curToken.binary.ptr,
                                        curToken.binary.length);
            break;
        case VL_MSGPACK_EXT:
            childIter = vlMsgPackInsertExt(dest, VL_MSGPACK_EXT, curToken.ext.extType, parent, key, keyLen,
                                           curToken.ext.ptr, curToken.ext.length);
            break;
    }

    return childIter;
}