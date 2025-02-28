#ifndef VL_ARENA_H
#define VL_ARENA_H

#include "vl_set.h"

typedef vl_uintptr_t       vl_arena_ptr;

#ifdef VL_ARENA_NULL
#undef VL_ARENA_NULL
#endif

/**
 * \brief An explicit NULL integer constant that indicates a bad location in an arena.
 *
 * Using a regular 0 = NULL here works, as no valid allocation will have that offset.
 * This is due to the implicit metadata prepended to each allocation.
 */
#define VL_ARENA_NULL 0

/**
 * \brief Metadata for allocations made with vl_arena.
 * These are the elements stored in a vl_arena free set.
 * Exposed due to possible uses in other data structures (see vl_hashtable implementation and tests).
 * \private
 */
typedef struct vl_arena_node{
    vl_arena_ptr        offset;
    vl_memsize_t        size;
} vl_arena_node;

/**
 * \brief An arena allocator.
 *
 * The vl_arena structure represents a block of memory that is sliced into
 * many smaller allocations. Returned blocks of memory are merged (or "coalesced") with other adjacent
 * blocks of memory, keeping the free set small in most cases.
 *
 * The allocation strategy used here is best described as Offset-Ordered First Fit Backwards Allocation.
 * The free set is traversed until the first block that can fit the request is found.
 * This allocator will then "slice" memory off of the end of the block, (or, in rare cases,
 * encompass the entirety of the block) to avoid re-ordering the free set whenever possible.
 * It is a combination of the first-fit and offset-ordered memory management approaches.
 *
 * For VL, Arenas are different from Linear Pools in a few notable ways.
 * The most significant difference between the two is that Arenas support
 * variably-sized allocations, whereas Linear Pools do not.
 * Pools also have notably less overhead in their management, and tend to map memory
 * linearly rather than having a "best-fit-and-slice" routine.
 *
 * This block of memory is automatically and dynamically resized when necessary by doubling its capacity.
 * One downside of Arenas that is also present in LinearPools is that all references are unstable.
 * If the underlying block of memory for the arena is resized, all standard pointers (e.g, language-level)
 * may be invalidated. vl_arena_ptr values would still remain valid, but would need freshly sampled pointers.
 *
 * (Writers note: This took a lot of research. I want a beer. :) )
 */
typedef struct{
    vl_memory*  data;       //Block of memory.
    vl_set      freeSet;    //set is ordered according to the offset of each free node.
} vl_arena;

/**
 * \brief Initializes the vl_arena structure with the given initial size.
 *
 * This function initializes a vl_arena structure with the specified initial size.
 * The initial size determines the number of elements that the arena can hold initially.
 *
 * \param arena The vl_arena structure to be initialized.
 * \param initialSize The initial size of the arena.
 */
void        vlArenaInit(vl_arena* arena, vl_memsize_t initialSize);

/**
 * \brief Frees memory allocated by an arena instance.
 *
 * \param arena Pointer to the arena instance to be freed.
 */
void        vlArenaFree(vl_arena* arena);

/**
 * \brief Creates a new arena with the specified initial size.
 *
 * This function initializes a new arena with the specified initial size. The arena is used to dynamically allocate
 * memory and manage it efficiently using a memory region structure.
 *
 * \param initialSize The initial size of the arena.
 * \return A pointer to the newly created arena, or NULL if the arena creation failed.
 */
vl_arena*    vlArenaNew(vl_memsize_t initialSize);

/**
 * \brief Deletes the given VL arena, freeing all allocated memory.
 *
 * \param arena Pointer to the VL arena to be deleted.
 */
void        vlArenaDelete(vl_arena* arena);

/**
 * \brief Clears all the elements in the given arena.
 *
 * This function removes all the elements in the arena, making it empty.
 * The memory allocated for the elements is not released by this function.
 *
 * \param arena Pointer to the vl_arena structure.
 */
void        vlArenaClear(vl_arena* arena);


/**
 * \brief Clones the specified arena to another.
 *
 * Clones the entirety of the src arena to the dest arena.
 *
 * The 'src' arena pointer must be non-null and initialized.
 * The 'dest' arena pointer may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' arena pointer is null, a new arena is created via vlArenaNew.
 *
 * \param src pointer
 * \param dest pointer
 * \return pointer to arena that was copied to or created.
 */
vl_arena*    vlArenaClone(const vl_arena* src, vl_arena* dest);

/**
 * \brief Reserves storage in the underlying allocation of the given arena.
 *
 * This is done by doubling the size until the requested growth is met or exceeded.
 * This function will always result in the reallocation of the underlying memory.
 *
 * \param arena pointer
 * \param numBytes total bytes to reserve
 */
void        vlArenaReserve(vl_arena* arena, vl_memsize_t numBytes);

/**
 * \brief Take memory from the given arena.
 *
 * This function is used to allocate memory from the specified arena.
 * Memory is allocated by finding a free block of memory of equal or greater size
 * within the internally managed block of memory.
 *
 * \param arena A pointer to the arena from which memory will be allocated.
 * \param size The size of the memory to be allocated in bytes.
 *
 * \return A pointer to the allocated memory, or NULL if allocation fails.
 */
vl_arena_ptr vlArenaMemAlloc(vl_arena* arena, vl_memsize_t size);

/**
 * \brief Reallocates memory for the given pointer in the given arena.
 *
 * This function reallocates memory for a previously allocated block of memory
 * in the specified arena. If the reallocation is successful, the old block of
 * memory will be freed and the new block will have a size specified by the 'size'
 * parameter.
 *
 * \param arena The arena to perform the reallocation on.
 * \param ptr The pointer to the previously allocated block of memory.
 * \param size The desired size of the newly reallocated block of memory.
 *
 * \return A pointer to the newly reallocated block of memory, or NULL if reallocation fails.
 * \note The contents of the memory block pointed to by 'ptr' are preserved up to the lesser of the new and old sizes.
 */
vl_arena_ptr    vlArenaMemRealloc(vl_arena* arena, vl_arena_ptr ptr, vl_memsize_t size);

/**
 * \brief Frees a memory block allocated in a vl_arena.
 *
 * This function frees the memory block referenced by the given pointer from the
 * specified vl_arena. The memory block must have been previously allocated using
 * the vlArenaMemAlloc() or vlArenaMemRealloc() functions.
 *
 * \param arena The vl_arena structure representing the arena.
 * \param ptr The pointer to the memory block to be freed.
 *
 * \note The behavior is undefined if ptr does not point to a memory block allocated in the specified arena.
 *
 * \sa vlArenaMemAlloc()
 * \sa vlArenaMemRealloc()
 */
void        vlArenaMemFree(vl_arena* arena, vl_arena_ptr ptr);

/**
 * \brief Copies a block of memory to the end of the specified arena allocation
 *
 * This function handles the case wherein the specified source pointer resides within the arena.
 *
 * \param arena pointer
 * \param dst arena pointer
 * \param src memory to copy from
 * \param length number of bytes from src
 * \par Complexity of O(n) linear.
 * \return new dst arena pointer
 */
vl_arena_ptr    vlArenaMemPrepend(vl_arena* arena, vl_arena_ptr dst, const void* src, vl_memsize_t length);

/**
 * \brief Copies a block of memory to the beginning of the specified arena allocation
 *
 * This function handles the case wherein the specified source pointer resides within the arena.
 *
 * \param arena pointer
 * \param dst arena pointer
 * \param src memory to copy from
 * \param length number of bytes from src
 * \par Complexity of O(n) linear.
 * \return new dst arena pointer
 */
vl_arena_ptr    vlArenaMemAppend(vl_arena* arena, vl_arena_ptr dst, const void* src, vl_memsize_t length);

/**
 * \brief Sampling function that calculates a transient pointer into the specified arena.
 *
 * \param arena The arena on which the operation is performed.
 * \param ptr The arena pointer indicating the memory location on the arena.
 */
vl_transient*       vlArenaMemSample(vl_arena* arena, vl_arena_ptr ptr);

/**
 * \brief Get the size of a memory block allocated in a VL arena.
 *
 * This function returns the size of the memory block allocated at the specified pointer
 * in a VL arena. The size is calculated based on the arena's metadata and may not represent
 * the exact size of the data stored in the block.
 *
 * \param arena Pointer to the VL arena.
 * \param ptr Pointer to the memory block.
 *
 * \return Size of the memory block allocated at the specified pointer in the VL arena.
 *
 * \note This function assumes that the provided pointer is a valid memory block allocated
 *       in the specified VL arena. Passing an invalid or previously freed pointer
 *       to this function may result in undefined behavior.
 */
vl_memsize_t      vlArenaMemSize(vl_arena* arena, vl_arena_ptr ptr);

/**
 * \brief Get the total capacity of the arena.
 *
 * This function returns the total capacity of the given arena, in bytes.
 * The capacity represents the maximum number of elements that
 * the arena can store without requiring reallocation.
 *
 * \param arena Pointer to the arena structure.
 * \return The total capacity of the arena.
 */
vl_memsize_t      vlArenaTotalCapacity(vl_arena* arena);

/**
 * \brief Get the total amount of free memory in the arena.
 *
 * This function returns the total amount of free memory in the specified arena.
 *
 * \param arena The pointer to the arena structure.
 * \return The total amount of free memory in the arena.
 */
vl_memsize_t      vlArenaTotalFree(vl_arena* arena);

#endif //VL_ARENA_H