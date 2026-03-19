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

#ifndef VL_STREAM_FILESYS_H
#define VL_STREAM_FILESYS_H

#include <vl/vl_filesys.h>
#include <vl/vl_stream.h>

/**
 * \brief Opens a file stream using a vl_filesys_path object.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_stream` pointer and is responsible for calling `vlStreamDelete`.
 * The stream manages the internal `FILE*` and closes it on deletion.
 * - **Lifetime**: The stream is valid until its reference count reaches zero.
 * - **Thread Safety**: Thread-safe (the returned stream has an internal mutex).
 * - **Nullability**: Returns `NULL` if `path` or `mode` is `NULL`, or if the file cannot be opened.
 * - **Error Conditions**: Returns `NULL` if `fopen` (or platform equivalent) fails or if heap allocation fails.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_stream` struct, an internal context struct, and
 * synchronization primitives.
 * - **Return-value Semantics**: Returns a pointer to the new stream, or `NULL` if the file could not be opened.
 *
 * \param path The path object containing the UTF-8 path.
 * \param mode The standard C fopen mode string (e.g., "rb", "wb", "r+").
 * \return A new stream object, or NULL if the file could not be opened.
 */
VL_API vl_stream* vlStreamOpenFile(const vl_filesys_path* path, const char* mode);

/**
 * \brief Opens a file stream using a raw UTF-8 string path.
 *
 * This is a convenience wrapper that creates a temporary path object if needed
 * to handle platform-specific encoding conversion.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_stream` pointer.
 * - **Lifetime**: The stream is valid until its reference count reaches zero.
 * - **Thread Safety**: Thread-safe.
 * - **Nullability**: Returns `NULL` if `pathStr` or `mode` is `NULL`. `sys` can be `NULL`.
 * - **Error Conditions**: Same as `vlStreamOpenFile`.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Same as `vlStreamOpenFile`.
 * - **Return-value Semantics**: Returns a pointer to the new stream, or `NULL` if failure.
 *
 * \param sys The filesystem context (can be NULL if not using arena/pool
 * features for this op).
 * \param pathStr The UTF-8 path string.
 * \param mode The standard C fopen mode string.
 * \return A new stream object, or NULL if failure.
 */
VL_API vl_stream* vlStreamOpenFileStr(vl_filesys* sys, const char* pathStr, const char* mode);

#endif // VL_STREAM_FILESYS_H
