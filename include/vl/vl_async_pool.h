#ifndef VL_ASYNC_POOL_H
#define VL_ASYNC_POOL_H

#include "vl_atomic.h"
#include "vl_atomic_ptr.h"
#include "vl_memory.h"

/**
 * \brief Lock-free Memory Pool with Thread-safe Operations
 *
 * A high-performance memory allocation system designed for concurrent access,
 * providing thread-safe memory management for fixed-size elements. This pool
 * implements a non-blocking algorithm that ensures thread safety without locks.
 *
 * Key Features:
 * - Lock-free operations: All core operations are non-blocking
 * - Thread-safe: Supports concurrent access from multiple threads
 * - Fixed-size elements: Optimized for elements of consistent size
 * - Alignment control: Supports custom memory alignment requirements
 * - Memory reuse: Efficiently recycles freed elements
 *
 * Memory Management:
 * - Uses geometric growth for block allocation (2^4 to 2^16 elements)
 * - Maintains a Treiber stack for freed elements
 * - Employs tagged pointers to prevent ABA problems
 * - Alignment-aware memory allocation and access
 *
 * Performance Characteristics:
 * - Take/Return operations are O(1) amortized
 * - Non-blocking but not wait-free (operations may retry)
 * - Space overhead per element includes alignment padding
 * - Memory blocks grow geometrically up to a maximum size
 *
 * Thread Safety Notes:
 * - Take/Return operations are fully thread-safe
 * - Global operations (clear/reset) require external synchronization
 * - Element content synchronization must be handled by the caller
 *
 * Memory Layout:
 * - Elements are stored in blocks with proper alignment
 * - Each block contains a header followed by element storage
 * - Free elements form a lock-free stack
 *
 * \warning Operations that affect the entire pool (clear/reset/free) are not
 *          thread-safe and require external synchronization
 *
 * \sa vl_pool, vl_tagged_ptr
 * \related https://en.wikipedia.org/wiki/ABA_problem
 * \related https://en.wikipedia.org/wiki/Treiber_stack
 */

typedef struct {
    vl_atomic_ptr freeStack;                // Head of the Treiber stack, storing free elements.
    vl_atomic_uint32_t freeLength;          // Total number of nodes in the free stack.

    vl_atomic_ptr primaryBlock;             //  Head of the list of memory blocks.
    vl_atomic_bool_t allocatingFlag;        //  Atomic allocation flag set when allocating a new block.
    vl_atomic_uint16_t totalBlocks;         //  Total number of allocated blocks.

    vl_uint16_t elementSize;                //  Size, in bytes, of each pool element.
    vl_uint16_t elementAlign;               //  Byte alignment of each pool element.
    vl_uint16_t nodeSize;                   //  Size, in bytes, of an individual pool node.
} vl_async_pool;

/**
 * \private
 */
typedef struct {
    vl_uintptr_t next;
} vl_async_pool_header;

/**
 * \brief Initializes the specified async pool, with the specified alignment.
 *
 * The pool must be later freed via vlAsyncPoolFree.
 *
 * \warning alignment must be a power of 2.
 *
 * \sa vlAsyncPoolFree
 * \param pool pointer to pool that will be initialized
 * \param elementSize total size of a single element, in bytes.
 * \param elementAlign byte alignment of pool elements.
 */
VL_API void vlAsyncPoolInitAligned(vl_async_pool *pool, vl_uint16_t elementSize, vl_uint16_t elementAlign);

/**
 * \brief Initializes the specified async pool.
 *
 * The pool must be later freed via vlAsyncPoolFree.
 *
 * \sa vlAsyncPoolFree
 * \param pool pointer to pool that will be initialized
 * \param elementSize total size of a single element, in bytes.
 */
static inline void vlAsyncPoolInit(vl_async_pool *pool, vl_uint16_t elementSize) {
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
VL_API void vlAsyncPoolFree(vl_async_pool *pool);

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
VL_API vl_async_pool *vlAsyncPoolNewAligned(vl_uint16_t elementSize, vl_uint16_t elementAlign);

/**
 * \brief Allocates and initializes a new async pool.
 *
 * The specified pool must later be deleted via vlAsyncPoolDelete.
 *
 * \sa vlAsyncPoolDelete
 * \param elementSize
 * \return pointer to newly allocated async pool.
 */
static inline vl_async_pool *vlAsyncPoolNew(vl_uint16_t elementSize) {
    return vlAsyncPoolNewAligned(elementSize, VL_DEFAULT_MEMORY_ALIGN);
}

/**
 * \brief Deinitializes and deletes the specified async pool, and all associated memory.
 *
 * The specified pool must have been initialized via vlAsyncPoolNew.
 *
 * \warning This will invalidate all elements taken prior to this call.
 *          Manual synchronization is highly recommended.
 *
 * \sa vlAsyncPoolNew
 * \param pool pointer to async pool to delete
 */
VL_API void vlAsyncPoolDelete(vl_async_pool *pool);

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
VL_API void vlAsyncPoolReset(vl_async_pool *pool);

/**
 * \brief   Resets the state of all blocks and the pool, retaining memory but invalidating taken elements.
 *
 * This does not free any associated memory.
 *
 * \warning This will invalidate all elements taken prior to this call.
 *          Manual synchronization is highly recommended.
 *
 * \param pool pointer to the async pool to clear
 */
VL_API void vlAsyncPoolClear(vl_async_pool *pool);

/**
 * \brief Takes an element from the specified async pool.
 * \param pool pointer to async pool
 * \par Complexity of O(1) constant.
 * \return pointer to taken element
 */
VL_API void *vlAsyncPoolTake(vl_async_pool *pool);

/**
 * \brief Returns an element to the specified async pool.
 *
 * \param pool pointer to async pool
 * \param element pointer to returned element
 * \par Complexity of O(1) constant.
 */
VL_API void vlAsyncPoolReturn(vl_async_pool *pool, void *element);

#endif //VL_ASYNC_POOL_H
