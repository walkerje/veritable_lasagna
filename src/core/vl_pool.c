#include "vl_pool.h"

#include <memory.h>

/**
 * \brief Union for handling pool indices in an endian-independent way
 *
 * This union provides safe access to node and element indices regardless
 * of the platform's endianness. The internal structure is automatically
 * adjusted based on the platform's byte order at compile time.
 *
 * \private
 */

typedef union
{
    VL_POOL_INDEX_T idx;
    /**
     * \private
     */
    struct
    {
#ifdef VL_SYSTEM_LITTLE_ENDIAN
        VL_POOL_ORDINAL_T nodeIndex;
        VL_POOL_ORDINAL_T elementIndex;
#else
        VL_POOL_ORDINAL_T elementIndex;
        VL_POOL_ORDINAL_T nodeIndex;
#endif
    };
} vl_pool_ordinal;

vl_pool_node* vl_PoolNodeNew(vl_pool* pool)
{
    const VL_POOL_ORDINAL_T blockSize = (VL_POOL_ORDINAL_T)pool->growthIncrement;
    const vl_uint16_t alignedHeaderSize = VL_MEMORY_PAD_UP(sizeof(vl_pool_node), pool->elementAlign);

    vl_pool_node* node =
        (vl_pool_node*)vlMemAllocAligned(alignedHeaderSize + (pool->elementSize * blockSize), pool->elementAlign);

    if (node == NULL)
        return NULL;

    node->totalTaken = 0;
    node->blockSize = blockSize;
    node->elements = (void*)VL_MEMORY_PAD_UP((vl_uintptr_t)(node) + sizeof(vl_pool_node), pool->elementAlign);

    node->nextLookup = (vl_pool_node*)pool->lookupHead;
    node->blockOrdinal = pool->lookupTotal;

    pool->lookupHead = node;
    pool->growthIncrement *= 2;

    if (pool->lookupTotal >= pool->lookupCapacity)
    {
        const vl_dsidx_t prevCapacity = pool->lookupCapacity;
        pool->lookupCapacity *= 2;
        pool->lookupTable = (void*)vlMemRealloc((vl_memory*)pool->lookupTable, sizeof(void*) * pool->lookupCapacity);
        memset(pool->lookupTable + prevCapacity, 0, sizeof(void*) * prevCapacity);
    }

    pool->lookupTable[node->blockOrdinal] = node;
    pool->lookupTotal++;

    return node;
}

void vlPoolInitAligned(vl_pool* pool, vl_uint16_t elementSize, vl_uint16_t alignment)
{
    if (pool == NULL)
        return;

    pool->elementSize = VL_MEMORY_PAD_UP(elementSize, alignment);
    pool->elementAlign = alignment;

    { // free stack
        pool->freeCapacity = VL_POOL_DEFAULT_SIZE;
        pool->freeStack = (vl_pool_idx*)vlMemAlloc(sizeof(vl_pool_idx) * pool->freeCapacity);
        pool->freeTop = pool->freeStack;

        if (!pool->freeStack)
            return;
    }

    pool->growthIncrement = VL_POOL_DEFAULT_SIZE;
    pool->lookupCapacity = VL_POOL_DEFAULT_SIZE;

    pool->lookupTable = (void*)vlMemAlloc(sizeof(void*) * pool->lookupCapacity);
    if (pool->lookupTable == NULL)
        return;

    memset(pool->lookupTable, 0, sizeof(void*) * pool->lookupCapacity);
    pool->lookupHead = NULL;
    pool->lookupTotal = 0;

    (*pool->lookupTable) = vl_PoolNodeNew(pool);
    if (*(pool->lookupTable) == NULL)
    {
        vlMemFree((vl_memory*)pool->lookupTable);
        return;
    }
    pool->growthIncrement = VL_POOL_DEFAULT_SIZE;
}

vl_pool_idx vlPoolTake(vl_pool* pool)
{
    vl_pool_ordinal result = {.idx = 0};
    vl_pool_node* curNode;

    if (pool->freeStack < pool->freeTop)
    {
        pool->freeTop--;
        result.idx = *pool->freeTop;

        curNode = pool->lookupTable[result.nodeIndex];
        curNode->totalTaken++;

        return result.idx;
    }

    curNode = pool->lookupHead;
    while (curNode != NULL)
    {
        // skip expended node.
        if (curNode->totalTaken >= curNode->blockSize)
        {
            curNode = curNode->nextLookup;
            continue;
        }

        const vl_pool_ordinal ordinal = {.nodeIndex = curNode->blockOrdinal, .elementIndex = curNode->totalTaken};

        result.idx = ordinal.idx;
        curNode->totalTaken++;

        return result.idx;
    }

    // handle no suitable node (spawn a new node and take an element!)
    curNode = vl_PoolNodeNew(pool);
    if (curNode == NULL)
        return VL_POOL_INVALID_IDX;

    result.nodeIndex = (VL_POOL_ORDINAL_T)curNode->blockOrdinal;
    curNode->totalTaken++;

    return result.idx;
}

void vlPoolReturn(vl_pool* pool, vl_pool_idx idx)
{
    const vl_pool_ordinal ordinal = {.idx = idx};

    vl_pool_node* node = pool->lookupTable[ordinal.nodeIndex];
    node->totalTaken--;

    const vl_dsidx_t distance = (vl_dsidx_t)(pool->freeTop - pool->freeStack);
    if (distance >= pool->freeCapacity)
    {
        pool->freeCapacity *= 2;
        pool->freeStack = (void*)vlMemRealloc((vl_memory*)pool->freeStack, pool->freeCapacity * sizeof(vl_pool_idx));
        pool->freeTop = pool->freeStack + distance;
    }

    *pool->freeTop = idx;
    pool->freeTop++;
}

void* vlPoolSample(vl_pool* pool, vl_pool_idx idx)
{
    const vl_pool_ordinal ordinal = {.idx = idx};
    const vl_pool_node* node = pool->lookupTable[ordinal.nodeIndex];
    return ((vl_uint8_t*)node->elements) + (ordinal.elementIndex * pool->elementSize);
}

void vlPoolClear(vl_pool* pool)
{
    vl_pool_node* curNode = (vl_pool_node*)pool->lookupHead;

    while (curNode != NULL)
    {
        curNode->totalTaken = 0;
        curNode = curNode->nextLookup;
    }

    pool->freeTop = pool->freeStack;
}

void vlPoolFree(vl_pool* pool)
{
    {
        vl_pool_node *curNode, *temp;

        curNode = (vl_pool_node*)pool->lookupHead;

        while (curNode != NULL)
        {
            temp = curNode->nextLookup;
            vlMemFree((vl_memory*)curNode);
            curNode = temp;
        }
    }

    vlMemFree((vl_memory*)pool->freeStack);
    vlMemFree((vl_memory*)pool->lookupTable);

    pool->elementSize = 0;
    pool->growthIncrement = 0;
    pool->lookupTotal = 0;
    pool->lookupCapacity = 0;

    pool->lookupTable = NULL;
    pool->lookupHead = NULL;
    pool->freeStack = NULL;
    pool->freeTop = NULL;
}

void vlPoolReset(vl_pool* pool)
{
    vl_pool_node *curNode = pool->lookupHead, *temp;

    while (curNode)
    {
        curNode->totalTaken = 0;

        temp = curNode->nextLookup;

        if (temp == NULL)
            break;

        pool->lookupTable[curNode->blockOrdinal] = NULL;
        pool->lookupTotal--;

        vlMemFree((vl_memory*)curNode);

        curNode = temp;
    }

    pool->growthIncrement = curNode->blockSize;
    pool->lookupHead = curNode;

    pool->freeTop = pool->freeStack;
}

void vlPoolReserve(vl_pool* pool, vl_dsidx_t n)
{
    if (n == 0)
        return;

    vl_dsidx_t available = 0;

    // Count free capacity across all nodes
    for (vl_dsidx_t i = 0; i < pool->lookupTotal; i++)
    {
        const vl_pool_node* node = pool->lookupTable[i];
        if (node != NULL)
            available += (node->blockSize - node->totalTaken);
    }

    // Allocate nodes until we have enough space
    while (available < n)
    {
        vl_pool_node* newNode = vl_PoolNodeNew(pool);
        available += newNode->blockSize;
    }
}

vl_pool* vlPoolClone(const vl_pool* src, vl_pool* dest)
{
    if (dest == NULL)
        dest = vlPoolNewAligned(src->elementSize, src->elementAlign);
    else
    {
        // prepare existing pool.
        vlPoolReset(dest);
    }

    dest->growthIncrement = src->growthIncrement;
    dest->elementSize = src->elementSize;
    dest->elementAlign = src->elementAlign;
    dest->lookupTotal = src->lookupTotal;
    dest->lookupCapacity = src->lookupCapacity;

    if (dest->lookupCapacity < src->lookupCapacity)
    {
        dest->lookupTable =
            (void*)vlMemRealloc((vl_memory*)(vl_memory*)dest->lookupTable, src->lookupCapacity * sizeof(void*));
        dest->lookupCapacity = src->lookupCapacity;
    }

    {
        const size_t freeStackSize = src->freeCapacity * sizeof(vl_pool_idx);
        const vl_dsidx_t freeDistance = (vl_dsidx_t)(src->freeTop - src->freeStack);
        if (dest->freeCapacity < src->freeCapacity)
        {
            dest->freeStack = (void*)vlMemRealloc((vl_memory*)dest->freeStack, freeStackSize);
            dest->freeCapacity = src->freeCapacity;
        }

        memcpy(dest->freeStack, src->freeStack, freeStackSize);
        dest->freeTop = dest->freeStack + freeDistance;
    }

    vl_pool_node *srcNode, *destNode;

    dest->lookupHead = NULL;
    for (vl_uint32_t i = 0; i <= src->lookupHead->blockOrdinal; i++)
    {
        srcNode = src->lookupTable[i];

        if (dest->lookupTable[i] != NULL)
            vlMemFree((vl_memory*)dest->lookupTable[i]);

        destNode = (void*)vlMemClone((vl_memory*)srcNode);
        destNode->elements =
            (void*)VL_MEMORY_PAD_UP((vl_uintptr_t)(destNode) + sizeof(vl_pool_node), src->elementAlign);
        destNode->nextLookup = (vl_pool_node*)dest->lookupHead;

        dest->lookupTable[destNode->blockOrdinal] = destNode;
        dest->lookupHead = destNode;
    }

    return dest;
}

vl_pool* vlPoolNewAligned(vl_uint16_t elementSize, vl_uint16_t alignment)
{
    vl_pool* result = (vl_pool*)malloc(sizeof(vl_pool));

    vlPoolInitAligned(result, elementSize, alignment);

    return result;
}

void vlPoolDelete(vl_pool* pool)
{
    vlPoolFree(pool);
    free(pool);
}
