/**
 * \file
 * The vl_alloc header defines an interface to allocate and delete blocks of memory allocated on the system heap.
 * The purpose behind this is three fold:
 *  - To provide a consistent, in-library, and well-documented facility to almost all memory allocation within the library.
 *  - To provide a way to track all functions which might eventually allocate, reallocate, or delete memory.
 *  - To provide metadata regarding size and explicit alignment of these allocations.
 */

#ifndef VL_ALLOC_H
#define VL_ALLOC_H

#include <stdalign.h>
#include <malloc.h>
#include "vl_numtypes.h"
#include "vl_compare.h"

typedef VL_MEMORY_SIZE_T    vl_memsize_t;

#ifndef VL_KB
/**
 * Convenience macro to define blocks of memory which contain a multiple of X-many kilobytes.
 * \param x total kilobytes.
 */
#define VL_KB(x)   ((vl_memsize_t) (x) << 10)
#endif

#ifndef VL_MB
/**
 * Convenience macro to define blocks of memory which contain a multiple of X-many megabytes.
 * \param x total megabytes.
 */
#define VL_MB(x)   ((vl_memsize_t) (x) << 20)
#endif

#ifndef VL_DEFAULT_MEMORY_SIZE
/**
 * \brief Default 1kb allocation size.
 */
#define VL_DEFAULT_MEMORY_SIZE VL_KB(1)
#endif

#ifndef VL_DEFAULT_MEMORY_ALIGN
/**
 * \brief Default memory alignment. Defaults to maximum system word size.
 */
#define VL_DEFAULT_MEMORY_ALIGN alignof(vl_ularge_t)
#endif

#ifndef VL_MEMORY_PAD_UP
/**
 * \brief Calculate the next offset such that it is a multiple of an alignment.
 *
 * This will return len when already a multiple of pad.
 *
 * \par len size of the memory block
 * \par pad bytes to pad it to.
 * \return len
 */
#define VL_MEMORY_PAD_UP(len, pad) (vl_ularge_t)((len) + ((len) % (pad)))
#endif

/**
 * The typedef for vl_memory is simply void.
 * This is intended to improve code readability and intent.
 *
 * This is used only to indicate blocks of memory allocated through vlMemAlloc(/Aligned) and vlMemRealloc.
 *
 * \sa vlMemAlloc
 * \sa vlMemAllocAligned
 * \sa vlMemRealloc
 *
 * vl_memory pointers have a header associated with them.
 * This allows for implicit size tracking, among other things.
 */
typedef VL_MEMORY_T vl_memory;

/**
 * The typedef for vl_transient is, similarly, void.
 * This is intended to improve code readability and intent.
 *
 * This is used to indicate pointers to blocks of memory which that might be moved, deleted,
 * or erased through some operation on a data structure or allocator.
 */
typedef VL_MEMORY_T vl_transient;

/**
 * \brief Attempts to allocate a block of memory.
 *
 * Returns NULL on failure.
 *
 * \param allocSize size of the allocation, in bytes.
 * \return pointer to allocated block, or NULL.
 */
 vl_memory*  vlMemAlloc(vl_memsize_t allocSize);

/**
 * \brief Reallocates the specified block of memory to hold the specified total number of bytes.
 *
 * If the specified memory block is explicitly aligned, its alignment is preserved.
 *
 * \param mem pointer to block
 * \param allocSize new size of the allocation.
 * \return pointer to reallocated memory
 */
vl_memory*  vlMemRealloc(vl_memory* mem, vl_memsize_t allocSize);

/**
 * \brief Allocates a block of memory with an alignment.
 *
 * allocSize must be >= align or align must be < sizeof(uintptr_t), or this will return NULL.
 * The alignment must be a power of 2 (16, 32, 64, etc), otherwise the behavior is undefined.
 *
 * Guarantees that the returned pointer will have an value that is a multiple of the specified alignment.
 *
 * The VL_MEMORY_PAD_UP macro may be used to ensure that the actual length
 * of the memory block is also a multiple of the alignment.
 *
 * \sa VL_MEMORY_PAD_UP
 *
 * \param allocSize size of the allocation, in bytes.
 * \param align
 * \return pointer to the aligned block
 */
vl_memory*  vlMemAllocAligned(vl_memsize_t allocSize, vl_uint_t align);

/**
 * \brief Clones the specified block of memory, returning a pointer to its new clone.
 *
 * Returns NULL on failure.
 *
 * If the source block has an alignment, the result will also have an alignment.
 *
 * \param mem pointer
 * \return cloned memory pointer
 */
vl_memory*          vlMemClone(vl_memory* mem);

/**
 * \brief Returns the size (in total number of bytes) of the specified block of vl_memory.
 * \par mem pointer to memory block
 * \return size of the specified memory block, in bytes.
 */
vl_memsize_t        vlMemSize(vl_memory* mem);

/**
 * \brief Returns the alignment of the specified block of memory.
 *
 * Minimum alignment is defined as VL_DEFAULT_MEMORY_ALIGN, or the largest available word size.
 *
 * \param mem pointer to memory block
 * \return alignment
 */
vl_uint_t           vlMemAlignment(vl_memory* mem);

/**
 * \brief Sorts the specified buffer in-place according to the specified element and comparator function.
 *
 * This function implements an iterative Quicksort, allocating a temporary workspace on either the stack
 * or the heap depending on the size of the data being sorted.
 *
 * \param buffer
 * \param elementSize
 * \param numElements
 * \param comparator
 * \par Complexity of O(n log(n)) (space complexity of O(n)).
 */
void        vlMemSort(void* buffer, vl_memsize_t elementSize, vl_dsidx_t numElements, vl_compare_function comparator);

/**
 * \brief Copies data from one buffer to another, with a stride applied to both.
 *
 * Stride is the amount of space (in bytes) between each element.
 *
 * \param src memory block pointer
 * \param srcStride total number of bytes between each element in "src"
 * \param dest memory block pointer
 * \param dstStride total number of bytes between each element in "dest"
 * \param elementSize total number of bytes wide of each element
 * \param numElements total number of elements
 * \par Complexity of O(n) linear.
 */
void        vlMemCopyStride(const void* src, vl_dsoffs_t srcStride, void* dest, vl_dsoffs_t dstStride, vl_memsize_t elementSize, vl_dsidx_t numElements);

/**
 * \brief Frees the specified block of memory.
 * \param mem pointer to block.
 */
void        vlMemFree(vl_memory* mem);

#endif //VL_ALLOC_H