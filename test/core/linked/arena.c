#include "arena.h"
#include <vl/vl_arena.h>

vl_bool_t vlTestArenaGrowth() {
    vl_arena *arena = vlArenaNew(128);

    //force a resize by requesting an allocation larger than the initial capacity...
    vlArenaMemAlloc(arena, 512);
    const vl_bool_t result = vlArenaTotalCapacity(arena) >= 512;

    vlArenaDelete(arena);
    return result;
}

vl_bool_t vlTestArenaCoalesce() {
    const vl_memsize_t initSize = 128;

    vl_arena *arena = vlArenaNew(initSize);

    vl_arena_ptr a = vlArenaMemAlloc(arena, 8);
    vl_arena_ptr b = vlArenaMemAlloc(arena, 8);

    //remember, blocks are dispensed relative to the end of the first suitable free block
    //meaning pointer B will have a *lower* offset than pointer A.

    //only free block that exists should be at the beginning of the allocation
    vl_bool_t result = arena->freeSet.totalElements == 1;

    //free block A, which sits at the end of the underlying buffer.
    vlArenaMemFree(arena, a);

    // block B should now be sandwiched between two free blocks; the initial block and
    // the block that used to be claimed by pointer A.
    result = result && (arena->freeSet.totalElements == 2);

    vlArenaMemFree(arena, b);
    //there should only be a single free block again,
    //now that A and B have both been freed.
    result = result && (arena->freeSet.totalElements == 1);

    //sample the free node and make sure the freed blocks properly coalesced.
    vl_arena_node freeNode = *((vl_arena_node *) vlSetSample(&arena->freeSet, vlSetFront(&arena->freeSet)));

    vlArenaDelete(arena);

    return result && ((freeNode.offset == 0) && (freeNode.size == initSize));
}
