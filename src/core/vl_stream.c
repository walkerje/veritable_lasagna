#include <stdlib.h>
#include <vl/vl_stream.h>

vl_stream* vlStreamNew(void* userData)
{
    vl_stream* stream = (vl_stream*)malloc(sizeof(vl_stream));
    if (!stream)
        return NULL;

    // Zero out v-table
    stream->read = NULL;
    stream->write = NULL;
    stream->seek = NULL;
    stream->tell = NULL;
    stream->flush = NULL;
    stream->close = NULL;

    stream->userData = userData;

    // Initialize synchronization primitives
    stream->lock = vlMutexNew();

    // Initialize atomics
    vlAtomicInit(&stream->refCount, 1);
    vlAtomicInit(&stream->totalRead, 0);
    vlAtomicInit(&stream->totalWritten, 0);

    return stream;
}

void vlStreamRetain(vl_stream* stream)
{
    if (stream)
        vlAtomicFetchAdd(&stream->refCount, 1);
}

void vlStreamDelete(vl_stream* stream)
{
    if (!stream)
        return;

    // Atomic decrement returns the new value
    vl_ularge_t oldRefCount = vlAtomicLoad(&stream->refCount);

    while (vlAtomicCompareExchangeStrong(&stream->refCount, &oldRefCount, oldRefCount - 1))
    {
    }

    if (vlAtomicLoad(&stream->refCount) != 0)
        return;

    // Last reference dropped, close and cleanup

    if (stream->close)
        stream->close(stream->userData);

    vlMutexDelete(stream->lock);

    free(stream);
}

//-----------------------------------------------------------------------------
// V-Table Setters
//-----------------------------------------------------------------------------
// Note: These should ideally be called during setup, before sharing the stream.
// If setting dynamically, one might want to lock, but usually setup is
// single-threaded.

void vlStreamSetRead(vl_stream* stream, vl_stream_func_read func)
{
    if (stream)
        stream->read = func;
}
void vlStreamSetWrite(vl_stream* stream, vl_stream_func_write func)
{
    if (stream)
        stream->write = func;
}
void vlStreamSetSeek(vl_stream* stream, vl_stream_func_seek func)
{
    if (stream)
        stream->seek = func;
}
void vlStreamSetTell(vl_stream* stream, vl_stream_func_tell func)
{
    if (stream)
        stream->tell = func;
}
void vlStreamSetFlush(vl_stream* stream, vl_stream_func_flush func)
{
    if (stream)
        stream->flush = func;
}
void vlStreamSetClose(vl_stream* stream, vl_stream_func_close func)
{
    if (stream)
        stream->close = func;
}

//-----------------------------------------------------------------------------
// Operations
//-----------------------------------------------------------------------------

vl_memsize_t vlStreamRead(vl_stream* stream, void* outBuffer, vl_memsize_t outLength)
{
    if (!stream || !stream->read || !outBuffer)
        return 0;

    vlMutexObtain(stream->lock);
    vl_memsize_t bytesRead = stream->read(outBuffer, outLength, stream->userData);

    // Update stats while locked (or use atomic add if you want less critical
    // section) Using atomic add is safer if 'read' itself is somehow re-entrant
    // (though mutex prevents that here)
    if (bytesRead > 0)
    {
        vlAtomicFetchAdd(&stream->totalRead, bytesRead);
    }
    vlMutexRelease(stream->lock);

    return bytesRead;
}

vl_memsize_t vlStreamWrite(vl_stream* stream, const void* inBuffer, vl_memsize_t inLength)
{
    if (!stream || !stream->write || !inBuffer)
        return 0;

    vlMutexObtain(stream->lock);
    vl_memsize_t bytesWritten = stream->write(inBuffer, inLength, stream->userData);

    if (bytesWritten > 0)
    {
        vlAtomicFetchAdd(&stream->totalWritten, bytesWritten);
    }
    vlMutexRelease(stream->lock);

    return bytesWritten;
}

vl_bool_t vlStreamSeek(vl_stream* stream, vl_int64_t offset, vl_stream_origin origin)
{
    if (!stream)
        return VL_FALSE;

    vl_bool_t result = VL_FALSE;
    vlMutexObtain(stream->lock);
    if (stream->seek)
    {
        result = stream->seek(offset, origin, stream->userData);
    }
    vlMutexRelease(stream->lock);

    return result;
}

vl_int64_t vlStreamTell(vl_stream* stream)
{
    if (!stream)
        return -1;

    vl_int64_t pos = -1;
    vlMutexObtain(stream->lock);
    if (stream->tell)
    {
        pos = stream->tell(stream->userData);
    }
    vlMutexRelease(stream->lock);

    return pos;
}

void vlStreamFlush(vl_stream* stream)
{
    if (!stream)
        return;

    vlMutexObtain(stream->lock);
    if (stream->flush)
    {
        stream->flush(stream->userData);
    }
    vlMutexRelease(stream->lock);
}
