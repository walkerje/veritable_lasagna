#include "vl_memory.h"

#include <malloc.h>
#include <memory.h>

#ifdef VL_MEMORY_ORIGIN_INLINE
#undef VL_MEMORY_ORIGIN_INLINE
#endif

#ifdef VL_MEMORY_HEADER_INLINE
#undef VL_MEMORY_HEADER_INLINE
#endif

/**
 * \brief Returns the original result of the malloc system call.
 *
 * \par headerPtr pointer to the memory header of an allocation
 * \return pointer to buffer origin
 * \private
 */
#define VL_MEMORY_ORIGIN_INLINE(headerPtr) (void*)(((vl_uint8_t*)(headerPtr) - ((headerPtr)->headOffset)))

/**
 * \brief Returns a pointer to the specified block of memory
 * Assumes header is stored immediately adjacent to the memory.
 */
#define VL_MEMORY_HEADER_INLINE(memPtr) ((vl_memory_header*)(memPtr) - 1)

/**
 * \brief Header for memory allocations.
 * \private
 */
typedef struct vl_memory_header{
    vl_memsize_t          length;   //size of the user-end allocation, in bytes.
    vl_uint_t       headOffset;     //total number of bytes from the beginning of the allocation
    vl_uint_t       alignment;      //byte alignment of user-end pointer. For unaligned allocations, this defaults to VL_DEFAULT_MEMORY_ALIGN.
} vl_memory_header;

vl_memory* vlMemAlloc(vl_memsize_t allocSize){
    vl_memory_header* header = malloc(allocSize + sizeof(vl_memory_header));

    if(header == NULL)
        return NULL;

    header->length      = allocSize;
    header->alignment   = VL_DEFAULT_MEMORY_ALIGN;
    header->headOffset  = 0;

    return (vl_memory*)(header) + sizeof(vl_memory_header);
}

vl_memory* vlMemAllocAligned(vl_memsize_t size, vl_uint_t align){
    if(align <= VL_DEFAULT_MEMORY_ALIGN)
        return vlMemAlloc(size); // lowest that can be specified is the largest possible word size
                                 // this is generally acceptable because alignment has recursive properties.

    vl_memory*          origin      = malloc(size + align + sizeof(vl_memory_header));

    if(origin == NULL)
        return NULL;

    const vl_uintptr_t      userOffset  = (vl_uintptr_t)origin + align + sizeof(vl_memory_header);
    vl_memory*              user        = (vl_memory*)(userOffset - (userOffset % align));
    vl_memory_header*       header      = VL_MEMORY_HEADER_INLINE(user);

    header->length      = size;
    header->alignment   = align;
    header->headOffset  = (vl_uint_t)((vl_uintptr_t)header - (vl_uintptr_t)origin);

    return user;
}

vl_memory* vlMemRealloc(vl_memory* mem, vl_memsize_t allocSize){
    const vl_uint16_t align = mem ? VL_MEMORY_HEADER_INLINE(mem)->alignment : VL_DEFAULT_MEMORY_ALIGN;

    if(align <= VL_DEFAULT_MEMORY_ALIGN){
        void* origin = mem ? mem - sizeof(vl_memory_header) : NULL;
        vl_memory_header* header = realloc(origin, allocSize + sizeof(vl_memory_header));
        header->length = allocSize;
        return (vl_memory*)(header + 1);
    }

    const vl_memory_header  oldHeader   = *VL_MEMORY_HEADER_INLINE(mem);
    const vl_ularge_t       oldSize     = oldHeader.length + sizeof(vl_memory_header);
    const vl_ularge_t       newSize     = allocSize + sizeof(vl_memory_header);
    vl_memory* const        origin      = realloc(VL_MEMORY_ORIGIN_INLINE(VL_MEMORY_HEADER_INLINE(mem)), newSize + oldHeader.alignment);

    if(origin == NULL)
        return NULL;

    const vl_uintptr_t      userOffset  = (vl_uintptr_t)origin + align + sizeof(vl_memory_header);
    vl_memory* const        user        = (vl_memory*)(userOffset - (userOffset % align));
    const vl_uint_t         headOffset  = (vl_uint_t)((vl_uintptr_t)(VL_MEMORY_HEADER_INLINE(user)) - (vl_uintptr_t)origin);

    if(headOffset != oldHeader.headOffset)
        memmove(origin + headOffset, origin + oldHeader.headOffset, oldSize > newSize ? newSize : oldSize);

    vl_memory_header* const header      = VL_MEMORY_HEADER_INLINE(user);
    header->length                      = allocSize;
    header->alignment                   = oldHeader.alignment;
    header->headOffset                  = headOffset;

    return user;
}

vl_memory* vlMemClone(vl_memory* mem){
    if(mem == NULL)
        return NULL;

    const vl_memsize_t size = vlMemSize(mem);
    vl_memory* alloc = vlMemAllocAligned(size, VL_MEMORY_HEADER_INLINE(mem)->alignment);

    if(alloc != NULL)
        memcpy(alloc, mem, size);

    return alloc;
}

#ifdef vl_MemSortSwap
#undef vl_MemSortSwap
#endif

#define vl_MemSortSwap(temp, a, b, memSize)     memcpy(temp, a, memSize); memcpy(a, b, memSize); memcpy(b, temp, memSize)

vl_int_t vl_MemSortPartition(vl_usmall_t* buffer, void* pivot, void* swap, vl_int_t low, vl_int_t high, vl_memsize_t elementSize, vl_compare_function comparator){
    memcpy(pivot, buffer + (high * elementSize), elementSize);
    vl_int_t i = (low - 1);

    for(vl_int_t j = low; j <= high - 1; j++){
        if(comparator(buffer + (j * elementSize), pivot) <= 0){
            i++;
            vl_MemSortSwap(swap, buffer + (i * elementSize), buffer + (j * elementSize), elementSize);
        }
    }

    vl_MemSortSwap(swap, buffer + ((i + 1) * elementSize), buffer + (high * elementSize), elementSize);
    return (i + 1);
}

#undef vl_MemSortSwap

void vl_MemSortQuicksort(vl_int_t* stack, void* buffer, void* pivot, void* swap, vl_memsize_t elementSize, vl_dsidx_t numElements, vl_compare_function comparator){
    vl_int_t top = -1, p, low, high;

    stack[++top] = 0;
    stack[++top] = numElements - 1;

    while(top >= 0){
        high    = stack[top--];
        low     = stack[top--];

        p       = vl_MemSortPartition(buffer, pivot, swap, low, high, elementSize, comparator);

        if(p - 1 > low){
            stack[++top] = low;
            stack[++top] = p - 1;
        }

        if(p + 1 < high){
            stack[++top] = p + 1;
            stack[++top] = high;
        }
    }
}

void vlMemSort(void* buffer, vl_memsize_t elementSize, vl_dsidx_t numElements, vl_compare_function comparator){
    const vl_memsize_t sortIndexSize = sizeof(vl_int_t) * numElements;

    //The heap allocation here isn't great.
    //It's done here with the notion that some data sets are simply... large.
    //Making the allocation re-usable, or conditionally using a recursive implementation, might be better in the future.
    //If MSVC would just allow variable-length arrays, portably allocating on the stack wouldn't be an issue!
    vl_usmall_t* const      tempMem         = malloc(sortIndexSize + (elementSize * 2));
    vl_usmall_t* const      pivot           = tempMem;
    vl_usmall_t* const      swapTemp        = pivot + elementSize;
    vl_usmall_t* const      memStack        = swapTemp + elementSize;

    vl_MemSortQuicksort((vl_int_t *) memStack, buffer, pivot, swapTemp, elementSize, numElements, comparator);

    free(tempMem);
}

void vlMemCopyStride(const void* src, vl_dsoffs_t srcStride, void* dest, vl_dsoffs_t dstStride, vl_memsize_t elementSize, vl_dsidx_t numElements){
    if(dstStride == 0 && srcStride == 0){
        memcpy(dest, src, elementSize * numElements);
        return;
    }

    const vl_usmall_t* srcPtr = src;
    vl_usmall_t* dstPtr = dest;

    for(vl_uint_t i = 0; i < numElements; i++){
        memcpy(dstPtr, srcPtr, elementSize);
        srcPtr += srcStride;
        dstPtr += dstStride;
    }
}

void vlMemReverse(void* mem, vl_memsize_t numBytes){
    vl_usmall_t* first = mem;
    vl_usmall_t* last = first + (numBytes - 1);
    vl_usmall_t temp;
    numBytes -= (numBytes % 2);
    numBytes /= 2;

    //Swap bytes from both ends, meeting in the middle of the memory block.
    for(vl_dsidx_t curByte = 0; curByte < numBytes; curByte++){
        temp = *last;
        *last = *first;
        *first = temp;
        first++;
        last--;
    }
}

vl_uint_t vlMemAlignment(vl_memory* mem){
    return VL_MEMORY_HEADER_INLINE(mem)->alignment;
}

vl_memsize_t vlMemSize(vl_memory* mem){
    return VL_MEMORY_HEADER_INLINE(mem)->length;
}

void vlMemFree(vl_memory* mem){
    free(VL_MEMORY_ORIGIN_INLINE(VL_MEMORY_HEADER_INLINE(mem)));
}