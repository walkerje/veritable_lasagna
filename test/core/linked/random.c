#include "random.h"
#include <vl/vl_memory.h>
#include <vl/vl_rand.h>
#include <string.h>

vl_bool_t vlTestRandomVec4f() {
    vl_rand curRand = vlRandInit();

    float vec4[4];
    vec4[0] = vec4[1] = vec4[2] = vec4[3] = 0.0f;

    vlRandFx4(&curRand, vec4);

    const float sum = vec4[0] + vec4[1] + vec4[2] + vec4[3];

    return (vl_bool_t)(sum > 0.0f && sum < 4.0f);
}

vl_bool_t vlTestRandomFill(vl_memsize_t regionSize) {
    vl_rand curRand = vlRandInit();

    vl_memory *region, *blank;
    region = vlMemAlloc(regionSize);
    blank = vlMemAlloc(regionSize);

    memset(region, 0, regionSize);
    memset(blank, 0, regionSize);
    vlRandFill(&curRand, region, regionSize);

    const vl_bool_t result = memcmp(region, blank, regionSize) != 0;

    vlMemFree(region);
    vlMemFree(blank);
    return result;
}
