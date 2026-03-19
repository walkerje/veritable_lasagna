/**
 * ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
 * ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
 * ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
 *  ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
 *   ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
 * ====---: A Data Structure and Algorithms library for C11.  :---====
 *
 * Copyright 2026 Jesse Walker, released under the MIT license.
 * Git Repository:  https://github.com/walkerje/veritable_lasagna
 * \private
 */

#ifndef VL_LOG_H
#define VL_LOG_H

#include "vl_memory.h"
#include "vl_numtypes.h"
#include "vl_stream.h"

/**
 * \file vl_log.h
 * \brief Thread-safe, sink-based logging for Veritable Lasagna.
 *
 * This module provides a small logging system built around two ideas:
 *
 * - **logger instances** (`vl_logger`) which own buffering, synchronization,
 *   and dispatch behavior
 * - **log sinks** (`vl_log_sink`) which define where bytes are written
 *
 * The API supports both:
 *
 * - **independent logger instances**, created with `vlLoggerNew()`
 * - a **global convenience logger**, accessed through the `vlLog*()` family
 *
 * ## Design overview
 *
 * A logger accepts preformatted messages and forwards them to zero or more
 * attached sinks. Sinks are modular output targets such as:
 *
 * - standard output
 * - a generic `vl_stream`
 * - future custom backends implemented by the caller
 *
 * Each logger may operate either:
 *
 * - **synchronously**, where the calling thread performs the sink writes
 * - **asynchronously**, where callers enqueue messages and a worker thread
 *   performs the actual output
 *
 * ## Thread safety
 *
 * All logger operations are intended to be safe for concurrent use from
 * multiple threads.
 *
 * In async mode, producer threads only enqueue work and signal the worker,
 * which helps reduce I/O contention. In sync mode, writes occur before the
 * logging call returns.
 *
 * ## Message boundaries
 *
 * Messages are written exactly as provided. This API does **not** append a
 * newline automatically, so callers should include `\n` when line-oriented
 * output is desired.
 *
 * ## Ownership model
 *
 * Logger instances own their attached sinks. When a sink is added to a logger,
 * the logger stores a copy of the sink descriptor and assumes ownership of the
 * sink's `sink_data`. Sink cleanup is performed automatically when the logger
 * is destroyed.
 */

/**
 * \brief Opaque logger instance type.
 *
 * A `vl_logger` represents an independently configured logging pipeline.
 * Multiple logger instances may coexist at the same time, each with its own
 * sink set and async worker state.
 *
 * Use `vlLoggerNew()` to create an instance and `vlLoggerDelete()` to destroy
 * it.
 */
typedef struct vl_logger_ vl_logger;

/**
 * \brief Virtual function table for a log sink.
 *
 * Each sink implementation supplies callbacks that allow a logger to:
 *
 * - write a message
 * - flush buffered output
 * - destroy sink-owned resources
 *
 * The `sink_data` pointer passed to these functions is the same pointer stored
 * in the corresponding `vl_log_sink`.
 *
 * \warning Sink callbacks should not recursively log back into the same logger,
 * as that may cause deadlock, reentrancy issues, or unbounded recursion.
 */
typedef struct vl_log_sink_vtbl_
{
    /**
     * \brief Write a message to the sink.
     *
     * \param sink_data Implementation-specific sink state.
     * \param msg       Pointer to message bytes.
     * \param len       Number of bytes to write from `msg`.
     */
    void (*write)(void* sink_data, const char* msg, vl_memsize_t len);

    /**
     * \brief Flush any buffered data held by the sink.
     *
     * \param sink_data Implementation-specific sink state.
     */
    void (*flush)(void* sink_data);

    /**
     * \brief Destroy sink state and release owned resources.
     *
     * \param sink_data Implementation-specific sink state.
     *
     * This is called by the logger when the sink is being torn down.
     */
    void (*destroy)(void* sink_data);
} vl_log_sink_vtbl;

/**
 * \brief Public descriptor for attaching an output sink to a logger.
 *
 * A sink is defined by:
 *
 * - a callback table (`vtbl`)
 * - an implementation-defined state pointer (`sink_data`)
 *
 * The logger stores a copy of this struct when `vlLoggerAddSink()` succeeds.
 *
 * ## Ownership
 *
 * After a successful `vlLoggerAddSink()`, the logger owns `sink_data` and will
 * eventually pass it to the sink's `destroy` callback.
 *
 * Therefore, callers should treat the sink as "moved" into the logger after
 * successful attachment.
 */
typedef struct vl_log_sink_
{
    /**
     * \brief Sink callback table.
     *
     * Must not be NULL for a valid sink.
     */
    const vl_log_sink_vtbl* vtbl;

    /**
     * \brief Sink implementation state.
     *
     * Ownership transfers to the logger after successful attachment.
     */
    void* sink_data;
} vl_log_sink;

/**
 * \brief Configuration used when creating a logger.
 *
 * This structure controls per-logger behavior. The configuration is consumed at
 * logger creation time by `vlLoggerNew()` or by `vlLogInit()` for the global
 * logger.
 */
typedef struct vl_log_config_
{
    /**
     * \brief Enable asynchronous logging.
     *
     * If `VL_TRUE`, the logger creates a background worker thread and logging
     * calls enqueue records for later output.
     *
     * If `VL_FALSE`, logging calls write directly to attached sinks before
     * returning.
     *
     * Async mode is typically preferred for reducing contention in code paths that
     * log frequently.
     */
    vl_bool_t async;
} vl_log_config;

/**
 * \brief Create a new logger instance.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_logger` handle and is responsible for calling `vlLoggerDelete`.
 * - **Lifetime**: The logger remains valid until `vlLoggerDelete`.
 * - **Thread Safety**: This function is thread-safe.
 * - **Nullability**: Returns `NULL` if the logger could not be created.
 * - **Error Conditions**: Returns `NULL` if heap allocation fails or if internal synchronization primitives/worker
 * threads cannot be initialized.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates the logger structure, internal message queue, mutexes, and semaphores
 * on the heap.
 * - **Return-value Semantics**: Returns an opaque handle to the new logger, or `NULL` on failure.
 *
 * \param config Optional logger configuration. If NULL, implementation-defined
 *               defaults are used.
 *
 * \return A newly allocated logger instance, or NULL on allocation or
 *         initialization failure.
 *
 * The returned logger initially has no sinks attached. Messages sent to such a
 * logger are accepted but may produce no externally visible output until one or
 * more sinks are added.
 *
 * \sa vlLoggerAddSink
 * \sa vlLoggerDelete
 */
VL_API vl_logger* vlLoggerNew(const vl_log_config* config);

/**
 * \brief Destroy a logger instance and release all associated resources.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the logger and all its associated sinks and internal resources.
 * - **Lifetime**: The logger handle becomes invalid immediately after this call.
 * - **Thread Safety**: Safe to call from any thread, provided no other thread is currently using the logger.
 * - **Nullability**: Safe to call with `NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion. Deleting a logger that is being used by another thread.
 * - **Memory Allocation Expectations**: Deallocates all heap-allocated resources associated with the logger.
 * - **Return-value Semantics**: None (void).
 *
 * \param logger Logger to destroy. NULL is ignored.
 *
 * Destruction performs an implicit flush of pending messages before tearing down
 * worker state and attached sinks.
 *
 * After this call returns, `logger` must not be used again.
 *
 * \sa vlLoggerFlush
 */
VL_API void vlLoggerDelete(vl_logger* logger);

/**
 * \brief Attach a sink to a logger instance.
 *
 * \param logger Logger receiving the sink.
 * \param sink   Sink descriptor to attach.
 *
 * \return `VL_TRUE` on success, `VL_FALSE` on failure.
 *
 * On success, the logger stores a copy of `sink` and assumes ownership of
 * `sink.sink_data`.
 *
 * On failure, ownership remains with the caller unless the implementation
 * documents otherwise.
 *
 * \note Attaching the same underlying sink state to multiple loggers is unsafe
 * unless that sink implementation explicitly supports shared ownership.
 */
VL_API vl_bool_t vlLoggerAddSink(vl_logger* logger, vl_log_sink sink);

/**
 * \brief Write a preformatted message through a specific logger.
 *
 * ## Contract
 * - **Ownership**: The logger copies the `msg` internally. The caller retains ownership of the `msg` pointer.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (atomic enqueue).
 * - **Nullability**: Safe to call with `NULL` for `msg` (treated as empty). `logger` should not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing `NULL` for `logger`.
 * - **Memory Allocation Expectations**: Allocates memory on the heap to store the message record.
 * - **Return-value Semantics**: None (void).
 *
 * \param logger Logger instance to receive the message.
 * \param msg    NUL-terminated UTF-8 message string. If NULL, it is treated as
 *               an empty string.
 *
 * The message is copied internally so the caller retains ownership of `msg`.
 * No newline is appended automatically.
 *
 * In async mode, this call typically enqueues the message and returns before
 * sink I/O completes.
 *
 * \sa vlLoggerFlush
 */
VL_API void vlLoggerMessage(vl_logger* logger, const char* msg);

/**
 * \brief Write a formatted message through a specific logger.
 *
 * \param logger    Logger instance to receive the message.
 * \param msgFormat `printf`-style format string.
 * \param ...       Format arguments.
 *
 * The formatted message is materialized into logger-owned memory before being
 * enqueued or written.
 *
 * No newline is appended automatically.
 */
VL_API void vlLoggerMessageF(vl_logger* logger, const char* msgFormat, ...);

/**
 * \brief Flush pending messages for a specific logger.
 *
 * \param logger Logger instance to flush. NULL is ignored.
 *
 * Behavior depends on the logger mode:
 *
 * - **async mode**: blocks until all messages enqueued before the call have
 *   been processed, then flushes sinks
 * - **sync mode**: messages are already written by the time logging calls
 *   return, so this primarily flushes sink buffers
 *
 * This function is useful as a synchronization barrier before shutdown,
 * abnormal termination, or assertions in tests.
 */
VL_API void vlLoggerFlush(vl_logger* logger);

/**
 * \brief Create a sink that writes to standard output.
 *
 * \return A sink descriptor suitable for `vlLoggerAddSink()`.
 *
 * This sink writes to `stdout` and flushes it when requested by the logger.
 *
 * The returned sink is intended to be attached to exactly one logger.
 */
VL_API vl_log_sink vlLogSinkStdout(void);

/**
 * \brief Create a sink that writes to an existing `vl_stream`.
 *
 * \param stream Stream to write to.
 * \return A sink descriptor suitable for `vlLoggerAddSink()`.
 *
 * The sink retains the stream when created and releases it when the sink is
 * destroyed. This allows the stream to remain valid for as long as the logger
 * needs it.
 *
 * \note The stream pointer should be non-NULL.
 * \note Whether stream writes are buffered depends on the underlying stream
 * implementation.
 */
VL_API vl_log_sink vlLogSinkStream(vl_stream* stream);

/* ---------------- Global convenience API ---------------- */

/**
 * \brief Initialize the global logger.
 *
 * \param config Optional configuration for the global logger. If NULL,
 *               implementation defaults are used.
 *
 * This function initializes the process-wide convenience logger used by the
 * `vlLog*()` functions.
 *
 * The global logger is intended for applications that want a simple shared
 * logging facility without manually managing a `vl_logger*`.
 *
 * Typical usage:
 * - call `vlLogInit()` once during startup
 * - attach one or more sinks
 * - use `vlLogMessage*()` during runtime
 * - call `vlLogShutdown()` during shutdown
 */
VL_API void vlLogInit(const vl_log_config* config);

/**
 * \brief Flush the global logger.
 *
 * If the global logger has not been initialized, this function does nothing.
 *
 * \sa vlLoggerFlush
 */
VL_API void vlLogFlush(void);

/**
 * \brief Shut down and destroy the global logger.
 *
 * If the global logger is active, this performs an implicit flush, destroys all
 * attached sinks, stops background worker state if present, and resets the
 * global logger to an uninitialized state.
 *
 * Reinitialization after shutdown is permitted by calling `vlLogInit()` again.
 */
VL_API void vlLogShutdown(void);

/**
 * \brief Write a preformatted message through the global logger.
 *
 * \param msg NUL-terminated message string. If NULL, treated as an empty
 *            string.
 *
 * If the global logger has not yet been initialized, it may be initialized
 * lazily using default behavior.
 */
VL_API void vlLogMessage(const char* msg);

/**
 * \brief Write a formatted message through the global logger.
 *
 * \param msgFormat `printf`-style format string.
 * \param ...       Format arguments.
 *
 * If the global logger has not yet been initialized, it may be initialized
 * lazily using default behavior.
 */
VL_API void vlLogMessageF(const char* msgFormat, ...);

/**
 * \brief Write an error message through the global logger.
 *
 * \param msg NUL-terminated message string.
 *
 * This currently behaves like a semantic alias of `vlLogMessage()`. It exists
 * to make call sites more expressive and to leave room for future
 * severity-aware routing or formatting.
 */
VL_API void vlLogError(const char* msg);

/**
 * \brief Write a formatted error message through the global logger.
 *
 * \param msgFormat `printf`-style format string.
 * \param ...       Format arguments.
 *
 * This currently behaves like a semantic alias of `vlLogMessageF()`.
 */
VL_API void vlLogErrorF(const char* msgFormat, ...);

/**
 * \brief Attach a sink to the global logger.
 *
 * \param sink Sink descriptor to attach.
 * \return `VL_TRUE` on success, `VL_FALSE` on failure.
 *
 * If the global logger is not initialized yet, it may be initialized lazily
 * before the sink is attached.
 */
VL_API vl_bool_t vlLogAddSink(vl_log_sink sink);

/**
 * \brief Convenience helper that attaches a stdout sink to the global logger.
 *
 * \return `VL_TRUE` on success, `VL_FALSE` on failure.
 */
VL_API vl_bool_t vlLogAddStdoutSink(void);

/**
 * \brief Convenience helper that attaches a `vl_stream` sink to the global
 * logger.
 *
 * \param stream Stream to attach as a sink target.
 * \return `VL_TRUE` on success, `VL_FALSE` on failure.
 */
VL_API vl_bool_t vlLogAddStreamSink(vl_stream* stream);

/* ---------------- Debug compile control ----------------
 *
 * The logger provides macros for debug-only logs that compile out completely
 * when disabled.
 *
 * - Define `VL_ENABLE_DEBUG_LOG` to 1 to force-enable debug logging macros.
 * - Define `VL_ENABLE_DEBUG_LOG` to 0 to force-disable debug logging macros.
 * - If not defined, the default is: enabled when !NDEBUG.
 *
 * Important: To avoid cross-compiler issues with empty variadic macro arguments
 * in C11, debug and release logging macros are provided as two forms:
 * - `*_MSG0(msg)` : message-only (no variadic arguments)
 * - `*_MSGF(fmt, ...)` : formatted
 */

#ifndef VL_ENABLE_DEBUG_LOG
#if !defined(NDEBUG)
#define VL_ENABLE_DEBUG_LOG 1
#else
#define VL_ENABLE_DEBUG_LOG 0
#endif
#endif

#if VL_ENABLE_DEBUG_LOG

#include "vl_ansi_term.h"

/**
 * \def VL_LOGD_MSG0
 * \brief Debug-only message log routed through the global logger.
 */
#define VL_LOGD_MSG0(msg)                                                                                              \
    vlLogMessageF(VL_ANSI_FG_MAGENTA "[DBG | %s @ line %d] " VL_ANSI_RESET "%s", __FILE__, __LINE__, msg)

/**
 * \def VL_LOGD_MSGF
 * \brief Debug-only formatted log routed through the global logger.
 */
#define VL_LOGD_MSGF(fmt, ...)                                                                                         \
    vlLogMessageF(VL_ANSI_FG_MAGENTA "[DBG | %s @ line %d] " VL_ANSI_RESET fmt, __FILE__, __LINE__, __VA_ARGS__)

/**
 * \def VL_LOGD_ERR0
 * \brief Debug-only error log routed through the global logger.
 */
#define VL_LOGD_ERR0(msg)                                                                                              \
    vlLogErrorF(VL_ANSI_FG_YELLOW "[DBG | %s @ line %d] " VL_ANSI_RESET "%s", __FILE__, __LINE__, msg)

/**
 * \def VL_LOGD_ERRF
 * \brief Debug-only formatted error log routed through the global logger.
 */
#define VL_LOGD_ERRF(fmt, ...)                                                                                         \
    vlLogErrorF(VL_ANSI_FG_YELLOW "[DBG | %s @ line %d] " VL_ANSI_RESET fmt, __FILE__, __LINE__, __VA_ARGS__)
#else
#define VL_LOGD_MSG0(msg) ((void)0)
#define VL_LOGD_MSGF(fmt, ...) ((void)0)
#define VL_LOGD_ERR0(msg) ((void)0)
#define VL_LOGD_ERRF(fmt, ...) ((void)0)
#endif

/**
 * \def VL_LOG_MSG0
 * \brief Always-enabled message log routed through the global logger.
 */
#define VL_LOG_MSG0(msg) vlLogMessage(msg)

/**
 * \def VL_LOG_MSGF
 * \brief Always-enabled formatted message log routed through the global logger.
 */
#define VL_LOG_MSGF(fmt, ...) vlLogMessageF((fmt), __VA_ARGS__)

/**
 * \def VL_LOG_ERR0
 * \brief Always-enabled error log routed through the global logger.
 */
#define VL_LOG_ERR0(msg) vlLogError(msg)

/**
 * \def VL_LOG_ERRF
 * \brief Always-enabled formatted error log routed through the global logger.
 */
#define VL_LOG_ERRF(fmt, ...) vlLogErrorF((fmt), __VA_ARGS__)

#endif // VL_LOG_H
