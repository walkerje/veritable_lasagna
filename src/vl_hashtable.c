#include "vl_hashtable.h"
#include <stdlib.h>
#include <string.h>

/**
 * \brief Compare two byte sequences.
 * \private
 */
static inline vl_usmall_t vl_HashTableBinCompare(const void* a, vl_memsize_t aSize, const void* b, vl_memsize_t bSize){
    return aSize != bSize ? 0 : memcmp(a, b, aSize) == 0;
}

//grows the mapping table and re-builds collision chains.
vl_memsize_t vl_HashTableGrow(vl_hashtable* table){
    vl_memsize_t newSize  = vlMemSize(table->table) * 2;
    table->table    = vlMemRealloc(table->table, newSize);

    memset(table->table, 0, newSize);

    newSize /= sizeof(vl_hash_iter);
    vl_hash_iter* mapping = (vl_hash_iter*)table->table;

    VL_HASHTABLE_FOREACH(table, curIter){
        vl_hashtable_header* curHeader = (vl_hashtable_header*)vlArenaMemSample(&table->data, curIter);
        const vl_hash_iter tableIndex = curHeader->keyHash % newSize;
        const vl_hash_iter mappedNext = mapping[tableIndex];

        if(mappedNext == VL_HASHTABLE_ITER_INVALID){
            curHeader->next = 0;    //reset the next pointer...
            mapping[tableIndex] = curIter;
            continue;
        }

        //insert the node at the head of the chain...
        curHeader->next = mapping[tableIndex];
        mapping[tableIndex] = curIter;
    }

    return newSize;
}

void vlHashTableInit(vl_hashtable* table, vl_hash_function hashFunc){
    vlArenaInit(&table->data, VL_HASHTABLE_DEFAULT_SIZE);
    table->hashFunc = hashFunc;
    table->totalElements = 0;
    table->table = vlMemAlloc(sizeof(vl_hash_iter) * 16);
    memset(table->table, 0, vlMemSize(table->table));
}

void vlHashTableFree(vl_hashtable* table){
    vlArenaFree(&table->data);
    vlMemFree(table->table);
}

vl_hashtable* vlHashTableNew(vl_hash_function func){
    vl_hashtable* table = malloc(sizeof(vl_hashtable));
    vlHashTableInit(table, func);
    return table;
}

void vlHashTableDelete(vl_hashtable* table){
    vlHashTableFree(table);
    free(table);
}

vl_hash_iter vlHashTableInsert(vl_hashtable* table, const void* key, vl_memsize_t keySize, vl_memsize_t dataSize){
    const vl_hash hash = table->hashFunc(key, keySize);
    vl_dsidx_t tableSize = vlMemSize(table->table) / sizeof(vl_hash_iter);

    if(table->totalElements + 1 >= (tableSize * VL_HASHTABLE_RESIZE_FACTOR))
        tableSize = vl_HashTableGrow(table);

    vl_hash_iter* mapping = (vl_hash_iter*)table->table;

    const vl_memsize_t nodeSize = sizeof(vl_hashtable_header) + keySize + dataSize;

    const vl_dsidx_t tableIndex = hash % tableSize;
    const vl_hash_iter rootIter = mapping[tableIndex];
    vl_hash_iter tailIter = rootIter;

    while(tailIter != VL_HASHTABLE_ITER_INVALID){
        vl_hashtable_header* tailHeader = (vl_hashtable_header*)vlArenaMemSample(&table->data, tailIter);

        //compare keys and hashes...
        if(hash == tailHeader->keyHash){
            if(vl_HashTableBinCompare(tailHeader + 1, tailHeader->keySize, key, keySize)){

                //If the keys match, reallocate the node. This is equivalent to value replacement...
                if(tailHeader->valSize != dataSize){
                    tailHeader->valSize = dataSize;
                    vl_arena_ptr newTailIter = vlArenaMemRealloc(&table->data, tailIter, nodeSize);
                    vl_hashtable_header* newHeader = (vl_hashtable_header*)vlArenaMemSample(&table->data, newTailIter);
                    newHeader->valSize = dataSize;

                    if(tailIter == rootIter)
                        mapping[tableIndex] = newTailIter;

                    tailIter = newTailIter;
                }else if(tailIter == rootIter)
                    mapping[tableIndex] = tailIter;

                return tailIter;
            }
        }

        //if next iter is invalid, we found the end of the chain.
        if(tailHeader->next == VL_HASHTABLE_ITER_INVALID){
            break;
        }

        // if the keys aren't equivalent, and if there's another node in the chain to examine,
        // advance to the next node.
        tailIter = tailHeader->next;
    }

    //finally, add a new node...
    const vl_hash_iter newNode = vlArenaMemAlloc(&table->data, nodeSize);
    vl_hashtable_header* header = (vl_hashtable_header*)vlArenaMemSample(&table->data, newNode);
    header->next = VL_HASHTABLE_ITER_INVALID;
    header->keySize = keySize;
    header->valSize = dataSize;
    header->keyHash = hash;
    memcpy(header + 1, key, keySize);
    table->totalElements++;

    //if we found a tail, set the new node as the next element in the chain...
    if(tailIter != VL_HASHTABLE_ITER_INVALID)
        ((vl_hashtable_header*)(vlArenaMemSample(&table->data, tailIter)))->next = newNode;
    else //otherwise, set its value in the table. it will be the head of a new chain.
        mapping[tableIndex] = newNode;

    return newNode;
}

vl_hash_iter vlHashTableFind(vl_hashtable* table, const void* key, vl_memsize_t keySize){
    const vl_hash hash = table->hashFunc(key, keySize);
    const vl_dsidx_t tableSize = (vlMemSize(table->table) / sizeof(vl_hash_iter));
    const vl_dsidx_t tableIndex = hash % tableSize;
    vl_hash_iter curIter = ((vl_hash_iter*)table->table)[tableIndex];
    vl_hashtable_header* curHeader;

    if(curIter == 0)
        return 0;

    while(curIter != VL_HASHTABLE_ITER_INVALID) {
        curHeader = (vl_hashtable_header*)vlArenaMemSample(&table->data, curIter);

        if (vl_HashTableBinCompare(curHeader + 1, curHeader->keySize, key, keySize))
            return curIter;

        curIter = curHeader->next; // Move to the next node
    }

    return VL_HASHTABLE_ITER_INVALID;
}

void vlHashTableRemoveKey(vl_hashtable* table, const void* key, vl_memsize_t keyLen){
    vlHashTableRemoveIter(table, vlHashTableFind(table, key, keyLen));
}

void vlHashTableRemoveIter(vl_hashtable* table, vl_hash_iter iter){
    const vl_dsidx_t tableSize = (vlMemSize(table->table) / sizeof(vl_hash_iter));
    vl_hashtable_header* header = (vl_hashtable_header*)vlArenaMemSample(&table->data, iter);

    const vl_dsidx_t tableIndex = header->keyHash % tableSize;

    vl_hash_iter rootIter = ((vl_hash_iter*)table->table)[tableIndex];

    if(rootIter == VL_HASHTABLE_ITER_INVALID)
        return;//Trying to remove element that does not exist is a no-op...

    //first or only element in the list. simply free it and bump the head value forward.
    //if the next element in the chain doesn't exist, this will cleanly clear the slot
    //in the table.
    if(rootIter == iter){
        ((vl_hash_iter*)table->table)[tableIndex] = header->next;
        vlArenaMemFree(&table->data, iter);
        table->totalElements--;
        return;
    }

    //if the removed element is somewhere in the middle, or the last element, in the list, find it.
    vl_hashtable_header* curHeader = (vl_hashtable_header*)vlArenaMemSample(&table->data, rootIter);

    if(curHeader->next == VL_HASHTABLE_ITER_INVALID){
        //if the current header is the only header in the chain, just clear out its spot.
        ((vl_hash_iter*)table->table)[tableIndex] = VL_HASHTABLE_ITER_INVALID;
    }else{
        //otherwise, we must find the node that precedes it and fix the chain due to node removal.
        while(curHeader->next != iter)
            curHeader = (vl_hashtable_header*)vlArenaMemSample(&table->data, curHeader->next);

        curHeader->next = header->next;
    }

    vlArenaMemFree(&table->data, iter);
    table->totalElements--;
}

void vlHashTableClear(vl_hashtable* table){
    vlArenaClear(&table->data);
    table->totalElements = 0;
    memset(table->table, 0, vlMemSize(table->table));
}

vl_hashtable* vlHashTableClone(const vl_hashtable* src, vl_hashtable* dest){
    const vl_memsize_t bucketBufferSize = vlMemSize(src->table);

    if(dest == NULL)
        dest = vlHashTableNew(src->hashFunc);

    vlArenaClone(&src->data, &dest->data);

    dest->table = vlMemRealloc(dest->table, bucketBufferSize);
    memcpy(dest->table, src->table, bucketBufferSize);

    dest->totalElements = src->totalElements;
    dest->hashFunc = dest->hashFunc;

    return dest;
}

vl_hash_iter vlHashTableCopyElement(vl_hashtable* src, vl_hash_iter iter, vl_hashtable* dest){
    if(src->hashFunc != dest->hashFunc)
        return VL_HASHTABLE_ITER_INVALID;

    vl_memsize_t keySize, dataSize;
    const vl_transient* keyPtr = vlHashTableSampleKey(src, iter, &keySize);
    const vl_transient* dataPtr = vlHashTableSampleValue(src, iter, &dataSize);

    const vl_hash_iter destIter = vlHashTableInsert(dest, keyPtr, keySize, dataSize);
    memcpy(vlHashTableSampleValue(dest, destIter, NULL), dataPtr, dataSize);

    return destIter;
}

int vlHashTableCopy(vl_hashtable* src, vl_hashtable* dest){
    if(src->hashFunc != dest->hashFunc)
        return 0;

    int totalCopied = 0;

    vl_memsize_t keySize, dataSize;
    VL_HASHTABLE_FOREACH(src, srcIter){
        const vl_transient* keyPtr = vlHashTableSampleKey(src, srcIter, &keySize);
        const vl_transient* dataPtr = vlHashTableSampleValue(src, srcIter, &dataSize);

        const vl_hash_iter destIter = vlHashTableInsert(dest, keyPtr, keySize, dataSize);
        memcpy(vlHashTableSampleValue(dest, destIter, NULL), dataPtr, dataSize);
        totalCopied++;
    }

    return totalCopied;
}

void vlHashTableReserve(vl_hashtable* table, vl_memsize_t buckets, vl_memsize_t heapSize){
    vlArenaReserve(&table->data, heapSize);

    vl_memsize_t newSize  = vlMemSize(table->table);
    while(newSize < vlMemSize(table->table) + (buckets * sizeof(vl_arena_ptr)))
        newSize *= 2;
    table->table    = vlMemRealloc(table->table, newSize);

    memset(table->table, 0, newSize);

    newSize /= sizeof(vl_hash_iter);
    vl_hash_iter* mapping = (vl_hash_iter*)table->table;

    const vl_memsize_t totalBuckets = newSize / sizeof(vl_arena_ptr);
    VL_HASHTABLE_FOREACH(table, curIter){
        vl_hashtable_header* curHeader = (vl_hashtable_header*)vlArenaMemSample(&table->data, curIter);
        vl_hash_iter tableIndex = curHeader->keyHash % totalBuckets;
        vl_hash_iter mappedNext = mapping[tableIndex];
        if(mappedNext == VL_HASHTABLE_ITER_INVALID){
            curHeader->next = 0;    //reset the next pointer...
            mapping[tableIndex] = curIter;
            continue;
        }

        //insert the node at the head of the new chain...
        curHeader->next = mapping[tableIndex];
        mapping[tableIndex] = curIter;
    }

}

const vl_transient* vlHashTableSampleKey(vl_hashtable* table, vl_hash_iter iter, vl_memsize_t* outSize){
    const vl_hashtable_header* curHeader = (const vl_hashtable_header*)vlArenaMemSample(&table->data, iter);
    if(outSize)
        *outSize = curHeader->keySize;
    return (vl_transient*)(curHeader + 1);
}

vl_transient* vlHashTableSampleValue(vl_hashtable* table, vl_hash_iter iter, vl_memsize_t* outSize){
    vl_hashtable_header* curHeader = (vl_hashtable_header*)vlArenaMemSample(&table->data, iter);
    if(outSize)
        *outSize = curHeader->valSize;
    return (vl_transient*)(curHeader + 1) + curHeader->keySize;
}

vl_hash_iter vlHashTableFront(vl_hashtable* table){
    //these finds should be very efficient, considering the size of the free
    //set is often very small because we are merging adjacent free blocks.
    vl_set_iter firstFree = vlSetFront(&table->data.freeSet);

    //if the free set is empty (thus, invalid front iterator), return offset to first the first block.
    if(firstFree == VL_SET_ITER_INVALID)
        return sizeof(vl_memsize_t);

    const vl_arena_node* freeNode = (const vl_arena_node*)vlSetSample(&table->data.freeSet, firstFree);

    //or, if the first node does not have a zero offset, also return the first offset.
    if(freeNode->offset != 0)
        return sizeof(vl_memsize_t);

    const vl_hash_iter nodeEnd = freeNode->size + freeNode->offset;
    //return the offset to the first node after the first free node.
    //if this happens to point to the end of the allocation, return 0;
    //there are no allocated blocks to refer to.
    return (vlMemSize(table->data.data) == nodeEnd) ? VL_HASHTABLE_ITER_INVALID : (nodeEnd + sizeof(vl_memsize_t));
}

vl_hash_iter vlHashTableNext(vl_hashtable* table, vl_hash_iter iter){
    vl_hash_iter nextIter = iter + vlArenaMemSize(&table->data, iter);

    //these finds should be very efficient, considering the size of the free
    //set is often very small because we are merging adjacent free blocks.
    const vl_set_iter nextFree = vlSetFind(&table->data.freeSet, &nextIter);

    if(nextFree != VL_SET_ITER_INVALID){
        const vl_arena_node* freeNode = (const vl_arena_node*)vlSetSample(&table->data.freeSet, nextFree);
        nextIter = freeNode->offset + freeNode->size;
    }

    //if we've reached the end of the table buffer, return a NULL arena pointer.
    //otherwise, return the iterator plus an offset for its block size header.
    return nextIter == vlMemSize(table->data.data) ? 0 : (nextIter + sizeof(vl_memsize_t));
}