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

#ifndef VL_FIXED_POOL
#define VL_FIXED_POOL

#include "vl_memory.h"
#include "vl_numtypes.h"

#ifndef VL_POOL_DEFAULT_SIZE
#define VL_POOL_DEFAULT_SIZE 16
#endif

/**
 * Configure pool index type based on availability of integer types.
 *
 * 64-bit configurations use a 32-bit index and 32-bit ordinal.
 * 32-bit configurations use a 16-bit index and 16-bit ordinal.
 * 16-bit configurations use a 8-bit index and 8-bit ordinal.
 */
#if defined(VL_U64_T) && defined(VL_U32_T)
#define VL_POOL_INDEX_T VL_U64_T
#define VL_POOL_ORDINAL_T VL_U32_T
#define VL_POOL_LOOKUP_MAX 0xFFFFFFFF
#define VL_POOL_INVALID_IDX 0xFFFFFFFFFFFFFFFF
#elif defined(VL_U32_T) && defined(VL_U16_T)
#define VL_POOL_INDEX_T VL_U32_T
#define VL_POOL_ORDINAL_T VL_U16_T
#define VL_POOL_LOOKUP_MAX 0xFFFF
#define VL_POOL_INVALID_IDX 0xFFFFFFFF
#elif defined(VL_U16_T) && defined(VL_U8_T)
#define VL_POOL_INDEX_T VL_U16_T
#define VL_POOL_ORDINAL_T VL_U8_T
#define VL_POOL_LOOKUP_MAX 0xFF
#define VL_POOL_INVALID_IDX 0xFFFF
#else
#error Failed to configure vl_pool types.
#endif

/**
 * Integer index type for a given pool.
 */
typedef VL_POOL_INDEX_T vl_pool_idx;

typedef struct vl_pool_node vl_pool_node;

/**
 * \brief Node structure of pools.
 *
 * These are headers for blocks of memory allocated by fixed pools.
 * Free nodes are tracked implicitly using a counter.
 *
 * \private
 */
typedef struct vl_pool_node
{
    VL_POOL_ORDINAL_T totalTaken; // Total elements taken from this node.
    VL_POOL_ORDINAL_T blockSize; // Size of this node, in elements
    VL_POOL_ORDINAL_T blockOrdinal; // Ordinal location of this block.
    vl_pool_node* nextLookup; // Pointer to the next element in the chain.
    void* elements; // Pointer to element data
} vl_pool_node;

/**
 * \brief Fixed-size memory pool with stable indices and geometric growth.
 *
 * The vl_pool structure implements a memory allocator optimized for managing
 * large numbers of fixed-size elements with O(1) allocation, deallocation,
 * and access by index.
 *
 * Elements are allocated from internally managed blocks that grow
 * geometrically. Each element is identified by a compact integer index
 * (`vl_pool_idx`) which encodes both the block ordinal and the element ordinal
 * within that block.
 *
 * ## Key Properties
 * - Fixed-size elements with configurable alignment
 * - Stable memory addresses for all live elements
 * - O(1) amortized allocation and deallocation
 * - O(1) indexed access via `vlPoolSample`
 * - Geometric growth to minimize allocation overhead
 *
 * ## Index Semantics
 * Each element is identified by a `vl_pool_idx`, which is an opaque packed
 * value containing:
 * - a block ordinal (identifying which internal block owns the element)
 * - an element ordinal (index within that block)
 *
 * Indices are valid from the time they are returned by `vlPoolTake` until
 * they are returned via `vlPoolReturn`. Indices must not be reused or
 * dereferenced after:
 * - `vlPoolClear`
 * - `vlPoolReset`
 * - `vlPoolFree`
 *
 * No bounds checking is performed when sampling indices; supplying an invalid
 * index results in undefined behavior.
 *
 * ## Growth Behavior
 * - The pool begins with a single block of `VL_POOL_DEFAULT_SIZE` elements
 * - Each subsequently allocated block doubles in size
 * - Blocks are never shrunk except by `vlPoolReset` or `vlPoolFree`
 *
 * This growth strategy provides amortized O(1) allocation while limiting
 * fragmentation and allocation frequency.
 *
 * ## Free List Behavior
 * Returned indices are stored in an internal free stack and reused before
 * allocating new elements from blocks. This ensures efficient reuse of
 * memory without relocating elements.
 *
 * ## Thread Safety
 * This structure is **not thread-safe**.
 *
 * Concurrent calls to `vlPoolTake`, `vlPoolReturn`, or `vlPoolSample` must be
 * externally synchronized.
 *
 * ## Memory Ownership
 * - All memory associated with the pool is owned by the pool
 * - Pointers returned by `vlPoolSample` remain valid until the element is
 *   returned or the pool is cleared/reset/freed
 * - The user must not free memory returned by the pool directly
 *
 * ## Typical Use Cases
 * - Object pools with stable handles
 * - Entity/component storage
 * - Resource indexing systems
 * - High-performance fixed-size allocations
 */
typedef struct vl_pool
{
    vl_uint16_t elementSize; /**< Size of each element in bytes (aligned). */
    vl_uint16_t elementAlign; /**< Alignment of each element in bytes. */

    vl_pool_idx growthIncrement; /**< Size (in elements) of the next block to allocate. */

    vl_dsidx_t lookupTotal; /**< Number of active block entries in the lookup table. */
    vl_dsidx_t lookupCapacity; /**< Allocated capacity of the lookup table. */

    vl_pool_node** lookupTable; /**< Direct-access table mapping block ordinals to blocks. */
    vl_pool_node* lookupHead; /**< Head of the internal block list. */

    vl_dsidx_t freeCapacity; /**< Capacity of the free index stack. */
    vl_pool_idx* freeStack; /**< Base of the free index stack. */
    vl_pool_idx* freeTop; /**< Pointer to the next free slot in the stack. */
} vl_pool;

/**
 * \brief Initializes the specified fixed pool instance with specified element size and alignment.
 *
 * This pool should be de-initialized via vlPoolFree.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the `pool` struct. The function initializes internal management
 * structures.
 * - **Lifetime**: The `pool` struct must remain valid for the duration of its use. Internal allocations are valid until
 * `vlPoolFree` or `vlPoolDelete`.
 * - **Thread Safety**: Not thread-safe. Concurrent access must be synchronized.
 * - **Nullability**: If `pool` is `NULL`, the function returns immediately (no-op).
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: `alignment` is not a power of 2. Passing an already initialized pool without first calling
 * `vlPoolFree` (causes memory leak).
 * - **Memory Allocation Expectations**: Allocates initial lookup table, free index stack, and the first memory block
 * node via `vlMemAlloc` and `vlMemAllocAligned`.
 * - **Return-value Semantics**: None (void).
 *
 * \warning alignment must be a power of 2.
 *
 * \sa vlPoolFree
 * \param pool pointer
 * \param elementSize size of each element, in bytes.
 * \param alignment alignment of each element, in bytes.
 * \par Complexity O(1) constant.
 */
VL_API void vlPoolInitAligned(vl_pool* pool, vl_uint16_t elementSize, vl_uint16_t alignment);

/**
 * \brief Initializes the specified fixed pool instance.
 *
 * This pool should be freed via vlPoolFree.
 *
 * \sa vlPoolFree
 * \param pool pointer
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 */
static inline void vlPoolInit(vl_pool* pool, vl_uint16_t elementSize)
{
    vlPoolInitAligned(pool, elementSize, VL_DEFAULT_MEMORY_ALIGN);
}

/**
 * \brief De-initializes the specified pool instance.
 *
 * This will clear up all memory associated with members of the pool, including all allocated block nodes and management
 * tables. This pool should have been initialized via vlPoolInit(Ext).
 *
 * ## Contract
 * - **Ownership**: Releases ownership of all internally allocated memory blocks and management structures. Does NOT
 * release the `pool` struct itself.
 * - **Lifetime**: All indices previously returned by `vlPoolTake` and all pointers from `vlPoolSample` become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `pool` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double free.
 * - **Memory Allocation Expectations**: Deallocates all memory blocks and management structures via `vlMemFree`.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlPoolInit
 * \param pool pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlPoolFree(vl_pool* pool);

/**
 * \brief Allocates and initializes a new fixed pool instance with specific alignment.
 *
 * This pool should later be deleted via vlPoolDelete.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_pool` pointer and is responsible for calling `vlPoolDelete`.
 * - **Lifetime**: The pool instance is valid until it is passed to `vlPoolDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `NULL` if heap allocation for the `vl_pool` struct fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: `alignment` is not a power of 2.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_pool` struct and its management structures.
 * - **Return-value Semantics**: Returns a pointer to the newly allocated and initialized pool, or `NULL`.
 *
 * \warning alignment must be a power of 2.
 *
 * \sa vlPoolDelete
 * \param elementSize size of each element, in bytes.
 * \param alignment alignment of each element, in bytes.
 * \par Complexity O(1) constant.
 * \return pointer to pool instance
 */
VL_API vl_pool* vlPoolNewAligned(vl_uint16_t elementSize, vl_uint16_t alignment);

/**
 * \brief Allocates and initializes a fixed pool instance.
 *
 * This pool should later be deleted via vlPoolDelete.
 *
 * \sa vlPoolDelete
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 * \return pointer to pool instance
 */
static inline vl_pool* vlPoolNew(vl_uint16_t elementSize)
{
    return vlPoolNewAligned(elementSize, VL_DEFAULT_MEMORY_ALIGN);
}

/**
 * \brief De-initializes and deletes the specified fixed pool instance.
 *
 * This frees all internally allocated memory blocks, management structures, and the `vl_pool` struct itself.
 * This pool should have been initialized via vlPoolNew(Ext).
 *
 * ## Contract
 * - **Ownership**: Releases ownership of all internally allocated memory blocks, management structures, and the
 * `vl_pool` struct.
 * - **Lifetime**: All indices, pointers, and the pool struct itself become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `pool` should not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Deallocates all pool-related memory via `vlMemFree` and `free`.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlPoolNew
 * \param pool pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlPoolDelete(vl_pool* pool);

/**
 * \brief Takes a new index from the fixed pool, which corresponds to a valid memory
 * location within the pool.
 *
 * If no free slots are available in existing blocks, a new memory block node is automatically allocated.
 *
 * ## Contract
 * - **Ownership**: The pool owns the memory associated with the returned index.
 * - **Lifetime**: The index is valid until it is returned via `vlPoolReturn` or the pool is destroyed/reset.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `VL_POOL_INVALID_IDX` if allocation fails when growing the pool.
 * - **Error Conditions**: Returns `VL_POOL_INVALID_IDX` on allocation failure.
 * - **Undefined Behavior**: Passing an uninitialized pool.
 * - **Memory Allocation Expectations**: May trigger heap allocation for new block nodes or lookup table/free stack
 * expansion via `vlMemAllocAligned` and `vlMemRealloc`.
 * - **Return-value Semantics**: Returns a handle (`vl_pool_idx`) to an element, or `VL_POOL_INVALID_IDX` on failure.
 *
 * \param pool pointer
 * \par Complexity O(1) constant.
 * \return index of new pool item.
 */
VL_API vl_pool_idx vlPoolTake(vl_pool* pool);

/**
 * \brief Gives the specified index back to the fixed pool, allowing it to be re-used.
 *
 * ## Contract
 * - **Ownership**: Transfers ownership of the element slot back to the pool.
 * - **Lifetime**: The index becomes invalid for the caller immediately.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `idx` can be `VL_POOL_INVALID_IDX` (no-op).
 * - **Error Conditions**: None (potential reallocation failure of free stack is not handled in current implementation).
 * - **Undefined Behavior**: Passing an index that was not previously taken or already returned.
 * - **Memory Allocation Expectations**: May trigger heap reallocation of the internal free index stack.
 * - **Return-value Semantics**: None (void).
 *
 * \param pool pointer
 * \param idx element index
 * \par Complexity O(1) constant.
 */
VL_API void vlPoolReturn(vl_pool* pool, vl_pool_idx idx);

/**
 * \brief Samples the specified fixed pool and retrieves a pointer to the memory
 * associated with the specified pool index.
 *
 * ## Contract
 * - **Ownership**: Ownership remains with the pool.
 * - **Lifetime**: The returned pointer is valid until the element is returned, or the pool is cleared/reset/destroyed.
 * - **Thread Safety**: Safe for concurrent reads if no other thread is writing to the same element or reallocating pool
 * management structures.
 * - **Nullability**: Returns `NULL` if `idx` is `VL_POOL_INVALID_IDX` or if the block associated with the index is not
 * found.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid index (e.g., out of bounds, already returned).
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a pointer to the raw element data, or `NULL`.
 *
 * \param pool pointer to the fixed pool
 * \param idx numeric index of pooled memory instance
 * \par Complexity O(1) constant.
 * \return pointer to element data.
 */
VL_API void* vlPoolSample(vl_pool* pool, vl_pool_idx idx);

/**
 * \brief Clears the specified pool.
 *
 * Resets each underlying memory block's counter to 0 and resets the free index stack. This effectively marks all pooled
 * slots as available for reuse.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: All previously taken indices and their associated pointers from `vlPoolSample` become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `pool` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param pool pool pointer
 * \par Complexity O(n) linear where n is the number of blocks.
 */
VL_API void vlPoolClear(vl_pool* pool);

/**
 * \brief Resets the specified pool to its initial state.
 *
 * Deletes all memory block nodes except for the initial one, and resets all counters and the free stack.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of all but the first memory block node.
 * - **Lifetime**: All indices and pointers become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `pool` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Deallocates block nodes via `vlMemFree`.
 * - **Return-value Semantics**: None (void).
 *
 * \param pool
 * \par Complexity O(n) linear.
 */
VL_API void vlPoolReset(vl_pool* pool);

/**
 * \brief Ensures that at least n additional elements can be taken without
 * triggering further allocations.
 *
 * This may create one or more new block nodes, following the geometric growth strategy (doubling size).
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `pool` must not be `NULL`.
 * - **Error Conditions**: Allocation failure when creating new nodes.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: May allocate multiple new block nodes and reallocate management tables.
 * - **Return-value Semantics**: None (void).
 *
 * \param pool pointer
 * \param n total additional elements to reserve space for
 */
VL_API void vlPoolReserve(vl_pool* pool, vl_dsidx_t n);

/**
 * \brief Clones the specified source fixed pool.
 *
 * Clones the entirety of the src pool to the dest pool, including all management state and element data.
 *
 * The 'src' pool pointer must be non-null and initialized.
 * The 'dest' pool pointer may be null, but if it is not null it must be
 * initialized.
 *
 * If the 'dest' pool pointer is null, a new pool is initialized via vlPoolNew.
 * Otherwise, its element size is set to the source's and the destination is
 * reset via vlPoolReset.
 *
 * ## Contract
 * - **Ownership**: If `dest` is `NULL`, the caller owns the returned `vl_pool`. If `dest` is provided, ownership
 * remains with the caller.
 * - **Lifetime**: The `dest` pool remains valid until deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` must not be `NULL`. `dest` can be `NULL`.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: Passing an uninitialized pool.
 * - **Memory Allocation Expectations**: Allocates a new `vl_pool` struct (if `dest` is `NULL`) and multiple memory
 * block nodes.
 * - **Return-value Semantics**: Returns the pointer to the cloned pool (`dest` or a new instance), or `NULL` on
 * failure.
 *
 * \sa vlPoolNew
 * \param src pointer to pool to clone
 * \param dest pointer to target pool, or NULL
 * \par Complexity O(n) linear.
 * \return dest, or pool initialized via vlPoolNew
 */
VL_API vl_pool* vlPoolClone(const vl_pool* src, vl_pool* dest);

#endif
