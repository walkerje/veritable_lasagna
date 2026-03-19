#include "vl_buffer.h"

#include <memory.h>
#include <stdlib.h>

vl_buffer* vlBufferNewExt(vl_memsize_t size, vl_uint16_t align)
{
    vl_buffer* buffer = malloc(sizeof(vl_buffer));
    if (!buffer)
        return NULL;

    vlBufferInitExt(buffer, size, align);
    return buffer;
}

void vlBufferInitExt(vl_buffer* buffer, vl_memsize_t initialSize, vl_uint16_t align)
{
    if (!buffer)
        return;

    buffer->size = 0;
    buffer->offset = 0;
    buffer->data = vlMemAllocAligned(initialSize, align);
}

void vlBufferReset(vl_buffer* buffer, vl_memsize_t newCapacity)
{
    buffer->size = 0;
    buffer->offset = 0;
    buffer->data = vlMemRealloc(buffer->data, newCapacity);
}

void vlBufferClear(vl_buffer* buffer)
{
    buffer->offset = 0;
    buffer->size = 0;
    memset(buffer->data, 0, vlMemSize(buffer->data));
}

void vlBufferShrinkToFit(vl_buffer* buffer)
{
    if (buffer->size > 0)
        buffer->data = vlMemRealloc(buffer->data, buffer->size);
}

vl_buffer* vlBufferClone(const vl_buffer* src, vl_buffer* dest)
{
    const vl_memsize_t size = vlMemSize(src->data);

    if (dest == NULL)
    {
        dest = vlBufferNewExt(size, (vl_uint16_t)vlMemAlignment(src->data));
    }
    else if (vlMemAlignment(dest->data) != vlMemAlignment(src->data))
    {
        vlMemFree(dest->data);
        dest->data = vlMemAllocAligned(size, vlMemAlignment(src->data));
    }
    else if (size != vlMemSize(dest->data))
    {
        dest->data = vlMemRealloc(dest->data, size);
    }

    memcpy(dest->data, src->data, size);

    dest->offset = src->offset;
    dest->size = src->size;

    return dest;
}

vl_memsize_t vlBufferCopy(vl_buffer* src, vl_buffer* dest, vl_memsize_t len)
{
    len = ((src->offset + len) > src->size) ? ((src->offset + len) - src->size) : len;
    vlBufferWrite(dest, len, (src->data + src->offset));
    src->offset += len;
    return len;
}

vl_uintptr_t vlBufferWrite(vl_buffer* buffer, vl_memsize_t size, const void* src)
{
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

vl_memsize_t vlBufferRead(vl_buffer* buffer, vl_memsize_t size, void* dest)
{
    const vl_memsize_t actualRead = buffer->offset + size > buffer->size ? buffer->size - buffer->offset : size;

    if (actualRead > 0)
        memcpy(dest, buffer->data + buffer->offset, size);

    buffer->offset += actualRead;
    return actualRead;
}
void vlBufferDelete(vl_buffer* buffer)
{
    vlMemFree(buffer->data);
    free(buffer);
}

void vlBufferFree(vl_buffer* buffer) { vlMemFree(buffer->data); }
