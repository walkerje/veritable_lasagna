#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_buffer.h>
    int buffer_test_reset(){
        int result = 1;

        vl_buffer* buffer = vlBufferNew();

        const int payloadSize = 16;
        uint16_t payload[payloadSize];
        for(int i = 0; i < payloadSize; i++)
            payload[i] = (uint16_t)i + 1;

        vlBufferWrite(buffer, sizeof(uint16_t) * payloadSize, payload);
        result = result && (buffer->size == (sizeof(uint16_t) * payloadSize));

        vlBufferReset(buffer);


        vlBufferWrite(buffer, sizeof(uint16_t) * payloadSize, payload);
        result = result && (buffer->size == (sizeof(uint16_t) * payloadSize));

        vlBufferDelete(buffer);
        return result;
    }

    int buffer_test_clear(){
        int result = 1;

        vl_buffer* buffer = vlBufferNew();

        const int payloadSize = 16;
        uint16_t payload[payloadSize];
        for(int i = 0; i < payloadSize; i++)
            payload[i] = (uint16_t)i + 1;

        //write the payload
        vlBufferWrite(buffer, sizeof(uint16_t) * payloadSize, payload);
        result = result && (buffer->size == (sizeof(uint16_t) * payloadSize));

        //clear the buffer
        vlBufferClear(buffer);
        //size should be 0
        result = result && (buffer->size == 0);
        //contents should not match
        result = result && (memcmp(payload, buffer->data, sizeof(uint16_t) * payloadSize) != 0);
        //write the payload again, and check the offset is the same
        vlBufferWrite(buffer, sizeof(uint16_t) * payloadSize, payload);
        result = result && (buffer->size == (sizeof(uint16_t) * payloadSize));

        vlBufferDelete(buffer);
        return result;
    }

    int buffer_test_shrink_to_fit(){
        int result = 1;

        vl_buffer* buffer = vlBufferNew();

        const int payloadSize = 16;
        uint16_t payload[payloadSize];
        for(int i = 0; i < payloadSize; i++)
            payload[i] = (uint16_t)i + 1;

        //write the payload
        vlBufferWrite(buffer, sizeof(uint16_t) * payloadSize, payload);
        result = result && (buffer->size == (sizeof(uint16_t) * payloadSize));

        //make sure the buffer size is the same as its memory block
        vlBufferShrinkToFit(buffer);
        result = result && (vlMemSize(buffer->data) == buffer->size);

        vlBufferDelete(buffer);
        return result;
    }

    int buffer_test_clone(){
        int result = 1;

        vl_buffer* buffer = vlBufferNew();

        const int payloadSize = 16;
        uint16_t payload[payloadSize];
        for(int i = 0; i < payloadSize; i++)
            payload[i] = (uint16_t)i + 1;

        //write the payload
        vlBufferWrite(buffer, sizeof(uint16_t) * payloadSize, payload);
        result = result && (buffer->size == (sizeof(uint16_t) * payloadSize));

        const size_t size = buffer->size;
        const uintptr_t offset = buffer->offset;

        vl_buffer* clone = vlBufferClone(buffer, NULL);
        vlBufferDelete(buffer);//clone the buffer, then delete the original

        //make sure the contents are equal
        result = result && (clone->offset == offset) && (clone->size == size);
        result = result && (memcmp(payload, clone->data, sizeof(uint16_t) * payloadSize) == 0);

        vlBufferDelete(clone);
        return result;
    }

    int buffer_test_copy(){
        int result = 1;

        vl_buffer* buffer = vlBufferNew();
        vl_buffer* otherBuffer = vlBufferNew();

        const int payloadSize = 16;
        uint16_t payload[payloadSize];
        for(int i = 0; i < payloadSize; i++)
            payload[i] = (uint16_t)i + 1;

        //write the payload
        vlBufferWrite(buffer, sizeof(uint16_t) * payloadSize, payload);
        result = result && (buffer->size == (sizeof(uint16_t) * payloadSize));

        //set buffer offset to beginning, copy it to the other buffer, then delete the original.
        vlBufferSeekBegin(buffer);
        vlBufferCopy(buffer, otherBuffer, buffer->size);
        vlBufferDelete(buffer);

        //contents should match the original payload
        result = result && (memcmp(payload, otherBuffer->data, sizeof(uint16_t) * payloadSize) == 0);

        vlBufferDelete(otherBuffer);
        return result;
    }
}

TEST(buffer, reset){
    EXPECT_TRUE(buffer_test_reset());
}

TEST(buffer, clear){
    EXPECT_TRUE(buffer_test_clear());
}

TEST(buffer, shrink_to_fit){
    EXPECT_TRUE(buffer_test_shrink_to_fit());
}

TEST(buffer, clone){
    EXPECT_TRUE(buffer_test_clone());
}

TEST(buffer, copy){
    EXPECT_TRUE(buffer_test_copy());
}