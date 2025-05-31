#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_pool.h>

int pool_test_clear() {
    int result = 1;
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

int pool_test_clone() {
#define VL_TEST_SIZE (65536)

    int result = 1;
    vl_pool *pool = vlPoolNew(sizeof(int));

    //take some indices
    vl_pool_idx indices[VL_TEST_SIZE];

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
    return result;
#undef VL_TEST_SIZE
}

int pool_test_return() {
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

int pool_test_reserve() {
    vl_bool_t result = VL_TRUE;
    vl_pool *pool = vlPoolNew(sizeof(int));

    vlPoolReserve(pool, 128);

    result = pool->lookupTotal > 1;

    vlPoolDelete(pool);
    return result;
}

int pool_test_alignment() {
    {
        // Test various alignment values
        vl_pool pool;
        vlPoolInitAligned(&pool, sizeof(double), 16);

        // Verify element alignment
        vl_pool_idx idx = vlPoolTake(&pool);
        void *ptr = vlPoolSample(&pool, idx);
        const int result = (((uintptr_t) ptr % 16) == 0);

        vlPoolFree(&pool);
        return result;
    }
}
}

TEST(pool, clear) {
    ASSERT_TRUE(pool_test_clear());
}

TEST(pool, clone) {
    ASSERT_TRUE(pool_test_clone());
}

TEST(pool, elem_return) {
    ASSERT_TRUE(pool_test_return());
}

TEST(pool, reserve) {
    ASSERT_TRUE(pool_test_reserve());
}

TEST(pool, align) {
    ASSERT_TRUE(pool_test_alignment());
}