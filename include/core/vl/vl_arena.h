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

#ifndef VL_ARENA_H
#define VL_ARENA_H

#include "vl_set.h"

typedef vl_uintptr_t vl_arena_ptr;

#ifdef VL_ARENA_NULL
#undef VL_ARENA_NULL
#endif

/**
 * \brief An explicit NULL integer constant that indicates a bad location in an
 * arena.
 *
 * Using a regular 0 = NULL here works, as no valid allocation will have that
 * offset. This is due to the implicit metadata prepended to each allocation.
 */
#define VL_ARENA_NULL 0

/**
 * \brief Metadata for allocations made with vl_arena.
 * These are the elements stored in a vl_arena free set.
 * Exposed due to possible uses in other data structures (see vl_hashtable
 * implementation and tests).
 * \private
 */
typedef struct vl_arena_node
{
    vl_arena_ptr offset;
    vl_memsize_t size;
} vl_arena_node;

/**
 * \brief An arena allocator for efficient memory management.
 *
 * The Arena allocator provides a memory management strategy that efficiently
 * handles multiple allocations from a single large memory block. It implements
 * a sophisticated allocation strategy with the following key features:
 *
 * \par Allocation Strategy
 * - Uses Offset-Ordered First Fit Backwards Allocation
 * - Free blocks are maintained in order based on their memory offset
 * - Allocations are made from the end of free blocks when possible
 * - Adjacent freed blocks are automatically merged (coalesced)
 *
 * \par Performance Characteristics
 * - Fast allocation from a pre-allocated memory block
 * - Efficient memory reuse through block coalescing
 * - Automatic growth by doubling capacity when needed
 * - Minimal fragmentation due to offset-ordered allocation
 *
 * \par Important Limitations
 * - Memory references (pointers) may become invalid after arena growth
 * - Use vl_arena_ptr for stable references across reallocations
 * - Standard pointers must be re-sampled after any operation that might grow
 * the arena
 *
 * \par Implementation Details
 * The arena consists of:
 * - A contiguous block of memory for allocations
 * - An ordered free set tracking available memory blocks
 * - Metadata for each allocation to track sizes
 *
 * \struct vl_arena
 * \note All operations that might cause arena growth can invalidate existing
 * pointers
 */

typedef struct
{
    vl_memory* data; // Block of memory.
    vl_set freeSet; // set is ordered according to the offset of each free node.
} vl_arena;

/**
 * \brief Initializes the vl_arena structure with the given initial size.
 *
 * This function initializes a vl_arena structure with the specified initial
 * size. The initial size determines the number of elements that the arena can
 * hold initially.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the `arena` struct. The function initializes the internal memory
 * block and free set.
 * - **Lifetime**: The arena is valid until `vlArenaFree` or `vlArenaDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `arena` must not be `NULL`.
 * - **Error Conditions**: If `vlMemAlloc` fails for the initial data block, `arena->data` will be `NULL`.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates `initialSize` bytes via `vlMemAlloc`.
 * - **Return-value Semantics**: None (void).
 *
 * \param arena The vl_arena structure to be initialized.
 * \param initialSize The initial size of the arena.
 */
VL_API void vlArenaInit(vl_arena* arena, vl_memsize_t initialSize);

/**
 * \brief Frees memory allocated by an arena instance.
 *
 * De-initializes the arena by freeing its underlying memory block and the free set.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal data block and free set structures. Does NOT free the `arena`
 * struct itself.
 * - **Lifetime**: All `vl_arena_ptr` handles and sampled pointers become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `arena` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double free.
 * - **Memory Allocation Expectations**: Deallocates the data block via `vlMemFree`.
 * - **Return-value Semantics**: None (void).
 *
 * \param arena Pointer to the arena instance to be freed.
 */
VL_API void vlArenaFree(vl_arena* arena);

/**
 * \brief Creates a new arena with the specified initial size.
 *
 * This function initializes a new arena with the specified initial size. The
 * arena is used to dynamically allocate memory and manage it efficiently using
 * a memory region structure.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_arena` pointer and is responsible for calling `vlArenaDelete`.
 * - **Lifetime**: The arena is valid until `vlArenaDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `NULL` if heap allocation for the `vl_arena` struct fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates the `vl_arena` struct and its initial data block.
 * - **Return-value Semantics**: Returns a pointer to the newly created arena, or `NULL`.
 *
 * \param initialSize The initial size of the arena.
 * \return A pointer to the newly created arena, or NULL if the arena creation
 * failed.
 */
VL_API vl_arena* vlArenaNew(vl_memsize_t initialSize);

/**
 * \brief Deletes the given VL arena, freeing all allocated memory and the arena struct itself.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal data block, free set, and the `vl_arena` struct.
 * - **Lifetime**: All handles, pointers, and the arena struct pointer itself become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `arena` can be `NULL` (safely handled by `free`).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Deallocates internal memory via `vlMemFree` and the struct via `free`.
 * - **Return-value Semantics**: None (void).
 *
 * \param arena Pointer to the VL arena to be deleted.
 */
VL_API void vlArenaDelete(vl_arena* arena);

/**
 * \brief Clears all the allocations in the given arena.
 *
 * This function removes all the elements in the arena, making it empty.
 * The memory allocated for the internal data block is not released, but all previous allocations are marked as free.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: All previously returned `vl_arena_ptr` handles and sampled pointers become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `arena` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized arena.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param arena Pointer to the vl_arena structure.
 */
VL_API void vlArenaClear(vl_arena* arena);

/**
 * \brief Clones the specified arena to another.
 *
 * Clones the entirety of the src arena to the dest arena, including all allocated blocks and free set state.
 *
 * The 'src' arena pointer must be non-null and initialized.
 * The 'dest' arena pointer may be null, but if it is not null it must be
 * initialized.
 *
 * If the 'dest' arena pointer is null, a new arena is created via vlArenaNew.
 *
 * ## Contract
 * - **Ownership**: If `dest` is `NULL`, the caller owns the returned `vl_arena` pointer. If `dest` is provided,
 * ownership remains with the caller.
 * - **Lifetime**: The cloned arena is valid until it is deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` must not be `NULL`. `dest` can be `NULL`.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: Passing an uninitialized arena.
 * - **Memory Allocation Expectations**: Allocates a new `vl_arena` struct (if `dest` is `NULL`) and a new data block.
 * - **Return-value Semantics**: Returns the pointer to the cloned arena (`dest` or a new instance), or `NULL` on
 * failure.
 *
 * \param src pointer
 * \param dest pointer
 * \return pointer to arena that was copied to or created.
 */
VL_API vl_arena* vlArenaClone(const vl_arena* src, vl_arena* dest);

/**
 * \brief Reserves storage in the underlying allocation of the given arena.
 *
 * This is done by doubling the size until the requested growth is met or
 * exceeded. This function will always result in the reallocation of the
 * underlying memory.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Existing `vl_arena_ptr` handles remain stable, but all sampled standard pointers are invalidated.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `arena` must not be `NULL`.
 * - **Error Conditions**: If `vlMemRealloc` fails, `arena->data` may become `NULL`.
 * - **Undefined Behavior**: Passing an uninitialized arena.
 * - **Memory Allocation Expectations**: Triggers reallocation of the underlying data block and potentially the free
 * set.
 * - **Return-value Semantics**: None (void).
 *
 * \param arena pointer
 * \param numBytes total bytes to reserve
 */
VL_API void vlArenaReserve(vl_arena* arena, vl_memsize_t numBytes);

/**
 * \brief Take memory from the given arena.
 *
 * This function is used to allocate memory from the specified arena.
 * Memory is allocated by finding a free block of memory of equal or greater
 * size within the internally managed block of memory.
 *
 * ## Contract
 * - **Ownership**: The arena maintains ownership of the memory. The caller receives an opaque `vl_arena_ptr` handle.
 * - **Lifetime**: The handle is valid until the block is freed via `vlArenaMemFree`, the arena is cleared, or the arena
 * is destroyed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `VL_ARENA_NULL` if allocation fails.
 * - **Error Conditions**: Returns `VL_ARENA_NULL` if no block is large enough and internal expansion (doubling) fails.
 * - **Undefined Behavior**: Passing an uninitialized arena.
 * - **Memory Allocation Expectations**: May trigger expansion of the underlying arena data block (doubling) and
 * reallocation of the free set.
 * - **Return-value Semantics**: Returns a `vl_arena_ptr` handle to the allocated memory, or `VL_ARENA_NULL` on failure.
 *
 * \param arena A pointer to the arena from which memory will be allocated.
 * \param size The size of the memory to be allocated in bytes.
 *
 * \return A pointer to the allocated memory, or NULL if allocation fails.
 */
VL_API vl_arena_ptr vlArenaMemAlloc(vl_arena* arena, vl_memsize_t size);

/**
 * \brief Reallocates memory for the given pointer in the given arena.
 *
 * This function reallocates memory for a previously allocated block of memory
 * in the specified arena. If the reallocation is successful, the old block of
 * memory will be freed and the new block will have a size specified by the
 * 'size' parameter.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: The old `vl_arena_ptr` handle may be invalidated and replaced by a new one. Sampled pointers are
 * invalidated.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `VL_ARENA_NULL` if reallocation fails.
 * - **Error Conditions**: Returns `VL_ARENA_NULL` on failure.
 * - **Undefined Behavior**: Passing an invalid or previously freed `vl_arena_ptr`.
 * - **Memory Allocation Expectations**: May allocate a new block and copy data, or expand an existing block if adjacent
 * free space allows.
 * - **Return-value Semantics**: Returns a new `vl_arena_ptr` handle to the reallocated memory, or `VL_ARENA_NULL`.
 *
 * \param arena The arena to perform the reallocation on.
 * \param ptr The pointer to the previously allocated block of memory.
 * \param size The desired size of the newly reallocated block of memory.
 *
 * \return A pointer to the newly reallocated block of memory, or NULL if
 * reallocation fails.
 * \note The contents of the memory block pointed to by 'ptr' are preserved up
 * to the lesser of the new and old sizes.
 */
VL_API vl_arena_ptr vlArenaMemRealloc(vl_arena* arena, vl_arena_ptr ptr, vl_memsize_t size);

/**
 * \brief Frees a memory block allocated in a vl_arena.
 *
 * This function frees the memory block referenced by the given pointer from the
 * specified vl_arena. The memory block must have been previously allocated
 * using the vlArenaMemAlloc() or vlArenaMemRealloc() functions.
 *
 * ## Contract
 * - **Ownership**: Transfers ownership of the memory block back to the arena's free set.
 * - **Lifetime**: The `vl_arena_ptr` handle becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `ptr` should not be `VL_ARENA_NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Freeing an invalid handle, a handle from a different arena, or double-freeing.
 * - **Memory Allocation Expectations**: Adds a node to the internal free set and may trigger block coalescing.
 * - **Return-value Semantics**: None (void).
 *
 * \param arena The vl_arena structure representing the arena.
 * \param ptr The pointer to the memory block to be freed.
 *
 * \note The behavior is undefined if ptr does not point to a memory block
 * allocated in the specified arena.
 *
 * \sa vlArenaMemAlloc()
 * \sa vlArenaMemRealloc()
 */
VL_API void vlArenaMemFree(vl_arena* arena, vl_arena_ptr ptr);

/**
 * \brief Copies a block of memory to the beginning of the specified arena
 * allocation.
 *
 * This function handles the case wherein the specified source pointer resides
 * within the arena.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: The `dst` handle may be replaced by a new handle. Sampled pointers are invalidated.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `arena`, `dst`, and `src` should not be `NULL`/`VL_ARENA_NULL`.
 * - **Error Conditions**: Returns `VL_ARENA_NULL` if reallocation fails.
 * - **Undefined Behavior**: Passing invalid handles.
 * - **Memory Allocation Expectations**: Triggers `vlArenaMemRealloc`.
 * - **Return-value Semantics**: Returns the new `vl_arena_ptr` handle to the modified allocation.
 *
 * \param arena pointer
 * \param dst arena pointer
 * \param src memory to copy from
 * \param length number of bytes from src
 * \par Complexity of O(n) linear.
 * \return new dst arena pointer
 */
VL_API vl_arena_ptr vlArenaMemPrepend(vl_arena* arena, vl_arena_ptr dst, const void* src, vl_memsize_t length);

/**
 * \brief Copies a block of memory to the end of the specified arena allocation.
 *
 * This function handles the case wherein the specified source pointer resides
 * within the arena.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Same as `vlArenaMemPrepend`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Same as `vlArenaMemPrepend`.
 * - **Error Conditions**: Returns `VL_ARENA_NULL` if reallocation fails.
 * - **Undefined Behavior**: Same as `vlArenaMemPrepend`.
 * - **Memory Allocation Expectations**: Triggers `vlArenaMemRealloc`.
 * - **Return-value Semantics**: Returns the new `vl_arena_ptr` handle.
 *
 * \param arena pointer
 * \param dst arena pointer
 * \param src memory to copy from
 * \param length number of bytes from src
 * \par Complexity of O(n) linear.
 * \return new dst arena pointer
 */
VL_API vl_arena_ptr vlArenaMemAppend(vl_arena* arena, vl_arena_ptr dst, const void* src, vl_memsize_t length);

/**
 * \brief Sampling function that calculates a transient pointer into the
 * specified arena.
 *
 * ## Contract
 * - **Ownership**: Ownership remains with the arena.
 * - **Lifetime**: The returned pointer is valid until the next operation that might grow the arena (e.g.,
 * `vlArenaMemAlloc`, `vlArenaReserve`, `vlArenaMemRealloc`).
 * - **Thread Safety**: Safe for concurrent reads if no thread is writing to or growing the arena.
 * - **Nullability**: Returns `NULL` if `arena->data` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid `vl_arena_ptr`.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a transient `vl_transient*` pointer to the memory location.
 *
 * \param arena The arena on which the operation is performed.
 * \param ptr The arena pointer indicating the memory location on the arena.
 */
VL_API vl_transient* vlArenaMemSample(vl_arena* arena, vl_arena_ptr ptr);

/**
 * \brief Get the size of a memory block allocated in a VL arena.
 *
 * This function returns the size of the memory block allocated at the specified
 * pointer in a VL arena. The size is calculated based on the arena's metadata
 * and may not represent the exact size of the data stored in the block.
 *
 * ## Contract
 * - **Ownership**: Does not affect ownership.
 * - **Lifetime**: The arena and the allocation must be valid.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `arena` and `ptr` must not be `NULL`/`VL_ARENA_NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid handle.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the size of the data portion of the allocation in bytes.
 *
 * \param arena Pointer to the VL arena.
 * \param ptr Pointer to the memory block.
 *
 * \return Size of the memory block allocated at the specified pointer in the VL
 * arena.
 *
 * \note This function assumes that the provided pointer is a valid memory block
 * allocated in the specified VL arena. Passing an invalid or previously freed
 * pointer to this function may result in undefined behavior.
 */
VL_API vl_memsize_t vlArenaMemSize(vl_arena* arena, vl_arena_ptr ptr);

/**
 * \brief Get the total capacity of the arena.
 *
 * This function returns the total capacity of the given arena, in bytes.
 * The capacity represents the maximum number of elements that
 * the arena can store without requiring reallocation.
 *
 * ## Contract
 * - **Ownership**: Does not affect ownership.
 * - **Lifetime**: The arena must be valid.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `arena` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the total size of the underlying memory block in bytes.
 *
 * \param arena Pointer to the arena structure.
 * \return The total capacity of the arena.
 */
VL_API vl_memsize_t vlArenaTotalCapacity(vl_arena* arena);

/**
 * \brief Get the total amount of free memory in the arena.
 *
 * This function returns the total amount of free memory in the specified arena by summing all blocks in the free set.
 *
 * ## Contract
 * - **Ownership**: Does not affect ownership.
 * - **Lifetime**: The arena must be valid.
 * - **Thread Safety**: Safe for concurrent reads if no thread is modifying the arena.
 * - **Nullability**: `arena` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the total sum of all free block sizes in bytes.
 *
 * \param arena The pointer to the arena structure.
 * \return The total amount of free memory in the arena.
 */
VL_API vl_memsize_t vlArenaTotalFree(vl_arena* arena);

#endif // VL_ARENA_H
