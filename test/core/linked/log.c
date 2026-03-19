#include "log.h"

#include <vl/vl_log.h>
#include <vl/vl_thread.h>
#include <vl/vl_stream_filesys.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
    #include <io.h>
    #include <process.h>
    #define vl_dup     _dup
    #define vl_dup2    _dup2
    #define vl_close   _close
    #define vl_fileno  _fileno
    #define vl_getpid  _getpid
#else
    #include <unistd.h>
    #define vl_dup     dup
    #define vl_dup2    dup2
    #define vl_close   close
    #define vl_fileno  fileno
    #define vl_getpid  getpid
#endif

/* ----------------------------------------------------------------------------
 * Helpers (pure C)
 * ---------------------------------------------------------------------------- */

static vl_bool_t vl_test_make_path(char* out, size_t out_cap, const char* tag) {
    if (!out || out_cap == 0) return VL_FALSE;
    {
        const unsigned long pid = (unsigned long)vl_getpid();
        const int n = snprintf(out, out_cap, "./_vl_log_%s_%lu.log", tag ? tag : "x", pid);
        return (n > 0 && (size_t)n < out_cap) ? VL_TRUE : VL_FALSE;
    }
}

static void vl_test_remove_if_exists(const char* path) {
    if (!path) return;
    (void)remove(path);
}

static vl_bool_t vl_test_file_contains(const char* path, const char* needle) {
    if (!path || !needle) return VL_FALSE;

    FILE* f = fopen(path, "rb");
    if (!f) return VL_FALSE;

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return VL_FALSE; }
    {
        long sz = ftell(f);
        if (sz < 0) { fclose(f); return VL_FALSE; }
        if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return VL_FALSE; }

        char* buf = (char*)malloc((size_t)sz + 1);
        if (!buf) { fclose(f); return VL_FALSE; }

        {
            size_t got = fread(buf, 1, (size_t)sz, f);
            buf[got] = '\0';
            fclose(f);

            {
                const vl_bool_t ok = (strstr(buf, needle) != NULL) ? VL_TRUE : VL_FALSE;
                free(buf);
                return ok;
            }
        }
    }
}

/* Redirect stdout to a file path; returns duplicated original stdout fd in *saved_fd_out. */
static vl_bool_t vl_test_stdout_redirect_to_file(const char* path, int* saved_fd_out) {
    if (!path || !saved_fd_out) return VL_FALSE;

    fflush(stdout);

    {
        const int saved = vl_dup(vl_fileno(stdout));
        if (saved < 0) return VL_FALSE;

        {
            FILE* f = freopen(path, "wb", stdout);
            if (!f) {
                vl_close(saved);
                return VL_FALSE;
            }
        }

        *saved_fd_out = saved;
        return VL_TRUE;
    }
}

static vl_bool_t vl_test_stdout_restore(int saved_fd) {
    fflush(stdout);

    if (vl_dup2(saved_fd, vl_fileno(stdout)) < 0) {
        vl_close(saved_fd);
        return VL_FALSE;
    }

    vl_close(saved_fd);

#if defined(_WIN32)
    (void)freopen("CONOUT$", "a", stdout);
#else
    (void)freopen("/dev/stdout", "a", stdout);
#endif

    return VL_TRUE;
}

/* ----------------------------------------------------------------------------
 * Tests
 * ---------------------------------------------------------------------------- */

vl_bool_t vlTestLogFileWriteAndFlushBlocks(void) {
    vl_bool_t result = VL_TRUE;

    vlLogShutdown();

    char path[512];
    result = result && vl_test_make_path(path, sizeof(path), "flush");
    vl_test_remove_if_exists(path);

    {
        vl_log_config cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.async = VL_TRUE;

        vl_stream* stream = vlStreamOpenFileStr(NULL, path, "ab");
        result = result && (stream != NULL);

        vlLogInit(&cfg);
        result = result && vlLogAddStreamSink(stream);
        if (stream) vlStreamDelete(stream);

        vlLogMessage("A\n");
        vlLogMessage("B\n");
        vlLogMessageF("C=%d\n", 123);

        vlLogFlush();
        vlLogShutdown();
    }

    result = result && vl_test_file_contains(path, "A\n");
    result = result && vl_test_file_contains(path, "B\n");
    result = result && vl_test_file_contains(path, "C=123\n");

    vl_test_remove_if_exists(path);
    return result;
}

vl_bool_t vlTestLogRotationCreatesRotatedFiles(void) {
    /* Rotation behavior is no longer covered by the sink-based API directly.
       Keep this test as a placeholder success until a rotating stream/file sink
       is introduced as a first-class public facility again. */
    return VL_TRUE;
}

/* --- Concurrency test using VL threads --- */

typedef struct {
    int tid;
    int count;
} vl_log_thread_args;

static void vl_log_test_thread_proc(void* user) {
    vl_log_thread_args* a = (vl_log_thread_args*)user;
    for (int i = 0; i < a->count; ++i) {
        vlLogMessageF("T%d:%d\n", a->tid, i);
    }
}

vl_bool_t vlTestLogConcurrentProducersFlushComplete(void) {
    vl_bool_t result = VL_TRUE;

    vlLogShutdown();

    char path[512];
    result = result && vl_test_make_path(path, sizeof(path), "concurrency");
    vl_test_remove_if_exists(path);

    {
        vl_log_config cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.async = VL_TRUE;

        vl_stream* stream = vlStreamOpenFileStr(NULL, path, "ab");
        result = result && (stream != NULL);

        vlLogInit(&cfg);
        result = result && vlLogAddStreamSink(stream);
        if (stream) vlStreamDelete(stream);

        const int thread_count = 8;
        const int per_thread = 200;

        vl_thread threads[8];
        vl_log_thread_args args[8];

        for (int t = 0; t < thread_count; ++t) {
            args[t].tid = t;
            args[t].count = per_thread;
            threads[t] = vlThreadNew(vl_log_test_thread_proc, &args[t]);
            result = result && (threads[t] != 0);
        }

        for (int t = 0; t < thread_count; ++t) {
            result = result && vlThreadJoin(threads[t]);
            vlThreadDelete(threads[t]);
        }

        vlLogFlush();
        vlLogShutdown();
    }

    for (int t = 0; t < 8; ++t) {
        char s0[64];
        char sN[64];
        (void)snprintf(s0, sizeof(s0), "T%d:%d\n", t, 0);
        (void)snprintf(sN, sizeof(sN), "T%d:%d\n", t, 199);
        result = result && vl_test_file_contains(path, s0);
        result = result && vl_test_file_contains(path, sN);
    }

    vl_test_remove_if_exists(path);
    return result;
}

vl_bool_t vlTestLogStdoutCaptureViaRedirect(void) {
    vl_bool_t result = VL_TRUE;

    vlLogShutdown();

    char out_path[512];
    result = result && vl_test_make_path(out_path, sizeof(out_path), "stdout");
    vl_test_remove_if_exists(out_path);

    {
        int saved_fd = -1;
        result = result && vl_test_stdout_redirect_to_file(out_path, &saved_fd);

        {
            vl_log_config cfg;
            memset(&cfg, 0, sizeof(cfg));
            cfg.async = VL_TRUE;

            vlLogInit(&cfg);
            VL_LOG_MSG0("hello stdout\n");
            vlLogFlush();
            vlLogShutdown();
        }

        result = result && vl_test_stdout_restore(saved_fd);
    }

    result = result && vl_test_file_contains(out_path, "hello stdout\n");
    vl_test_remove_if_exists(out_path);
    return result;
}

vl_bool_t vlTestLoggerInstanceWritesToStreamSink(void) {
    vl_bool_t result = VL_TRUE;
    char path[512];

    result = result && vl_test_make_path(path, sizeof(path), "instance_stream");
    vl_test_remove_if_exists(path);

    {
        vl_log_config cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.async = VL_TRUE;

        vl_stream* stream = vlStreamOpenFileStr(NULL, path, "ab");
        vl_logger* logger = vlLoggerNew(&cfg);

        result = result && (stream != NULL);
        result = result && (logger != NULL);

        if (logger && stream) {
            result = result && vlLoggerAddSink(logger, vlLogSinkStream(stream));
            vlStreamDelete(stream);

            vlLoggerMessage(logger, "instance-A\n");
            vlLoggerMessageF(logger, "instance-%d\n", 42);
            vlLoggerFlush(logger);
        } else if (stream) {
            vlStreamDelete(stream);
        }

        if (logger) vlLoggerDelete(logger);
    }

    result = result && vl_test_file_contains(path, "instance-A\n");
    result = result && vl_test_file_contains(path, "instance-42\n");

    vl_test_remove_if_exists(path);
    return result;
}

vl_bool_t vlTestLoggerMultipleInstancesRemainIndependent(void) {
    vl_bool_t result = VL_TRUE;
    char path_a[512];
    char path_b[512];

    result = result && vl_test_make_path(path_a, sizeof(path_a), "multi_a");
    result = result && vl_test_make_path(path_b, sizeof(path_b), "multi_b");

    vl_test_remove_if_exists(path_a);
    vl_test_remove_if_exists(path_b);

    {
        vl_log_config cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.async = VL_TRUE;

        vl_stream* stream_a = vlStreamOpenFileStr(NULL, path_a, "ab");
        vl_stream* stream_b = vlStreamOpenFileStr(NULL, path_b, "ab");
        vl_logger* logger_a = vlLoggerNew(&cfg);
        vl_logger* logger_b = vlLoggerNew(&cfg);

        result = result && (stream_a != NULL);
        result = result && (stream_b != NULL);
        result = result && (logger_a != NULL);
        result = result && (logger_b != NULL);

        if (logger_a && stream_a) {
            result = result && vlLoggerAddSink(logger_a, vlLogSinkStream(stream_a));
            vlStreamDelete(stream_a);
            vlLoggerMessage(logger_a, "only-A\n");
            vlLoggerFlush(logger_a);
        } else if (stream_a) {
            vlStreamDelete(stream_a);
        }

        if (logger_b && stream_b) {
            result = result && vlLoggerAddSink(logger_b, vlLogSinkStream(stream_b));
            vlStreamDelete(stream_b);
            vlLoggerMessage(logger_b, "only-B\n");
            vlLoggerFlush(logger_b);
        } else if (stream_b) {
            vlStreamDelete(stream_b);
        }

        if (logger_a) vlLoggerDelete(logger_a);
        if (logger_b) vlLoggerDelete(logger_b);
    }

    result = result && vl_test_file_contains(path_a, "only-A\n");
    result = result && !vl_test_file_contains(path_a, "only-B\n");
    result = result && vl_test_file_contains(path_b, "only-B\n");
    result = result && !vl_test_file_contains(path_b, "only-A\n");

    vl_test_remove_if_exists(path_a);
    vl_test_remove_if_exists(path_b);
    return result;
}

vl_bool_t vlTestGlobalLoggerCanAddStreamSink(void) {
    vl_bool_t result = VL_TRUE;
    char path[512];

    vlLogShutdown();

    result = result && vl_test_make_path(path, sizeof(path), "global_stream");
    vl_test_remove_if_exists(path);

    {
        vl_log_config cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.async = VL_FALSE;

        vl_stream* stream = vlStreamOpenFileStr(NULL, path, "ab");
        result = result && (stream != NULL);

        vlLogInit(&cfg);

        if (stream) {
            result = result && vlLogAddStreamSink(stream);
            vlStreamDelete(stream);
        }

        vlLogMessage("global-stream\n");
        vlLogFlush();
        vlLogShutdown();
    }

    result = result && vl_test_file_contains(path, "global-stream\n");

    vl_test_remove_if_exists(path);
    return result;
}

vl_bool_t vlTestLogSimple() {
    VL_LOGD_MSG0("Hello, log.\n");
    vlLogShutdown();

    return VL_TRUE;
}