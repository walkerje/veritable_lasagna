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

#ifndef VL_MEMORY_H
#define VL_MEMORY_H

#include <malloc.h>

#include "vl_compare.h"
#include "vl_numtypes.h"

typedef VL_MEMORY_SIZE_T vl_memsize_t;

#ifndef VL_KB
/**
 * Convenience macro to define blocks of memory which contain a multiple of
 * X-many kilobytes.
 * \param x total kilobytes.
 */
#define VL_KB(x) ((vl_memsize_t)(x) << 10)
#endif

#ifndef VL_MB
/**
 * Convenience macro to define blocks of memory which contain a multiple of
 * X-many megabytes.
 * \param x total megabytes.
 */
#define VL_MB(x) ((vl_memsize_t)(x) << 20)
#endif

#ifndef VL_DEFAULT_MEMORY_SIZE
/**
 * \brief Default 1kb allocation size.
 */
#define VL_DEFAULT_MEMORY_SIZE VL_KB(1)
#endif

#ifndef VL_DEFAULT_MEMORY_ALIGN

#ifdef _MSC_VER
#define VL_ALIGNOF(T) __alignof(T)
#else
#define VL_ALIGNOF(T) __alignof__(T)
#endif

/**
 * \brief Default memory alignment. Defaults to maximum system word size.
 */
#define VL_DEFAULT_MEMORY_ALIGN VL_ALIGNOF(vl_ularge_t)
#endif

#ifndef VL_MEMORY_PAD_UP
/**
 * \brief Calculate the next offset such that it is a multiple of an alignment.
 *
 * This will return len when already a multiple of pad.
 *
 * \warning pad must be a power of 2.
 *
 * \par len size of the memory block
 * \par pad bytes to pad it to.
 * \return len
 */
#define VL_MEMORY_PAD_UP(len, pad) (((len) + (pad) - 1) & ~((pad) - 1))
#endif

/**
 * \brief Structure alignment hint.
 *
 * Hints to the compiler that the declared structure should be aligned to the
 * specified value.
 */
#ifndef VL_ALIGN_HINT
#if defined(_MSC_VER)
#define VL_ALIGN_HINT(x) __declspec(align(x))
#elif defined(__GNUC__) || defined(__clang__)
#define VL_ALIGN_HINT(x) __attribute__((aligned(x)))
#else
#define VL_ALIGN_HINT(x)
#warning VL_ALIGN_HINT failed to resolve. Structure alignment is not guaranteed.
#endif
#endif

/**
 * The typedef for vl_memory is defined as the smallest possible word size.
 * This is intended to improve code readability and intent.
 *
 * This is used only to indicate blocks of memory allocated through
 * vlMemAlloc(/Aligned) and vlMemRealloc.
 *
 * \sa vlMemAlloc
 * \sa vlMemAllocAligned
 * \sa vlMemRealloc
 *
 * vl_memory pointers have a header associated with them.
 */
typedef VL_MEMORY_T vl_memory;

/**
 * The typedef for vl_transient is, similarly, the smallest possible word size.
 * This is intended to improve code readability and intent.
 *
 * This is used to indicate pointers to blocks of memory which that might be
 * moved, deleted, or erased through some operation on a data structure,
 * allocator, or stack scope.
 */
typedef VL_MEMORY_T vl_transient;

/**
 * \brief Attempts to allocate a block of memory.
 *
 * Returns NULL on failure.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned memory and is responsible for calling `vlMemFree`.
 * - **Lifetime**: The memory block is valid until it is passed to `vlMemFree` or `vlMemRealloc`.
 * - **Thread Safety**: This function is thread-safe as it uses the system `malloc`.
 * - **Nullability**: Returns `NULL` on allocation failure. `allocSize` is not checked for zero, but `malloc(0)` is
 * implementation-defined.
 * - **Error Conditions**: Returns `NULL` if the underlying `malloc` fails.
 * - **Undefined Behavior**: Using the returned pointer after it has been freed, or passing it to the standard `free`
 * function instead of `vlMemFree`.
 * - **Memory Allocation Expectations**: Allocates `allocSize` plus the size of an internal header (`vl_memory_header`).
 * - **Return-value Semantics**: Returns a pointer to the start of the user-data portion of the allocation, or `NULL` if
 * allocation failed.
 *
 * \param allocSize size of the allocation, in bytes.
 * \return pointer to allocated block, or NULL.
 */
VL_API vl_memory* vlMemAlloc(vl_memsize_t allocSize);

/**
 * \brief Reallocates the specified block of memory to hold the specified total
 * number of bytes.
 *
 * If the specified memory block is explicitly aligned, its alignment is
 * preserved.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the returned pointer. The original `mem` pointer may become
 * invalid upon success.
 * - **Lifetime**: The new memory block is valid until it is passed to `vlMemFree` or another `vlMemRealloc`.
 * - **Thread Safety**: This function is thread-safe as it uses the system `realloc`.
 * - **Nullability**: If `mem` is `NULL`, this function behaves like `vlMemAlloc`. If `allocSize` is zero, behavior is
 * `realloc`-dependent.
 * - **Error Conditions**: Returns `NULL` if the underlying `realloc` fails. In this case, the original `mem` pointer
 * remains valid and its memory is not leaked.
 * - **Undefined Behavior**: Passing a pointer not originally allocated by `vlMemAlloc` or `vlMemAllocAligned`.
 * - **Memory Allocation Expectations**: Reallocates the underlying block to the new size plus internal header size.
 * Preserves existing alignment if any.
 * - **Return-value Semantics**: Returns a pointer to the new user-data portion of the allocation, or `NULL` if
 * reallocation failed.
 *
 * \param mem pointer to block
 * \param allocSize new size of the allocation.
 * \return pointer to reallocated memory
 */
VL_API vl_memory* vlMemRealloc(vl_memory* mem, vl_memsize_t allocSize);

/**
 * \brief Allocates a block of memory with an alignment.
 *
 * Guarantees that the returned pointer will have a value that is a multiple of
 * the specified alignment.
 *
 * The VL_MEMORY_PAD_UP macro may be used to ensure that the actual length
 * of the memory block is also a multiple of the alignment.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned memory and is responsible for calling `vlMemFree`.
 * - **Lifetime**: The memory block is valid until it is passed to `vlMemFree` or `vlMemRealloc`.
 * - **Thread Safety**: This function is thread-safe as it uses the system `malloc`.
 * - **Nullability**: Returns `NULL` on allocation failure.
 * - **Error Conditions**: Returns `NULL` if the underlying `malloc` fails.
 * - **Undefined Behavior**: `align` is not a power of 2.
 * - **Memory Allocation Expectations**: Allocates `allocSize + align + sizeof(vl_memory_header)` to guarantee
 * alignment.
 * - **Return-value Semantics**: Returns a pointer to the aligned start of the user-data portion of the allocation, or
 * `NULL` if allocation failed.
 *
 * \sa VL_MEMORY_PAD_UP
 *
 * \param allocSize size of the allocation, in bytes.
 * \param align
 * \return pointer to the aligned block
 */
VL_API vl_memory* vlMemAllocAligned(vl_memsize_t allocSize, vl_uint_t align);

/**
 * \brief Type-safe heap allocation for a single element.
 *
 * Allocates memory on the heap for a single object of the specified type.
 * The allocated memory is properly sized and return type is cast to the desired
 * type.
 *
 * This macro provides a convenient, type-safe alternative to manual
 * `vlMemAlloc(sizeof(type))` calls, reducing the risk of size mismatches.
 *
 * ## Example
 * ```c
 * struct my_data {
 *     int x;
 *     float y;
 * };
 *
 * struct my_data *obj = vlMemAllocType(struct my_data);
 * if (obj != NULL) {
 *     obj->x = 42;
 *     vlMemFree((vl_memory*)obj);
 * }
 * ```
 *
 * \param element_type The type to allocate (e.g., `int`, `struct foo`,
 * `MyType`)
 * \return Pointer to allocated and zero-initialized memory of the specified
 * type, or NULL on allocation failure.
 *
 * \note The returned pointer is managed with vlMemFree() like any heap
 * allocation.
 * \note Memory alignment follows the alignment of the specified element type.
 *
 * \sa vlMemAllocTypeArray, vlMemAlloc, vlMemFree
 */
#define vlMemAllocType(element_type) ((element_type*)vlMemAllocAligned(sizeof(element_type), VL_ALIGNOF(element_type)))

/**
 * \brief Type-safe heap allocation for an array of elements.
 *
 * Allocates memory on the heap for an array of the specified type and count.
 * The allocated memory is properly sized and the return type is cast to the
 * desired type.
 *
 * This macro provides a convenient, type-safe alternative to manual
 * `vlMemAlloc(count * sizeof(type))` calls, eliminating manual size
 * calculations and reducing overflow risks.
 *
 * ## Example
 * ```c
 * int *numbers = vlMemAllocTypeArray(int, 100);
 * if (numbers != NULL) {
 *     numbers[0] = 42;
 *     numbers[99] = -1;
 *     vlMemFree((vl_memory*)numbers);
 * }
 * ```
 *
 * \param element_type The element type in the array (e.g., `int`, `struct foo`)
 * \param count Number of elements to allocate space for
 * \return Pointer to allocated and zero-initialized array of the specified type
 * and count, or NULL on allocation failure.
 *
 * \note The returned pointer is managed with vlMemFree() like any heap
 * allocation.
 * \note Memory alignment follows the alignment of the specified element type.
 * \warning Verify that count > 0 to avoid allocating zero bytes; behavior is
 * undefined for negative counts.
 *
 * \sa vlMemAllocType, vlMemAlloc, vlMemFree
 */
#define vlMemAllocTypeArray(element_type, count)                                                                       \
    ((element_type*)vlMemAllocAligned(sizeof(element_type) * (count), VL_ALIGNOF(element_type)))

/**
 * \brief Allocate memory on the stack (automatic storage).
 *
 * Allocates memory from the stack for the specified size. The allocated memory
 * is automatically freed when the enclosing scope exits. Stack allocation is
 * very fast but is limited by stack size (typically a few MB on most systems).
 *
 * ## Characteristics
 * - **Speed**: Nearly free (just stack pointer adjustment)
 * - **Automatic cleanup**: Memory is freed when scope exits
 * - **Size limit**: Constrained by stack size (~1-8 MB typically)
 * - **Alignment**: Uses default system alignment; for custom alignment, use
 * vlMemAllocStackAligned()
 *
 * ## Contract
 * - **Ownership**: The current function scope maintains implicit ownership; the memory is automatically reclaimed on
 * function exit.
 * - **Lifetime**: Valid only within the scope of the calling function.
 * - **Thread Safety**: Thread-safe (allocates from the thread-local stack).
 * - **Nullability**: Never returns `NULL`.
 * - **Error Conditions**: Stack overflow if `allocSize` is too large for the remaining stack space (leads to process
 * crash).
 * - **Undefined Behavior**: Accessing the returned pointer after the allocating function has returned.
 * - **Memory Allocation Expectations**: Allocates `allocSize` bytes directly on the stack via `alloca` or equivalent.
 * - **Return-value Semantics**: Returns a pointer to the allocated stack memory.
 *
 * ## Example
 * ```c
 * void process_data(void) {
 *     vl_transient *buf = vlMemAllocStack(1024);
 *     // Use buf...
 *     // buf is automatically freed when process_data() returns
 * }
 * ```
 *
 * ## When to Use
 * - Temporary buffers within a single function scope
 * - Known, reasonably small sizes (< 1 MB)
 * - Performance-critical code where allocation speed matters
 * - Scratch space for intermediate computations
 *
 * \param allocSize Number of bytes to allocate
 * \return Pointer to allocated stack memory
 *
 * \note No NULL return is possible (stack overflow causes a crash instead)
 * \note Stack memory is NOT managed by vlMemFree(); it's automatic
 * \note Allocating large amounts may cause stack overflow—verify size limits
 *
 * \sa vlMemAllocStackAligned, vlMemAllocStackType, vlMemAllocStackTypeArray
 */
inline vl_transient* vlMemAllocStack(vl_memsize_t allocSize)
{
#ifdef _MSC_VER
    return (vl_transient*)_alloca(allocSize);
#elif defined(__GNUC__) || defined(__clang__)
    return (vl_transient*)__builtin_alloca(allocSize);
#else
#error Failed to resolve stack allocation method.
#endif
}

/**
 * \brief Allocate aligned memory on the stack (automatic storage).
 *
 * Allocates memory from the stack with custom alignment requirements.
 * The allocated memory is automatically freed when the enclosing scope exits.
 *
 * ## Characteristics
 * - **Speed**: Very fast (stack pointer adjustment + alignment math)
 * - **Automatic cleanup**: Memory is freed when scope exits
 * - **Custom alignment**: Guarantees pointer is aligned to specified boundary
 * - **Size limit**: Constrained by stack size (~1-8 MB typically)
 *
 * ## Alignment Requirements
 * The alignment must be a power of 2 (16, 32, 64, 128, etc.).
 * Passing a non-power-of-2 alignment will produce undefined behavior.
 *
 * ## Contract
 * - **Ownership**: The current function scope maintains implicit ownership; the memory is automatically reclaimed on
 * function exit.
 * - **Lifetime**: Valid only within the scope of the calling function.
 * - **Thread Safety**: Thread-safe (allocates from the thread-local stack).
 * - **Nullability**: Never returns `NULL`.
 * - **Error Conditions**: Stack overflow if the padded allocation size is too large for the remaining stack space.
 * - **Undefined Behavior**: Accessing the returned pointer after the allocating function has returned. `align` is not a
 * power of 2.
 * - **Memory Allocation Expectations**: Allocates `allocSize + align - 1` bytes on the stack and returns an aligned
 * pointer.
 * - **Return-value Semantics**: Returns a pointer to the aligned stack memory.
 *
 * ## Example
 * ```c
 * // Allocate 256 bytes aligned to 16-byte boundary (common for SIMD)
 * vl_transient *simd_buf = vlMemAllocStackAligned(256, 16);
 * // Allocate 512 bytes aligned to 64-byte boundary (cache line)
 * vl_transient *cache_buf = vlMemAllocStackAligned(512, 64);
 * ```
 *
 * ## When to Use
 * - SIMD operations requiring aligned data (SSE, AVX, NEON, etc.)
 * - Cache-line alignment for performance-sensitive data
 * - Temporary aligned scratch buffers within a function
 * - Interfacing with APIs that require specific alignment
 *
 * \param allocSize Number of bytes to allocate
 * \param align Required byte alignment (must be a power of 2)
 * \return Pointer to allocated and aligned stack memory
 *
 * \warning align MUST be a power of 2, or behavior is undefined
 * \note No NULL return is possible (stack overflow causes a crash instead)
 * \note Stack memory is NOT managed by vlMemFree(); it's automatic
 * \note The actual stack space used may be slightly more than allocSize due to
 * alignment padding
 *
 * \sa vlMemAllocStack, vlMemAllocStackType, vlMemAllocStackTypeArray
 */
inline vl_transient* vlMemAllocStackAligned(vl_memsize_t allocSize, vl_uint16_t align)
{
    return (vl_transient*)(((vl_uintptr_t)vlMemAllocStack((allocSize) + (align)-1) + (align)-1) & ~((align)-1));
}

/**
 * \brief Type-safe stack allocation for a single element.
 *
 * Allocates memory on the stack for a single object of the specified type.
 * The allocated memory is automatically aligned to the type's natural alignment
 * requirement and automatically freed when the enclosing scope exits.
 *
 * This macro combines type safety with automatic alignment, ensuring the
 * allocation is properly sized and aligned for the type without manual
 * alignment calculations.
 *
 * ## Example
 * ```c
 * struct point {
 *     float x, y, z;
 * };
 *
 * struct point *p = vlMemAllocStackType(struct point);
 * p->x = 1.0f;
 * p->y = 2.0f;
 * p->z = 3.0f;
 * // p is automatically freed when scope exits
 * ```
 *
 * ## When to Use
 * - Temporary struct/object instances within a function
 * - When you need both type safety and proper alignment
 * - Quick prototyping or temporary calculations
 * - The type's size is small and known at compile time
 *
 * \param element_type The type to allocate (e.g., `int`, `struct foo`,
 * `MyType`)
 * \return Pointer to allocated and aligned stack memory of the specified type,
 *         automatically freed when scope exits
 *
 * \note No NULL return is possible (stack overflow causes a crash instead)
 * \note Stack memory is NOT managed by vlMemFree(); it's automatic
 * \note Alignment is automatically derived from the type using VL_ALIGNOF()
 *
 * \sa vlMemAllocStackTypeArray, vlMemAllocStackAligned, vlMemAllocType
 */
#define vlMemAllocStackType(element_type)                                                                              \
    ((element_type*)vlMemAllocStackAligned(sizeof(element_type), VL_ALIGNOF(element_type)))

/**
 * \brief Type-safe stack allocation for an array of elements.
 *
 * Allocates memory on the stack for an array of the specified type and count.
 * The allocated memory is automatically aligned to the type's natural alignment
 * requirement and automatically freed when the enclosing scope exits.
 *
 * This macro combines type safety, proper sizing, and automatic alignment,
 * eliminating manual calculations and reducing errors.
 *
 * ## Example
 * ```c
 * int *numbers = vlMemAllocStackTypeArray(int, 50);
 * for (int i = 0; i < 50; i++) {
 *     numbers[i] = i * 2;
 * }
 * // numbers is automatically freed when scope exits
 * ```
 *
 * ## When to Use
 * - Temporary arrays within a function scope
 * - Known, small array sizes (typically < 1 MB of stack)
 * - Quick prototyping or intermediate computations
 * - When automatic alignment and type safety are desired
 *
 * \param element_type The element type in the array (e.g., `int`, `struct foo`)
 * \param count Number of elements to allocate space for
 * \return Pointer to allocated and aligned stack array of the specified type
 * and count, automatically freed when scope exits
 *
 * \note No NULL return is possible (stack overflow causes a crash instead)
 * \note Stack memory is NOT managed by vlMemFree(); it's automatic
 * \note Alignment is automatically derived from the element type using
 * VL_ALIGNOF()
 * \warning Verify that count > 0 to avoid allocating zero bytes; behavior is
 * undefined for negative counts.
 *
 * \sa vlMemAllocStackType, vlMemAllocTypeArray, vlMemAllocStackAligned
 */
#define vlMemAllocStackTypeArray(element_type, count)                                                                  \
    ((element_type*)vlMemAllocStackAligned((count) * sizeof(element_type), VL_ALIGNOF(element_type)))

/**
 * \brief Clones the specified block of memory, returning a pointer to its new
 * clone.
 *
 * Returns NULL on failure.
 *
 * If the source block has an alignment, the result will also have an alignment.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned memory and is responsible for calling `vlMemFree`.
 * - **Lifetime**: The cloned memory block is valid until it is passed to `vlMemFree` or `vlMemRealloc`.
 * - **Thread Safety**: Safe to call concurrently on different source blocks. Not thread-safe if the source block is
 * being modified by another thread.
 * - **Nullability**: Returns `NULL` if `mem` is `NULL` or if allocation fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: Passing a pointer not originally allocated by `vlMemAlloc` or `vlMemAllocAligned`.
 * - **Memory Allocation Expectations**: Allocates a new block of memory with the same size and alignment as the source
 * block.
 * - **Return-value Semantics**: Returns a pointer to the start of the user-data portion of the cloned allocation, or
 * `NULL` if cloning failed.
 *
 * \param mem pointer
 * \return cloned memory pointer
 */
VL_API vl_memory* vlMemClone(vl_memory* mem);

/**
 * \brief Returns the size (in total number of bytes) of the specified block of
 * vl_memory.
 *
 * ## Contract
 * - **Ownership**: Does not transfer or affect ownership.
 * - **Lifetime**: The `mem` pointer must be valid at the time of the call.
 * - **Thread Safety**: Thread-safe for concurrent reads of the same memory block's metadata.
 * - **Nullability**: Returns 0 if `mem` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing a pointer not originally allocated by `vlMemAlloc` or `vlMemAllocAligned`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the size of the user-data portion of the allocation in bytes.
 *
 * \par mem pointer to memory block
 * \return size of the specified memory block, in bytes.
 */
VL_API vl_memsize_t vlMemSize(vl_memory* mem);

/**
 * \brief Returns the alignment of the specified block of memory.
 *
 * Minimum alignment is defined as VL_DEFAULT_MEMORY_ALIGN, or the largest
 * available word size.
 *
 * ## Contract
 * - **Ownership**: Does not transfer or affect ownership.
 * - **Lifetime**: The `mem` pointer must be valid at the time of the call.
 * - **Thread Safety**: Thread-safe for concurrent reads of the same memory block's metadata.
 * - **Nullability**: Returns `VL_DEFAULT_MEMORY_ALIGN` if `mem` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing a pointer not originally allocated by `vlMemAlloc` or `vlMemAllocAligned`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the byte alignment of the user-data portion of the allocation.
 *
 * \param mem pointer to memory block
 * \return alignment
 */
VL_API vl_uint_t vlMemAlignment(vl_memory* mem);

/**
 * \brief Sorts the specified buffer in-place according to the specified element
 * and comparator function.
 *
 * This function implements an iterative Quicksort.
 *
 * ## Contract
 * - **Ownership**: Does not transfer or affect ownership of the `buffer`.
 * - **Lifetime**: The `buffer` must remain valid for the duration of the sort.
 * - **Thread Safety**: Not thread-safe if multiple threads access the same `buffer` concurrently.
 * - **Nullability**: `buffer` must not be `NULL`. `comparator` must not be `NULL`.
 * - **Error Conditions**: If internal temporary memory allocation fails, the function returns without sorting the
 * buffer.
 * - **Undefined Behavior**: Passing a `NULL` `buffer` or `comparator`. Overlapping memory regions during sort
 * operations.
 * - **Memory Allocation Expectations**: Allocates temporary scratch space on the heap proportional to `numElements` and
 * `elementSize`.
 * - **Return-value Semantics**: None (void).
 *
 * \param buffer
 * \param elementSize
 * \param numElements
 * \param comparator
 * \par Complexity of O(n log(n)) (space complexity of O(n)).
 */
VL_API void vlMemSort(void* buffer, vl_memsize_t elementSize, vl_dsidx_t numElements, vl_compare_function comparator);

/**
 * \brief Copies data from one buffer to another, with a stride applied to both.
 *
 * Stride is the amount of space (in bytes) between each element.
 *
 * ## Contract
 * - **Ownership**: Does not transfer or affect ownership of the buffers.
 * - **Lifetime**: Both `src` and `dest` buffers must remain valid for the duration of the copy.
 * - **Thread Safety**: Not thread-safe if multiple threads access `src` or `dest` concurrently where at least one
 * thread is writing.
 * - **Nullability**: `src` and `dest` should not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Overlapping memory regions when `srcStride` or `dstStride` allow it, as `memcpy` is used
 * for individual elements.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param src memory block pointer
 * \param srcStride total number of bytes between each element in "src"
 * \param dest memory block pointer
 * \param dstStride total number of bytes between each element in "dest"
 * \param elementSize total number of bytes wide of each element
 * \param numElements total number of elements
 * \par Complexity of O(n) linear.
 */
VL_API void vlMemCopyStride(const void* src, vl_dsoffs_t srcStride, void* dest, vl_dsoffs_t dstStride,
                            vl_memsize_t elementSize, vl_dsidx_t numElements);

/**
 * \brief Reverses the bytes in a series of elements of a defined length and
 * stride between them.
 *
 * This function operates on a sequence of elements (or sub-arrays) within a
 * memory block, where each element is separated by a defined stride. It
 * reverses the bytes of each individual element but does not alter the overall
 * structure of the memory. The elements are processed sequentially, one by one,
 * with each element's bytes reversed in-place.
 *
 * This is useful when you need to reverse the bytes in each element of a
 * collection of data structures that are tightly packed but may have a varying
 * stride (i.e., distance between consecutive elements in memory).
 *
 * Example:
 * Suppose we have an array of 32-bit integers, each 4 bytes, with a stride of 8
 * bytes:
 *
 *    Input (elements of size 4 bytes, stride of 8 bytes):
 *    [0xAABBCCDD, 0x11223344, 0x55667788]
 *    Memory block: [0xAABBCCDD, pad, 0x11223344, pad, 0x55667788, pad]
 *    (Where 'pad' represents the unused memory between elements, i.e., stride.)
 *
 *    Output (each element reversed):
 *    [0xDDCCBBAA, 0x44332211, 0x88776655]
 *    Memory block: [0xDDCCBBAA, pad, 0x44332211, pad, 0x88776655, pad]
 *
 * The bytes within each element are reversed, but the stride between elements
 * is respected.
 *
 * ## Contract
 * - **Ownership**: Does not transfer or affect ownership of `src`.
 * - **Lifetime**: The `src` buffer must remain valid for the duration of the operation.
 * - **Thread Safety**: Not thread-safe if multiple threads access `src` concurrently.
 * - **Nullability**: `src` should not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Invalid `srcStride` or `elementSize` that leads to out-of-bounds memory access.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param src memory block pointer. The base address of the memory containing
 * the elements to be reversed.
 * \param srcStride total number of bytes between each sub-array (or element).
 * This defines the gap between consecutive elements.
 * \param elementSize size of each sub-array (or element) in bytes. This is the
 * number of bytes that constitute one element.
 * \param numElements total number of sub-arrays (or elements). This is the
 * number of individual elements to process.
 * \par Complexity of O(n) linear.
 */
VL_API void vlMemReverseSubArraysStride(void* src, vl_dsoffs_t srcStride, vl_memsize_t elementSize,
                                        vl_dsidx_t numElements);

#ifndef vlMemReverse
/**
 * \brief Reverses the order of bytes in the specified block of memory.
 *
 * This is to be used on tightly packed sequences of bytes.
 * Attempting to blindly reverse the memory of a structure can result in
 * unexpected results due to padding inserted by the compiler.
 *
 * Assuming the input is of the sequence [0x0A, 0x0B, 0x0C, 0x0D, 0x0E],
 * the expected output would be [0x0E, 0x0D, 0x0C, 0x0B, 0x0A].
 *
 * \sa vlMemReverseSubArraysStride
 *
 * \param src memory block pointer
 * \param numBytes total number of bytes to reverse
 * \par Complexity of O(n) linear.
 */
#define vlMemReverse(src, numBytes) vlMemReverseSubArraysStride(src, 1, numBytes, 1)
#endif

#ifndef vlMemReverseSubArrays
/**
 * \brief Reverses the bytes in a tightly packed series of elements of a defined
 * length.
 *
 * This is to be used on tightly packed sequences of elements.
 * An element is defined as a discrete sub-array of bytes in a larger sequence.
 *
 * Assuming you have the following input: [0xAABBCCDD, 0x11223344, 0x55667788],
 * the expected output would be: [0xDDCCBBAA, 0x44332211, 0x88776655].
 *
 * This differs from the vlMemReverse function, where given that input
 * the expected output would be: [0x88776655, 0x44332211, 0xDDCCBBAA].
 *
 * \sa vlMemReverseSubArraysStride
 *
 * \param src memory block pointer
 * \param elementSize size of each sub array, or element, in bytes
 * \param numElements total number of sub arrays, or elements
 * \par Complexity of O(n) linear.
 */
#define vlMemReverseSubArrays(src, elementSize, numElements)                                                           \
    vlMemReverseSubArraysStride(src, elementSize, elementSize, numElements)
#endif

/**
 * \brief Frees the specified block of memory.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the memory block.
 * - **Lifetime**: The memory block and its associated pointer become invalid after this call.
 * - **Thread Safety**: This function is thread-safe as it uses the system `free`.
 * - **Nullability**: Safe to call with `NULL` (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing a pointer not originally allocated by `vlMemAlloc` or `vlMemAllocAligned`, or
 * double-freeing the same pointer.
 * - **Memory Allocation Expectations**: Deallocates the user-data portion and the internal header.
 * - **Return-value Semantics**: None (void).
 *
 * \param mem pointer to block.
 */
VL_API void vlMemFree(vl_memory* mem);

#endif // VL_MEMORY_H
