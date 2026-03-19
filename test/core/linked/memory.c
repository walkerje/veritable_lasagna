#include "memory.h"
#include <vl/vl_memory.h>
#include <vl/vl_numtypes.h>
#include <vl/vl_rand.h>

vl_bool_t vlTestMemReverse() {
    vl_bool_t result = VL_TRUE;
    vl_memory *mem = vlMemAlloc(VL_KB(1));

    vl_rand rand = vlRandInit();
    vlRandFill(&rand, mem, vlMemSize(mem));

    vl_memory *memReversed = vlMemClone(mem);
    vlMemReverse(memReversed, vlMemSize(memReversed));

    for (vl_memsize_t i = 0; i < vlMemSize(mem) && result; i++)
        result = (result && (mem[i] == memReversed[vlMemSize(memReversed) - i - 1]));

    vlMemFree(memReversed);
    vlMemFree(mem);
    return result;
}

vl_bool_t vlTestMemAlign(vl_int_t alignment) {
    vl_bool_t result = VL_TRUE;
    vl_memory *mem = vlMemAllocAligned(VL_KB(1), alignment);

    result = result && ((((vl_uintptr_t) mem) % alignment) == 0);
    mem = vlMemRealloc(mem, VL_MB(1));
    result = result && ((((vl_uintptr_t) mem) % alignment) == 0);

    vlMemFree(mem);

    return result;
}

vl_bool_t vlTestMemSort(vl_int_t numArrayLen) {
    vl_memory *const mem = vlMemAlloc(sizeof(vl_int_t) * numArrayLen);
    vl_int_t *const numbers = (vl_int_t *) mem;
    vl_bool_t result = VL_TRUE;

    vl_rand rand = vlRandInit();
    vlRandFill(&rand, mem, vlMemSize(mem));

    vlMemSort(mem, sizeof(vl_int_t), numArrayLen, vlCompareInt);

    // Ensure sorted in ascending order.
    for (vl_uint_t i = 1; i < numArrayLen && result; i++)
        result = result && (numbers[i] >= numbers[i - 1]);

    vlMemFree(mem);

    return result;
}
