#include "vl_buffer.h"
#include <stdlib.h>
#include <memory.h>

vl_buffer *vlBufferNew(void) {
    return vlBufferNewSz(VL_DEFAULT_MEMORY_SIZE);
}

vl_buffer *vlBufferNewSz(vl_memsize_t initialSize) {
    vl_buffer *buffer = malloc(sizeof(vl_buffer));

    buffer->size = 0;
    buffer->offset = 0;
    buffer->data = vlMemAlloc(initialSize);

    return buffer;
}

void vlBufferInitSz(vl_buffer *buffer, vl_memsize_t initialSize) {
    buffer->size = 0;
    buffer->offset = 0;
    buffer->data = vlMemAllocAligned(initialSize, sizeof(vl_uintptr_t));
}

void vlBufferReset(vl_buffer *buffer) {
    buffer->size = 0;
    buffer->offset = 0;
}

void vlBufferResetSz(vl_buffer *buffer, vl_memsize_t initialCapacity) {
    buffer->size = 0;
    buffer->offset = 0;
    buffer->data = vlMemRealloc(buffer->data, initialCapacity);
}

void vlBufferClear(vl_buffer *buffer) {
    buffer->offset = 0;
    buffer->size = 0;
    memset(buffer->data, 0, vlMemSize(buffer->data));
}

void vlBufferShrinkToFit(vl_buffer *buffer) {
    if (buffer->size > 0)
        buffer->data = vlMemRealloc(buffer->data, buffer->size);
}

vl_buffer *vlBufferClone(const vl_buffer *src, vl_buffer *dest) {
    const vl_memsize_t size = vlMemSize(src->data);

    if (dest == NULL)
        dest = vlBufferNewSz(size);
    else if (size != vlMemSize(dest->data))
        dest->data = vlMemRealloc(dest->data, size);

    memcpy(dest->data, src->data, size);

    dest->offset = src->offset;
    dest->size = src->size;

    return dest;
}

vl_memsize_t vlBufferCopy(vl_buffer *src, vl_buffer *dest, vl_memsize_t len) {
    len = ((src->offset + len) > src->size) ? ((src->offset + len) - src->size) : len;
    vlBufferWrite(dest, len, (src->data + src->offset));
    src->offset += len;
    return len;
}

vl_uintptr_t vlBufferWrite(vl_buffer *buffer, vl_memsize_t size, const void *src) {
    const vl_memsize_t initSize = vlMemSize(buffer->data);
    vl_memsize_t newSize = initSize;
    while (newSize <= size + buffer->offset)
        newSize *= 2;
    if (newSize > initSize)
        buffer->data = vlMemRealloc(buffer->data, newSize);

    if (src)
        memcpy(buffer->data + buffer->offset, src, size);

    const vl_uintptr_t oldOffset = buffer->offset;
    buffer->offset += size;
    buffer->size = buffer->offset > buffer->size ? buffer->offset : buffer->size;
    return oldOffset;
}

vl_memsize_t vlBufferRead(vl_buffer *buffer, vl_memsize_t size, void *dest) {
    const vl_memsize_t actualRead = buffer->offset + size > buffer->size ? buffer->size - buffer->offset : size;

    if (actualRead > 0)
        memcpy(dest, buffer->data + buffer->offset, size);

    buffer->offset += actualRead;
    return actualRead;
}

void vlBufferSeek(vl_buffer *buffer, vl_uintptr_t offset) {
    buffer->offset = offset;
}

void vlBufferSeekRelative(vl_buffer *buffer, vl_intptr_t offset) {
    buffer->offset += offset;
}

void vlBufferSeekBegin(vl_buffer *buffer) {
    buffer->offset = 0;
}

void vlBufferSeekEnd(vl_buffer *buffer) {
    buffer->offset = buffer->size;
}

vl_transient *vlBufferBegin(vl_buffer *buffer) {
    return buffer->data;
}

vl_transient *vlBufferEnd(vl_buffer *buffer) {
    return buffer->data + buffer->size;
}

void vlBufferDelete(vl_buffer *buffer) {
    vlMemFree(buffer->data);
    free(buffer);
}

void vlBufferFree(vl_buffer *buffer) {
    vlMemFree(buffer->data);
}