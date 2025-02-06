#include "vl_linear_pool.h"
#include <stdlib.h>

void vlLinearPoolInit(vl_linearpool* pool, vl_memsize_t elementSize){
    vlBufferInit(&pool->buffer);
    vlBufferInit(&pool->freeStack);
    pool->elementSize = elementSize;
    pool->totalTaken = 0;
}

void vlLinearPoolFree(vl_linearpool* pool){
    vlBufferFree(&pool->buffer);
    vlBufferFree(&pool->freeStack);
}

vl_linearpool* vlLinearPoolNew(vl_memsize_t elementSize){
    vl_linearpool* pool = malloc(sizeof(vl_linearpool));
    vlLinearPoolInit(pool, elementSize);
    return pool;
}

void vlLinearPoolDelete(vl_linearpool* pool){
    vlLinearPoolFree(pool);
    free(pool);
}

void vlLinearPoolClear(vl_linearpool* pool){
    vlBufferReset(&pool->buffer);
    vlBufferReset(&pool->freeStack);
    pool->totalTaken = 0;
}

vl_linearpool* vlLinearPoolClone(const vl_linearpool* src, vl_linearpool* dest){
    if(dest == NULL)
        dest = vlLinearPoolNew(src->elementSize);

    vlBufferClone(&src->buffer, &dest->buffer);
    vlBufferClone(&src->freeStack, &dest->freeStack);

    dest->elementSize = src->elementSize;
    dest->totalTaken = src->totalTaken;

    return dest;
}

void vlLinearPoolReserve(vl_linearpool* pool, vl_memsize_t n){
    const vl_memsize_t initSize = vlMemSize(pool->buffer.data);
    vl_memsize_t memSize = initSize;
    const vl_memsize_t minSize = memSize + (n * pool->elementSize);

    while(memSize < minSize)
        memSize *= 2;

    pool->buffer.data = vlMemRealloc(pool->buffer.data, memSize);
}

vl_linearpool_idx vlLinearPoolTellIndex(vl_linearpool* pool, const vl_transient* dataPtr){
    if(dataPtr < pool->buffer.data || dataPtr >= (pool->buffer.data + pool->buffer.size))
        return VL_POOL_INVALID_IDX;

    return (vl_linearpool_idx)((dataPtr - pool->buffer.data) / pool->elementSize);
}

vl_linearpool_idx vlLinearPoolTake(vl_linearpool* pool){
    vl_buffer* freeStack = &pool->freeStack;
    vl_buffer* buffer = &pool->buffer;
    vl_linearpool_idx result;

    if(freeStack->offset == 0){
        result = pool->totalTaken;
        vlBufferWrite(buffer, pool->elementSize, NULL);
    }else{
        vl_linearpool_idx* idPtr = (vl_linearpool_idx*)(freeStack->data + freeStack->offset);

        idPtr--;
        result = *idPtr;

        freeStack->offset -= sizeof(vl_linearpool_idx);
    }

    pool->totalTaken++;
    return result;
}

void vlLinearPoolReturn(vl_linearpool* pool, vl_linearpool_idx offset){
    vl_buffer* freeStack = &pool->freeStack;
    vlBufferWrite(freeStack, sizeof(vl_linearpool_idx), &offset);
    pool->totalTaken--;
}
