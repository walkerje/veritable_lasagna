#include "vl_stack.h"
#include <stdlib.h>
#include <memory.h>

/**
 * \brief Stack header. Element data is stored immediately after this header.
 * \private
 */
typedef struct{
    vl_memsize_t      size;
    vl_memsize_t      previousOffset;
} vl_stack_header;

void vlStackInit(vl_stack* stack){
    stack->depth = 0;
    stack->headOffset = 0;
    vlBufferInit(&stack->buffer);

    vl_stack_header* head = (vl_stack_header*)stack->buffer.data;
    head->size = 0;
    head->previousOffset = 0;
}

void vlStackFree(vl_stack* stack){
    vlBufferFree(&stack->buffer);
}

vl_stack* vlStackNew(){
    vl_stack* stack = (vl_stack*)malloc(sizeof(vl_stack));
    vlStackInit(stack);
    return stack;
}

void vlStackDelete(vl_stack* stack){
    vlBufferFree(&stack->buffer);
    free(stack);
}

void vlStackReset(vl_stack* stack){
    stack->depth = 0;
    stack->headOffset = 0;
    stack->buffer.offset = 0;
}

vl_stack_offset vlStackPush(vl_stack* stack, vl_memsize_t size){
    const vl_uintptr_t head = vlBufferWrite(&stack->buffer, size + sizeof(vl_stack_header), NULL);

    vl_stack_header* header = (vl_stack_header*)((stack->buffer.data) + head);
    header->size = size;
    header->previousOffset = stack->headOffset;

    stack->headOffset = head;
    stack->depth++;
    return head + sizeof(vl_stack_header);
}

vl_stack_offset vlStackPushValue(vl_stack* stack, const void* data, vl_memsize_t size){
    const vl_stack_offset offset = vlStackPush(stack, size);
    memcpy(vlStackSample(stack, offset), data, size);
    return offset;
}

vl_transient* vlStackSample(vl_stack* stack, vl_stack_offset offset){
    return (vl_usmall_t*)(stack->buffer.data) + offset;
}

vl_memsize_t vlStackSampleSize(vl_stack* stack, vl_stack_offset offset){
    vl_stack_header* header = (vl_stack_header*)((vl_usmall_t*)(stack->buffer.data) + offset - sizeof(vl_stack_header));
    return header->size;
}

vl_transient* vlStackPeek(vl_stack* stack){
    return (vl_usmall_t*)(stack->buffer.data) + stack->headOffset + sizeof(vl_stack_header);
}

vl_memsize_t vlStackPeekSize(vl_stack* stack){
    const vl_stack_header* head = (const vl_stack_header*)((vl_usmall_t*)(stack->buffer.data) + stack->headOffset);
    return head->size;
}

void vlStackPop(vl_stack* stack){
    if(stack->depth <= 0)
        return;

    stack->buffer.offset = stack->headOffset;
    vl_stack_header* header = (vl_stack_header*)((vl_usmall_t*)(stack->buffer.data) + stack->headOffset);
    stack->headOffset = header->previousOffset;
    stack->depth--;
}