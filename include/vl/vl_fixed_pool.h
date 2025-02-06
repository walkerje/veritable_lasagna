#ifndef VL_FIXED_POOL
#define VL_FIXED_POOL

#include "vl_numtypes.h"

#ifndef VL_FIXEDPOOL_DEFAULT_SIZE
#define VL_FIXEDPOOL_DEFAULT_SIZE 16
#endif

/**
 * Configure fixed pool index type based on availability of integer types.
 *
 * 64-bit configurations use a 32-bit index and 32-bit ordinal.
 * 32-bit configurations use a 16-bit index and 16-bit ordinal.
 * 16-bit configurations use a 8-bit index and 8-bit ordinal.
 */
#if defined(VL_U64_T) && defined(VL_U32_T)
#define VL_FIXEDPOOL_INDEX_T         VL_U64_T
#define VL_FIXEDPOOL_ORDINAL_T       VL_U32_T
#define VL_FIXEDPOOL_SHIFT           32
#define VL_FIXEDPOOL_LOOKUP_MAX      0xFFFFFFFF
#define VL_FIXEDPOOL_MASK            0x00000000FFFFFFFF
#define VL_FIXEDPOOL_INVALID_IDX     0xFFFFFFFFFFFFFFFF
#elif defined(VL_U32_T) && defined(VL_U16_T)
#define VL_FIXEDPOOL_INDEX_T         VL_U32_T
#define VL_FIXEDPOOL_ORDINAL_T       VL_U16_T
#define VL_FIXEDPOOL_SHIFT           16
#define VL_FIXEDPOOL_LOOKUP_MAX      0xFFFF
#define VL_FIXEDPOOL_MASK            0x0000FFFF
#define VL_FIXEDPOOL_INVALID_IDX     0xFFFFFFFF
#elif defined(VL_U16_T) && defined(VL_U8_T)
#define VL_FIXEDPOOL_INDEX_T         VL_U16_T
#define VL_FIXEDPOOL_ORDINAL_T       VL_U8_T
#define VL_FIXEDPOOL_SHIFT           8
#define VL_FIXEDPOOL_LOOKUP_MAX      0xFF
#define VL_FIXEDPOOL_MASK            0x00FF
#define VL_FIXEDPOOL_INVALID_IDX     0xFFFF
#else
#error Failed to configure vl_linearpool types.
#endif

/**
 * Integer index type for a given pool.
 */
typedef VL_FIXEDPOOL_INDEX_T     vl_fixedpool_idx;

typedef struct vl_fixed_pool_node vl_fixed_pool_node;

/**
 * \brief Node structure of fixed pools.
 *
 * These are headers for blocks of memory allocated by fixed pools.
 * Free nodes are tracked implicitly using a counter.
 *
 * \private
 */
typedef struct vl_fixed_pool_node{
    VL_FIXEDPOOL_ORDINAL_T              totalTaken;     //Total elements taken from this node.
    VL_FIXEDPOOL_ORDINAL_T              blockSize;      //Size of this node, in elements
    VL_FIXEDPOOL_ORDINAL_T              blockOrdinal;   //Ordinal location of this block.
    vl_fixed_pool_node*                 nextLookup;     //Pointer to next element in the chain.
    void*                               elements;       //Pointer to element data
} vl_fixed_pool_node;


/**
 * \brief A non-trivial pool allocator, useful for avoiding many smaller heap allocations.
 *
 * The vl_fixedpool structure represents a collection of memory blocks.
 * These memory blocks are allocated as-needed, doubling in size for geometric growth.
 *
 * Elements are never moved or copied unless the pool is cleared or cloned.
 * This makes it more suitable for use cases where pointer stability is required.
 *
 * Fixed pools differ from linear pools in that the former offers pointer stability, whereas the latter does not.
 * The trade-off is more overhead in comparison to linear pools, while spending less time on growing/copying
 * a single underlying buffer.
 *
 * This is implemented as an abstraction over a singly-linked list, lookup table, and stack.
 * It offers O(1) element sampling, and O(n) element taking/returning (where n=1, in most cases).
 *
 * If pointer stability is not necessary for your use case, or low overhead memory management is necessary to you,
 * consider using a Linear Pool instead.
 *
 * \sa vl_linearpool
 */
typedef struct{
    vl_ularge_t         elementSize;                    //Size of each element, in bytes.
    vl_fixedpool_idx    growthIncrement;                //Size of the next allocated block, in elements.

    vl_dsidx_t          lookupTotal, lookupCapacity;    //Number of used locations in the lookup table, and capacity thereof.
    vl_fixed_pool_node  **lookupTable,  *lookupHead;    //Lookup table, and the "leading" lookup element.

    vl_dsidx_t          freeCapacity;                   //Size of the free stack, in elements.
    vl_fixedpool_idx    *freeStack,     *freeTop;       //Pointer to the base of the free stack, and the top.
} vl_fixedpool;

/**
 * \brief Initializes the specified fixed pool instance.
 *
 * This pool should be freed via vlFixedPoolFree.
 *
 * \sa vlFixedPoolFree
 * \param pool pointer
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 */
void vlFixedPoolInit(vl_fixedpool* pool, vl_ularge_t elementSize);

/**
 * \brief De-initializes the specified pool instance.
 *
 * This will clear up all memory associated with members of the pool.
 *
 * This pool should have been initialized via vlFixedPoolInit.
 * \sa vlFixedPoolInit
 * \param pool pointer
 * \par Complexity O(1) constant.
 */
void vlFixedPoolFree(vl_fixedpool* pool);

/**
 * \brief Allocates and initializes a fixed pool instance.
 *
 * This pool should later be deleted via vlFixedPoolDelete.
 *
 * \sa vlFixedPoolDelete
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 * \return pointer to pool instance
 */
vl_fixedpool* vlFixedPoolNew(vl_ularge_t elementSize);

/**
 * \brief De-initializes and deletes the specified fixed pool.
 * This will clear up all memory associated with members of the pool.
 * This pool should have been initialized via vlFixedPoolNew.
 *
 * \sa vlFixedPoolNew
 * \param pool pointer
 * \par Complexity O(1) constant.
 */
void vlFixedPoolDelete(vl_fixedpool* pool);

/**
 * Takes a new index from the fixed pool, which corresponds to a valid memory location within the pool.
 * \param pool pointer
 * \par Complexity O(1) constant.
 * \return index of new pool item.
 */
vl_fixedpool_idx vlFixedPoolTake(vl_fixedpool* pool);

/**
 * Gives the specified index back to the fixed pool, allowing it to be re-used.
 * \param pool pointer
 * \param idx element index
 * \par Complexity O(1) constant.
 */
void vlFixedPoolReturn(vl_fixedpool* pool, vl_fixedpool_idx idx);

#ifndef vlFixedPoolSample
/**
 * Samples the specified fixed pool and retrieves a pointer to the memory associated with the specified pool index.
 * \param pool pointer to the fixed pool
 * \param idx numeric index of pooled memory instance
 * \par Complexity O(1) constant.
 * \return FIXED POINTER to element data.
 */
#define vlFixedPoolSample(pool, idx) (void*)(((vl_usmall_t*)((pool)->lookupTable[(idx) & VL_FIXEDPOOL_MASK])->elements) + (((idx) >> VL_FIXEDPOOL_SHIFT) * (pool)->elementSize))
#endif

/**
 * Clears the specified pool. This does not clear any buffers associated with the pool.
 * Rather, this resets each underlying memory block's counter to 0, and resets the free stack.
 * \param pool fixed pool pointer
 * \par Complexity O(n) linear.
 */
void        vlFixedPoolClear(vl_fixedpool* pool);

/**
 * Resets the specified pool. This deletes all but the initial memory block.
 * This leaves the pool in a "new" state, freeing memory in the process.
 *
 * \param pool
 * \par Complexity O(n) linear.
 */
void        vlFixedPoolReset(vl_fixedpool* pool);

/**
 * \brief Ensures space for n-many elements in the pool.
 *
 * This is accomplished by using the standard buffer resizing method for this library,
 * which is doubling the size of the underlying storage until it is greater than a
 * specified minimum size. A new node may be created twice the size of all created before it.
 *
 * This function will only sometimes result in allocation of a new node.
 *
 * \param pool pointer
 * \param n total elements to reserve space for
 */
void        vlFixedPoolReserve(vl_fixedpool* pool, vl_dsidx_t n);

/**
 * \brief Clones the specified source fixed pool.
 *
 * Clones the entirety of the src pool to the dest pool.
 *
 * The 'src' pool pointer must be non-null and initialized.
 * The 'dest' pool pointer may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' pool pointer is null, a new pool is initialized via vlFixedPoolNew.
 * Otherwise, its element size is set to the source's and the destination is reset via vlFixedPoolReset.
 *
 * \sa vlFixedPoolNew
 * \param src pointer to pool to clone
 * \param dest pointer to target pool, or NULL
 * \par Complexity O(n) linear.
 * \return dest, or pool initialized via vlFixedPoolNew
 */
vl_fixedpool*  vlFixedPoolClone(const vl_fixedpool* src, vl_fixedpool* dest);

#endif