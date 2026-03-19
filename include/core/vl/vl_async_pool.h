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

#ifndef VL_ASYNC_POOL_H
#define VL_ASYNC_POOL_H

#include "vl_atomic.h"
#include "vl_atomic_ptr.h"
#include "vl_memory.h"

/**
 * \brief Lock-free, thread-safe pool allocator for fixed-size elements.
 *
 * The vl_async_pool structure implements a non-blocking memory pool optimized
 * for high-performance concurrent allocation and deallocation of fixed-size
 * elements.
 *
 * Memory is managed in internally allocated blocks which grow geometrically.
 * Individual elements are returned as raw pointers and may be taken and
 * returned concurrently by multiple threads without external synchronization.
 *
 * This pool uses a Treiber stack for recycling freed elements and employs
 * tagged pointers to mitigate the ABA problem during concurrent operations.
 *
 * ## Key Properties
 * - Lock-free (non-blocking) take and return operations
 * - Thread-safe for concurrent producers and consumers
 * - Fixed-size, alignment-aware elements
 * - Pointer-based API (no indices or handles)
 * - Amortized O(1) allocation and deallocation
 *
 * ## Concurrency Guarantees
 * - `vlAsyncPoolTake` and `vlAsyncPoolReturn` are fully thread-safe
 * - Operations are lock-free but not wait-free (may retry under contention)
 * - Element memory itself is not synchronized; users must ensure safe access
 *
 * Operations that modify global pool state (clear, reset, free) are **not**
 * thread-safe and must be externally synchronized.
 *
 * ## Memory Model
 * - Elements are allocated from geometrically growing blocks
 * - Freed elements are pushed onto a lock-free free stack for reuse
 * - Blocks are never shrunk except via reset or free
 * - Returned pointers remain valid until explicitly returned or the pool
 *   is cleared, reset, or freed
 *
 * ## ABA Mitigation
 * - The free stack head uses tagged pointers
 * - This prevents stale CAS success during concurrent pop/push operations
 *
 * ## Lifetime Rules
 * - Elements returned by `vlAsyncPoolTake` must be returned exactly once
 * - Returning an element not owned by this pool results in undefined behavior
 * - Returning the same element more than once results in undefined behavior
 *
 * ## Thread Safety Notes
 * - Safe:
 *   - Concurrent `Take` / `Return`
 * - Unsafe (require external synchronization):
 *   - `Clear`
 *   - `Reset`
 *   - `Free` / `Delete`
 *
 * ## Typical Use Cases
 * - High-frequency object allocation in multi-threaded systems
 * - Job systems and task schedulers
 * - Networking buffers and message nodes
 * - Lock-free data structures requiring fast node allocation
 *
 * \warning This structure is not safe for global mutation while in use.
 *          All threads must be synchronized before clearing, resetting,
 *          or freeing the pool.
 *
 * \sa vl_pool
 * \sa vl_tagged_ptr
 */
typedef struct vl_async_pool
{
    vl_atomic_ptr freeStack; /**< Head of the Treiber free stack (tagged pointer). */
    vl_atomic_uint32_t freeLength; /**< Number of elements currently in the free stack. */

    vl_atomic_ptr primaryBlock; /**< Head of the internal block list. */
    vl_atomic_bool_t allocatingFlag; /**< Spin flag indicating block allocation in progress. */
    vl_atomic_uint16_t totalBlocks; /**< Total number of allocated blocks. */

    vl_uint16_t elementSize; /**< Size of each pool element in bytes. */
    vl_uint16_t elementAlign; /**< Byte alignment of each pool element. */
    vl_uint16_t nodeSize; /**< Total size of a pool node including header and padding. */
} vl_async_pool;

/**
 * \private
 */
typedef struct
{
    vl_uintptr_t next;
} vl_async_pool_header;

/**
 * \brief Initializes the specified async pool, with the specified alignment.
 *
 * The pool must be later freed via vlAsyncPoolFree.
 *
 * ## Contract
 * - **Ownership**: The caller provides the `pool` memory. The pool manages its own internal blocks.
 * - **Lifetime**: The pool remains valid until `vlAsyncPoolFree` or `vlAsyncPoolDelete`.
 * - **Thread Safety**: Not thread-safe for the same `pool` instance.
 * - **Nullability**: `pool` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: Does not allocate immediately; allocation is deferred until the first element
 * is taken.
 * - **Return-value Semantics**: None (void).
 *
 * \warning alignment must be a power of 2.
 *
 * \sa vlAsyncPoolFree
 * \param pool pointer to pool that will be initialized
 * \param elementSize total size of a single element, in bytes.
 * \param elementAlign byte alignment of pool elements.
 */
VL_API void vlAsyncPoolInitAligned(vl_async_pool* pool, vl_uint16_t elementSize, vl_uint16_t elementAlign);

/**
 * \brief Initializes the specified async pool.
 *
 * The pool must be later freed via vlAsyncPoolFree.
 *
 * \sa vlAsyncPoolFree
 * \param pool pointer to pool that will be initialized
 * \param elementSize total size of a single element, in bytes.
 */
static inline void vlAsyncPoolInit(vl_async_pool* pool, vl_uint16_t elementSize)
{
    vlAsyncPoolInitAligned(pool, elementSize, VL_DEFAULT_MEMORY_ALIGN);
}

/**
 * \brief Frees the specified async pool, and all associated memory.
 *
 * The pool must have been initialized via vlAsyncPoolInit.
 *
 * \warning This will invalidate all elements taken prior to this call.
 *          Manual synchronization is highly recommended.
 *
 * \sa vlAsyncPoolInit
 * \param pool
 */
VL_API void vlAsyncPoolFree(vl_async_pool* pool);

/**
 * \brief Allocates and initializes a new async pool, and specified alignment.
 *
 * The specified pool must later be deleted via vlAsyncPoolDelete.
 *
 * \warning alignment must be a power of 2.
 *
 * \sa vlAsyncPoolDelete
 * \param elementSize total size of a single element, in bytes.
 * \param elementAlign byte alignment of pool elements.
 * \return pointer to newly allocated async pool.
 */
VL_API vl_async_pool* vlAsyncPoolNewAligned(vl_uint16_t elementSize, vl_uint16_t elementAlign);

/**
 * \brief Allocates and initializes a new async pool.
 *
 * The specified pool must later be deleted via vlAsyncPoolDelete.
 *
 * \sa vlAsyncPoolDelete
 * \param elementSize
 * \return pointer to newly allocated async pool.
 */
static inline vl_async_pool* vlAsyncPoolNew(vl_uint16_t elementSize)
{
    return vlAsyncPoolNewAligned(elementSize, VL_DEFAULT_MEMORY_ALIGN);
}

/**
 * \brief Deinitializes and deletes the specified async pool, and all associated
 * memory.
 *
 * The specified pool must have been initialized via vlAsyncPoolNew.
 *
 * \warning This will invalidate all elements taken prior to this call.
 *          Manual synchronization is highly recommended.
 *
 * \sa vlAsyncPoolNew
 * \param pool pointer to async pool to delete
 */
VL_API void vlAsyncPoolDelete(vl_async_pool* pool);

/**
 * \brief   Resets the specified async pool, returning it to its state when it
 *          was first initialized.
 *
 * This frees all allocated blocks of nodes up to the first.
 *
 * \warning This will invalidate all elements taken prior to this call.
 *          Manual synchronization is highly recommended.
 *
 * \param pool pointer to the async pool to reset
 */
VL_API void vlAsyncPoolReset(vl_async_pool* pool);

/**
 * \brief   Resets the state of all blocks and the pool, retaining memory but
 * invalidating taken elements.
 *
 * This does not free any associated memory.
 *
 * \warning This will invalidate all elements taken prior to this call.
 *          Manual synchronization is highly recommended.
 *
 * \param pool pointer to the async pool to clear
 */
VL_API void vlAsyncPoolClear(vl_async_pool* pool);

/**
 * \brief Takes an element from the specified async pool.
 *
 * ## Contract
 * - **Ownership**: The pool retains ownership of the underlying memory. The caller receives a pointer to an element
 * that must eventually be returned via `vlAsyncPoolReturn`.
 * - **Lifetime**: The returned pointer is valid until it is returned to the pool or the pool is cleared/deleted.
 * - **Thread Safety**: Thread-safe (lock-free).
 * - **Nullability**: Returns `NULL` if a new block cannot be allocated when the pool is empty.
 * - **Error Conditions**: Returns `NULL` on heap allocation failure for new blocks.
 * - **Undefined Behavior**: Passing `NULL`.
 * - **Memory Allocation Expectations**: May allocate a new block of elements on the heap if the pool is empty.
 * - **Return-value Semantics**: Returns a pointer to an available element, or `NULL` on failure.
 *
 * \param pool pointer to async pool
 * \par Complexity of O(1) constant.
 * \return pointer to taken element
 */
VL_API void* vlAsyncPoolTake(vl_async_pool* pool);

/**
 * \brief Returns an element to the specified async pool.
 *
 * \param pool pointer to async pool
 * \param element pointer to returned element
 * \par Complexity of O(1) constant.
 */
VL_API void vlAsyncPoolReturn(vl_async_pool* pool, void* element);

#endif // VL_ASYNC_POOL_H
