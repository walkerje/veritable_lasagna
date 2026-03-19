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

#ifndef VL_BUFFER_H
#define VL_BUFFER_H

#include "vl_memory.h"
#include "vl_numtypes.h"

/**
 * \brief A multi-purpose byte buffer.
 *
 * Buffers are the most basic data structure used by the VL library (aside from
 * simple blocks of memory) Every other data structure uses a vl_buffer
 * somewhere in their implementation. Most comparable to a standard Vector or
 * ArrayList, the buffer is capable of resizing itself according to write
 * operations by doubling its size for new elements.
 *
 * Buffers do offer stateful functionality used to treat them as streams (e.g,
 * seeking). Unless otherwise specified, the default initial capacity is equal
 * to VL_DEFAULT_MEMORY_SIZE.
 */
typedef struct
{
    /**
     * \brief Virtual size of the buffer. Actual capacity of the allocation is
     * always equal to or larger.
     */
    vl_memsize_t size;

    /**
     * \brief Read-write offset of the buffer.
     */
    vl_dsoffs_t offset;

    /**
     * \brief Actual allocation managed by the buffer.
     */
    vl_memory* data;
} vl_buffer;

/**
 * \brief Allocates a new buffer, and initializes it with the specified capacity.
 * The buffer must be deleted with vlBufferDelete(...) when it is no longer
 * being used to avoid leaking memory.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_buffer` pointer and is responsible for calling `vlBufferDelete`.
 * - **Lifetime**: The buffer is valid until it is passed to `vlBufferDelete`.
 * - **Thread Safety**: Not thread-safe. Concurrent access to the same buffer must be synchronized.
 * - **Nullability**: Returns `NULL` if heap allocation for the `vl_buffer` struct fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_buffer` struct and then uses `vlMemAllocAligned`
 * for the data portion.
 * - **Return-value Semantics**: Returns a pointer to the newly allocated and initialized buffer, or `NULL`.
 *
 * \par Complexity O(1) constant
 * \param size initial capacity N
 * \param align byte-level alignment of the allocated memory
 * \return pointer to buffer.
 */
VL_API vl_buffer* vlBufferNewExt(vl_memsize_t size, vl_uint16_t align);

/**
 * \brief Allocates a new buffer, and initializes it with the default capacity.
 * The buffer must be deleted with vlBufferDelete(...) when it is no longer
 * being used to avoid leaking memory.
 * \par Complexity O(1) constant
 * \return pointer to buffer.
 */
static inline vl_buffer* vlBufferNew(void) { return vlBufferNewExt(VL_DEFAULT_MEMORY_SIZE, VL_DEFAULT_MEMORY_ALIGN); }

/**
 * \brief Initializes a buffer instance with specific size and alignment.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the `buffer` struct. The function manages the internal data
 * allocation.
 * - **Lifetime**: The `buffer` struct must remain valid as long as it is in use. Internal data is valid until
 * `vlBufferFree` or `vlBufferDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: If `buffer` is `NULL`, the function returns immediately (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an already initialized buffer without first calling `vlBufferFree` (causes memory
 * leak).
 * - **Memory Allocation Expectations**: Uses `vlMemAllocAligned` to allocate the initial data block.
 * - **Return-value Semantics**: None (void).
 *
 * \param buffer pointer to buffer
 * \param size initial capacity N
 * \param align byte-level alignment of the allocated memory
 * \par Complexity O(1) constant
 */
VL_API void vlBufferInitExt(vl_buffer* buffer, vl_memsize_t size, vl_uint16_t align);

/**
 * \brief Initializes a buffer instance for the first time.
 * This will create the initial allocation for the specified capacity.
 *
 * This buffer should be deleted via vlBufferFree.
 *
 * \sa vlBufferFree
 * \param buffer struct pointer
 * \param initialCapacity total number of initial bytes the buffer will have the
 * capacity to hold.
 * \par Complexity O(1) constant
 */
static inline void vlBufferInit(vl_buffer* buffer)
{
    vlBufferInitExt(buffer, VL_DEFAULT_MEMORY_SIZE, VL_DEFAULT_MEMORY_ALIGN);
}

/**
 * \brief Resets the state of the specified buffer, setting the offset integer
 * to zero and the computed size to zero. This will re-allocate the buffer to hold the specified capacity.
 *
 * ## Contract
 * - **Ownership**: Ownership of the buffer remains unchanged.
 * - **Lifetime**: The buffer remains valid. Existing data may be lost or overwritten.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `buffer` must not be `NULL`.
 * - **Error Conditions**: If `vlMemRealloc` fails, `buffer->data` may become `NULL`.
 * - **Undefined Behavior**: Passing a pointer to an uninitialized buffer.
 * - **Memory Allocation Expectations**: Uses `vlMemRealloc` to change the capacity of the underlying data block.
 * - **Return-value Semantics**: None (void).
 *
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \param newCapacity new capacity, in bytes
 */
VL_API void vlBufferReset(vl_buffer* buffer, vl_memsize_t newCapacity);

/**
 * \brief Sets the entirety of the buffer to zero.
 * Also resets offset and computed size to zero.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `buffer` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \par Complexity O(n) where n = buffer capacity
 * \param buffer struct pointer
 */
VL_API void vlBufferClear(vl_buffer* buffer);

/**
 * \brief Resizes the specified buffer to hold a capacity equal to the current
 * computed size.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `buffer` must not be `NULL`.
 * - **Error Conditions**: If `vlMemRealloc` fails, the buffer's data pointer may be lost.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Uses `vlMemRealloc` to shrink the data block.
 * - **Return-value Semantics**: None (void).
 *
 * \par Complexity O(n) where n = new buffer capacity
 * \param buffer struct pointer
 */
VL_API void vlBufferShrinkToFit(vl_buffer* buffer);

/**
 * \brief Clones the source buffer to the destination buffer.
 *
 * This is a 'complete' clone of the entirety of the buffer, and thus includes
 * all related state (offset, size) alongside a copy of the underlying block of memory.
 *
 * When simply copying data from one buffer to another, consider the more
 * stateful vlBufferCopy.
 *
 * The 'src' buffer must be non-null and point to an initialized buffer.
 * The 'dest' buffer must be either null, or a pointer to an initialized buffer.
 *
 * If the 'dest' buffer is null, a new instance is created via vlBufferNew.
 * It must later be disposed using vlBufferDelete if this is the case.
 *
 * ## Contract
 * - **Ownership**: If `dest` is `NULL`, the caller owns the returned `vl_buffer`. If `dest` is provided, ownership
 * remains with the caller.
 * - **Lifetime**: The `dest` buffer is valid until `vlBufferDelete` (if newly allocated) or `vlBufferFree`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` must not be `NULL`. `dest` can be `NULL`.
 * - **Error Conditions**: Returns `NULL` if new buffer allocation fails when `dest` is `NULL`.
 * - **Undefined Behavior**: Passing an uninitialized buffer.
 * - **Memory Allocation Expectations**: May allocate a new `vl_buffer` struct and/or a new data block via
 * `vlMemAllocAligned` or `vlMemRealloc`.
 * - **Return-value Semantics**: Returns the pointer to the cloned buffer (`dest` or the new instance).
 *
 * \sa vlBufferNew
 * \sa vlBufferCopy
 * \param src source buffer pointer
 * \param dest destination buffer pointer, or NULL.
 * \par Complexity O(n) where n = buffer capacity.
 * \return 'dest' buffer, or buffer created via vlBufferNew.
 */
VL_API vl_buffer* vlBufferClone(const vl_buffer* src, vl_buffer* dest);

/**
 * \brief Copies a series of bytes from one buffer to another.
 *
 * The read from the src buffer and the write to the dest buffer
 * are relative to their current respective offsets.
 *
 * If there are less bytes available in the source buffer than those specified
 * by len, the copy is performed up to the end of the source.
 *
 * ## Contract
 * - **Ownership**: Ownership of buffers is unchanged.
 * - **Lifetime**: Both buffers must be valid during the call.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` and `dest` must not be `NULL`.
 * - **Error Conditions**: Same as `vlBufferWrite` regarding potential `dest` allocation failures.
 * - **Undefined Behavior**: Passing uninitialized buffers.
 * - **Memory Allocation Expectations**: May trigger expansion of the `dest` buffer.
 * - **Return-value Semantics**: Returns the total number of bytes successfully copied from `src` to `dest`.
 *
 * \param src source buffer pointer
 * \param dest destination buffer pointer
 * \param len total number of bytes to attempt to copy.
 * \par Complexity O(n) where n = len
 * \return total number of bytes copied from src to dest.
 */
VL_API vl_memsize_t vlBufferCopy(vl_buffer* src, vl_buffer* dest, vl_memsize_t len);

/**
 * \brief Performs a copy from the specified source pointer into the buffer.
 * The bytes are written at the current offset integer belonging to the buffer
 * state. This function will resize the buffer when necessary, doubling capacity iteratively until it fits.
 * The buffer offset will be incremented by the total number of bytes written.
 *
 * Providing a NULL source pointer will reserve space in the buffer without
 * copying any data. Existing data within the reserved range is not modified,
 * and can be directly written to.
 *
 * ## Contract
 * - **Ownership**: Ownership of `buffer` and `src` is unchanged.
 * - **Lifetime**: Both must be valid during the call.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `buffer` must not be `NULL`. `src` can be `NULL` to reserve space.
 * - **Error Conditions**: If internal reallocation fails, `buffer->data` may become invalid or `NULL`.
 * - **Undefined Behavior**: Passing an uninitialized buffer.
 * - **Memory Allocation Expectations**: Automatically grows the buffer capacity (doubling) if the write exceeds current
 * capacity.
 * - **Return-value Semantics**: Returns the offset at which the write started.
 *
 * \par Complexity O(n) where n = param size
 * \param buffer struct pointer
 * \param size total number of bytes to write
 * \param src pointer to the memory that will be copied from
 * \return the offset at which the bytes were written into the buffer.
 */
VL_API vl_memsize_t vlBufferWrite(vl_buffer* buffer, vl_memsize_t size, const void* src);

/**
 * \brief Copies bytes from the buffer to the specified destination.
 * Performs a copy from the buffer to the specified destination pointer.
 * This will increment the buffer offset.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Buffer and `dest` must be valid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `buffer` and `dest` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: If the requested `size` exceeds available data, the function may still attempt to copy
 * `size` bytes, potentially reading into uninitialized capacity beyond `buffer->size`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the number of bytes that were within the valid data range of the buffer (up to
 * `buffer->size`).
 *
 * \par Complexity O(n) where n = param size
 * \param buffer struct pointer
 * \param size total number of bytes to attempt to copy
 * \param dest pointer to the memory that will be copied to
 * \return actual number of bytes copied
 */
VL_API vl_memsize_t vlBufferRead(vl_buffer* buffer, vl_memsize_t size, void* dest);

/**
 * \brief Sets the buffer offset relative to the beginning of the allocation.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `buffer` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Setting an offset beyond the current capacity may lead to undefined behavior on subsequent
 * reads or writes.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \param offset total number of bytes, positive, to seek forward.
 */
static inline void vlBufferSeek(vl_buffer* buffer, vl_uintptr_t offset) { buffer->offset = offset; }

/**
 * \brief Seeks the internal offset relative to the current offset.
 * Adds the specified offset to the offset integer belonging to the buffer
 * state.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `buffer` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Seeking to a negative offset or beyond current capacity.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \param offset total number of bytes, positive or negative, to seek.
 */
static inline void vlBufferSeekRelative(vl_buffer* buffer, vl_intptr_t offset) { buffer->offset += offset; }

/**
 * \brief Seeks the internal offset to the beginning of the buffer.
 * Moves the offset integer belonging to the buffer state to the beginning of
 * the allocation (e.g, zero)
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `buffer` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 */
static inline void vlBufferSeekBegin(vl_buffer* buffer) { buffer->offset = 0; }

/**
 * \brief Seeks the internal offset to the end of the buffer.
 * Moves the offset integer belonging to the buffer state to the end of the
 * allocation (e.g, the buffer size)
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `buffer` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \par Complexity O(1) constant
 * \sa vlBufferEnd
 * \param buffer struct pointer
 */
static inline void vlBufferSeekEnd(vl_buffer* buffer) { buffer->offset = buffer->size; }

/**
 * \brief Returns a pointer to the beginning of the buffer allocation
 *
 * ## Contract
 * - **Ownership**: None. The caller must not free the returned pointer.
 * - **Lifetime**: The pointer is valid only until the next buffer reallocation (e.g., `vlBufferWrite`, `vlBufferReset`)
 * or until the buffer is deleted/freed.
 * - **Thread Safety**: Thread-safe for read access as long as no other thread is writing to or reallocating the buffer.
 * - **Nullability**: `buffer` must not be `NULL`. Returns `NULL` if the buffer has no data allocation.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Using the pointer after the buffer has been reallocated or deleted.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a pointer to the first byte of the data allocation.
 *
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \return pointer to the first byte of the allocation
 */
static inline vl_transient* vlBufferBegin(vl_buffer* buffer) { return buffer->data; }

/**
 * \brief Returns a pointer to the end of the buffer allocation
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Same as `vlBufferBegin`.
 * - **Thread Safety**: Same as `vlBufferBegin`.
 * - **Nullability**: `buffer` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Using the pointer after reallocation or deletion.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a pointer to one byte past the last valid data byte in the buffer.
 *
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \return pointer to one past the last byte of the allocation
 */
static inline vl_transient* vlBufferEnd(vl_buffer* buffer) { return buffer->data + buffer->size; }

/**
 * \brief Frees the internal data of the specified buffer.
 *
 * The buffer should have been initialized via vlBufferInit(Ext).
 * This function does NOT free the `vl_buffer` struct itself.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal data block. The caller still owns the `vl_buffer` struct.
 * - **Lifetime**: The internal data becomes invalid. The `vl_buffer` struct remains valid but is in an uninitialized
 * state.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Safe to call if `buffer` is `NULL` (due to `vlMemFree` check).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double free of internal data.
 * - **Memory Allocation Expectations**: Deallocates the internal data block.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlBufferInit
 * \sa vlBufferInitExt
 * \param buffer struct pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlBufferFree(vl_buffer* buffer);

/**
 * \brief Deletes the specified buffer and its internal data.
 *
 * The buffer should have been initialized via vlBufferNew(Ext) or
 * vlBufferClone. This function frees both the internal data and the `vl_buffer` struct itself.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of both the internal data block and the `vl_buffer` struct.
 * - **Lifetime**: Both the buffer and its pointer become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Safe to call if `buffer` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Deallocates the internal data block and the `vl_buffer` struct.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlBufferNew
 * \sa vlBufferNewExt
 * \sa vlBufferClone
 * \param buffer struct pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlBufferDelete(vl_buffer* buffer);

#endif // VL_BUFFER_H
