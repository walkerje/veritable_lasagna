#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <vl/vl_memory.h>
#include <vl/vl_stream_filesys.h>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#endif

//=============================================================================
// File Stream Implementation (stdio wrapper)
//=============================================================================

typedef struct
{
    FILE* handle;
    vl_bool_t ownsHandle; // Usually true, unless wrapping stdout/stdin
} vl_stream_ctx_file;

static vl_memsize_t StreamFileRead(void* buf, vl_memsize_t size, void* user)
{
    vl_stream_ctx_file* ctx = (vl_stream_ctx_file*)user;
    if (!ctx->handle)
        return 0;

    // fread returns number of elements, so we read 1-byte elements
    return (vl_memsize_t)fread(buf, 1, size, ctx->handle);
}

static vl_memsize_t StreamFileWrite(const void* buf, vl_memsize_t size, void* user)
{
    vl_stream_ctx_file* ctx = (vl_stream_ctx_file*)user;
    if (!ctx->handle)
        return 0;

    return (vl_memsize_t)fwrite(buf, 1, size, ctx->handle);
}

static vl_bool_t StreamFileSeek(vl_int64_t offset, vl_stream_origin origin, void* user)
{
    vl_stream_ctx_file* ctx = (vl_stream_ctx_file*)user;
    if (!ctx->handle)
        return VL_FALSE;

    int stdioOrigin = SEEK_SET;
    switch (origin)
    {
    case VL_STREAM_SEEK_SET:
        stdioOrigin = SEEK_SET;
        break;
    case VL_STREAM_SEEK_CUR:
        stdioOrigin = SEEK_CUR;
        break;
    case VL_STREAM_SEEK_END:
        stdioOrigin = SEEK_END;
        break;
    }

    // fseek returns 0 on success
#if defined(_WIN32)
    return _fseeki64(ctx->handle, offset, stdioOrigin) == 0;
#else
    return fseeko(ctx->handle, (off_t)offset, stdioOrigin) == 0;
#endif
}

static vl_int64_t StreamFileTell(void* user)
{
    vl_stream_ctx_file* ctx = (vl_stream_ctx_file*)user;
    if (!ctx->handle)
        return -1;

#if defined(_WIN32)
    return _ftelli64(ctx->handle);
#else
    return (vl_int64_t)ftello(ctx->handle);
#endif
}

static void StreamFileFlush(void* user)
{
    vl_stream_ctx_file* ctx = (vl_stream_ctx_file*)user;
    if (ctx->handle)
    {
        fflush(ctx->handle);
    }
}

static void StreamFileClose(void* user)
{
    vl_stream_ctx_file* ctx = (vl_stream_ctx_file*)user;
    if (ctx->ownsHandle && ctx->handle)
    {
        fclose(ctx->handle);
    }
    vlMemFree((vl_memory*)ctx);
}

//=============================================================================
// Internal Helpers
//=============================================================================

static vl_stream* CreateFileStream(FILE* fp)
{
    if (!fp)
        return NULL;

    vl_stream_ctx_file* ctx = (vl_stream_ctx_file*)vlMemAlloc(sizeof(vl_stream_ctx_file));
    if (!ctx)
    {
        fclose(fp);
        return NULL;
    }

    ctx->handle = fp;
    ctx->ownsHandle = VL_TRUE;

    vl_stream* s = vlStreamNew(ctx);
    if (!s)
    {
        StreamFileClose(ctx); // cleans up handle and memory
        return NULL;
    }

    vlStreamSetRead(s, StreamFileRead);
    vlStreamSetWrite(s, StreamFileWrite);
    vlStreamSetSeek(s, StreamFileSeek);
    vlStreamSetTell(s, StreamFileTell);
    vlStreamSetFlush(s, StreamFileFlush);
    vlStreamSetClose(s, StreamFileClose);

    return s;
}

//=============================================================================
// Public API
//=============================================================================

vl_stream* vlStreamOpenFile(const vl_filesys_path* path, const char* mode)
{
    if (!path)
        return NULL;
    const char* pathStr = (const char*)vlFSPathString(path);
    if (!pathStr)
        return NULL;

    FILE* fp = NULL;

#if defined(_WIN32)
    // Windows requires WideChar for proper unicode path handling
    int pathLen = MultiByteToWideChar(CP_UTF8, 0, pathStr, -1, NULL, 0);
    int modeLen = MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);

    if (pathLen > 0 && modeLen > 0)
    {
        wchar_t* wPath = (wchar_t*)vlMemAlloc(pathLen * sizeof(wchar_t));
        wchar_t* wMode = (wchar_t*)vlMemAlloc(modeLen * sizeof(wchar_t));

        if (wPath && wMode)
        {
            MultiByteToWideChar(CP_UTF8, 0, pathStr, -1, wPath, pathLen);
            MultiByteToWideChar(CP_UTF8, 0, mode, -1, wMode, modeLen);

            // Use _wfopen for Unicode support
            fp = _wfopen(wPath, wMode);
        }

        if (wPath)
            vlMemFree((vl_memory*)wPath);
        if (wMode)
            vlMemFree((vl_memory*)wMode);
    }
#else
    // POSIX uses UTF-8 natively
    fp = fopen(pathStr, mode);
#endif

    return CreateFileStream(fp);
}

vl_stream* vlStreamOpenFileStr(vl_filesys* sys, const char* pathStr, const char* mode)
{
    if (!pathStr)
        return NULL;

    // Direct open without wrapping in vl_filesys_path object if possible,
    // duplicating the logic slightly to avoid dependency overhead if 'sys' is
    // NULL

    // However, since we might already depend on vlFSPathString logic,
    // let's just reuse the internal logic if we can.

    // If we want to strictly use the public API:
    if (sys)
    {
        vl_filesys_path* p = vlFSPathNew(sys, pathStr);
        vl_stream* s = vlStreamOpenFile(p, mode);
        vlFSPathDelete(p);
        return s;
    }

    // If sys is null, fallback to direct open (duplicating the logic above)
    // This allows opening files without a full vl_filesys context.
    FILE* fp = NULL;
#if defined(_WIN32)
    int pathLen = MultiByteToWideChar(CP_UTF8, 0, pathStr, -1, NULL, 0);
    int modeLen = MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);
    if (pathLen > 0 && modeLen > 0)
    {
        wchar_t* wPath = (wchar_t*)vlMemAlloc(pathLen * sizeof(wchar_t));
        wchar_t* wMode = (wchar_t*)vlMemAlloc(modeLen * sizeof(wchar_t));
        if (wPath && wMode)
        {
            MultiByteToWideChar(CP_UTF8, 0, pathStr, -1, wPath, pathLen);
            MultiByteToWideChar(CP_UTF8, 0, mode, -1, wMode, modeLen);
            fp = _wfopen(wPath, wMode);
        }
        if (wPath)
            vlMemFree((vl_memory*)wPath);
        if (wMode)
            vlMemFree((vl_memory*)wMode);
    }
#else
    fp = fopen(pathStr, mode);
#endif

    return CreateFileStream(fp);
}
