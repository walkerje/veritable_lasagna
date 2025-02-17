#ifndef VL_ATOMIC_H
#define VL_ATOMIC_H

#ifdef __cplusplus
#error The vl_atomic header does not work under C++ compilation due to using the _Atomic keyword.
#endif

#include <stdatomic.h>
#include "vl_numtypes.h"

/**
 * This header provides a wrapper over the C11 atomic functions, providing an abstraction using the VL number types.
 * Primarily used for "all in one place" documentation, the header is intended to "smooth out" usage of atomic types
 * to conform to the way the rest of the library is documented and used.
 */

#ifdef VL_ATOMIC
#undef VL_ATOMIC
#endif

#ifdef VL_ATOMIC_TYPEDEF
#undef VL_ATOMIC_TYPEDEF
#endif

#define VL_ATOMIC(x) _Atomic x
#define VL_ATOMIC_TYPEDEF(x) typedef VL_ATOMIC(x)

VL_ATOMIC_TYPEDEF(vl_bool_t)        vl_atomic_bool_t;

VL_ATOMIC_TYPEDEF(vl_int_t)         vl_atomic_int_t;
VL_ATOMIC_TYPEDEF(vl_uint_t)        vl_atomic_uint_t;

VL_ATOMIC_TYPEDEF(vl_intptr_t)      vl_atomic_intptr_t;
VL_ATOMIC_TYPEDEF(vl_uintptr_t)     vl_atomic_uintptr_t;

VL_ATOMIC_TYPEDEF(vl_ularge_t)      vl_atomic_ularge_t;
VL_ATOMIC_TYPEDEF(vl_usmall_t)      vl_atomic_usmall_t;
VL_ATOMIC_TYPEDEF(vl_ilarge_t)      vl_atomic_ilarge_t;
VL_ATOMIC_TYPEDEF(vl_ismall_t)      vl_atomic_ismall_t;

#ifdef VL_U8_T
#define VL_ATOMIC_U8_T           vl_atomic_uint8_t

VL_ATOMIC_TYPEDEF(vl_uint8_t)       VL_ATOMIC_U8_T;
#endif

#ifdef VL_U16_T
#define VL_ATOMIC_U16_T          vl_atomic_uint16_t

VL_ATOMIC_TYPEDEF(vl_uint16_t)      VL_ATOMIC_U16_T;
#endif

#ifdef VL_U32_T
#define VL_ATOMIC_U32_T          vl_atomic_uint32_t

VL_ATOMIC_TYPEDEF(vl_uint32_t)      VL_ATOMIC_U32_T;
#endif

#ifdef VL_U64_T
#define VL_ATOMIC_U64_T          vl_atomic_uint64_t

VL_ATOMIC_TYPEDEF(vl_uint64_t)      VL_ATOMIC_U64_T;
#endif

#ifdef VL_I8_T
#define VL_ATOMIC_I8_T            vl_atomic_int8_t

VL_ATOMIC_TYPEDEF(vl_int8_t)        VL_ATOMIC_I8_T;
#endif

#ifdef VL_I16_T
#define VL_ATOMIC_I16_T           vl_atomic_int16_t

VL_ATOMIC_TYPEDEF(vl_int16_t)       VL_ATOMIC_I16_T;
#endif

#ifdef VL_I32_T
#define VL_ATOMIC_I32_T           vl_atomic_int32_t

VL_ATOMIC_TYPEDEF(vl_int32_t)       VL_ATOMIC_I32_T;
#endif

#ifdef VL_I64_T
#define VL_ATOMIC_I64_T           vl_atomic_int64_t

VL_ATOMIC_TYPEDEF(vl_int64_t)       VL_ATOMIC_I64_T;
#endif

VL_ATOMIC_TYPEDEF(vl_bool_t)                vl_atomic_flag_t;


/**
 * \brief Memory order enumeration. Dictates reordering of memory accesses surrounding and including atomic operations.
 *
 * Documentation of different orders taken from the following webpage:
 * https://en.cppreference.com/w/c/atomic/memory_order
 */
typedef enum VL_MEMORY_ORDER{
    /**
     * Relaxed operation: there are no synchronization or ordering constraints imposed
     * on other reads or writes, only this operation's atomicity is guaranteed.
     *
     * Useful for independent, atomic counters.
     */
    VL_MEMORY_ORDER_RELAXED = memory_order_relaxed,

    /**
     * A load operation with this memory order performs the acquire operation on the affected
     * memory location: no reads or writes in the current thread can be reordered before this load.
     * All writes in other threads that release the same atomic variable are visible in the current thread.
     *
     * Useful for the consumer side of producer-consumer synchronization.
     */
    VL_MEMORY_ORDER_ACQUIRE = memory_order_acquire,

    /**
     * A store operation with this memory order performs the release operation: no reads or writes
     * in the current thread can be reordered after this store. All writes in the current thread
     * are visible in other threads that acquire the same atomic variable and writes that carry a
     * dependency into the atomic variable become visible in other threads that consume the same atomic.
     *
     * Useful for the producer side of producer-consumer synchronization.
     */
    VL_MEMORY_ORDER_RELEASE = memory_order_release,

    /**
     * A read-modify-write operation with this memory order is both an acquire operation and a release
     * operation. No memory reads or writes in the current thread can be reordered before the load, nor
     * after the store. All writes in other threads that release the same atomic variable are visible before
     * the modification and the modification is visible in other threads that acquire the same atomic variable.
     *
     * Useful for read-modify-write operations, specifically in lock-free data structures.
     */
    VL_MEMORY_ORDER_ACQ_REL = memory_order_acq_rel,

    /**
     * A load operation with this memory order performs an acquire operation, a store performs a release operation,
     * and read-modify-write performs both an acquire operation and a release operation, plus a single total
     * order exists in which all threads observe all modifications in the same order.
     *
     * Useful for the maximum amount of safety, probably at the cost of performance.
     */
    VL_MEMORY_ORDER_SEQ_CST = memory_order_seq_cst
} vl_memory_order_t;

//Load & Store Ops

/**
 * \brief Performs a generic atomic read operation.
 *
 * This operation will use the VL_MEMORY_ORDER_SEQ_CST memory order.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic object to access
 * \return Non-atomic value at the specified pointer.
 */
#define vlAtomicLoad(ptr)                   atomic_load(ptr)

/**
 * \brief Performs a generic atomic read operation.
 *
 * The memory order used must be among the following:
 * \sa VL_MEMORY_ORDER_RELAXED
 * \sa VL_MEMORY_ORDER_CONSUME
 * \sa VL_MEMORY_ORDER_ACQUIRE
 * \sa VL_MEMORY_ORDER_SEQ_CST
 *
 * \note Using a memory order not listed here will result in undefined behavior.
 * \param ptr Pointer to the atomic object to access
 * \param order Which memory ordering scheme to use for the operation.
 * \return Non-atomic value at the specified pointer.
 */
#define vlAtomicLoadExplicit(ptr, order)    atomic_load_explicit(ptr, (memory_order)(order))

/**
 * \brief Performs a generic atomic write operation.
 *
 * This operation will use the VL_MEMORY_ORDER_SEQ_CST memory order.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic object to write.
 * \param val Value to store.
 */
#define vlAtomicStore(ptr, val)                      atomic_store(ptr, val)

/**
 * \brief Performs a generic atomic write operation.
 *
 * The memory order used must be among the following:
 * \sa VL_MEMORY_ORDER_RELAXED
 * \sa VL_MEMORY_ORDER_RELEASE
 * \sa VL_MEMORY_ORDER_ACQUIRE
 * \sa VL_MEMORY_ORDER_SEQ_CST
 *
 * \note Using a memory order not listed here will result in undefined behavior.
 * \param ptr Pointer to the atomic object to access
 * \param order Which memory ordering scheme to use for the operation.
 */
#define vlAtomicStoreExplicit(ptr, order)       atomic_store_explicit(ptr, (memory_order)(order))

//Arithmetic

/**
 * \brief Performs a generic atomic addition operation.
 *
 * The memory order used for this operation is VL_MEMORY_ORDER_SEQ_CST.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic object to perform the operation on.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */
#define vlAtomicFetchAdd(ptr, arg)  atomic_fetch_add(ptr, arg)

/**
 * \brief Performs a generic atomic subtraction operation.
 *
 * The memory order used for this operation is VL_MEMORY_ORDER_SEQ_CST.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic object to perform the operation on.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */
#define vlAtomicFetchSub(ptr, arg)  atomic_fetch_sub(ptr, arg)

/**
 * \brief Performs a generic atomic bitwise OR operation.
 *
 * The memory order used for this operation is VL_MEMORY_ORDER_SEQ_CST.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic object to perform the operation on.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */

#define vlAtomicFetchOr(ptr, arg)   atomic_fetch_or(ptr, arg)

/**
 * \brief Performs a generic atomic bitwise XOR operation.
 *
 * The memory order used for this operation is VL_MEMORY_ORDER_SEQ_CST.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic object to perform the operation on.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */
#define vlAtomicFetchXor(ptr, arg)  atomic_fetch_xor(ptr, arg)

/**
 * \brief Performs a generic atomic bitwise AND operation.
 *
 * The memory order used for this operation is VL_MEMORY_ORDER_SEQ_CST.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic object to perform the operation on.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */
#define vlAtomicFetchAnd(ptr, arg)  atomic_fetch_and(ptr, arg)

/**
 * \brief Performs a generic atomic addition operation.
 *
 * \sa VL_MEMORY_ORDER
 * \param ptr Pointer to the atomic object to access
 * \param order Which memory ordering scheme to use for the operation.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */
#define vlAtomicFetchAddExplicit(ptr, arg, order)    atomic_fetch_add_explicit(ptr, arg, (memory_order)(order))

/**
 * \brief Performs a generic atomic subtraction operation.
 *
 * \sa VL_MEMORY_ORDER
 * \param ptr Pointer to the atomic object to access
 * \param order Which memory ordering scheme to use for the operation.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */
#define vlAtomicFetchSubExplicit(ptr, arg, order)    atomic_fetch_sub_explicit(ptr, arg, (memory_order)(order))

/**
 * \brief Performs a generic atomic bitwise OR operation.
 *
 * \sa VL_MEMORY_ORDER
 * \param ptr Pointer to the atomic object to access
 * \param order Which memory ordering scheme to use for the operation.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */
#define vlAtomicFetchOrExplicit(ptr, arg, order)     atomic_fetch_or_explicit(ptr, arg, (memory_order)(order))

/**
 * \brief Performs a generic atomic bitwise XOR operation.
 *
 * \sa VL_MEMORY_ORDER
 * \param ptr Pointer to the atomic object to access
 * \param order Which memory ordering scheme to use for the operation.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */
#define vlAtomicFetchXorExplicit(ptr, arg, order)    atomic_fetch_xor_explicit(ptr, arg, (memory_order)(order))

/**
 * \brief Performs a generic atomic bitwise AND operation.
 *
 * \sa VL_MEMORY_ORDER
 * \param ptr Pointer to the atomic object to access
 * \param order Which memory ordering scheme to use for the operation.
 * \return The previous value held by the atomic object referred to by the specified pointer.
 */
#define vlAtomicFetchAndExplicit(ptr, arg, order)    atomic_fetch_and_explicit(ptr, arg, (memory_order)(order))

//Compare & Swap

/**
 * \brief Atomically compares the memory at ptr with. If *ptr == *expectedPtr, *ptr = desired.
 *
 * If the comparison is true, this performs a read-modify-write operation.
 * Otherwise, this only performs a read operation.
 *
 * The memory order used for these operations is VL_MEMORY_ORDER_SEQ_CST.
 *
 * The weak variant of an atomic compare-exchange is allowed to fail spuriously.
 * That is, act as if *ptr != *expectedPtr, even if they are equal.
 * This variant likely offers better performance in loops.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic object to perform the operation on.
 * \param expectedPtr Pointer to do a bitwise comparison with.
 * \param desired Value to copy to the memory pointed to by ptr.
 * \return a boolean indicating the result of the comparison *ptr == *expectedPtr
 */
#define vlAtomicCompareExchangeWeak(ptr, expectedPtr, desired)            atomic_compare_exchange_weak(ptr, expectedPtr, desired)

/**
 * \brief Atomically compares the memory at ptr with. If *ptr == *expectedPtr, *ptr = desired.
 *
 * If the comparison is true, this performs a read-modify-write operation.
 * Otherwise, this only performs a read operation.
 *
 * The memory order used for these operations is VL_MEMORY_ORDER_SEQ_CST.
 *
 * Guarantees that if the comparison fails, it’s because the value has actually changed.
 * Will never fail spuriously.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic object to perform the operation on.
 * \param expectedPtr Pointer to do a bitwise comparison with.
 * \param desired Value to copy to the memory pointed to by ptr.
 * \return a boolean indicating the result of the comparison *ptr == *expectedPtr
 */
#define vlAtomicCompareExchangeStrong(ptr, expectedPtr, desired)          atomic_compare_exchange_strong(ptr, expectedPtr, desired)

/**
 * \brief Atomically compares the memory at ptr with. If *ptr == *expectedPtr, *ptr = desired.
 *
 * If the comparison is true, this performs a read-modify-write operation.
 * Otherwise, this only performs a read operation.
 *
 * The weak variant of an atomic compare-exchange is allowed to fail spuriously.
 * That is, act as if *ptr != *expectedPtr, even if they are equal.
 * This variant likely offers better performance in loops.
 *
 * \sa VL_MEMORY_ORDER
 * \param ptr Pointer to the atomic object to perform the operation on.
 * \param expectedPtr Pointer to do a bitwise comparison with.
 * \param desired Value to copy to the memory pointed to by ptr.
 * \param trueOrder Memory order used in the case of a true comparison.
 * \param falseOrder Memory order used in the case of a false comparison.
 * \return a boolean indicating the result of the comparison *ptr == *expectedPtr
 */
#define vlAtomicCompareExchangeWeakExplicit(ptr, expectedPtr, desired, trueOrder, falseOrder)    atomic_compare_exchange_weak_explicit(ptr, expectedPtr, desired, (memory_order)(trueOrder), (memory_order)(falseOrder))

/**
 * \brief Atomically compares the memory at ptr with. If *ptr == *expectedPtr, *ptr = desired.
 *
 * If the comparison is true, this performs a read-modify-write operation.
 * Otherwise, this only performs a read operation.
 *
 * Guarantees that if the comparison fails, it’s because the value has actually changed.
 * Will never fail spuriously.
 *
 * \sa VL_MEMORY_ORDER
 * \param ptr Pointer to the atomic object to perform the operation on.
 * \param expectedPtr Pointer to do a bitwise comparison with.
 * \param desired Value to copy to the memory pointed to by ptr.
 * \param trueOrder Memory order used in the case of a true comparison.
 * \param falseOrder Memory order used in the case of a false comparison.
 * \return a boolean indicating the result of the comparison *ptr == *expectedPtr
 */
#define vlAtomicCompareExchangeStrongExplicit(ptr, expectedPtr, desired, trueOrder, falseOrder)  atomic_compare_exchange_strong_explicit(ptr, expectedPtr, desired, (memory_order)(trueOrder), (memory_order)(falseOrder))

//Thread Fencing

/**
 * \brief Prepares memory synchronization of non-atomic and relaxed atomic accesses.
 *
 * \sa VL_MEMORY_ORDER
 * \param order Memory order used for the current context.
 */
#define vlAtomicThreadFence(order)          atomic_thread_fence((memor_order)(order))

/**
 * \brief Atomically sets a flag to true and returns the old value.
 *
 * The memory order used for this operation is VL_MEMORY_ORDER_SEQ_CST.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic flag instance
 * \return the previous value of the flag pointer to by ptr
 */
#define vlAtomicFlagTestAndSet(ptr)         atomic_flag_test_and_set(ptr)

/**
 * \brief Atomically sets the flag to false.
 *
 * The memory order used for this operation is VL_MEMORY_ORDER_SEQ_CST.
 *
 * \sa VL_MEMORY_ORDER_SEQ_CST
 * \param ptr Pointer to the atomic flag instance
 */
#define vlAtomicFlagClear(ptr)              atomic_flag_clear(ptr)

/**
 * \brief Atomically sets a flag to true and returns the old value.
 *
 * \sa VL_MEMORY_ORDER
 * \param ptr Pointer to the atomic flag instance
 * \param order Memory order used for the operation.
 * \return the previous value of the flag pointer to by ptr
 */
#define vlAtomicFlagTestAndSetExplicit(ptr, order)      atomic_flag_test_and_set_explicit(ptr, (memory_order)(order))

/**
 * \brief Atomically sets a flag to false.
 *
 * \sa VL_MEMORY_ORDER
 * \param ptr Pointer to the atomic flag instance
 * \param order Memory order used for the operation.
 * \return the previous value of the flag pointer to by ptr
 */
#define vlAtomicFlagClearExplicit(ptr, order)           atomic_flag_clear_explicit(ptr, (memory_order)(order))

//Init

/**
 * \brief Initializes an existing atomic object.
 *
 * This is the only way to properly initialize dynamically-allocated atomic objects.
 * Atomic objects allocated on the stack and directly assigned do not need to be initialized.
 *
 * \ptr Pointer to the atomic object to initialize.
 */
#define vlAtomicInit(ptr)                   atomic_init(ptr)

#undef VL_ATOMIC_TYPEDEF
#endif //VL_ATOMIC_H
