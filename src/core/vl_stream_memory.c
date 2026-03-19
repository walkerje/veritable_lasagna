#include <stdlib.h>
#include <string.h>
#include <vl/vl_buffer.h>
#include <vl/vl_memory.h>
#include <vl/vl_stream_memory.h>

//=============================================================================
// 1. Buffer Adapter (vl_buffer)
//=============================================================================

typedef struct
{
    vl_buffer* buffer;
    vl_bool_t ownsBuffer;
} vl_stream_ctx_buffer;

static vl_memsize_t StreamBufferRead(void* buf, vl_memsize_t size, void* user)
{
    vl_stream_ctx_buffer* ctx = (vl_stream_ctx_buffer*)user;

    // vlBufferRead handles offset increment and bounds checking internally
    return vlBufferRead(ctx->buffer, size, buf);
}

static vl_memsize_t StreamBufferWrite(const void* buf, vl_memsize_t size, void* user)
{
    vl_stream_ctx_buffer* ctx = (vl_stream_ctx_buffer*)user;
    // vlBufferWrite handles offset increment and resizing internally
    const vl_uintptr_t oldOffset = vlBufferWrite(ctx->buffer, size, buf);
    return ctx->buffer->offset - oldOffset;
}

static vl_bool_t StreamBufferSeek(vl_int64_t offset, vl_stream_origin origin, void* user)
{
    vl_stream_ctx_buffer* ctx = (vl_stream_ctx_buffer*)user;

    switch (origin)
    {
    case VL_STREAM_SEEK_SET:
        // vlBufferSeek takes absolute uintptr
        if (offset < 0)
            return VL_FALSE;
        vlBufferSeek(ctx->buffer, (vl_uintptr_t)offset);
        break;

    case VL_STREAM_SEEK_CUR:
        // vlBufferSeekRelative takes signed intptr
        vlBufferSeekRelative(ctx->buffer, (vl_intptr_t)offset);
        break;

    case VL_STREAM_SEEK_END:
        // Helper for seeking relative to end: SeekEnd() then SeekRelative()
        // OR calculate manually: size + offset
        {
            // Safety check: offset usually negative for SEEK_END
            vl_dsoffs_t endPos = ctx->buffer->size;
            if ((vl_int64_t)endPos + offset < 0)
                return VL_FALSE;

            vlBufferSeek(ctx->buffer, (vl_uintptr_t)((vl_int64_t)endPos + offset));
        }
        break;
    }
    return VL_TRUE;
}

static vl_int64_t StreamBufferTell(void* user)
{
    vl_stream_ctx_buffer* ctx = (vl_stream_ctx_buffer*)user;
    return (vl_int64_t)ctx->buffer->offset;
}

static void StreamBufferClose(void* user)
{
    vl_stream_ctx_buffer* ctx = (vl_stream_ctx_buffer*)user;
    if (ctx->ownsBuffer)
        vlBufferDelete(ctx->buffer);

    vlMemFree((vl_memory*)ctx);
}

vl_stream* vlStreamOpenBuffer(vl_buffer* buffer, vl_bool_t takeOwnership)
{
    if (!buffer)
        return NULL;

    vl_stream_ctx_buffer* ctx = (vl_stream_ctx_buffer*)vlMemAlloc(sizeof(vl_stream_ctx_buffer));
    if (!ctx)
        return NULL;

    ctx->buffer = buffer;
    ctx->ownsBuffer = takeOwnership;

    vl_stream* s = vlStreamNew(ctx);
    if (!s)
    {
        vlMemFree((vl_memory*)ctx);
        return NULL;
    }

    vlStreamSetRead(s, StreamBufferRead);
    vlStreamSetWrite(s, StreamBufferWrite);
    vlStreamSetSeek(s, StreamBufferSeek);
    vlStreamSetTell(s, StreamBufferTell);
    // No flush needed for memory buffer
    vlStreamSetClose(s, StreamBufferClose);

    return s;
}

//=============================================================================
// 2. Memory Adapter (Raw Memory Block)
//=============================================================================

typedef struct
{
    vl_uint8_t* start;
    vl_memsize_t size;
    vl_memsize_t cursor;
    vl_bool_t readOnly;
} vl_stream_ctx_mem;

static vl_memsize_t StreamMemRead(void* buf, vl_memsize_t size, void* user)
{
    vl_stream_ctx_mem* ctx = (vl_stream_ctx_mem*)user;
    vl_memsize_t available = ctx->size - ctx->cursor;

    if (available == 0)
        return 0;
    if (size > available)
        size = available;

    memcpy(buf, ctx->start + ctx->cursor, size);
    ctx->cursor += size;
    return size;
}

static vl_memsize_t StreamMemWrite(const void* buf, vl_memsize_t size, void* user)
{
    vl_stream_ctx_mem* ctx = (vl_stream_ctx_mem*)user;
    if (ctx->readOnly)
        return 0;

    vl_memsize_t available = ctx->size - ctx->cursor;
    if (available == 0)
        return 0;

    if (size > available)
        size = available; // Truncate write

    memcpy(ctx->start + ctx->cursor, buf, size);
    ctx->cursor += size;
    return size;
}

static vl_bool_t StreamMemSeek(vl_int64_t offset, vl_stream_origin origin, void* user)
{
    vl_stream_ctx_mem* ctx = (vl_stream_ctx_mem*)user;
    vl_int64_t newPos = 0;

    switch (origin)
    {
    case VL_STREAM_SEEK_SET:
        newPos = offset;
        break;
    case VL_STREAM_SEEK_CUR:
        newPos = (vl_int64_t)ctx->cursor + offset;
        break;
    case VL_STREAM_SEEK_END:
        newPos = (vl_int64_t)ctx->size + offset;
        break;
    }

    if (newPos < 0)
        return VL_FALSE;
    if (newPos > (vl_int64_t)ctx->size)
        newPos = ctx->size;

    ctx->cursor = (vl_memsize_t)newPos;
    return VL_TRUE;
}

static vl_int64_t StreamMemTell(void* user) { return (vl_int64_t)((vl_stream_ctx_mem*)user)->cursor; }

static void StreamMemClose(void* user) { vlMemFree(user); }

static vl_stream* InternalOpenMem(void* mem, vl_memsize_t size, vl_bool_t readOnly)
{
    if (!mem && size > 0)
        return NULL;

    vl_stream_ctx_mem* ctx = (vl_stream_ctx_mem*)vlMemAlloc(sizeof(vl_stream_ctx_mem));
    if (!ctx)
        return NULL;

    ctx->start = (vl_uint8_t*)mem;
    ctx->size = size;
    ctx->cursor = 0;
    ctx->readOnly = readOnly;

    vl_stream* s = vlStreamNew(ctx);
    if (!s)
    {
        vlMemFree((vl_memory*)ctx);
        return NULL;
    }

    vlStreamSetRead(s, StreamMemRead);
    if (!readOnly)
    {
        vlStreamSetWrite(s, StreamMemWrite);
    }
    vlStreamSetSeek(s, StreamMemSeek);
    vlStreamSetTell(s, StreamMemTell);
    vlStreamSetClose(s, StreamMemClose);

    return s;
}

vl_stream* vlStreamOpenMemory(const void* memory, vl_memsize_t size)
{
    return InternalOpenMem((void*)memory, size, VL_TRUE);
}

vl_stream* vlStreamOpenMemoryMutable(void* memory, vl_memsize_t size)
{
    return InternalOpenMem(memory, size, VL_FALSE);
}
