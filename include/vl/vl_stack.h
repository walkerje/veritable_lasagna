#ifndef VL_STACK_H
#define VL_STACK_H

#include "vl_buffer.h"

typedef vl_dsoffs_t vl_stack_offset;

/**
 * \brief A virtual stack allocator.
 *
 * The vl_stack structure represents a stack allocator data structure.
 * It is built on top of the vl_buffer structure, which manages dynamic resizing.
 *
 * This structure might not be be quite as easy to use as you might anticipate,
 * due to the necessity to explicitly define the size of each level of the stack in bytes.
 * This should be fairly easy to define using sizeof() or simply computing the size.
 *
 * This structure also allows for sampling properties low in the stack, if an appropriate offset is given.
 * Many stack data structure implementations prohibit this, however there are notable applications for this kind of use.
 */
typedef struct{
    vl_dsidx_t      depth;
    vl_uintptr_t    headOffset;
    vl_buffer       buffer;
} vl_stack;

/**
 * Initializes the underlying memory of an existing vl_stack pointer.
 * The stack allocator should be freed with vlStackFree.
 *
 * @param stack stack pointer
 * \sa vlStackFree
 * \par Complexity of O(1) constant.
 */
void            vlStackInit(vl_stack* stack);

/**
 * \brief Frees the specified stack instance's allocation.
 *
 * Frees the specified stack allocator.
 * This stack allocator should first be initialized via vlStackInit.
 * \sa vlStackInit
 * \param stack pointer
 * \par Complexity of O(1) constant.
 */
void            vlStackFree(vl_stack* stack);

/**
 * \brief Allocates on the heap, initializes, and returns a stack allocator instance.
 * \sa vlStackDelete
 * \par Complexity of O(1) constant.
 * \return stack allocator pointer located on the heap.
 */
vl_stack*       vlStackNew();

/**
 * \brief Deletes the specified stack.
 * This stack allocator should be initialized via vlStackNew.
 * \sa vlStackNew
 * \param stack pointer
 * \par Complexity of O(1) constant.
 */
void            vlStackDelete(vl_stack* stack);

/**
 * \brief Resets the specified stack allocator, allowing it to be used as if it had just been initialized.
 *
 * This does NOT modify any of the information in the underlying buffer, but rather resets some
 * variables used for book-keeping.
 * \param stack pointer
 * \par Complexity of O(1) constant.
 */
void            vlStackReset(vl_stack* stack);

/**
 * \brief Reserves a new block of memory at the top of the stack, returning its offset.
 *
 * This may force the underlying buffer to grow in size, which may invalidate the result of all prior calls
 * to vlStackPeek and vlStackSample.
 *
 * \sa vlStackPeek
 * \sa vlStackPeekSize
 * \sa vlStackSample
 * \sa vlStackSampleSize
 *
 * \param stack pointer
 * \param size total bytes in the reserved block
 * \par Complexity of O(1) constant.
 * \return offset of the pushed stack level.
 */
vl_stack_offset vlStackPush(vl_stack* stack, vl_memsize_t size);

/**
 * \brief Reserves and assigns new block of memory at the top of the stack, returning its offset.
 *
 * This may force the underlying buffer to grow in size, which may invalidate the result of all prior calls
 * to vlStackPeek and vlStackSample.
 *
 * \sa vlStackPeek
 * \sa vlStackPeekSize
 * \sa vlStackSample
 * \sa vlStackSampleSize
 *
 * \param stack pointer
 * \param data pointer to the element data
 * \param size total bytes in the reserved block
 * \par Complexity of O(1) constant.
 * \return offset of the pushed stack level.
 */
vl_stack_offset vlStackPushValue(vl_stack* stack, const void* data, vl_memsize_t size);

/**
 * \brief Returns a pointer to the top level of the stack.
 *
 * Results in undefined behavior if the stack is empty.
 *
 * \param stack pointer
 * \par Complexity of O(1) constant.
 * \return pointer to the top level of the stack.
 */
vl_transient*           vlStackPeek(vl_stack* stack);

/**
 * \brief Returns the size of the top level of the stack, in bytes.
 *
 * Results in undefined behavior if the stack is empty.
 *
 * \param stack pointer
 * \par Complexity of O(1) constant.
 * \return size of the top level of the stack, in bytes.
 */
vl_memsize_t          vlStackPeekSize(vl_stack* stack);

/**
 * \brief Samples the stack at the specified offset.
 *
 * Results in undefined behavior if the offset is invalid.
 *
 * \param stack pointer
 * \param offset level of the stack to sample
 * \par Complexity of O(1) constant.
 * \return pointer to memory managed by the stack allocator.
 */
vl_transient*           vlStackSample(vl_stack* stack, vl_stack_offset offset);

#ifndef vlStackSize

/**
 * \brief Returns the size of the specified stack.
 * \param stackPtr pointer
 * \par Complexity of O(1) constant.
 * \return size of the stack
 */
#define vlStackSize(stackPtr) ((stackPtr)->depth)
#endif

#ifndef vlStackEmpty
/**
 * \brief Returns a boolean indicating if the specified stack is empty.
 * \param stackPtr pointer
 * \par Complexity of O(1) constant.
 * \return 0 if stack has elements, 1 if stack is empty.
 */
#define vlStackEmpty(stackPtr) ((stackPtr)->depth == 0)
#endif

/**
 * \brief Samples the size of the stack level at the specified offset.
 *
 * Results in undefined behavior if the specified offset is invalid.
 *
 * \param stack pointer
 * \param offset level of the stack
 * \par Complexity of O(1) constant.
 * \return size of the specified level of the stack, in bytes.
 */
vl_memsize_t            vlStackSampleSize(vl_stack* stack, vl_stack_offset offset);

/**
 * \brief Pops the top level of the stack, allowing it to be overwritten in the future.
 *
 * \param stack
 * \par Complexity of O(1) constant.
 */
void                    vlStackPop(vl_stack* stack);

#endif //VL_STACK_H
