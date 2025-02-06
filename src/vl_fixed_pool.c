#include "vl_fixed_pool.h"
#include <malloc.h>
#include <memory.h>

vl_fixed_pool_node* vl_PoolNodeNew(vl_fixedpool* pool){
    const VL_FIXEDPOOL_ORDINAL_T blockSize = pool->growthIncrement;
    vl_fixed_pool_node* node = (vl_fixed_pool_node*)malloc(sizeof(vl_fixed_pool_node) + (pool->elementSize * blockSize));

    node->totalTaken = 0;
    node->blockSize = blockSize;
    node->elements  = node + 1;
    node->nextLookup = (vl_fixed_pool_node*)pool->lookupHead;
    node->blockOrdinal = pool->lookupTotal;

    pool->lookupHead = node;
    pool->growthIncrement *= 2;

    if(pool->lookupTotal >= pool->lookupCapacity){
        const vl_dsidx_t prevCapacity = pool->lookupCapacity;
        pool->lookupCapacity *= 2;
        pool->lookupTable = realloc(pool->lookupTable, sizeof(void*) * pool->lookupCapacity);
        memset(pool->lookupTable + prevCapacity, 0, sizeof(void*) * prevCapacity);
    }

    pool->lookupTable[node->blockOrdinal] = node;
    pool->lookupTotal++;

    return node;
}

void vlFixedPoolInit(vl_fixedpool* pool, vl_ularge_t elementSize){
    if(pool == NULL)
        return;

    pool->elementSize = elementSize;

    {//free stack
        pool->freeCapacity = VL_FIXEDPOOL_DEFAULT_SIZE;
        pool->freeStack = malloc(sizeof(vl_fixedpool_idx) * pool->freeCapacity);
        pool->freeTop = pool->freeStack;
    }

    pool->growthIncrement = VL_FIXEDPOOL_DEFAULT_SIZE;
    pool->lookupCapacity = VL_FIXEDPOOL_DEFAULT_SIZE;

    pool->lookupTable = malloc(sizeof(void*) * pool->lookupCapacity);
    memset(pool->lookupTable, 0, sizeof(void*) * pool->lookupCapacity);
    pool->lookupHead = NULL;
    pool->lookupTotal = 0;

    (*pool->lookupTable) = vl_PoolNodeNew(pool);
    pool->growthIncrement = VL_FIXEDPOOL_DEFAULT_SIZE;
}

vl_fixedpool_idx vlFixedPoolTake(vl_fixedpool* pool){
    vl_fixedpool_idx result = 0;
    vl_fixed_pool_node* curNode;

    if(pool->freeStack < pool->freeTop){
        pool->freeTop--;
        result = *pool->freeTop;

        curNode = pool->lookupTable[result & VL_FIXEDPOOL_MASK];
        curNode->totalTaken++;

        return result;
    }

    curNode = pool->lookupHead;
    while(curNode != NULL){
        //skip expended node.
        if(curNode->totalTaken >= curNode->blockSize){
            curNode = curNode->nextLookup;
            continue;
        }

        result = ((VL_FIXEDPOOL_ORDINAL_T) curNode->blockOrdinal) | ((VL_FIXEDPOOL_INDEX_T)(curNode->totalTaken) << VL_FIXEDPOOL_SHIFT);
        curNode->totalTaken++;

        return result;
    }

    //handle no suitable node (spawn a new node and take an element!)
    curNode = vl_PoolNodeNew(pool);
    result = (VL_FIXEDPOOL_ORDINAL_T) curNode->blockOrdinal;
    curNode->totalTaken++;

    return result;
}

void vlFixedPoolReturn(vl_fixedpool* pool, vl_fixedpool_idx idx){
    const VL_FIXEDPOOL_ORDINAL_T nodeIndex = idx & VL_FIXEDPOOL_MASK;

    vl_fixed_pool_node* node = pool->lookupTable[nodeIndex];
    node->totalTaken--;

    const vl_dsidx_t distance = pool->freeTop - pool->freeStack;
    if(pool->freeTop - pool->freeStack >= pool->freeCapacity){
        pool->freeCapacity *= 2;
        pool->freeStack = realloc(pool->freeStack, pool->freeCapacity * sizeof(vl_fixedpool_idx));
        pool->freeTop = pool->freeStack + distance;
    }

    *pool->freeTop = idx;
    pool->freeTop++;
}


//void* vlFixedPoolSample(vl_fixedpool* pool, vl_fixedpool_idx idx){
//    const VL_FIXEDPOOL_ORDINAL_T nodeIndex   = idx & VL_FIXEDPOOL_MASK;
//    const VL_FIXEDPOOL_ORDINAL_T elemIndex   = idx >> VL_FIXEDPOOL_SHIFT;
//    const vl_fixed_pool_node*     node       = pool->lookupTable[nodeIndex];
//    return ((vl_uint8_t*)node->elements) + (elemIndex * pool->elementSize); }

void  vlFixedPoolClear(vl_fixedpool* pool){
    vl_fixed_pool_node* curNode = (vl_fixed_pool_node*) pool->lookupHead;

    while(curNode != NULL){
        curNode->totalTaken = 0;
        curNode = curNode->nextLookup;
    }

    pool->freeTop = pool->freeStack;
}

void vlFixedPoolFree(vl_fixedpool* pool){
    {
        vl_fixed_pool_node* curNode, *temp;

        curNode = (vl_fixed_pool_node*) pool->lookupHead;

        while(curNode->nextLookup != NULL){
            temp = curNode->nextLookup;
            free(curNode);
            curNode = temp;
        }
    }

    free(pool->freeStack);
    free(pool->lookupTable);

    pool->elementSize       = 0;
    pool->growthIncrement   = 0;
    pool->lookupTotal       = 0;
    pool->lookupCapacity    = 0;

    pool->lookupTable       = NULL;
    pool->lookupHead        = NULL;
    pool->freeStack         = NULL;
    pool->freeTop           = NULL;
}

void vlFixedPoolReset(vl_fixedpool* pool){
    vl_fixed_pool_node* curNode = pool->lookupHead, *temp;

    while(curNode->nextLookup != NULL){
        temp = curNode->nextLookup;

        pool->lookupTable[curNode->blockOrdinal]    = NULL;
        pool->lookupTotal--;

        free(curNode);
        curNode = temp;
    }

    curNode->totalTaken = 0;

    pool->growthIncrement = curNode->blockSize;
    pool->lookupHead = curNode;

    pool->freeTop = pool->freeStack;
}

void vlFixedPoolReserve(vl_fixedpool* pool, vl_dsidx_t numElements){
    const vl_dsidx_t numFree = (pool->freeTop - pool->freeStack);
    vl_dsidx_t freeSpaces = pool->growthIncrement - numFree;

    while(freeSpaces < numElements){
        vl_PoolNodeNew(pool);//Keep allocating nodes until the # of free spaces is less than the # of elements.
        freeSpaces = pool->growthIncrement - numFree;
    }
}

vl_fixedpool* vlFixedPoolClone(const vl_fixedpool* src, vl_fixedpool* dest){
    if(dest == NULL)
        dest = vlFixedPoolNew(src->elementSize);
    else{
        //prepare existing pool.
        vlFixedPoolReset(dest);
    }

    dest->growthIncrement = src->growthIncrement;
    dest->elementSize = src->elementSize;
    dest->lookupTotal = src->lookupTotal;
    dest->lookupCapacity = src->lookupCapacity;

    if(dest->lookupCapacity < src->lookupCapacity){
        dest->lookupTable = realloc(dest->lookupTable, src->lookupCapacity * sizeof(void*));
        dest->lookupCapacity = src->lookupCapacity;
    }

    {
        const size_t freeStackSize = src->lookupCapacity * sizeof(vl_fixedpool_idx);
        const vl_dsidx_t freeDistance = src->freeTop - src->freeStack;
        if(dest->freeCapacity < src->freeCapacity){
            dest->freeStack = realloc(dest->freeStack, freeStackSize);
            dest->freeCapacity = src->freeCapacity;
        }

        memcpy(dest->freeStack, src->freeStack, freeStackSize);
        dest->freeTop = dest->freeStack + freeDistance;
    }

    vl_fixed_pool_node* srcNode, *destNode;

    for(int i = 0; i <= src->lookupHead->blockOrdinal; i++){
        srcNode = src->lookupTable[i];

        const size_t nodeSize = sizeof(vl_fixed_pool_node) + (dest->elementSize * srcNode->blockSize);

        destNode = (vl_fixed_pool_node*)malloc(nodeSize);
        memcpy(destNode, srcNode, nodeSize);

        destNode->elements = destNode + 1;
        destNode->nextLookup = (vl_fixed_pool_node*)dest->lookupHead;

        dest->lookupTable[destNode->blockOrdinal] = destNode;
        dest->lookupHead = destNode;
    }

    return dest;
}

vl_fixedpool* vlFixedPoolNew(vl_ularge_t elementSize){
    vl_fixedpool* result = (vl_fixedpool*)malloc(sizeof(vl_fixedpool));

    vlFixedPoolInit(result, elementSize);

    return result;
}

void vlFixedPoolDelete(vl_fixedpool* pool){
    vlFixedPoolFree(pool);
    free(pool);
}