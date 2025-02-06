#ifndef VL_LINEARPOOL_H
#define VL_LINEARPOOL_H

#include "vl_buffer.h"

/**
 * \brief A simple pool allocator, useful for avoiding many smaller heap allocations.
 *
 * The vl_linearpool structure represents a collection of fixed-size memory blocks.
 * These memory blocks are slices of a larger allocation.
 * This is useful for certain algorithms and tasks, but also for other data structures
 * which need many instances of the same size block of memory. Linked lists and trees
 * are good examples of this requirement.
 * \sa vl_linked_list
 * \sa vl_set
 *
 * This is implemented as two instances of the vl_buffer structure,
 * and thus allocates two separate blocks of underlying memory.
 * One is used as the actual storage for the pool's elements, while
 * the other behaves as a stack and is used to manage freed blocks of memory.
 *
 * This structure DOES NOT offer pointer stability, in exchange for low-overhead memory management.
 * Consider using a Fixed Pool if pointer stability is a requirement for your use-case.
 *
 * \sa vl_fixedpool
 */
typedef struct{
    vl_memsize_t        elementSize;    //size of each element, in bytes.
    vl_dsidx_t          totalTaken;     //total number of taken elements

    vl_buffer           buffer;
    vl_buffer           freeStack;      //uses the offset member as a relative stack pointer.
} vl_linearpool;

#define VL_POOL_INVALID_IDX VL_STRUCTURE_INDEX_MAX

#ifndef vlLinearPoolSample
/**
 * Samples the specified pool and retrieves a pointer to the memory associated with the specified pool index.
 * \param poolPtr pointer to the memory pool
 * \param poolIndex numeric index of pooled memory instance
 * \return TRANSIENT POINTER to element data.
 */
#define vlLinearPoolSample(poolPtr, poolIndex) (vl_transient*)(((vl_usmall_t*)((poolPtr)->buffer.data) + ((poolIndex) * (poolPtr)->elementSize)))
#endif

typedef vl_dsidx_t vl_linearpool_idx;

/**
 * \brief Initializes the specified pool instance.
 *
 * This pool should be freed via vlLinearPoolFree.
 *
 * \sa vlLinearPoolFree
 * \param pool pointer
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 */
void            vlLinearPoolInit(vl_linearpool* pool, vl_memsize_t elementSize);

/**
 * \brief De-initializes the specified pool instance.
 * This will clear up all memory associated with members of the pool.
 * This pool should have been initialized via vlLinearPoolInit.
 * \sa vlLinearPoolInit
 * \param pool
 * \par Complexity O(1) constant.
 */
void            vlLinearPoolFree(vl_linearpool* pool);

/**
 * \brief Allocates and initializes a pool instance.
 *
 * This pool should later be deleted via vlLinearPoolDelete.
 *
 * \sa vlLinearPoolDelete
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 * \return pointer to pool instance
 */
vl_linearpool*        vlLinearPoolNew(vl_memsize_t elementSize);

/**
 * \brief De-initializes and deletes the specified pool.
 * This will clear up all memory associated with members of the pool.
 * This pool should have been initialized via vlLinearPoolNew.
 *
 * \sa vlLinearPoolNew
 * \param pool pointer
 * \par Complexity O(1) constant.
 */
void            vlLinearPoolDelete(vl_linearpool* pool);

/**
 * Clears the specified pool. This does not clear any buffers associated with the pool.
 * Rather, this function resets the "next" offset at which the next (new) pool index will be to 0.
 * \param pool pointer
 * \par Complexity O(1) constant.
 */
void            vlLinearPoolClear(vl_linearpool* pool);

/**
 * \brief Clones the specified source pool.
 *
 * Clones the entirety of the src pool to the dest pool.
 *
 * The 'src' pool pointer must be non-null and initialized.
 * The 'dest' pool pointer may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' pool pointer is null, a new pool is initialized via vlLinearPoolNew.
 * Otherwise, its element size is set to the source's and all of its existing data is replaced.
 *
 * \sa vlLinearPoolNew
 * \param src pointer to pool to clone
 * \param dest pointer to target pool, or NULL
 * \par Complexity O(1) constant.
 * \return dest, or pool initialized via vlLinearPoolNew
 */
vl_linearpool*        vlLinearPoolClone(const vl_linearpool* src, vl_linearpool* dest);

/**
 * \brief Reserves space for n-many elements in the underlying pool buffer.
 *
 * This is accomplished by using the standard buffer resizing method for this library,
 * which is doubling the size of the underlying buffer until it is greater than a
 * specified minimum size.
 *
 * This function will always result in the reallocation of the underlying memory.
 *
 * \param pool pointer
 * \param n total elements to reserve space for
 */
void            vlLinearPoolReserve(vl_linearpool* pool, vl_memsize_t n);

/**
 * \brief Calculates the index of the specified element pointer.
 * May return VL_POOL_INVALID_IDX if the specified pointer is outside the pool buffer range.
 *
 * If the underlying pool allocation is implicitly resized, make sure the specified pointer
 * still refers to a current, relevant location in memory. Otherwise, this will return VL_POOL_INVALID_IDX.
 *
 * \param pool
 * \param dataPtr
 * \par Complexity O(1) constant.
 * \return index
 */
vl_linearpool_idx     vlLinearPoolTellIndex(vl_linearpool* pool, const vl_transient* dataPtr);

/**
 * Takes a new index from the pool, which corresponds to a valid memory location within the pool.
 * \param pool pointer
 * \par Complexity O(1) constant.
 * \return index of new pool item.
 */
vl_linearpool_idx     vlLinearPoolTake(vl_linearpool* pool);

/**
 * Gives the specified index back to the pool, allowing it to be re-used.
 * \param pool
 * \param offset
 * \par Complexity O(1) constant.
 */
void            vlLinearPoolReturn(vl_linearpool* pool, vl_linearpool_idx offset);

#endif //VL_LINEARPOOL_H