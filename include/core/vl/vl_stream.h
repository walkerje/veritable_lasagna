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

#ifndef VL_STREAM_H
#define VL_STREAM_H

#include <vl/vl_atomic.h>
#include <vl/vl_memory.h>
#include <vl/vl_mutex.h>

/**
 * \brief Stream seek origin.
 * Analogous to SEEK_SET, SEEK_CUR, SEEK_END.
 */
typedef enum vl_stream_origin_
{
    VL_STREAM_SEEK_SET = 0,
    VL_STREAM_SEEK_CUR = 1,
    VL_STREAM_SEEK_END = 2
} vl_stream_origin;

// Function Pointer Definitions
typedef vl_memsize_t (*vl_stream_func_read)(void* buffer, vl_memsize_t size, void* user);
typedef vl_memsize_t (*vl_stream_func_write)(const void* buffer, vl_memsize_t size, void* user);
typedef vl_bool_t (*vl_stream_func_seek)(vl_int64_t offset, vl_stream_origin origin, void* user);
typedef vl_int64_t (*vl_stream_func_tell)(void* user);
typedef void (*vl_stream_func_flush)(void* user);
typedef void (*vl_stream_func_close)(void* user);

/**
 * \struct vl_stream
 * \brief Generic, thread-safe byte stream abstraction.
 *
 * A \c vl_stream represents a sequential byte-oriented data source or sink,
 * such as a file, memory buffer, network socket, or virtual stream. It provides
 * a uniform interface for reading, writing, seeking, and querying position,
 * independent of the underlying storage mechanism.
 *
 * The stream object itself owns no I/O resources directly. Instead, all actual
 * I/O behavior is delegated to a user-provided function table (v-table), with
 * backend-specific state supplied via the \c userData pointer.
 *
 * ### Thread Safety
 * All public I/O operations on a \c vl_stream are serialized via an internal
 * mutex. Concurrent calls from multiple threads are safe but will not execute
 * in parallel. Backend implementations are not required to be reentrant.
 *
 * Reference counting is atomic and allows streams to be safely shared across
 * subsystems. The stream is destroyed automatically when the reference count
 * reaches zero.
 *
 * ### Lifetime and Ownership
 * The stream object manages its own lifetime but does not assume ownership of
 * \c userData. When the stream is destroyed, the \c close callback (if
 * provided) is invoked, and the backend is responsible for releasing any
 * resources associated with \c userData.
 *
 * ### Statistics
 * The \c totalRead and \c totalWritten fields track the total number of bytes
 * successfully read from or written to the stream through this interface.
 * These counters are monotonic and updated atomically.
 *
 * Streams should be fully configured (v-table set) before being exposed to
 * other threads.
 */
typedef struct vl_stream_
{
    /**
     * \brief Reads bytes from the stream into a buffer.
     * Returns the number of bytes actually read.
     */
    vl_stream_func_read read;

    /**
     * \brief Writes bytes from a buffer to the stream.
     * Returns the number of bytes actually written.
     */
    vl_stream_func_write write;

    /**
     * \brief Moves the current stream position.
     */
    vl_stream_func_seek seek;

    /**
     * \brief Returns the current stream position, or -1 if unsupported.
     */
    vl_stream_func_tell tell;

    /**
     * \brief Flushes any buffered data to the underlying medium.
     */
    vl_stream_func_flush flush;

    /**
     * \brief Closes the stream backend and releases associated resources.
     */
    vl_stream_func_close close;

    /**
     * \brief Backend-specific context pointer.
     *
     * This pointer is passed verbatim to all v-table functions and is managed
     * by the backend implementation.
     */
    void* userData;

    /**
     * \brief Mutex protecting all I/O operations on this stream.
     */
    vl_mutex lock;

    /**
     * \brief Atomic reference count.
     *
     * When this count reaches zero, the stream is closed and freed.
     */
    vl_atomic_ularge_t refCount;

    /**
     * \brief Total number of bytes successfully read from the stream.
     */
    vl_atomic_ularge_t totalRead;

    /**
     * \brief Total number of bytes successfully written to the stream.
     */
    vl_atomic_ularge_t totalWritten;
} vl_stream;

/**
 * \brief Creates a new stream object on the heap.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_stream` pointer and is responsible for calling `vlStreamDelete`.
 * - **Lifetime**: The stream is valid until its reference count reaches zero via `vlStreamDelete`.
 * - **Thread Safety**: Thread-safe (initializes internal mutex and atomics).
 * - **Nullability**: Returns `NULL` if heap allocation for the stream or its internal mutex fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_stream` struct and a `vl_mutex`.
 * - **Return-value Semantics**: Returns a pointer to the new stream with an initial reference count of 1, or `NULL`.
 *
 * \param userData pointer to backend-specific context.
 * \return pointer to new stream, or NULL.
 */
VL_API vl_stream* vlStreamNew(void* userData);

/**
 * \brief Decrements reference count and potentially deletes the stream.
 *
 * If the reference count reaches zero, the `close` callback is invoked, and all associated memory (including the
 * internal mutex and the stream struct itself) is freed.
 *
 * ## Contract
 * - **Ownership**: Releases the caller's reference. If it's the last reference, all ownership is released.
 * - **Lifetime**: The stream pointer becomes invalid if the reference count reaches zero.
 * - **Thread Safety**: Thread-safe (uses atomic decrement and CAS loop).
 * - **Nullability**: Safe to call with `NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion beyond the number of retains.
 * - **Memory Allocation Expectations**: Deallocates the stream struct and its internal mutex.
 * - **Return-value Semantics**: None (void).
 *
 * \param stream Pointer to the stream to delete.
 */
VL_API void vlStreamDelete(vl_stream* stream);

/**
 * \brief Increments the reference count of the stream.
 *
 * ## Contract
 * - **Ownership**: The caller gains a shared reference to the stream.
 * - **Lifetime**: Ensures the stream remains valid until the corresponding `vlStreamDelete` call.
 * - **Thread Safety**: Thread-safe (uses atomic increment).
 * - **Nullability**: Safe to call with `NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param stream Pointer to the stream to retain.
 */
VL_API void vlStreamRetain(vl_stream* stream);

//-----------------------------------------------------------------------------
// V-Table Configuration (Setters)
//-----------------------------------------------------------------------------

VL_API void vlStreamSetRead(vl_stream* stream, vl_stream_func_read func);
VL_API void vlStreamSetWrite(vl_stream* stream, vl_stream_func_write func);
VL_API void vlStreamSetSeek(vl_stream* stream, vl_stream_func_seek func);
VL_API void vlStreamSetTell(vl_stream* stream, vl_stream_func_tell func);
VL_API void vlStreamSetFlush(vl_stream* stream, vl_stream_func_flush func);
VL_API void vlStreamSetClose(vl_stream* stream, vl_stream_func_close func);

//-----------------------------------------------------------------------------
// I/O Operations (Thread Safe)
//-----------------------------------------------------------------------------

/**
 * \brief Reads data from the stream.
 *
 * ## Contract
 * - **Ownership**: Does not transfer ownership.
 * - **Lifetime**: `stream` and `outBuffer` must be valid for the duration of the call.
 * - **Thread Safety**: Thread-safe (serialized via internal mutex).
 * - **Nullability**: Returns 0 if `stream`, `stream->read`, or `outBuffer` is `NULL`.
 * - **Error Conditions**: Returns 0 if no read function is configured.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the number of bytes successfully read and updates the `totalRead` counter.
 *
 * \param stream pointer to the stream.
 * \param outBuffer pointer to the destination buffer.
 * \param outLength maximum number of bytes to read.
 * \return Number of bytes actually read.
 */
VL_API vl_memsize_t vlStreamRead(vl_stream* stream, void* outBuffer, vl_memsize_t outLength);

/**
 * \brief Writes data to the stream.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: `stream` and `inBuffer` must be valid.
 * - **Thread Safety**: Thread-safe (serialized via internal mutex).
 * - **Nullability**: Returns 0 if `stream`, `stream->write`, or `inBuffer` is `NULL`.
 * - **Error Conditions**: Returns 0 if no write function is configured.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the number of bytes successfully written and updates the `totalWritten`
 * counter.
 *
 * \param stream pointer to the stream.
 * \param inBuffer pointer to the source buffer.
 * \param inLength number of bytes to write.
 * \return Number of bytes actually written.
 */
VL_API vl_memsize_t vlStreamWrite(vl_stream* stream, const void* inBuffer, vl_memsize_t inLength);

/**
 * \brief Moves the stream position.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (serialized via internal mutex).
 * - **Nullability**: Returns `VL_FALSE` if `stream` is `NULL`.
 * - **Error Conditions**: Returns `VL_FALSE` if the backend `seek` function fails or is not provided.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns `VL_TRUE` if the seek operation was successful, `VL_FALSE` otherwise.
 *
 * \param stream pointer to the stream.
 * \param offset offset relative to the specified origin.
 * \param origin seek origin (beginning, current, or end).
 */
VL_API vl_bool_t vlStreamSeek(vl_stream* stream, vl_int64_t offset, vl_stream_origin origin);

/**
 * \brief Returns the current position in the stream.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (serialized via internal mutex).
 * - **Nullability**: Returns -1 if `stream` is `NULL`.
 * - **Error Conditions**: Returns -1 if the backend `tell` function fails or is not provided.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the current byte offset from the beginning of the stream, or -1 on error.
 *
 * \param stream pointer to the stream.
 * \return current byte offset, or -1 on error.
 */
VL_API vl_int64_t vlStreamTell(vl_stream* stream);

/**
 * \brief Flushes any buffered data to the underlying device.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Thread-safe (serialized via internal mutex).
 * - **Nullability**: Safe to call with `NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param stream pointer to the stream.
 */
VL_API void vlStreamFlush(vl_stream* stream);

#endif // VL_STREAM_H
