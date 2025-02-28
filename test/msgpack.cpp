#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_msgpack.h>
#include <vl/vl_msgpack_io.h>

    void vl_MsgPackEncodeTestMessage(vl_msgpack_encoder* enc){
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

    void vl_MsgPackPrint(vl_msgpack* pack, vl_msgpack_iter iter, vl_dsidx_t depth){
        for(int i = 0; i < depth; i++)
            putchar('\t');

        if(depth > 0){
            //Print the key before the value.
            printf("%s", "Key: ");

            const vl_msgpack_type parentType = vlMsgPackType(pack, vlMsgPackParent(pack, iter));
            if(parentType == VL_MSGPACK_ARRAY){
                const vl_dsidx_t index = vlMsgPackSampleKeyIndex(pack, iter);
                printf("%u", index);
            }else{
                vl_memsize_t stringLength;
                const char* keyString = (const char*)vlMsgPackSampleKey(pack, iter, &stringLength);
                printf("\"%.*s\"", (int)stringLength, keyString);
            }

            printf(" Value: ");
        }

        switch(vlMsgPackType(pack, iter)){
            case VL_MSGPACK_MAP:{
                printf("Map (Size: %d)\n", (int) vlMsgPackTotalChildren(pack, iter));
                VL_MSGPACK_FOREACH_CHILD(pack, iter, child)
                    vl_MsgPackPrint(pack, child, depth + 1);
                return;
            }
            case VL_MSGPACK_ARRAY:{
                printf("Array (Length: %d)\n", (int) vlMsgPackTotalChildren(pack, iter));
                VL_MSGPACK_FOREACH_CHILD(pack, iter, child)
                    vl_MsgPackPrint(pack, child, depth + 1);
                return;
            }
            case VL_MSGPACK_BOOL:{
                const vl_bool_t value = *((vl_bool_t*)vlMsgPackSampleValue(pack, iter, NULL));
                printf("Bool = %s", value ? "true" : "false");
                break;
            }
            case VL_MSGPACK_INT:{
                const vl_ilarge_t value = *((vl_ilarge_t*)vlMsgPackSampleValue(pack, iter, NULL));
                printf("Int = %lu", (unsigned long)value);
                break;
            }
            case VL_MSGPACK_UINT:{
                const vl_ularge_t value = *((vl_ularge_t*)vlMsgPackSampleValue(pack, iter, NULL));
                printf("UInt = %ld", (long)value);
                break;
            }
            case VL_MSGPACK_FLOAT32:{
                const vl_float32_t value = *((vl_float32_t*)vlMsgPackSampleValue(pack, iter, NULL));
                printf("Float32 = %f", value);
                break;
            }
            case VL_MSGPACK_FLOAT64:{
                const vl_float64_t value = *((vl_float64_t*)vlMsgPackSampleValue(pack, iter, NULL));
                printf("Float64 = %lf", value);
                break;
            }
            case VL_MSGPACK_STRING:{
                vl_memsize_t stringLength;
                const char* value = ((const char*)vlMsgPackSampleValue(pack, iter, &stringLength));
                printf("String = \"%.*s\"", (int)stringLength, value);
                break;
            }
            case VL_MSGPACK_NIL:
                printf("%s", "NIL");
                break;
            case VL_MSGPACK_EXT://fallthrough
            case VL_MSGPACK_BINARY:
                vl_memsize_t length;
                vlMsgPackSampleValue(pack, iter, &length);
                printf("Binary = (Length: %lu)", (unsigned long)length);
                break;
            default:
                break;
        }

        putchar('\n');
    }

    /**
     * This test encodes a test message, decodes it to dom, then re-encodes it from the DOM.
     * This test will only pass assuming that the output of the first test message encoding
     * is byte-level identical after going through the DOM and being re-encoded.
     * \private
     */
    vl_bool_t vl_MsgPackRoundTrip(){
        vl_msgpack_encoder* enc = vlMsgPackIOEncoderNew();
        vl_MsgPackEncodeTestMessage(enc);

        //Failed to encode the test message.
        if(enc->error != VL_MSGPACK_IO_ERR_NONE){
            vlMsgPackIOEncoderDelete(enc);
            return VL_FALSE;
        }

        vl_msgpack* pack = vlMsgPackNew();
        vl_msgpack_decoder dec;
        vlMsgPackIODecoderStart(&dec, enc->buffer.data, enc->buffer.offset);

        const char* encKey = "round-trip";
        vl_msgpack_iter decodeIter = vlMsgPackIODecodeToDOM(&dec, pack, vlMsgPackRoot(pack), encKey, strlen(encKey));

        //Failed to decode the test message to DOM.
        if(dec.error != VL_MSGPACK_IO_ERR_NONE){
            vlMsgPackIOEncoderDelete(enc);
            vlMsgPackDelete(pack);
            return VL_FALSE;
        }

        vl_MsgPackPrint(pack, decodeIter, 0);

        vl_msgpack_encoder* secondEnc = vlMsgPackIOEncoderNew();

        vlMsgPackIOEncodeFromDOM(secondEnc, pack, decodeIter);

        //Failed to re-encode from DOM.
        if(secondEnc->error != VL_MSGPACK_IO_ERR_NONE){
            vlMsgPackIOEncoderDelete(enc);
            vlMsgPackIOEncoderDelete(secondEnc);
            vlMsgPackDelete(pack);

            return VL_FALSE;
        }

        //Both encoding buffers should have the same write offset after they both encoded.
        //Elements must be strongly ordered.
        vl_bool_t result = enc->buffer.offset == secondEnc->buffer.offset;

        //Do both encoders have identical buffers?
        if(result)
            result = (memcmp(enc->buffer.data, secondEnc->buffer.data, enc->buffer.offset) == 0);

        vlMsgPackIOEncoderDelete(enc);
        vlMsgPackIOEncoderDelete(secondEnc);
        vlMsgPackDelete(pack);

        return result;
    }
}

TEST(msgpack, round_trip){
    EXPECT_TRUE(vl_MsgPackRoundTrip());
}
