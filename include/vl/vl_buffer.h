#ifndef VL_BUFFER_H
#define VL_BUFFER_H

#include "vl_numtypes.h"
#include "vl_memory.h"

/**
 * \brief A multi-purpose byte buffer.
 *
 * Buffers are the most basic data structure used by the VL library (aside from simple blocks of memory)
 * Every other data structure uses a vl_buffer somewhere in their implementation.
 * Most comparable to a standard Vector or ArrayList, the buffer
 * is capable of resizing itself according to write operations
 * by doubling its size for new elements.
 *
 * Buffers do offer stateful functionality used to treat them as streams (e.g, seeking).
 * Unless otherwise specified, the default initial capacity is equal to VL_DEFAULT_MEMORY_SIZE.
 */
typedef struct {
    /**
     * \brief Virtual size of the buffer. Actual capacity of the allocation is always equal to or larger.
     */
    vl_memsize_t size;

    /**
     * \brief Read-write offset of the buffer.
     */
    vl_dsoffs_t offset;

    /**
      * \brief Actual allocation managed by the buffer.
      */
    vl_memory *data;
} vl_buffer;

/**
 * \brief Allocates a new buffer, and initializes it with the default capacity.
 * The buffer must be deleted with vlBufferDelete(...) when it is no longer being used to avoid leaking memory.
 * \par Complexity O(1) constant
 * \return pointer to buffer.
 */
VL_API vl_buffer *vlBufferNew(void);

/**
 * \brief Allocates a new buffer, and initializes it with a capacity of N bytes.
 * The buffer must be deleted with vlBufferDelete(...) when it is no longer being used to avoid leaking memory.
 * \par Complexity O(1) constant
 * \param initialCapacity initial capacity N
 * \return pointer to buffer.
 */
VL_API vl_buffer *vlBufferNewSz(vl_memsize_t initialCapacity);

#ifndef vlBufferInit
/**
 * \brief Initializes a buffer instance for the first time.
 * This will create the initial allocation for the default capacity.
 *
 * This buffer should be deleted via vlBufferFree.
 *
 * \sa vlBufferFree
 * \param buffer struct pointer
 * \par Complexity O(1) constant
 */
#define vlBufferInit(bufferPtr) vlBufferInitSz(bufferPtr, VL_DEFAULT_MEMORY_SIZE)
#endif

/**
 * \brief Initializes a buffer instance for the first time.
 * This will create the initial allocation for the specified capacity.
 *
 * This buffer should be deleted via vlBufferFree.
 *
 * \sa vlBufferFree
 * \param buffer struct pointer
 * \param initialCapacity total number of initial bytes the buffer will have the capacity to hold.
 * \par Complexity O(1) constant
 */
VL_API void vlBufferInitSz(vl_buffer *buffer, vl_memsize_t initialCapacity);

/**
 * \brief Resets the state of the specified buffer, setting the offset integer to zero.
 * This will re-allocate the buffer to hold the default capacity.
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 */
VL_API void vlBufferReset(vl_buffer *buffer);

/**
 * \brief Resets the state of the specified buffer, setting the offset integer to zero.
 * This will re-allocate the buffer to hold the specified capacity.
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \param newCapacity new capacity, in bytes
 */
VL_API void vlBufferResetSz(vl_buffer *buffer, vl_memsize_t newCapacity);

/**
 * \brief Sets the entirety of the buffer to zero.
 * Also resets offset and computed size.
 * \par Complexity O(n) where n = buffer capacity
 * \param buffer struct pointer
 */
VL_API void vlBufferClear(vl_buffer *buffer);

/**
 * \brief Resizes the specified buffer to hold a capacity equal to the current size.
 * \par Complexity O(n) where n = new buffer capacity
 * \param buffer struct pointer
 */
VL_API void vlBufferShrinkToFit(vl_buffer *buffer);

/**
 * \brief Clones the source buffer to the destination buffer.
 *
 * This is a 'complete' clone of the entirety of the buffer, and thus includes all related state
 * alongside a copy of the underlying block of memory.
 *
 * When simply copying data from one buffer to another, consider the more stateful vlBufferCopy.
 *
 * The 'src' buffer must be non-null and point to an initialized buffer.
 * The 'dest' buffer must be either null, or a pointer to an initialized buffer.
 *
 * If the 'dest' buffer is null, a new instance is created via vlBufferNew.
 * It must later be disposed using vlBufferDelete if this is the case.
 *
 * \sa vlBufferNew
 * \sa vlBufferCopy
 * \param src source buffer pointer
 * \param dest destination buffer pointer, or NULL.
 * \par Complexity O(n) where n = buffer capacity.
 * \return 'dest' buffer, or buffer created via vlBufferNew.
 */
VL_API vl_buffer *vlBufferClone(const vl_buffer *src, vl_buffer *dest);

/**
 * \brief Copies a series of bytes from one buffer to another.
 *
 * The read from the src buffer and the write to the dest buffer
 * are relative to their current respective offsets.
 *
 * If there are less bytes available in the source buffer than those specified by len,
 * the copy is performed up to the end of the source.
 *
 * \param src source buffer pointer
 * \param dest destination buffer pointer
 * \param len total number of bytes to attempt to copy.
 * \par Complexity O(n) where n = len
 * \return total number of bytes copied from src to dest.
 */
VL_API vl_memsize_t vlBufferCopy(vl_buffer *src, vl_buffer *dest, vl_memsize_t len);

/**
 * \brief Performs a copy from the specified source pointer into the buffer.
 * The bytes are written at the current offset integer belonging to the buffer state.
 * This function will resize the buffer when necessary.
 * The buffer offset will be incremented by the total number of bytes written.
 * \par Complexity O(n) where n = param size
 * \param buffer struct pointer
 * \param size total number of bytes to write
 * \param src pointer to the memory that will be copied from
 * \return the offset at which the bytes were written into the buffer.
 */
VL_API vl_memsize_t vlBufferWrite(vl_buffer *buffer, vl_memsize_t size, const void *src);

/**
 * \brief Copies bytes from the buffer to the specified destination.
 * Performs a copy from the buffer to the specified destination pointer.
 * This will increment the buffer offset.
 * \par Complexity O(n) where n = param size
 * \param buffer struct pointer
 * \param size total number of bytes to attempt to copy
 * \param dest pointer to the memory that will be copied to
 * \return actual number of bytes copied
 */
VL_API vl_memsize_t vlBufferRead(vl_buffer *buffer, vl_memsize_t size, void *dest);

/**
 * \brief Sets the buffer offset relative to the beginning of the allocation.
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \param offset total number of bytes, positive, to seek forward.
 */
VL_API void vlBufferSeek(vl_buffer *buffer, vl_uintptr_t offset);

/**
 * \brief Seeks the internal offset relative to the current offset.
 * Adds the specified offset to the offset integer belonging to the buffer state.
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \param offset total number of bytes, positive or negative, to seek.
 */
VL_API void vlBufferSeekRelative(vl_buffer *buffer, vl_intptr_t offset);

/**
 * \brief Seeks the internal offset to the beginning of the buffer.
 * Moves the offset integer belonging to the buffer state to the beginning of the allocation (e.g, zero)
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 */
VL_API void vlBufferSeekBegin(vl_buffer *buffer);

/**
 * \brief Seeks the internal offset to the end of the buffer.
 * Moves the offset integer belonging to the buffer state to the end of the allocation (e.g, the buffer size)
 * \par Complexity O(1) constant
 * \sa vlBufferEnd
 * \param buffer struct pointer
 */
VL_API void vlBufferSeekEnd(vl_buffer *buffer);

/**
 * \brief Returns a pointer to the beginning of the buffer allocation
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \return pointer to the first byte of the allocation
 */
VL_API vl_transient *vlBufferBegin(vl_buffer *buffer);

/**
 * \brief Returns a pointer to the end of the buffer allocation
 * \par Complexity O(1) constant
 * \param buffer struct pointer
 * \return pointer to one past the last byte of the allocation
 */
VL_API vl_transient *vlBufferEnd(vl_buffer *buffer);

/**
 * \brief Frees the specified buffer.
 *
 * The buffer should have been initialized via vlBufferInit(Sz).
 *
 * \sa vlBufferInit
 * \sa vlBufferInitSz
 * \param buffer struct pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlBufferFree(vl_buffer *buffer);

/**
 * \brief Deletes the specified buffer.
 *
 * The buffer should have been initialized via vlBufferNew(Sz) or vlBufferClone.
 *
 * \sa vlBufferNew
 * \sa vlBufferClone
 * \param buffer struct pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlBufferDelete(vl_buffer *buffer);

#endif //VL_BUFFER_H
