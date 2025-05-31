#include "vl_arena.h"
#include <stdlib.h>
#include <string.h>

/**
 * \brief Comparator function for heap nodes.
 * \param a node A
 * \param b node B
 * \return comparison
 * \private
 */
int vl_ArenaNodeCompare(const void *a, const void *b) {
    const vl_arena_ptr nodeA = *(const vl_arena_ptr *) a;
    const vl_arena_ptr nodeB = *(const vl_arena_ptr *) b;
    return (nodeA > nodeB) - (nodeB > nodeA);
}

void vlArenaInit(vl_arena *arena, vl_memsize_t initialSize) {
    vl_arena_node initNode;
    initNode.offset = 0;
    initNode.size = initialSize;

    vlSetInit(&arena->freeSet, sizeof(vl_arena_node), vl_ArenaNodeCompare);
    vlSetInsert(&arena->freeSet, &initNode);

    arena->data = vlMemAlloc(initialSize);
}

void vlArenaFree(vl_arena *arena) {
    vlSetFree(&arena->freeSet);
    vlMemFree(arena->data);
}

vl_arena *vlArenaNew(vl_memsize_t initialSize) {
    vl_arena *arena = malloc(sizeof(vl_arena));
    vlArenaInit(arena, initialSize);
    return arena;
}

void vlArenaDelete(vl_arena *arena) {
    vlArenaFree(arena);
    free(arena);
}

void vlArenaClear(vl_arena *arena) {
    vlSetClear(&arena->freeSet);

    vl_arena_node initNode;
    initNode.offset = 0;
    initNode.size = vlMemSize(arena->data);

    vlSetInsert(&arena->freeSet, &initNode);
}

vl_arena *vlArenaClone(const vl_arena *src, vl_arena *dest) {
    const vl_memsize_t cloneMemSize = vlMemSize(src->data);

    if (dest == NULL)
        dest = vlArenaNew(cloneMemSize);
    else
        dest->data = vlMemRealloc(dest->data, cloneMemSize);

    vlSetClone(&src->freeSet, &dest->freeSet);
    memcpy(dest->data, src->data, cloneMemSize);

    return dest;
}

/**
 * This function merges adjacent blocks of memory on the left and right hand side of the free set iterator.
 * While this does search iteratively to the right and left, it should only ever find a maximum of one block
 * on each side that it can merge.
 *
 * This holds true because we will attempt to merge adjacent blocks every time a new free node is added to the set.
 *
 * Therefore, the complexity of this operation should be O(1) in all cases.
 * https://en.wikipedia.org/wiki/Coalescing_(computer_science)
 * \param arena
 * \param rightIter
 *
 * \private
 */
void vl_HeapCoalesce(vl_arena *arena, vl_set_iter rightIter) {

    //scan right along adjacent blocks of memory
    vl_arena_node *rightNode = (vl_arena_node *) vlSetSample(&arena->freeSet, rightIter);
    vl_set_iter tempIter = vlSetNext(&arena->freeSet, rightIter);
    while (tempIter != VL_SET_ITER_INVALID) {
        vl_arena_node *node = (vl_arena_node *) vlSetSample(&arena->freeSet, tempIter);
        if (rightNode->offset + rightNode->size != node->offset)
            break;

        rightNode = node;
        rightIter = tempIter;
        tempIter = vlSetNext(&arena->freeSet, tempIter);
    }

    //then iteratively collapse each node to the left, removing adjacent blocks on the right
    vl_set_iter leftIter = vlSetPrev(&arena->freeSet, rightIter);
    while (leftIter != VL_SET_ITER_INVALID) {
        vl_arena_node *leftNode = (vl_arena_node *) vlSetSample(&arena->freeSet, leftIter);
        if (leftNode->offset + leftNode->size != rightNode->offset)
            break;

        leftNode->size += rightNode->size;
        vlSetRemove(&arena->freeSet, rightIter);

        rightIter = leftIter;
        rightNode = leftNode;
        leftIter = vlSetPrev(&arena->freeSet, leftIter);
    }
}

void vlArenaReserve(vl_arena *arena, vl_memsize_t numBytes) {
    const vl_memsize_t freeMem = vlArenaTotalFree(arena);
    const vl_memsize_t initSize = vlMemSize(arena->data);
    numBytes -= numBytes <= freeMem ? 0 : freeMem;

    vl_memsize_t newSize = initSize;
    while (newSize <= numBytes + initSize)
        newSize *= 2;

    if (newSize != initSize)
        arena->data = vlMemRealloc(arena->data, newSize);

    const vl_memsize_t growth = vlMemSize(arena->data) - initSize;

    vl_arena_node newNode;
    newNode.offset = initSize;
    newNode.size = growth;

    vl_HeapCoalesce(arena, vlSetInsert(&arena->freeSet, &newNode));
}

vl_arena_ptr vlArenaMemAlloc(vl_arena *arena, vl_memsize_t size) {
    if (size == 0)
        return 0;

    vl_set_iter freeIter = vlSetFront(&arena->freeSet);
    size += sizeof(vl_memsize_t);//size of block is prepended...

    while (freeIter != VL_SET_ITER_INVALID) {
        vl_arena_node *arenaNode = (vl_arena_node *) vlSetSample(&arena->freeSet, freeIter);

        //perfect match in size. just remove the free item from the set, and return.
        if (arenaNode->size == size) {
            const vl_arena_ptr ptr = arenaNode->offset;
            vlSetRemove(&arena->freeSet, freeIter);
            *(vl_memsize_t *) (arena->data + ptr) = size;
            return ptr + sizeof(vl_memsize_t);
        }

        if (arenaNode->size > size) {
            //we can claim a piece of this.
            //slice it off the end of the block, and modify the node record size.
            //this avoids having to modify any arena node offsets,
            //and thus avoids manipulating the order of the free set.
            //a side effect of this method is that blocks are dispensed
            //in reverse order of what a programmer might otherwise expect.
            arenaNode->size -= size;
            const vl_arena_ptr ptr = arenaNode->offset + arenaNode->size;
            *(vl_memsize_t *) (arena->data + ptr) = size;
            return ptr + sizeof(vl_memsize_t);
        }

        freeIter = vlSetNext(&arena->freeSet, freeIter);
    }

    //if no block of memory was found, grow the data buffer.
    const vl_memsize_t initSize = vlMemSize(arena->data);

    vl_memsize_t newSize = initSize;
    while (newSize <= size + initSize)
        newSize *= 2;

    if (newSize != initSize)
        arena->data = vlMemRealloc(arena->data, newSize);

    const vl_memsize_t growth = vlMemSize(arena->data) - initSize;

    //the newest node has an offset of the previous size,
    //and a size of how much the data memory grew, sans the size of data we need.
    //"the missile knows where it is..." yeah, this comment isn't very clear.
    vl_arena_node newNode;
    newNode.offset = initSize;
    newNode.size = growth - size;

    vl_HeapCoalesce(arena, vlSetInsert(&arena->freeSet, &newNode));

    vl_memsize_t *sizePtr = (vl_memsize_t *) (arena->data + (newNode.offset + newNode.size));
    *sizePtr = size;

    return (vl_uintptr_t) (sizePtr + 1) - (vl_uintptr_t) arena->data;
}

vl_arena_ptr vlArenaMemRealloc(vl_arena *arena, vl_arena_ptr ptr, vl_memsize_t size) {
    ptr -= sizeof(vl_memsize_t);
    size += sizeof(vl_memsize_t);
    const vl_memsize_t origSize = *(vl_memsize_t *) (arena->data + ptr);
    if (size == origSize)
        return ptr;

    //if we're shrinking it...
    if (origSize > size) {
        //simply shrink the size and merge a new free block with the size difference.
        vl_arena_node newNode;
        newNode.offset = ptr + size;
        newNode.size = origSize - size;

        vl_HeapCoalesce(arena, vlSetInsert(&arena->freeSet, &newNode));

        *(vl_memsize_t *) (arena->data + ptr) = size;
        return ptr + sizeof(vl_memsize_t);
    }

    //if we're growing it...
    //we search the free set for an offset that indicates the block
    //that is being resized has an adjacent free block that follows it.
    const vl_arena_ptr searchOffset = ptr + origSize;

    vl_set_iter rightIter = vlSetFind(&arena->freeSet, &searchOffset);
    if (rightIter != VL_SET_ITER_INVALID) {
        vl_arena_node rightNode = *(vl_arena_node *) vlSetSample(&arena->freeSet, rightIter);
        if (rightNode.size + origSize >= size) {
            //we will take some memory from the free block on the right--
            //and, if we don't use the entire free block, modify
            //its offset and size then re-insert it.
            vlSetRemove(&arena->freeSet, rightIter);

            if (rightNode.size + origSize != size) {
                rightNode.offset = ptr + size;
                rightNode.size = (origSize + rightNode.size) - size;
                //don't need to merge here-- we're taking a node that essentially
                //already existed and modifying it. we can make the assumption
                //that it had already been merged.
                vlSetInsert(&arena->freeSet, &rightNode);
            }

            *(vl_memsize_t *) (arena->data + ptr) = size;
            return ptr + sizeof(vl_memsize_t);
        }
    }

    //worst possible case-- we must allocate separate memory,
    //copy the contents of the old block, then free the old block.
    //no choice at this point; there is no reasonably apparent alternative.
    const vl_arena_ptr result = vlArenaMemAlloc(arena, size);
    memcpy(vlArenaMemSample(arena, result), vlArenaMemSample(arena, ptr), origSize);
    vlArenaMemFree(arena, ptr + sizeof(vl_memsize_t));
    return result;
}

void vlArenaMemFree(vl_arena *arena, vl_arena_ptr ptr) {
    ptr -= sizeof(vl_memsize_t);

    vl_arena_node newNode;
    newNode.size = *(vl_memsize_t *) (arena->data + ptr);
    newNode.offset = ptr;

    vl_HeapCoalesce(arena, vlSetInsert(&arena->freeSet, &newNode));
}

vl_arena_ptr vlArenaMemPrepend(vl_arena *arena, vl_arena_ptr dstPtr, const void *src, vl_memsize_t length) {
    vl_arena_ptr srcOffset = 0;
    const vl_uintptr_t srcIntPtr = (vl_uintptr_t) src;
    const vl_uintptr_t arenaOriginIntPtr = (vl_uintptr_t) (arena->data);
    const vl_memsize_t originalSize = vlArenaMemSize(arena, dstPtr);

    if (srcIntPtr >= arenaOriginIntPtr && srcIntPtr < (arenaOriginIntPtr + vlMemSize(arena->data))) {
        //src pointer may be located in the arena.
        //this can break if the prepend operation resizes the underlying memory.
        srcOffset = (vl_arena_ptr) (srcIntPtr - arenaOriginIntPtr) + length; //add length due to offsetting the origin
    }

    dstPtr = vlArenaMemRealloc(arena, dstPtr, length + originalSize);

    if (srcOffset > 0)
        src = vlArenaMemSample(arena, srcOffset);

    vl_transient *dst = vlArenaMemSample(arena, dstPtr);
    memmove(dst + length, dst, originalSize);
    memcpy(dst, src, length);

    return dstPtr;
}

vl_arena_ptr vlArenaMemAppend(vl_arena *arena, vl_arena_ptr dstPtr, const void *src, vl_memsize_t length) {
    vl_arena_ptr srcOffset = 0;
    const vl_uintptr_t srcIntPtr = (vl_uintptr_t) src;
    const vl_uintptr_t arenaOriginIntPtr = (vl_uintptr_t) (arena->data);
    const vl_memsize_t originalSize = vlArenaMemSize(arena, dstPtr);

    if (srcIntPtr >= arenaOriginIntPtr && srcIntPtr < (arenaOriginIntPtr + vlMemSize(arena->data))) {
        //src pointer may be located in the arena.
        //this can break if the prepend operation resizes the underlying memory.
        srcOffset = (vl_arena_ptr) (srcIntPtr - arenaOriginIntPtr);
    }

    dstPtr = vlArenaMemRealloc(arena, dstPtr, length + originalSize);

    if (srcOffset > 0)
        src = vlArenaMemSample(arena, srcOffset);

    memcpy(vlArenaMemSample(arena, dstPtr) + originalSize, src, length);

    return dstPtr;
}

vl_transient *vlArenaMemSample(vl_arena *arena, vl_arena_ptr ptr) {
    return arena->data + ptr;
}

vl_memsize_t vlArenaMemSize(vl_arena *arena, vl_arena_ptr ptr) {
    return *(vl_memsize_t *) (arena->data + (ptr - sizeof(vl_memsize_t))) - sizeof(vl_memsize_t);
}

vl_memsize_t vlArenaTotalCapacity(vl_arena *arena) {
    return vlMemSize(arena->data);
}

vl_memsize_t vlArenaTotalFree(vl_arena *arena) {
    vl_memsize_t sum = 0;
    VL_SET_FOREACH(&arena->freeSet, iter) {
        const vl_arena_node *node = (const vl_arena_node *) vlSetSample(&arena->freeSet, iter);
        sum += node->size;
    }
    return sum;
}