#include "vl_msgpack.h"
#include <string.h>

static const char   ROOT_STRING[]       = {'@', 'R', 'O', 'O', 'T'};

/**
 * \private
 */
typedef struct vl_msgpack_element_{
    vl_msgpack_iter     parent;
    vl_msgpack_type     type;
    vl_msgpack_iter     nextSibling;
    vl_msgpack_iter     prevSibling;

    vl_int8_t           extType;

    /**
     * \private
     */
    union{
        /**
         * \private
         */
        struct{
            //Payload references other nodes in the hierarchy.
            //Maps are initialized as empty (firstChild = VL_MSGPACK_ITER_INVALID; totalChildren = 0)
            vl_msgpack_iter     firstChild;
            vl_msgpack_iter     lastChild;
            vl_dsidx_t          totalChildren;
        } mapBranch;

        /**
         * \private
         */
        struct{
            //Payload is stored in value arena as an actual array.
            //Arrays are initialized as full of NIL, with a defined length.
            //All child keys are implicitly vl_uint_t.
            vl_arena_ptr        array;
            vl_dsidx_t          arrayLength;
        } arrayBranch;

        /**
         * \private
         */
        struct{
            vl_arena_ptr        value;
        } leaf;
    } payload;
} vl_msgpack_element;

vl_hash vl_HashMsgPackKey(const void* data, vl_memsize_t size){
    const vl_hash* mem = data;
    const vl_hash parentHash = *mem;
    //combine parent key hash and FNV1A-hashed key
    return vlHashCombine(parentHash, vlHashString(mem + 1, size - sizeof(vl_hash)));
}

//Frees all child nodes of the specified parent.
//IMPORTANT: This leaves Array nodes in a broken state (uninitialized members).
//This is a recursive function that descends the tree relative to the given iterator.
void vl_MsgPackFreeChildren(vl_msgpack* pack, vl_msgpack_iter iter){
    vl_msgpack_element* const element = (vl_msgpack_element*)vlHashTableSampleValue(&pack->nodes, iter, NULL);

    switch(element->type){
        case VL_MSGPACK_ARRAY:
            {
                vl_arena_ptr arrayValue = element->payload.arrayBranch.array;
                vl_msgpack_iter* array = (vl_msgpack_iter*)vlArenaMemSample(&pack->values, arrayValue);
                for(vl_ularge_t curIdx = 0; curIdx < element->payload.arrayBranch.arrayLength; curIdx++){
                    const vl_msgpack_iter curIter = array[curIdx];

                    //Free all children of child... (RECURSIVE CALL HERE)
                    vl_MsgPackFreeChildren(pack, curIter);

                    vlHashTableRemoveIter(&pack->nodes, curIter);

                    //Invalidate the array contents as we go.
                    array[curIdx] = VL_HASHTABLE_ITER_INVALID;
                }
            }
            break;
        case VL_MSGPACK_MAP:
            {
                vl_msgpack_iter curIter = element->payload.mapBranch.firstChild, nextIter;
                const vl_msgpack_element* childElem;
                while(curIter != VL_MSGPACK_ITER_INVALID){
                    childElem = (vl_msgpack_element*)vlHashTableSampleValue(&pack->nodes, curIter, NULL);

                    //Free all children of child... (RECURSIVE CALL HERE)
                    vl_MsgPackFreeChildren(pack, curIter);
                    nextIter = childElem->nextSibling;

                    vlHashTableRemoveIter(&pack->nodes, curIter);//Remove child node from the tree
                    curIter = nextIter;//Then move on to the next child.
                }
            }
            break;
        case VL_MSGPACK_NIL:
            break;
        default:
            //Reached a terminal. Simply free its data and remove it from the node table.
            vlArenaMemFree(&pack->values, element->payload.leaf.value);
            break;
    }
}

vl_msgpack_iter vlMsgPackInsert(vl_msgpack* pack, vl_msgpack_type type, vl_msgpack_iter parent, const void* keyPtr, vl_memsize_t keyLen, const void* dataPtr, vl_memsize_t dataLen);

vl_msgpack_element* vl_MsgPackInitNode(vl_msgpack* pack, vl_msgpack_iter nodeIter, vl_msgpack_type type, vl_int8_t extType, vl_msgpack_iter parent, const void* dataPtr, vl_memsize_t dataLen){
    vl_msgpack_element* element = (vl_msgpack_element*)vlHashTableSampleValue(&pack->nodes, nodeIter, NULL);
    element->type = type;
    element->parent = parent;
    element->extType = extType;

    switch(type){
        case VL_MSGPACK_MAP:
            element->payload.mapBranch.firstChild       = VL_MSGPACK_ITER_INVALID;
            element->payload.mapBranch.lastChild        = VL_MSGPACK_ITER_INVALID;
            element->payload.mapBranch.totalChildren    = 0;
            break;
        case VL_MSGPACK_ARRAY:{
            //Assume data is length of array.
            const vl_dsidx_t length = *((const vl_dsidx_t*)dataPtr);
            vl_dsidx_t curIdx; //Array element index for iteration.
            vl_msgpack_iter     prevChildIter = VL_MSGPACK_ITER_INVALID;
            vl_msgpack_element* prevChildElem = NULL;

            element->payload.arrayBranch.array = vlArenaMemAlloc(&pack->values, sizeof(vl_msgpack_iter) * length);
            element->payload.arrayBranch.arrayLength = length;

            //First pass to initialize the array as containing all NIL.
            for(curIdx = 0; curIdx < length; curIdx++)
                //RECURSIVE CALL HERE.
                vlMsgPackInsert(pack, VL_MSGPACK_NIL, nodeIter, &curIdx, sizeof(vl_dsidx_t), NULL, 0);

            //Second pass to establish links in the hierarchy.
            for(curIdx = 0; curIdx < length; curIdx++){
                vl_msgpack_iter childIter = vlMsgPackFindChildIndexed(pack, nodeIter, curIdx);
                vl_msgpack_element* childElement = (vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, childIter, NULL);

                if(prevChildElem)
                    prevChildElem->nextSibling = childIter;

                childElement->nextSibling = VL_MSGPACK_ITER_INVALID;
                childElement->prevSibling = prevChildIter;
                prevChildIter = childIter;
                prevChildElem = childElement;
            }
            //Re-obtain a pointer to the array node; might have moved due to NIL initialization.
            element = (vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, nodeIter, NULL);
            break;
        }
        case VL_MSGPACK_NIL:
            element->payload.leaf.value = VL_ARENA_NULL;
            break;
        default:{
            const vl_arena_ptr valuePtr = vlArenaMemAlloc(&pack->values, dataLen);

            if(dataPtr)
                memcpy(vlArenaMemSample(&pack->values, valuePtr), dataPtr, dataLen);

            element->payload.leaf.value = valuePtr;
            break;
        }
    }

    return element;
}

void vlMsgPackInit(vl_msgpack* pack){
    if(pack == NULL)
        return;
    vlHashTableInit(&pack->nodes, vl_HashMsgPackKey);
    vlArenaInit(&pack->values, VL_KB(1));
    pack->root = vlMsgPackInsert(pack, VL_MSGPACK_MAP, VL_HASHTABLE_ITER_INVALID, ROOT_STRING, sizeof(ROOT_STRING), NULL, 0);
}

void vlMsgPackFree(vl_msgpack* pack){
    pack->root = VL_HASHTABLE_ITER_INVALID;
    vlArenaFree(&pack->values);
    vlHashTableFree(&pack->nodes);
}

vl_msgpack* vlMsgPackNew(){
    vl_msgpack* pack = malloc(sizeof(vl_msgpack));
    vlMsgPackInit(pack);
    return pack;
}

void vlMsgPackDelete(vl_msgpack* pack){
    vlMsgPackFree(pack);
    free(pack);
}

void vlMsgPackClear(vl_msgpack* pack){
    vlHashTableClear(&pack->nodes);
    vlArenaClear(&pack->values);
    pack->root = vlMsgPackInsert(pack, VL_MSGPACK_MAP, VL_HASHTABLE_ITER_INVALID, ROOT_STRING, sizeof(ROOT_STRING), NULL, 0);
}

vl_msgpack* vlMsgPackClone(vl_msgpack* src, vl_msgpack* dest){
    if(dest == NULL)
        dest = vlMsgPackNew();

    vlHashTableClone(&src->nodes, &dest->nodes);
    vlArenaClone(&src->values, &dest->values);
    dest->root = src->root;

    return dest;
}

vl_msgpack_iter vlMsgPackParent(vl_msgpack* pack, vl_msgpack_iter iter){
    return ((const vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, iter, NULL))->parent;
}

vl_dsidx_t vlMsgPackTotalChildren(vl_msgpack* pack, vl_msgpack_iter iter){
    const vl_msgpack_element* elem = ((const vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, iter, NULL));
    switch(elem->type){
        case VL_MSGPACK_MAP:
            return elem->payload.mapBranch.totalChildren;
        case VL_MSGPACK_ARRAY:
            return elem->payload.arrayBranch.arrayLength;
        default:
            return 0;
    }
}

vl_msgpack_iter vlMsgPackFirstChild(vl_msgpack* pack, vl_msgpack_iter iter){
    const vl_msgpack_element* elem = ((const vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, iter, NULL));
    switch(elem->type){
        case VL_MSGPACK_MAP:
            return elem->payload.mapBranch.firstChild;
        case VL_MSGPACK_ARRAY:
            return vlMsgPackFindChildIndexed(pack, iter, 0);
        default:
            return VL_MSGPACK_ITER_INVALID;
    }
}

vl_msgpack_iter vlMsgPackNextSibling(vl_msgpack* pack, vl_msgpack_iter iter){
    return ((const vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, iter, NULL))->nextSibling;
}

vl_msgpack_iter vlMsgPackPrevSibling(vl_msgpack* pack, vl_msgpack_iter iter){
    return ((const vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, iter, NULL))->prevSibling;
}

vl_msgpack_type vlMsgPackType(vl_msgpack* pack, vl_msgpack_iter iter){
    return ((const vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, iter, NULL))->type;
}

vl_int8_t       vlMsgPackExtType(vl_msgpack* pack, vl_msgpack_iter iter){
    return ((const vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, iter, NULL))->extType;
}

vl_msgpack_iter vlMsgPackInsertExt(vl_msgpack* pack, vl_msgpack_type type, vl_int8_t extType, vl_msgpack_iter parent, const void* keyPtr, vl_memsize_t keyLen, const void* dataPtr, vl_memsize_t dataLen){
    //Immediately discard attempts to insert an EXT type with no subtype.
    if(type == VL_MSGPACK_EXT && extType == VL_MSGPACK_EXT_NONE)
        return VL_MSGPACK_ITER_INVALID;

    const vl_memsize_t              tempKeyLen      = sizeof(vl_hash) + keyLen;
    const vl_arena_ptr              tempKeyMem      = vlArenaMemAlloc(&pack->values, tempKeyLen);
    vl_hash*                        tempKey         = (vl_hash*)vlArenaMemSample(&pack->values, tempKeyMem);

    //Construct a hierarchical key using the node value arena.
    {
        //Handle case of root node insertion for generation of hierarchical key.
        if(parent != VL_MSGPACK_ITER_INVALID){
            *tempKey = ((vl_hashtable_header*)vlArenaMemSample(&pack->nodes.data, parent))->keyHash;
        }else{
            //No parent hash! Use an FNV1A prime mixed with an offset.
            *tempKey = vlHashCombine(0x00000100000001b3, 0xcbf29ce484222325);
        }
        //Then copy the value of the key to the temporary buffer.
        memcpy(tempKey + 1, keyPtr, keyLen);
    }

    vl_msgpack_iter nodeIter = vlHashTableFind(&pack->nodes, tempKey, tempKeyLen);

    if(nodeIter != VL_MSGPACK_ITER_INVALID){
        //Node already exists in this slot. Explicitly overwrite it.

        //Go ahead and free the temporary key.
        vlArenaMemFree(&pack->values, tempKeyMem);

        //Then see if it has an existing value for its leaf arena field, or any existing children.
        vl_msgpack_element* const element = (vl_msgpack_element*)vlHashTableSampleValue(&pack->nodes, nodeIter, NULL);

        vl_arena_ptr valuePtr = VL_ARENA_NULL;
        switch(element->type){
            case VL_MSGPACK_ARRAY:
                valuePtr = element->payload.arrayBranch.array;
            case VL_MSGPACK_MAP://fallthrough
                //Free children of non-terminals.
                vl_MsgPackFreeChildren(pack, nodeIter);
            case VL_MSGPACK_NIL://NIL inherently has NO value associated with it.
                break;
            default:
                valuePtr = element->payload.leaf.value;
                break;
        }

        //Free existing value
        if(valuePtr != VL_ARENA_NULL)
            vlArenaMemFree(&pack->values,  valuePtr);

        //Initialize its new value.
        vl_MsgPackInitNode(pack, nodeIter, type, extType, parent, dataPtr, dataLen);

        return nodeIter;
    }

    //Create an entirely new node.
    nodeIter = vlHashTableInsert(&pack->nodes, tempKey, tempKeyLen, sizeof(vl_msgpack_element));
    vlArenaMemFree(&pack->values, tempKeyMem);//Free the temp key after inserting into the hierarchy table

    vl_msgpack_element* element = vl_MsgPackInitNode(pack, nodeIter, type, extType, parent, dataPtr, dataLen);

    //Node has been added to the hierarchy. Update the parent.
    if(parent != VL_HASHTABLE_ITER_INVALID){
        vl_msgpack_element* const parentElement = (vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, parent, NULL);

        switch(parentElement->type){
            case VL_MSGPACK_ARRAY: {
                const vl_dsidx_t ordinal = *(const vl_dsidx_t*)(keyPtr);
                vl_msgpack_iter* const array = (vl_msgpack_iter*)vlArenaMemSample(&pack->values, parentElement->payload.arrayBranch.array);
                array[ordinal] = nodeIter;
            }
                break;
            case VL_MSGPACK_MAP: {
                //Append to sibling list.
                vl_msgpack_iter* oldTailIter = &parentElement->payload.mapBranch.lastChild;
                vl_msgpack_iter* oldHeadIter = &parentElement->payload.mapBranch.firstChild;
                element->prevSibling = *oldTailIter;
                element->nextSibling = VL_MSGPACK_ITER_INVALID;

                if(*oldTailIter != VL_MSGPACK_ITER_INVALID) {
                    //Map already has children. Modify the child list links.
                    vl_msgpack_element* const siblingElement = (vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, *oldTailIter, NULL);
                    siblingElement->nextSibling = nodeIter;
                }

                parentElement->payload.mapBranch.totalChildren++;
                *oldTailIter = nodeIter;

                if(*oldHeadIter == VL_MSGPACK_ITER_INVALID)
                    *oldHeadIter = nodeIter;
            }
                break;
            default:
                break;
        }
    }

    return nodeIter;
}

void vlMsgPackRemove(vl_msgpack* pack, vl_msgpack_iter iter){
    //Free all its children.
    vl_MsgPackFreeChildren(pack, iter);

    vl_msgpack_element* element = (vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, iter, NULL);
    const vl_msgpack_iter parentIter = element->parent;

    //NEVER remove the root node, which is the only node without a parent.
    if(parentIter == VL_MSGPACK_ITER_INVALID)
        return;

    vl_msgpack_element* parentElement = (vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, parentIter, NULL);

    //Erase any value that might be associated with the node.
    {
        vl_arena_ptr valuePtr = VL_ARENA_NULL;
        if(element->type == VL_MSGPACK_ARRAY)
            valuePtr = element->payload.arrayBranch.array;
        else if(element->type != VL_MSGPACK_MAP)
            valuePtr = element->payload.leaf.value;

        if(valuePtr != VL_ARENA_NULL)
            vlArenaMemFree(&pack->values, valuePtr);
    }

    //Then update the parent, depending on its type.
    switch(vlMsgPackType(pack, parentIter)){
        case VL_MSGPACK_MAP:{
            vl_dsidx_t* totalChildren = &parentElement->payload.mapBranch.totalChildren;

            (*totalChildren)--;

            if(*totalChildren <= 0){
                //Last child taken.
                parentElement->payload.mapBranch.firstChild = VL_MSGPACK_ITER_INVALID;
                parentElement->payload.mapBranch.lastChild =  VL_MSGPACK_ITER_INVALID;
                return;
            }

            const vl_msgpack_iter left = element->prevSibling;
            const vl_msgpack_iter right = element->nextSibling;

            if(parentElement->payload.mapBranch.firstChild == iter)
                parentElement->payload.mapBranch.firstChild = right;
            if(parentElement->payload.mapBranch.lastChild == iter)
                parentElement->payload.mapBranch.lastChild = left;

            if(left != VL_MSGPACK_ITER_INVALID){
                vl_msgpack_element* leftElem = (vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, left, NULL);
                leftElem->nextSibling = right;
            }

            if(right != VL_MSGPACK_ITER_INVALID){
                vl_msgpack_element* rightElem = (vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, left, NULL);
                rightElem->prevSibling = left;
            }

            return;
        }
        case VL_MSGPACK_ARRAY:
            //Set type to NIL. No need to adjust sibling relationships.
            element->type = VL_MSGPACK_NIL;
            return;
        default:
            break;
    }
}

vl_msgpack_iter vlMsgPackFindChild(vl_msgpack* pack, vl_msgpack_iter parent, const void* keyPtr, vl_memsize_t keyLen){
    const vl_memsize_t              tempKeyLen      = sizeof(vl_hash) + keyLen;
    const vl_arena_ptr              tempKeyMem      = vlArenaMemAlloc(&pack->values, tempKeyLen);
    vl_hashtable_header const*      parentHeader    = (vl_hashtable_header*)vlArenaMemSample(&pack->nodes.data, parent);

    //Construct a hierarchical key using the node value arena.
    vl_hash* tempKey = (vl_hash*)vlArenaMemSample(&pack->values, tempKeyMem);
    *tempKey = parentHeader->keyHash;
    memcpy(tempKey + 1, keyPtr, keyLen);

    const vl_msgpack_iter nodeIter = vlHashTableFind(&pack->nodes, tempKey, tempKeyLen);
    vlArenaMemFree(&pack->values, tempKeyMem);

    return nodeIter;
}

vl_msgpack_iter vlMsgPackFindChildIndexed(vl_msgpack* pack, vl_msgpack_iter parent, vl_dsidx_t index){
    //Indexed keys are a simple tuple of the parent hash and the resulting ordinal.
    //We can construct a key for it without touching any dynamic allocation by constructing a key in
    //a buffer of a well-known size.
    vl_uint8_t key[sizeof(vl_hash) + sizeof(vl_dsidx_t)];
    memcpy(key, &((vl_hashtable_header*)vlArenaMemSample(&pack->nodes.data, parent))->keyHash, sizeof(vl_hash));
    memcpy(key + sizeof(vl_hash), &index, sizeof(vl_dsidx_t));
    return vlHashTableFind(&pack->nodes, key, sizeof(vl_hash) + sizeof(vl_dsidx_t));
}

const vl_transient* vlMsgPackSampleKey(vl_msgpack* pack, vl_msgpack_iter iter, vl_memsize_t* size){
    const vl_transient* ptr = (vl_transient*)((vl_hash*)(vlHashTableSampleKey(&pack->nodes, iter, size)) + 1);
    if(size)
        (*size) -= sizeof(vl_hash);
    return ptr;
}

vl_transient*       vlMsgPackSampleValue(vl_msgpack* pack, vl_msgpack_iter iter, vl_memsize_t* size){
    const  vl_msgpack_element* elem = (const vl_msgpack_element*) vlHashTableSampleValue(&pack->nodes, iter, NULL);
    switch(elem->type){
        case VL_MSGPACK_MAP:
        case VL_MSGPACK_ARRAY:
        case VL_MSGPACK_NIL:
            return NULL;
        default:
            if(elem->payload.leaf.value == VL_ARENA_NULL)
                return NULL;
            else if(size)
                *size = vlArenaMemSize(&pack->values, elem->payload.leaf.value);
            return vlArenaMemSample(&pack->values, elem->payload.leaf.value);
    }
}