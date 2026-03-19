#include "pool.h"
#include <vl/vl_pool.h>
#include <string.h>
#include <stdint.h>

vl_bool_t vlTestPoolClear() {
    vl_bool_t result = VL_TRUE;
    vl_pool *pool = vlPoolNew(sizeof(int));

    //take some indices
    vl_pool_idx indicesA[] = {
            vlPoolTake(pool),
            vlPoolTake(pool),
            vlPoolTake(pool),
            vlPoolTake(pool),
            vlPoolTake(pool)
    };

    //invalidate all the prior indices...
    vlPoolClear(pool);
    //then take another group of them.
    vl_pool_idx indicesB[] = {
            vlPoolTake(pool),
            vlPoolTake(pool),
            vlPoolTake(pool),
            vlPoolTake(pool),
            vlPoolTake(pool)
    };

    //contents of indicesA and indicesB should match.
    result = result && (memcmp(indicesA, indicesB, sizeof(vl_pool_idx) * 5) == 0);

    vlPoolDelete(pool);
    return result;
}

vl_bool_t vlTestPoolClone() {
#define VL_TEST_SIZE (65536)
    vl_bool_t result = VL_TRUE;
    vl_pool *pool = vlPoolNew(sizeof(int));

    //take some indices
    vl_pool_idx *indices = (vl_pool_idx *)vlMemAlloc(sizeof(vl_pool_idx) * VL_TEST_SIZE);

    for (int i = 0; i < VL_TEST_SIZE; i++) {
        indices[i] = vlPoolTake(pool);
        *((int *) vlPoolSample(pool, indices[i])) = i;
    }

    //clone then delete the original pool
    vl_pool *cloned = vlPoolClone(pool, NULL);
    vlPoolDelete(pool);

    //validate that the cloned pool indices and values match the original pool

    for (int i = 0; i < VL_TEST_SIZE && result; i++)
        result = result && (*((int *) vlPoolSample(cloned, indices[i])) == i);

    vlPoolDelete(cloned);
    vlMemFree((vl_memory *)indices);
    return result;
#undef VL_TEST_SIZE
}

vl_bool_t vlTestPoolElemReturn() {
#define VL_TEST_SIZE (37)
    vl_bool_t result = VL_TRUE;

    vl_pool poolInst;
    vlPoolInit(&poolInst, sizeof(int));
    vl_pool *pool = &poolInst;

    vl_pool_idx indices[VL_TEST_SIZE];

    for (int i = 0; i < VL_TEST_SIZE; i++)
        indices[i] = vlPoolTake(pool);

    for (int i = 0; i < VL_TEST_SIZE; i++)
        vlPoolReturn(pool, indices[i]);

    //returned elements should be taken in LIFO order due to the "free stack".
    for (int i = 0; i < VL_TEST_SIZE && result; i++)
        result = (vlPoolTake(pool) == indices[(VL_TEST_SIZE - 1) - i]);

    vlPoolFree(pool);
    return result;
#undef VL_TEST_SIZE
}

vl_bool_t vlTestPoolReserve() {
    vl_bool_t result = VL_TRUE;
    vl_pool *pool = vlPoolNew(sizeof(int));

    vlPoolReserve(pool, 128);

    result = pool->lookupTotal > 1;

    vlPoolDelete(pool);
    return result;
}

vl_bool_t vlTestPoolAlign() {
    // Test various alignment values
    vl_pool pool;
    vlPoolInitAligned(&pool, sizeof(double), 16);

    // Verify element alignment
    vl_pool_idx idx = vlPoolTake(&pool);
    void *ptr = vlPoolSample(&pool, idx);
    const vl_bool_t result = (((uintptr_t) ptr % 16) == 0);

    vlPoolFree(&pool);
    return result;
}
