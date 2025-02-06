#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_linear_pool.h>

    int pool_test_clear(){
        int result = 1;
        vl_linearpool* pool = vlLinearPoolNew(sizeof(int));

        //take some indices
        vl_linearpool_idx indicesA[] = {
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool)
        };

        //invalidate all the prior indices...
        vlLinearPoolClear(pool);
        //then take another group of them.
        vl_linearpool_idx indicesB[] = {
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool)
        };

        //contents of indicesA and indicesB should match.
        result = result && (memcmp(indicesA, indicesB, sizeof(vl_linearpool_idx) * 5) == 0);

        vlLinearPoolDelete(pool);
        return result;
    }

    int pool_test_clone(){
        int result = 1;
        vl_linearpool* pool = vlLinearPoolNew(sizeof(int));

        //take some indices
        vl_linearpool_idx indices[5] = {
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool)
        };

        for(int i = 0; i < 5; i++)
            *((int*)vlLinearPoolSample(pool, indices[i])) = i;

        //clone then delete the original pool
        vl_linearpool* cloned = vlLinearPoolClone(pool, NULL);
        vlLinearPoolDelete(pool);

        //validate that the cloned pool indices and values match the original pool

        for(int i = 0; i < 5 && result; i++)
            result = result && (*((int*)vlLinearPoolSample(cloned, indices[i])) == i);

        vlLinearPoolDelete(cloned);
        return result;
    }

    int pool_test_reserve(){
        int result = 1;
        vl_linearpool* pool = vlLinearPoolNew(sizeof(int));
        const size_t preReserve = vlMemSize(pool->buffer.data);
        vlLinearPoolReserve(pool, 256);
        const size_t postReserve = vlMemSize(pool->buffer.data);

        result = result && (preReserve < postReserve);

        vlLinearPoolDelete(pool);
        return result;
    }

    int pool_test_tell_index(){
        int result = 1;
        vl_linearpool* pool = vlLinearPoolNew(sizeof(int));

        vl_linearpool_idx indices[5] = {
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool),
                vlLinearPoolTake(pool)
        };

        //now, demonstrate that the index -> pointer -> index conversion works as expected

        for(int i = 0; i < 5 && result; i++){
            const vl_linearpool_idx idx = indices[i];
            const vl_transient* ptr = vlLinearPoolSample(pool, idx);
            result = result && (vlLinearPoolTellIndex(pool, ptr) == idx);
        }

        vlLinearPoolDelete(pool);
        return result;
    }
}

TEST(pool, clear){
    ASSERT_TRUE(pool_test_clear());
}

TEST(pool, clone){
    ASSERT_TRUE(pool_test_clone());
}

TEST(pool, reserve){
    ASSERT_TRUE(pool_test_reserve());
}

TEST(pool, tell_index){
    ASSERT_TRUE(pool_test_tell_index());
}