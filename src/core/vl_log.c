#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vl/vl_async_queue.h>
#include <vl/vl_atomic.h>
#include <vl/vl_log.h>
#include <vl/vl_memory.h>
#include <vl/vl_mutex.h>
#include <vl/vl_semaphore.h>
#include <vl/vl_stream.h>
#include <vl/vl_thread.h>

typedef struct vl_log_record_
{
    vl_ularge_t seq;
    char* text;
    vl_memsize_t len;
} vl_log_record;

typedef struct vl_log_sink_node_
{
    vl_log_sink sink;
    struct vl_log_sink_node_* next;
} vl_log_sink_node;

struct vl_logger_
{
    vl_mutex config_lock;

    vl_bool_t initialized;
    vl_bool_t running;
    vl_bool_t async;

    vl_thread worker;
    vl_async_queue queue;

    vl_semaphore sem_items;
    vl_semaphore sem_flushed;

    vl_atomic_ularge_t enqueued_seq;
    vl_atomic_ularge_t processed_seq;

    vl_log_sink_node* sinks;
};

static vl_logger* g_logger = NULL;

/* ---------------- Sink implementations ---------------- */

typedef struct vl_log_sink_stdout_data_
{
    int unused;
} vl_log_sink_stdout_data;

static void vlLogSinkStdoutWrite(void* sink_data, const char* msg, vl_memsize_t len)
{
    (void)sink_data;
    (void)fwrite(msg, 1, (size_t)len, stdout);
    (void)fflush(stdout);
}

static void vlLogSinkStdoutFlush(void* sink_data)
{
    (void)sink_data;
    (void)fflush(stdout);
}

static void vlLogSinkStdoutDestroy(void* sink_data) { vlMemFree((vl_memory*)sink_data); }

static const vl_log_sink_vtbl g_vl_log_sink_stdout_vtbl = {vlLogSinkStdoutWrite, vlLogSinkStdoutFlush,
                                                           vlLogSinkStdoutDestroy};

typedef struct vl_log_sink_stream_data_
{
    vl_stream* stream;
} vl_log_sink_stream_data;

static void vlLogSinkStreamWrite(void* sink_data, const char* msg, vl_memsize_t len)
{
    vl_log_sink_stream_data* data = (vl_log_sink_stream_data*)sink_data;
    if (!data || !data->stream)
        return;
    (void)vlStreamWrite(data->stream, msg, len);
    vlStreamFlush(data->stream);
}

static void vlLogSinkStreamFlush(void* sink_data)
{
    vl_log_sink_stream_data* data = (vl_log_sink_stream_data*)sink_data;
    if (!data || !data->stream)
        return;
    vlStreamFlush(data->stream);
}

static void vlLogSinkStreamDestroy(void* sink_data)
{
    vl_log_sink_stream_data* data = (vl_log_sink_stream_data*)sink_data;
    if (!data)
        return;
    if (data->stream)
        vlStreamDelete(data->stream);
    vlMemFree((vl_memory*)data);
}

static const vl_log_sink_vtbl g_vl_log_sink_stream_vtbl = {vlLogSinkStreamWrite, vlLogSinkStreamFlush,
                                                           vlLogSinkStreamDestroy};

/* ---------------- Internal helpers ---------------- */

static char* vlLogFormatHeapV(const char* fmt, va_list ap, vl_memsize_t* out_len)
{
    if (!fmt)
        fmt = "";

    va_list ap2;
    va_copy(ap2, ap);
    const int needed = vsnprintf(NULL, 0, fmt, ap2);
    va_end(ap2);

    if (needed < 0)
    {
        const char* fallback = "[vl_log] formatting error";
        const size_t flen = strlen(fallback);
        char* s = (char*)vlMemAlloc((vl_memsize_t)flen + 1);
        if (!s)
            return NULL;
        memcpy(s, fallback, flen + 1);
        if (out_len)
            *out_len = (vl_memsize_t)flen;
        return s;
    }

    {
        const vl_memsize_t len = (vl_memsize_t)needed;
        char* buf = (char*)vlMemAlloc(len + 1);
        if (!buf)
            return NULL;
        (void)vsnprintf(buf, (size_t)len + 1, fmt, ap);
        if (out_len)
            *out_len = len;
        return buf;
    }
}

static void vlLogWriteToSinks_NoLock(vl_logger* logger, const char* msg, vl_memsize_t len)
{
    vl_log_sink_node* node = logger->sinks;
    while (node)
    {
        if (node->sink.vtbl && node->sink.vtbl->write)
        {
            node->sink.vtbl->write(node->sink.sink_data, msg, len);
        }
        node = node->next;
    }
}

static void vlLogFlushSinks_NoLock(vl_logger* logger)
{
    vl_log_sink_node* node = logger->sinks;
    while (node)
    {
        if (node->sink.vtbl && node->sink.vtbl->flush)
        {
            node->sink.vtbl->flush(node->sink.sink_data);
        }
        node = node->next;
    }
}

static void vlLogProcessRecord(vl_logger* logger, const vl_log_record* rec)
{
    vlMutexObtain(logger->config_lock);
    vlLogWriteToSinks_NoLock(logger, rec->text, rec->len);
    vlMutexRelease(logger->config_lock);
}

static vl_bool_t vlLogDrainAvailable(vl_logger* logger)
{
    vl_bool_t did_any = VL_FALSE;
    vl_log_record rec;

    while (vlAsyncQueuePopFront(&logger->queue, &rec))
    {
        did_any = VL_TRUE;

        vlLogProcessRecord(logger, &rec);
        vlAtomicStore(&logger->processed_seq, rec.seq);
        vlSemaphorePost(logger->sem_flushed);

        vlMemFree((vl_memory*)rec.text);
    }

    return did_any;
}

static void vlLogWorkerProc(void* user)
{
    vl_logger* logger = (vl_logger*)user;

    for (;;)
    {
        (void)vlSemaphoreWait(logger->sem_items, 0);
        (void)vlLogDrainAvailable(logger);

        if (!logger->running && vlAsyncQueueSize(&logger->queue) == 0)
        {
            break;
        }
    }

    vlSemaphorePost(logger->sem_flushed);
}

static void vlLogEnqueueOwned(vl_logger* logger, char* owned_text, vl_memsize_t len)
{
    if (logger->async)
    {
        const vl_ularge_t seq = (vl_ularge_t)vlAtomicFetchAdd(&logger->enqueued_seq, 1) + 1;

        vl_log_record rec;
        rec.seq = seq;
        rec.text = owned_text;
        rec.len = len;

        vlAsyncQueuePushBack(&logger->queue, &rec);
        vlSemaphorePost(logger->sem_items);
    }
    else
    {
        const vl_ularge_t seq = (vl_ularge_t)vlAtomicFetchAdd(&logger->enqueued_seq, 1) + 1;

        vlMutexObtain(logger->config_lock);
        vlLogWriteToSinks_NoLock(logger, owned_text, len);
        vlLogFlushSinks_NoLock(logger);
        vlMutexRelease(logger->config_lock);

        vlAtomicStore(&logger->processed_seq, seq);
        vlMemFree((vl_memory*)owned_text);
    }
}

/* ---------------- Instance API ---------------- */

vl_logger* vlLoggerNew(const vl_log_config* config)
{
    vl_logger* logger = (vl_logger*)vlMemAlloc(sizeof(vl_logger));
    if (!logger)
        return NULL;

    memset(logger, 0, sizeof(vl_logger));
    logger->config_lock = vlMutexNew();
    logger->async = (config && config->async) ? VL_TRUE : VL_FALSE;

    vlAtomicInit(&logger->enqueued_seq, 0);
    vlAtomicInit(&logger->processed_seq, 0);

    if (logger->async)
    {
        vlAsyncQueueInit(&logger->queue, (vl_uint16_t)sizeof(vl_log_record));
        logger->sem_items = vlSemaphoreNew(0);
        logger->sem_flushed = vlSemaphoreNew(0);
        logger->running = VL_TRUE;
        logger->worker = vlThreadNew(vlLogWorkerProc, logger);
    }

    logger->initialized = VL_TRUE;
    return logger;
}

void vlLoggerDelete(vl_logger* logger)
{
    vl_log_sink_node* node;
    vl_log_sink_node* next;

    if (!logger)
        return;

    vlLoggerFlush(logger);

    if (logger->async)
    {
        logger->running = VL_FALSE;
        vlSemaphorePost(logger->sem_items);

        (void)vlThreadJoin(logger->worker);
        vlSemaphoreDelete(logger->sem_items);
        vlSemaphoreDelete(logger->sem_flushed);
        vlAsyncQueueFree(&logger->queue);
    }

    node = logger->sinks;
    while (node)
    {
        next = node->next;
        if (node->sink.vtbl && node->sink.vtbl->destroy)
        {
            node->sink.vtbl->destroy(node->sink.sink_data);
        }
        vlMemFree((vl_memory*)node);
        node = next;
    }

    vlMutexDelete(logger->config_lock);
    vlMemFree((vl_memory*)logger);
}

vl_bool_t vlLoggerAddSink(vl_logger* logger, vl_log_sink sink)
{
    vl_log_sink_node* node;

    if (!logger || !sink.vtbl)
        return VL_FALSE;

    node = (vl_log_sink_node*)vlMemAlloc(sizeof(vl_log_sink_node));
    if (!node)
        return VL_FALSE;

    node->sink = sink;
    node->next = NULL;

    vlMutexObtain(logger->config_lock);
    node->next = logger->sinks;
    logger->sinks = node;
    vlMutexRelease(logger->config_lock);

    return VL_TRUE;
}

void vlLoggerMessage(vl_logger* logger, const char* msg)
{
    vl_memsize_t len;
    char* owned;

    if (!logger)
        return;
    if (!msg)
        msg = "";

    len = (vl_memsize_t)strlen(msg);
    owned = (char*)vlMemAlloc(len + 1);
    if (!owned)
        return;

    memcpy(owned, msg, len + 1);
    vlLogEnqueueOwned(logger, owned, len);
}

void vlLoggerMessageF(vl_logger* logger, const char* msgFormat, ...)
{
    va_list ap;
    vl_memsize_t len = 0;
    char* owned;

    if (!logger)
        return;

    va_start(ap, msgFormat);
    owned = vlLogFormatHeapV(msgFormat, ap, &len);
    va_end(ap);

    if (!owned)
        return;
    vlLogEnqueueOwned(logger, owned, len);
}

void vlLoggerFlush(vl_logger* logger)
{
    if (!logger || !logger->initialized)
        return;

    const vl_ularge_t target = vlAtomicLoad(&logger->enqueued_seq);

    if (!logger->async)
    {
        vlMutexObtain(logger->config_lock);
        vlLogFlushSinks_NoLock(logger);
        vlMutexRelease(logger->config_lock);
        return;
    }

    if (vlAtomicLoad(&logger->processed_seq) >= target)
    {
        vlMutexObtain(logger->config_lock);
        vlLogFlushSinks_NoLock(logger);
        vlMutexRelease(logger->config_lock);
        return;
    }

    vlSemaphorePost(logger->sem_items);

    while (vlAtomicLoad(&logger->processed_seq) < target)
    {
        (void)vlSemaphoreWait(logger->sem_flushed, 0);
    }

    vlMutexObtain(logger->config_lock);
    vlLogFlushSinks_NoLock(logger);
    vlMutexRelease(logger->config_lock);
}

/* ---------------- Sink factories ---------------- */

vl_log_sink vlLogSinkStdout(void)
{
    vl_log_sink sink;
    vl_log_sink_stdout_data* data = (vl_log_sink_stdout_data*)vlMemAlloc(sizeof(vl_log_sink_stdout_data));

    sink.vtbl = NULL;
    sink.sink_data = NULL;

    if (!data)
        return sink;

    data->unused = 0;
    sink.vtbl = &g_vl_log_sink_stdout_vtbl;
    sink.sink_data = data;
    return sink;
}

vl_log_sink vlLogSinkStream(vl_stream* stream)
{
    vl_log_sink sink;
    vl_log_sink_stream_data* data = (vl_log_sink_stream_data*)vlMemAlloc(sizeof(vl_log_sink_stream_data));

    sink.vtbl = NULL;
    sink.sink_data = NULL;

    if (!data || !stream)
    {
        if (data)
            vlMemFree((vl_memory*)data);
        return sink;
    }

    vlStreamRetain(stream);
    data->stream = stream;

    sink.vtbl = &g_vl_log_sink_stream_vtbl;
    sink.sink_data = data;
    return sink;
}

/* ---------------- Global wrappers ---------------- */

void vlLogInit(const vl_log_config* config)
{
    if (g_logger)
        return;
    g_logger = vlLoggerNew(config);
    if (!g_logger)
        return;
    (void)vlLoggerAddSink(g_logger, vlLogSinkStdout());
}

void vlLogFlush(void) { vlLoggerFlush(g_logger); }

void vlLogShutdown(void)
{
    if (!g_logger)
        return;
    vlLoggerDelete(g_logger);
    g_logger = NULL;
}

vl_bool_t vlLogAddSink(vl_log_sink sink)
{
    if (!g_logger)
        vlLogInit(NULL);
    if (!g_logger)
        return VL_FALSE;
    return vlLoggerAddSink(g_logger, sink);
}

vl_bool_t vlLogAddStdoutSink(void) { return vlLogAddSink(vlLogSinkStdout()); }

vl_bool_t vlLogAddStreamSink(vl_stream* stream) { return vlLogAddSink(vlLogSinkStream(stream)); }

void vlLogMessage(const char* msg)
{
    if (!g_logger)
        vlLogInit(NULL);
    vlLoggerMessage(g_logger, msg);
}

void vlLogMessageF(const char* msgFormat, ...)
{
    va_list ap;
    vl_memsize_t len = 0;
    char* owned;

    if (!g_logger)
        vlLogInit(NULL);
    if (!g_logger)
        return;

    va_start(ap, msgFormat);
    owned = vlLogFormatHeapV(msgFormat, ap, &len);
    va_end(ap);

    if (!owned)
        return;
    vlLogEnqueueOwned(g_logger, owned, len);
}

void vlLogError(const char* msg) { vlLogMessage(msg); }

void vlLogErrorF(const char* msgFormat, ...)
{
    va_list ap;
    vl_memsize_t len = 0;
    char* owned;

    if (!g_logger)
        vlLogInit(NULL);
    if (!g_logger)
        return;

    va_start(ap, msgFormat);
    owned = vlLogFormatHeapV(msgFormat, ap, &len);
    va_end(ap);

    if (!owned)
        return;
    vlLogEnqueueOwned(g_logger, owned, len);
}
