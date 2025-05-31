#include "vl_async_pool.h"

//Minimum of 2^4 elements (16)
#define VL_ASYNC_POOL_BLOCK_MIN_SHIFT 4
#define VL_ASYNC_POOL_BLOCK_MIN (1 << VL_ASYNC_POOL_BLOCK_MIN_SHIFT)
//Maximum of 2^16 elements (65536)
#define VL_ASYNC_POOL_BLOCK_MAX_SHIFT 16
#define VL_ASYNC_POOL_BLOCK_MAX (1 << VL_ASYNC_POOL_BLOCK_MAX_SHIFT)

/**
 * Async pool block header.
 * \private
 */
typedef struct vl_async_block_ {
    vl_uintptr_t next, prev;
    vl_uint32_t elements;
    vl_atomic_uint32_t taken;

    void *value;
} vl_async_block;

static inline vl_bool_t vlAsyncPoolAllocate(vl_async_pool *pool) {
    vl_bool_t falseVal = VL_FALSE;

    if (!vlAtomicCompareExchangeWeakExplicit(&pool->allocatingFlag, &falseVal, VL_TRUE,
                                             VL_MEMORY_ORDER_ACQUIRE, VL_MEMORY_ORDER_RELAXED))
        return VL_FALSE;

    vl_tagged_ptr primaryBlock = vlAtomicLoad(&pool->primaryBlock);
    vl_async_block *primaryHeader = NULL;

    if (primaryBlock.ptr) {
        vl_memory *headMem = (vl_memory *) (primaryBlock.ptr);
        primaryHeader = (vl_async_block *) (headMem);

        if (primaryHeader->next) {
            vlAtomicPtrStore(&pool->primaryBlock, (void *) (primaryHeader->next));
            vlAtomicStoreExplicit(&pool->allocatingFlag, VL_FALSE, VL_MEMORY_ORDER_RELEASE);
            return VL_TRUE;
        }
    }

    const vl_uint32_t ordinal = vlAtomicLoad(&pool->totalBlocks);
    const vl_uint32_t shift = (((1 << ordinal) >= VL_ASYNC_POOL_BLOCK_MAX) ? VL_ASYNC_POOL_BLOCK_MAX_SHIFT : (ordinal >
                                                                                                              0 ?
                                                                                                              ordinal -
                                                                                                              1
                                                                                                                : ordinal));
    const vl_uint32_t elements = VL_ASYNC_POOL_BLOCK_MIN << shift;
    const vl_memsize_t headerSize = VL_MEMORY_PAD_UP(sizeof(vl_async_block), pool->elementAlign);
    const vl_memsize_t blockSize = headerSize + (elements * pool->nodeSize);

    vl_memory *block = vlMemAllocAligned(blockSize, pool->elementAlign);
    vl_async_block *header = (vl_async_block *) block;
    header->prev = (vl_uintptr_t) (primaryHeader);
    header->next = 0;
    header->elements = elements;
    header->value = block + headerSize;
    vlAtomicInit(&header->taken, 0);

    vlAtomicFetchAdd(&pool->totalBlocks, 1);
    vlAtomicPtrStore(&pool->primaryBlock, block);

    if (primaryHeader)
        primaryHeader->next = (vl_uintptr_t) (block);

    vlAtomicStoreExplicit(&pool->allocatingFlag, VL_FALSE, VL_MEMORY_ORDER_RELEASE);
    return VL_TRUE;
}

void vlAsyncPoolInitAligned(vl_async_pool *pool, vl_uint16_t elementSize, vl_uint16_t elementAlign) {
    pool->elementSize = VL_MEMORY_PAD_UP(elementSize, elementAlign);
    pool->elementAlign = elementAlign;
    pool->nodeSize = VL_MEMORY_PAD_UP(sizeof(vl_async_pool_header), elementAlign) + pool->elementSize;

    vlAtomicInit(&pool->freeStack, VL_TAGPTR_NULL);
    vlAtomicInit(&pool->freeLength, 0);

    vlAtomicInit(&pool->primaryBlock, VL_TAGPTR_NULL);
    vlAtomicInit(&pool->allocatingFlag, VL_FALSE);
    vlAtomicInit(&pool->totalBlocks, 0);

    vlAsyncPoolAllocate(pool);
}

void vlAsyncPoolFree(vl_async_pool *pool) {
    vl_tagged_ptr primaryBlock = vlAtomicLoad(&pool->primaryBlock);
    vl_uintptr_t current = primaryBlock.ptr;

    while (current) {
        vl_async_block *block = (vl_async_block *) (current);
        current = block->next;
        vlMemFree((vl_memory *) block);
    }
}

vl_async_pool *vlAsyncPoolNewAligned(vl_uint16_t elementSize, vl_uint16_t elementAlign) {
    vl_async_pool *pool = malloc(sizeof(vl_async_pool));
    vlAsyncPoolInitAligned(pool, elementSize, elementAlign);
    return pool;
}

void vlAsyncPoolDelete(vl_async_pool *pool) {
    vlAsyncPoolFree(pool);
    free(pool);
}

void vlAsyncPoolReset(vl_async_pool *pool) {
    vl_tagged_ptr primaryBlock = vlAtomicLoad(&pool->primaryBlock);
    vl_uintptr_t current = primaryBlock.ptr;
    vl_async_block *block = NULL;

    while (current) {
        block = (vl_async_block *) (current);
        vlAtomicStore(&block->taken, 0);
        current = block->prev;

        if (block->prev != 0)
            vlMemFree((vl_memory *) block);
        else
            vlAtomicPtrStore(&pool->primaryBlock, block);
    }

    block->prev = 0;
    block->next = 0;

    vlAtomicStore(&pool->freeLength, 0);
    vlAtomicStore(&pool->freeStack, VL_TAGPTR_NULL);
}

void vlAsyncPoolClear(vl_async_pool *pool) {
    vl_tagged_ptr primaryBlock = vlAtomicLoad(&pool->primaryBlock);
    vl_uintptr_t current = primaryBlock.ptr;

    while (current) {
        vl_async_block *block = (vl_async_block *) (current);
        vlAtomicStore(&block->taken, 0);
        current = block->prev;

        if (block->prev == 0)
            vlAtomicPtrStore(&pool->primaryBlock, block);
    }

    vlAtomicStore(&pool->freeLength, 0);
    vlAtomicStore(&pool->freeStack, VL_TAGPTR_NULL);
}

void *vlAsyncPoolTake(vl_async_pool *pool) {
    while (VL_TRUE) {
        vl_tagged_ptr freeTop = vlAtomicLoad(&pool->freeStack);

        if (freeTop.ptr) {
            vl_async_pool_header *topNode = (vl_async_pool_header *) freeTop.ptr;
            if (vlAtomicPtrCompareExchangeWeakExplicit(&pool->freeStack, &freeTop, (void *) topNode->next,
                                                       VL_MEMORY_ORDER_ACQUIRE, VL_MEMORY_ORDER_RELAXED)) {
                vlAtomicFetchSub(&pool->freeLength, 1);
                return (void *) ((vl_uintptr_t) topNode +
                                 VL_MEMORY_PAD_UP(sizeof(vl_async_pool_header), pool->elementAlign));
            } else continue;
        }

        //Take a fresh element from the primary block.
        vl_tagged_ptr primaryBlock = vlAtomicLoad(&pool->primaryBlock);
        vl_async_block *block = (vl_async_block *) primaryBlock.ptr;
        vl_uint32_t blockTaken = vlAtomicLoad(&block->taken);
        const vl_uint32_t nextTaken = blockTaken + 1;

        if (blockTaken >= block->elements) {
            vlAsyncPoolAllocate(pool);
            continue;
        }

        if (vlAtomicCompareExchangeWeak(&block->taken, &blockTaken, nextTaken)) {
            const vl_uintptr_t base = (vl_uintptr_t) block->value;
            return (void *) (base + (pool->nodeSize * blockTaken) +
                             VL_MEMORY_PAD_UP(sizeof(vl_async_pool_header), pool->elementAlign));

        }
    }
}

void vlAsyncPoolReturn(vl_async_pool *pool, void *element) {
    vl_uintptr_t elemAddr = (vl_uintptr_t) element;
    vl_uintptr_t headerOffset = VL_MEMORY_PAD_UP(sizeof(vl_async_pool_header), pool->elementAlign);
    vl_async_pool_header *node = (vl_async_pool_header *) (elemAddr - headerOffset);

    while (VL_TRUE) {
        vl_tagged_ptr freeTop = vlAtomicLoad(&pool->freeStack);
        node->next = freeTop.ptr;

        if (vlAtomicPtrCompareExchangeWeakExplicit(&pool->freeStack, &freeTop, node,
                                                   VL_MEMORY_ORDER_RELEASE, VL_MEMORY_ORDER_RELAXED))
            break;
    }
    vlAtomicFetchAdd(&pool->freeLength, 1);
}