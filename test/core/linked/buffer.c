#include "buffer.h"
#include <vl/vl_buffer.h>
#include <string.h>
#include <stdint.h>

#ifndef PAYLOAD_SIZE
#define PAYLOAD_SIZE 16
#endif

vl_bool_t vlTestBufferClear() {
    vl_bool_t result = VL_TRUE;

    vl_buffer *buffer = vlBufferNew();

    uint16_t payload[PAYLOAD_SIZE];
    for (int i = 0; i < PAYLOAD_SIZE; i++)
        payload[i] = (uint16_t) i + 1;

    //write the payload
    vlBufferWrite(buffer, sizeof(uint16_t) * PAYLOAD_SIZE, payload);
    result = result && (buffer->size == (sizeof(uint16_t) * PAYLOAD_SIZE));

    //clear the buffer
    vlBufferClear(buffer);
    //size should be 0
    result = result && (buffer->size == 0);
    //contents should not match
    result = result && (memcmp(payload, buffer->data, sizeof(uint16_t) * PAYLOAD_SIZE) != 0);
    //write the payload again, and check the offset is the same
    vlBufferWrite(buffer, sizeof(uint16_t) * PAYLOAD_SIZE, payload);
    result = result && (buffer->size == (sizeof(uint16_t) * PAYLOAD_SIZE));

    vlBufferDelete(buffer);
    return result;
}

vl_bool_t vlTestBufferShrinkToFit() {
    vl_bool_t result = VL_TRUE;

    vl_buffer *buffer = vlBufferNew();

    uint16_t payload[PAYLOAD_SIZE];
    for (int i = 0; i < PAYLOAD_SIZE; i++)
        payload[i] = (uint16_t) i + 1;

    //write the payload
    vlBufferWrite(buffer, sizeof(uint16_t) * PAYLOAD_SIZE, payload);
    result = result && (buffer->size == (sizeof(uint16_t) * PAYLOAD_SIZE));

    //make sure the buffer size is the same as its memory block
    vlBufferShrinkToFit(buffer);
    result = result && (vlMemSize(buffer->data) == buffer->size);

    vlBufferDelete(buffer);
    return result;
}

vl_bool_t vlTestBufferClone() {
    vl_bool_t result = VL_TRUE;

    vl_buffer *buffer = vlBufferNew();

    uint16_t payload[PAYLOAD_SIZE];
    for (int i = 0; i < PAYLOAD_SIZE; i++)
        payload[i] = (uint16_t) i + 1;

    //write the payload
    vlBufferWrite(buffer, sizeof(uint16_t) * PAYLOAD_SIZE, payload);
    result = result && (buffer->size == (sizeof(uint16_t) * PAYLOAD_SIZE));

    const vl_memsize_t size = buffer->size;
    const vl_memsize_t offset = buffer->offset;

    vl_buffer *clone = vlBufferClone(buffer, NULL);
    vlBufferDelete(buffer);//clone the buffer, then delete the original

    //make sure the contents are equal
    result = result && (clone->offset == offset) && (clone->size == size);
    result = result && (memcmp(payload, clone->data, sizeof(uint16_t) * PAYLOAD_SIZE) == 0);

    vlBufferDelete(clone);
    return result;
}

vl_bool_t vlTestBufferCopy() {
    vl_bool_t result = VL_TRUE;

    vl_buffer *buffer = vlBufferNew();
    vl_buffer *otherBuffer = vlBufferNew();

    uint16_t payload[PAYLOAD_SIZE];
    for (int i = 0; i < PAYLOAD_SIZE; i++)
        payload[i] = (uint16_t) i + 1;

    //write the payload
    vlBufferWrite(buffer, sizeof(uint16_t) * PAYLOAD_SIZE, payload);
    result = result && (buffer->size == (sizeof(uint16_t) * PAYLOAD_SIZE));

    //set buffer offset to beginning, copy it to the other buffer, then delete the original.
    vlBufferSeekBegin(buffer);
    vlBufferCopy(buffer, otherBuffer, buffer->size);
    vlBufferDelete(buffer);

    //contents should match the original payload
    result = result && (memcmp(payload, otherBuffer->data, sizeof(uint16_t) * PAYLOAD_SIZE) == 0);

    vlBufferDelete(otherBuffer);
    return result;
}