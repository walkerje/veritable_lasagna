//
// Created by silas on 12/25/2024.
//
#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_fixed_pool.h>

int pool_test_clear(){
    int result = 1;
    vl_fixedpool* pool = vlFixedPoolNew(sizeof(int));

    //take some indices
    vl_fixedpool_idx indicesA[] = {
            vlFixedPoolTake(pool),
            vlFixedPoolTake(pool),
            vlFixedPoolTake(pool),
            vlFixedPoolTake(pool),
            vlFixedPoolTake(pool)
    };

    //invalidate all the prior indices...
    vlFixedPoolClear(pool);
    //then take another group of them.
    vl_fixedpool_idx indicesB[] = {
            vlFixedPoolTake(pool),
            vlFixedPoolTake(pool),
            vlFixedPoolTake(pool),
            vlFixedPoolTake(pool),
            vlFixedPoolTake(pool)
    };

    //contents of indicesA and indicesB should match.
    result = result && (memcmp(indicesA, indicesB, sizeof(vl_fixedpool_idx) * 5) == 0);

    vlFixedPoolDelete(pool);
    return result;
}

int pool_test_clone(){
#define VL_TEST_SIZE (65536*8)

    int result = 1;
    vl_fixedpool* pool = vlFixedPoolNew(sizeof(int));

    //take some indices
    vl_fixedpool_idx indices[VL_TEST_SIZE];

    for(int i = 0; i < VL_TEST_SIZE; i++){
        indices[i] = vlFixedPoolTake(pool);
        *((int*)vlFixedPoolSample(pool, indices[i])) = i;
    }

    //clone then delete the original pool
    vl_fixedpool* cloned = vlFixedPoolClone(pool, NULL);
    vlFixedPoolDelete(pool);

    //validate that the cloned pool indices and values match the original pool

    for(int i = 0; i < VL_TEST_SIZE && result; i++)
        result = result && (*((int*)vlFixedPoolSample(cloned, indices[i])) == i);

    vlFixedPoolDelete(cloned);
    return result;
#undef VL_TEST_SIZE
}

int pool_test_return(){
#define VL_TEST_SIZE (37)
    vl_bool_t result = VL_TRUE;

    vl_fixedpool poolInst;
    vlFixedPoolInit(&poolInst, sizeof(int));
    vl_fixedpool* pool = &poolInst;

    vl_fixedpool_idx indices[VL_TEST_SIZE];

    for(int i = 0; i < VL_TEST_SIZE; i++)
        indices[i] = vlFixedPoolTake(pool);

    for(int i = 0; i < VL_TEST_SIZE; i++)
        vlFixedPoolReturn(pool, indices[i]);

    //returned elements should be taken in LIFO order due to the "free stack".
    for(int i = 0; i < VL_TEST_SIZE && result; i++)
        result = (vlFixedPoolTake(pool) == indices[(VL_TEST_SIZE - 1) - i]);

    vlFixedPoolFree(pool);
    return result;
#undef VL_TEST_SIZE
}

int pool_test_reserve(){
    vl_bool_t result = VL_TRUE;
    vl_fixedpool* pool = vlFixedPoolNew(sizeof(int));

    vlFixedPoolReserve(pool, 128);

    result = pool->lookupTotal > 1;

    vlFixedPoolDelete(pool);
    return result;
}

}

TEST(fixed_pool, clear){
    ASSERT_TRUE(pool_test_clear());
}

TEST(fixed_pool, clone){
    ASSERT_TRUE(pool_test_clone());
}

TEST(fixed_pool, elem_return){
    ASSERT_TRUE(pool_test_return());
}

TEST(fixed_pool, reserve) {
    ASSERT_TRUE(pool_test_reserve());
}