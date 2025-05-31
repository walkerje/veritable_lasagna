#ifndef VL_FIXED_POOL
#define VL_FIXED_POOL

#include "vl_numtypes.h"
#include "vl_memory.h"

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
#define VL_POOL_INDEX_T         VL_U64_T
#define VL_POOL_ORDINAL_T       VL_U32_T
#define VL_POOL_LOOKUP_MAX      0xFFFFFFFF
#define VL_POOL_INVALID_IDX     0xFFFFFFFFFFFFFFFF
#elif defined(VL_U32_T) && defined(VL_U16_T)
#define VL_POOL_INDEX_T         VL_U32_T
#define VL_POOL_ORDINAL_T       VL_U16_T
#define VL_POOL_LOOKUP_MAX      0xFFFF
#define VL_POOL_INVALID_IDX     0xFFFFFFFF
#elif defined(VL_U16_T) && defined(VL_U8_T)
#define VL_POOL_INDEX_T         VL_U16_T
#define VL_POOL_ORDINAL_T       VL_U8_T
#define VL_POOL_LOOKUP_MAX      0xFF
#define VL_POOL_INVALID_IDX     0xFFFF
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
typedef struct vl_pool_node {
    VL_POOL_ORDINAL_T totalTaken;       //Total elements taken from this node.
    VL_POOL_ORDINAL_T blockSize;        //Size of this node, in elements
    VL_POOL_ORDINAL_T blockOrdinal;     //Ordinal location of this block.
    vl_pool_node *nextLookup;           //Pointer to the next element in the chain.
    void *elements;                     //Pointer to element data
} vl_pool_node;


/**
 * \brief Memory pool allocator with fixed-size elements and alignment support.
 *
 * The vl_pool structure implements a memory allocator optimized for fixed-size elements
 * with configurable alignment. It manages memory in blocks that grow geometrically, providing
 * efficient memory reuse and stable pointer references.
 *
 * Key Features:
 * - O(1) allocation, deallocation, and access operations
 * - Configurable element alignment
 * - Pointer stability (elements never move unless explicitly cleared/cloned)
 * - Memory reuse through free-list tracking
 * - Geometric growth for efficient scaling
 *
 * Implementation Details:
 * - Uses a singly-linked list of memory blocks
 * - Maintains a lookup table for O(1) block access
 * - Employs a free stack for quick memory reuse
 * - Supports platform-specific index sizes (16/32/64-bit)
 *
 * Memory Layout:
 * - Each block contains aligned elements
 * - Headers are padded to maintain alignment
 * - Elements are stored contiguously within blocks
 */

typedef struct {
    vl_uint16_t elementSize;                        //Size of each element, in bytes.
    vl_uint16_t elementAlign;                       //Alignment of each element
    vl_pool_idx growthIncrement;                         //Size of the next allocated block, in elements.

    vl_dsidx_t lookupTotal, lookupCapacity;                 //Number of used locations in the lookup table, and capacity thereof.
    vl_pool_node **lookupTable, *lookupHead;                //Lookup table, and the "leading" lookup element.

    vl_dsidx_t freeCapacity;                                //Size of the free stack, in elements.
    vl_pool_idx *freeStack, *freeTop;                       //Pointer to the base of the free stack, and the top.
} vl_pool;

/**
 * \brief Initializes the specified fixed pool instance, with the specified alignment.
 *
 * This pool should be freed via vlPoolFree.
 *
 * \warning alignment must be a power of 2.
 *
 * \sa vlPoolFree
 * \param pool pointer
 * \param elementSize size of each element, in bytes.
 * \par Complexity O(1) constant.
 */
VL_API void vlPoolInitAligned(vl_pool *pool, vl_uint16_t elementSize, vl_uint16_t alignment);

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
static inline void vlPoolInit(vl_pool *pool, vl_uint16_t elementSize) {
    vlPoolInitAligned(pool, elementSize, VL_DEFAULT_MEMORY_ALIGN);
}

/**
 * \brief De-initializes the specified pool instance.
 *
 * This will clear up all memory associated with members of the pool.
 *
 * This pool should have been initialized via vlPoolInit.
 * \sa vlPoolInit
 * \param pool pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlPoolFree(vl_pool *pool);


/**
 * \brief Allocates and initializes a fixed pool instance, with the specified alignment.
 *
 * This pool should later be deleted via vlPoolDelete.
 *
 * \warning alignment must be a power of 2.
 *
 * \sa vlPoolDelete
 * \param elementSize size of each element, in bytes.
 * \param alignment alignment of each element, in bytes.
 * \par Complexity O(1) constant.
 * \return pointer to pool instance
 */
VL_API vl_pool *vlPoolNewAligned(vl_uint16_t elementSize, vl_uint16_t alignment);

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
static inline vl_pool *vlPoolNew(vl_uint16_t elementSize) {
    return vlPoolNewAligned(elementSize, VL_DEFAULT_MEMORY_ALIGN);
}

/**
 * \brief De-initializes and deletes the specified fixed pool.
 * This will clear up all memory associated with members of the pool.
 * This pool should have been initialized via vlPoolNew.
 *
 * \sa vlPoolNew
 * \param pool pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlPoolDelete(vl_pool *pool);

/**
 * Takes a new index from the fixed pool, which corresponds to a valid memory location within the pool.
 * \param pool pointer
 * \par Complexity O(1) constant.
 * \return index of new pool item.
 */
VL_API vl_pool_idx vlPoolTake(vl_pool *pool);

/**
 * Gives the specified index back to the fixed pool, allowing it to be re-used.
 * \param pool pointer
 * \param idx element index
 * \par Complexity O(1) constant.
 */
VL_API void vlPoolReturn(vl_pool *pool, vl_pool_idx idx);

/**
 * Samples the specified fixed pool and retrieves a pointer to the memory associated with the specified pool index.
 * \param pool pointer to the fixed pool
 * \param idx numeric index of pooled memory instance
 * \par Complexity O(1) constant.
 * \return pointer to element data.
 */
VL_API void *vlPoolSample(vl_pool *pool, vl_pool_idx idx);

/**
 * Clears the specified pool. This does not clear any buffers associated with the pool.
 * Rather, this resets each underlying memory block's counter to 0, and resets the free stack.
 * \param pool pool pointer
 * \par Complexity O(n) linear.
 */
VL_API void vlPoolClear(vl_pool *pool);

/**
 * Resets the specified pool. This deletes all but the initial memory block.
 * This leaves the pool in a "new" state, freeing memory in the process.
 *
 * \param pool
 * \par Complexity O(n) linear.
 */
VL_API void vlPoolReset(vl_pool *pool);

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
VL_API void vlPoolReserve(vl_pool *pool, vl_dsidx_t n);

/**
 * \brief Clones the specified source fixed pool.
 *
 * Clones the entirety of the src pool to the dest pool.
 *
 * The 'src' pool pointer must be non-null and initialized.
 * The 'dest' pool pointer may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' pool pointer is null, a new pool is initialized via vlPoolNew.
 * Otherwise, its element size is set to the source's and the destination is reset via vlPoolReset.
 *
 * \sa vlPoolNew
 * \param src pointer to pool to clone
 * \param dest pointer to target pool, or NULL
 * \par Complexity O(n) linear.
 * \return dest, or pool initialized via vlPoolNew
 */
VL_API vl_pool *vlPoolClone(const vl_pool *src, vl_pool *dest);

#endif