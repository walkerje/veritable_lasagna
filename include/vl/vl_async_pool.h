#ifndef VL_ASYNC_POOL_H
#define VL_ASYNC_POOL_H

#include "vl_atomic.h"
#include "vl_atomic_ptr.h"

/**
 * \brief A Non-Blocking Atomic (/Asynchronous) Memory Pool
 *
 * Represents a memory pool that works in elements of a fixed size.
 * This requires notably more time and space overhead in comparison
 * to vl_linear_pool and vl_fixed_pool, but offers the benefit of thread safe taking and returning.
 *
 * \sa vl_linear_pool
 * \sa vl_fixed_pool
 *
 * This structure is implemented almost entirely using atomic primitives.
 * The action of taking/returning elements to/from this pool are entirely atomic,
 * and well-suited for multi-threaded computing.
 *
 * Blocks of memory are allocated with a size following geometric growth up to a min/max of 2^(4/16) elements,
 * after which they have reached their maximum size and further allocated blocks continue to have that size.
 *
 * This structure is non-blocking; no single operation will block execution. All threads
 * operating on this pool will continue to make progress towards their individual operations.
 * Using Compare-And-Swap (CAS), failed operations are re-tried until their success (the "wait").
 *
 * Other pool-wide operations (clearing, resetting) require the user to bring their
 * own explicit synchronization methods. Similarly, element-level synchronization and
 * thread-safety also requires user-end synchronization, if applicable.
 *
 * Taken elements will never be moved by the pool structure algorithm, but may be invalidated
 * via clearing, resetting, freeing, or deleting the pool.
 *
 * Returned nodes are pushed onto a Treiber stack for later reuse.
 * \see https://en.wikipedia.org/wiki/Treiber_stack
 *
 * The ABA problem is prevented through the use of pointer tagging.
 * \sa vl_tagged_ptr
 * \see https://en.wikipedia.org/wiki/ABA_problem
 *
 * \note    This structure is non-blocking, but is not wait-free.
 */
typedef struct{
    vl_atomic_ptr           freeStack;          // Head of the Treiber stack, storing free elements.
    vl_atomic_uint32_t      freeLength;         // Total number of nodes in the free stack.

    vl_atomic_ptr           primaryBlock;       //  Head of the list of memory blocks.
    vl_atomic_bool_t        allocatingFlag;     //  Atomic allocation flag set when allocating a new block.
    vl_atomic_uint16_t      totalBlocks;        //  Total number of allocated blocks.

    vl_uint16_t             elementSize;        //  Size, in bytes, of each pool element.
    vl_uint16_t             nodeSize;           //  Size in bytes, of an individual pool node.
} vl_async_pool;

/**
 * \private
 */
typedef struct{
    /**
     * \private
     */
    union{
        vl_uintptr_t next;
        vl_tagged_ptr pad;
    };
} vl_async_pool_header;

/**
 * \brief Byte-level alignment of individual vlAsyncPool memory blocks.
 */
extern const vl_ularge_t VL_ASYNC_POOL_BLOCK_ALIGNMENT;

/**
 * \brief Byte-level alignment of individual vlAsyncPool element nodes.
 */
extern const vl_ularge_t VL_ASYNC_POOL_NODE_ALIGNMENT;

/**
 * \brief Initializes the specified async pool.
 *
 * The pool must be later freed via vlAsyncPoolFree.
 *
 * \sa vlAsyncPoolFree
 * \param pool pointer to pool that will be initialized
 * \param elementSize total size of a single element, in bytes.
 */
void            vlAsyncPoolInit(vl_async_pool* pool, vl_uint16_t elementSize);

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
void            vlAsyncPoolFree(vl_async_pool* pool);

/**
 * \brief Allocates and initializes a new async pool.
 *
 * The specified pool must later be deleted via vlAsyncPoolDelete.
 *
 * \sa vlAsyncPoolDelete
 * \param elementSize
 * \return pointer to newly allocated async pool.
 */
vl_async_pool*  vlAsyncPoolNew(vl_uint16_t elementSize);

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
void            vlAsyncPoolDelete(vl_async_pool* pool);

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
void            vlAsyncPoolReset(vl_async_pool* pool);

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
void            vlAsyncPoolClear(vl_async_pool* pool);

/**
 * \brief Takes an element from the specified async pool.
 * \param pool pointer to async pool
 * \par Complexity of O(1) constant.
 * \return pointer to taken element
 */
void*           vlAsyncPoolTake(vl_async_pool* pool);

/**
 * \brief Returns an element to the specified async pool.
 *
 * \param pool pointer to async pool
 * \param element pointer to returned element
 * \par Complexity of O(1) constant.
 */
void       vlAsyncPoolReturn(vl_async_pool* pool, void* element);

#endif //VL_ASYNC_POOL_H
