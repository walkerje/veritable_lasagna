/**
 * ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
 * ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
 * ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
 *  ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
 *   ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
 * ====---: A Data Structure and Algorithms library for C11.  :---====
 *
 * Copyright 2026 Jesse Walker, released under the MIT license.
 * Git Repository:  https://github.com/walkerje/veritable_lasagna
 * \private
 */

#ifndef VL_STACK_H
#define VL_STACK_H

#include "vl_buffer.h"

typedef vl_dsoffs_t vl_stack_offset;

/**
 * \brief A virtual stack allocator.
 *
 * The vl_stack structure represents a stack allocator data structure.
 * It is built on top of the vl_buffer structure, which manages dynamic
 * resizing.
 *
 * This structure might not be be quite as easy to use as you might anticipate,
 * due to the necessity to explicitly define the size of each level of the stack
 * in bytes. This should be fairly easy to define using sizeof() or simply
 * computing the size.
 *
 * This structure also allows for sampling properties low in the stack, if an
 * appropriate offset is given. Many stack data structure implementations
 * prohibit this, however there are notable applications for this kind of use.
 */
typedef struct
{
    vl_dsidx_t depth;
    vl_uintptr_t headOffset;
    vl_buffer buffer;
} vl_stack;

/**
 * \brief Initializes the underlying memory of an existing vl_stack pointer.
 * The stack allocator should be freed with vlStackFree.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the `stack` struct. The function initializes the internal buffer.
 * - **Lifetime**: The stack is valid until `vlStackFree` or `vlStackDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `stack` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an already initialized stack without first calling `vlStackFree` (causes memory
 * leak).
 * - **Memory Allocation Expectations**: Initializes an internal `vl_buffer` which allocates an initial data block.
 * - **Return-value Semantics**: None (void).
 *
 * \param stack stack pointer
 * \sa vlStackFree
 * \par Complexity of O(1) constant.
 */
VL_API void vlStackInit(vl_stack* stack);

/**
 * \brief Frees the specified stack instance's internal allocation.
 *
 * Frees the internal buffer used by the stack allocator.
 * This stack allocator should first be initialized via vlStackInit.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal buffer. Does NOT release the `stack` struct itself.
 * - **Lifetime**: The stack becomes invalid for use.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `stack` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double free.
 * - **Memory Allocation Expectations**: Deallocates internal buffer data.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlStackInit
 * \param stack pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlStackFree(vl_stack* stack);

/**
 * \brief Allocates on the heap, initializes, and returns a new stack allocator instance.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_stack` pointer and is responsible for calling `vlStackDelete`.
 * - **Lifetime**: The stack is valid until `vlStackDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `NULL` if heap allocation for the stack struct fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_stack` struct and its internal buffer.
 * - **Return-value Semantics**: Returns a pointer to the newly allocated and initialized stack, or `NULL`.
 *
 * \sa vlStackDelete
 * \par Complexity of O(1) constant.
 * \return stack allocator pointer located on the heap.
 */
VL_API vl_stack* vlStackNew(void);

/**
 * \brief Deletes the specified stack and its internal buffer.
 *
 * This stack allocator should have been initialized via vlStackNew.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal buffer and the `vl_stack` struct.
 * - **Lifetime**: The stack pointer and all its offsets become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Safe to call if `stack` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Deallocates internal resources and the stack struct.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlStackNew
 * \param stack pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlStackDelete(vl_stack* stack);

/**
 * \brief Resets the specified stack allocator, allowing it to be used as if it
 * had just been initialized.
 *
 * This does NOT modify any of the information in the underlying buffer, but
 * rather resets some variables used for book-keeping.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: All previously returned offsets and sampled pointers become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `stack` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized stack.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param stack pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlStackReset(vl_stack* stack);

/**
 * \brief Reserves a new block of memory at the top of the stack, returning its
 * offset.
 *
 * This may force the underlying buffer to grow in size, which may invalidate
 * the result of all prior calls to vlStackPeek and vlStackSample.
 *
 * ## Contract
 * - **Ownership**: The stack maintains ownership of the memory. The caller receives an offset.
 * - **Lifetime**: The offset remains stable across buffer reallocations, but standard pointers derived from it are
 * invalidated.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `stack` must not be `NULL`.
 * - **Error Conditions**: Internal buffer expansion failure.
 * - **Undefined Behavior**: Passing an uninitialized stack.
 * - **Memory Allocation Expectations**: May trigger expansion of the underlying `vl_buffer`.
 * - **Return-value Semantics**: Returns the `vl_stack_offset` of the newly pushed block.
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
VL_API vl_stack_offset vlStackPush(vl_stack* stack, vl_memsize_t size);

/**
 * \brief Reserves and assigns a new block of memory at the top of the stack,
 * returning its offset.
 *
 * This may force the underlying buffer to grow in size, which may invalidate
 * the result of all prior calls to vlStackPeek and vlStackSample.
 *
 * ## Contract
 * - **Ownership**: Unchanged. The stack maintains a copy of the provided data.
 * - **Lifetime**: Same as `vlStackPush`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `stack` must not be `NULL`. `data` should not be `NULL`.
 * - **Error Conditions**: Internal buffer expansion failure.
 * - **Undefined Behavior**: Passing an uninitialized stack.
 * - **Memory Allocation Expectations**: May trigger expansion of the underlying `vl_buffer`.
 * - **Return-value Semantics**: Returns the `vl_stack_offset` of the newly pushed block.
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
VL_API vl_stack_offset vlStackPushValue(vl_stack* stack, const void* data, vl_memsize_t size);

/**
 * \brief Returns a pointer to the top level of the stack.
 *
 * Results in undefined behavior if the stack is empty.
 *
 * ## Contract
 * - **Ownership**: Ownership remains with the stack.
 * - **Lifetime**: Valid until the next stack push, reset, or destruction.
 * - **Thread Safety**: Safe for concurrent reads if no thread is writing.
 * - **Nullability**: `stack` must not be `NULL`.
 * - **Error Conditions**: Undefined behavior if the stack is empty.
 * - **Undefined Behavior**: Peeking an empty stack.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a transient pointer to the top data block.
 *
 * \param stack pointer
 * \par Complexity of O(1) constant.
 * \return pointer to the top level of the stack.
 */
VL_API vl_transient* vlStackPeek(vl_stack* stack);

/**
 * \brief Returns the size of the top level of the stack, in bytes.
 *
 * Results in undefined behavior if the stack is empty.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `stack` must not be `NULL`.
 * - **Error Conditions**: Undefined behavior if the stack is empty.
 * - **Undefined Behavior**: Peeking size of an empty stack.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the size of the top element in bytes.
 *
 * \param stack pointer
 * \par Complexity of O(1) constant.
 * \return size of the top level of the stack, in bytes.
 */
VL_API vl_memsize_t vlStackPeekSize(vl_stack* stack);

/**
 * \brief Samples the stack at the specified offset.
 *
 * Results in undefined behavior if the offset is invalid.
 *
 * ## Contract
 * - **Ownership**: Ownership remains with the stack.
 * - **Lifetime**: Valid until the next operation that may reallocate the underlying buffer.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `stack` must not be `NULL`.
 * - **Error Conditions**: Undefined behavior if the offset is invalid.
 * - **Undefined Behavior**: Passing an invalid offset.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a transient pointer to the memory at the specified offset.
 *
 * \param stack pointer
 * \param offset level of the stack to sample
 * \par Complexity of O(1) constant.
 * \return pointer to memory managed by the stack allocator.
 */
VL_API vl_transient* vlStackSample(vl_stack* stack, vl_stack_offset offset);

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
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `stack` must not be `NULL`.
 * - **Error Conditions**: Undefined behavior if the offset is invalid.
 * - **Undefined Behavior**: Passing an invalid offset.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the size of the stack level in bytes.
 *
 * \param stack pointer
 * \param offset level of the stack
 * \par Complexity of O(1) constant.
 * \return size of the specified level of the stack, in bytes.
 */
VL_API vl_memsize_t vlStackSampleSize(vl_stack* stack, vl_stack_offset offset);

/**
 * \brief Pops the top level of the stack, allowing it to be overwritten in the
 * future.
 *
 * ## Contract
 * - **Ownership**: Releases the top element's storage back to the stack.
 * - **Lifetime**: Sampled pointers to the popped element become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `stack` must not be `NULL`.
 * - **Error Conditions**: None (no-op if the stack is empty).
 * - **Undefined Behavior**: Passing an uninitialized stack.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param stack
 * \par Complexity of O(1) constant.
 */
VL_API void vlStackPop(vl_stack* stack);

#endif // VL_STACK_H
