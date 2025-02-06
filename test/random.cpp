#include <gtest/gtest.h>

extern "C" {
#include <vl/vl_memory.h>
#include <vl/vl_rand.h>

int random_vec4(){
    vl_rand curRand = vlRandInit();

    float vec4[4];
    vec4[0] = vec4[1] = vec4[2] = vec4[3] = 0.0f;

    vlRandFx4(&curRand, vec4);

    const float sum = vec4[0] + vec4[1] + vec4[2] + vec4[3];

    return sum > 0.0f && sum < 4.0f;
}

int random_fill(int regionSize){
    vl_rand curRand = vlRandInit();

    vl_memory* region, *blank;
    region = vlMemAlloc(regionSize);
    blank = vlMemAlloc(regionSize);

    memset(region, 0, regionSize);
    memset(blank, 0, regionSize);
    vlRandFill(&curRand, region, regionSize);

    const int result = memcmp(region, blank, regionSize) != 0;

    vlMemFree(region);
    vlMemFree(blank);
    return result;
}

}

TEST(random, rand_vec4f){
    EXPECT_TRUE(random_vec4());
}

TEST(random, rand_fill_128B){
    EXPECT_TRUE(random_fill(128));
}

TEST(random, rand_fill_5KB){
    EXPECT_TRUE(random_fill(VL_KB(5)));
}

TEST(random, rand_fill_5MB){
    EXPECT_TRUE(random_fill(VL_MB(5)));
}