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

#ifndef VL_STREAM_MEMORY_H
#define VL_STREAM_MEMORY_H

#include <vl/vl_buffer.h>
#include <vl/vl_memory.h>
#include <vl/vl_stream.h>

/**
 * \brief Creates a stream that reads/writes to a dynamic vl_buffer.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_stream` pointer and is responsible for calling `vlStreamDelete`. If
 * `takeOwnership` is `VL_TRUE`, the stream assumes ownership of the `vl_buffer` and will delete it (via
 * `vlBufferDelete`) when the stream is destroyed.
 * - **Lifetime**: The stream is valid until its reference count reaches zero. The `vl_buffer` must remain valid for the
 * duration of the stream's life if `takeOwnership` is `VL_FALSE`.
 * - **Thread Safety**: Thread-safe (the stream has its own internal mutex).
 * - **Nullability**: Returns `NULL` if `buffer` is `NULL`.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: Accessing the `vl_buffer` directly while it is being managed by a stream from multiple
 * threads without external synchronization.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_stream` struct, an internal context struct, and
 * synchronization primitives.
 * - **Return-value Semantics**: Returns a pointer to the new stream, or `NULL` if failure.
 *
 * \param buffer The buffer to use.
 * \param takeOwnership If VL_TRUE, the stream will delete the buffer when
 * closed.
 * \return A new stream object.
 */
VL_API vl_stream* vlStreamOpenBuffer(vl_buffer* buffer, vl_bool_t takeOwnership);

/**
 * \brief Creates a read-only stream from a raw memory block.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_stream` pointer. The memory block is NOT owned by the stream.
 * - **Lifetime**: The stream is valid until its reference count reaches zero. The underlying memory block must remain
 * valid and unchanged for the entire lifetime of the stream.
 * - **Thread Safety**: Thread-safe (internal mutex).
 * - **Nullability**: Returns `NULL` if `memory` is `NULL`.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: Modifying the underlying memory while the stream is active.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_stream` struct, an internal context struct, and
 * synchronization primitives.
 * - **Return-value Semantics**: Returns a pointer to the new read-only stream, or `NULL`.
 *
 * \param memory Pointer to the data.
 * \param size Size of the data in bytes.
 * \return A new stream object.
 */
VL_API vl_stream* vlStreamOpenMemory(const void* memory, vl_memsize_t size);

/**
 * \brief Creates a writable stream from a fixed-size memory block.
 * Writing past the end will fail.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_stream` pointer and the underlying memory block.
 * - **Lifetime**: The stream is valid until its reference count reaches zero. The underlying memory block must remain
 * valid for the entire lifetime of the stream.
 * - **Thread Safety**: Thread-safe (internal mutex).
 * - **Nullability**: Returns `NULL` if `memory` is `NULL`.
 * - **Error Conditions**: Returns `NULL` on allocation failure. Write operations will fail if they exceed the specified
 * `size`.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_stream` struct, an internal context struct, and
 * synchronization primitives.
 * - **Return-value Semantics**: Returns a pointer to the new writable stream, or `NULL`.
 *
 * \param memory Pointer to the writable data.
 * \param size Size of the data in bytes.
 * \return A new stream object.
 */
VL_API vl_stream* vlStreamOpenMemoryMutable(void* memory, vl_memsize_t size);

#endif // VL_STREAM_MEMORY_H
