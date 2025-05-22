#ifndef VL_ATOMIC_PTR_H
#define VL_ATOMIC_PTR_H

#include "vl_atomic.h"

/**
 * \brief A Tagged Pointer structure for use with Atomic, lock-free algorithms.
 *
 * Using extra state, a tagged pointer helps to distinguish atomic changes to memory
 * to eliminate the ABA problem. The pointer tag is a counter that is incremented with write operation.
 * The tag occupies as many bytes as the native system pointer type.
 *
 * \see https://en.wikipedia.org/wiki/ABA_problem
 *
 * This structure may depend on 128-bit Compare-and-swap on modern platforms to achieve proper atomicity.
 * (Double-width compare-and-swap, aka DWCAS, hardware instruction or similar). Depending on the platform and compiler,
 * it may require an implicit lock.
 */
typedef struct vl_tagged_ptr {
    vl_uintptr_t ptr;
    vl_ularge_t  tag;
} vl_tagged_ptr;

/**
 * \brief Atomic variant of vl_tagged_ptr.
 * \copydoc vl_tagged_ptr
 */
typedef VL_ATOMIC vl_tagged_ptr vl_atomic_ptr;

/**
 * \brief Default initialization value for tagged/atomic pointers.
 *
 * \code
 * ptr = NULL;
 * tag = 0;
 * \endcode
 */
extern const vl_tagged_ptr VL_TAGPTR_NULL;

#define vlAtomicPtrInit(atPtr, valPtr) vlAtomicInit((atPtr), valPtr);

/**
 * \brief Atomically stores a new pointer with an incremented tag.
 *
 * This function prevents the ABA problem by incrementing the tag every time the pointer is updated.
 *
 * \param atomicPtr The atomic tagged pointer to store into.
 * \param ptr       The raw pointer to store.
 *
 * \sa vl_tagged_ptr, vlAtomicPtrCompareExchangeStrong, vlAtomicPtrCompareExchangeWeak
 */
static inline void vlAtomicPtrStore(vl_atomic_ptr* atomicPtr, void* ptr){
    vl_tagged_ptr pV = vlAtomicLoad(atomicPtr);

    pV.ptr = (vl_uintptr_t)ptr;
    pV.tag++;

    vlAtomicStore(atomicPtr, pV);
}

/**
 * \brief Performs an atomic strong compare-and-swap with memory order control.
 *
 * Compares \p *atPtr with \p *expected and, if equal, replaces it with \p newValue
 * and an incremented tag. Ensures full memory ordering via explicit order parameters.
 *
 * \param atPtr      Pointer to the atomic tagged pointer.
 * \param expected   Pointer to the expected tagged pointer (updated on failure).
 * \param newValue   New raw pointer value to store.
 * \param trueOrder  Memory order on success.
 * \param falseOrder Memory order on failure.
 * \return \c VL_TRUE if the swap was successful; otherwise \c VL_FALSE.
 *
 * \sa vlAtomicPtrCompareExchangeWeakExplicit
 */
static inline vl_bool_t vlAtomicPtrCompareExchangeStrongExplicit(vl_atomic_ptr* atPtr, vl_tagged_ptr* expected, const void* newValue, vl_memory_order_t trueOrder, vl_memory_order_t falseOrder){
   vl_tagged_ptr result = *expected;
   result.ptr = (vl_uintptr_t)newValue;
   result.tag++;
   return vlAtomicCompareExchangeStrongExplicit(atPtr, expected, result, trueOrder, falseOrder);
}

/**
 * \brief Performs an atomic weak compare-and-swap with memory order control.
 *
 * Similar to \ref vlAtomicPtrCompareExchangeStrongExplicit, but may fail spuriously.
 * Intended for use in retry loops.
 *
 * \param atPtr      Pointer to the atomic tagged pointer.
 * \param expected   Pointer to the expected tagged pointer (updated on failure).
 * \param newValue   New raw pointer value to store.
 * \param trueOrder  Memory order on success.
 * \param falseOrder Memory order on failure.
 * \return \c VL_TRUE if the swap was successful; otherwise \c VL_FALSE.
 *
 * \sa vlAtomicPtrCompareExchangeStrongExplicit
 */
static inline vl_bool_t vlAtomicPtrCompareExchangeWeakExplicit(vl_atomic_ptr* atPtr, vl_tagged_ptr* expected, const void* newValue, vl_memory_order_t trueOrder, vl_memory_order_t falseOrder){
    vl_tagged_ptr result = *expected;
    result.ptr = (vl_uintptr_t)newValue;
    result.tag++;
    return vlAtomicCompareExchangeWeakExplicit(atPtr, expected, result, trueOrder, falseOrder);
}

/**
 * \brief Convenience macro for strong atomic tagged pointer CAS with sequential consistency.
 *
 * \param atPtr        Pointer to atomic tagged pointer.
 * \param atPtrExpected Expected pointer/tag (updated on failure).
 * \param valPtr       New pointer value.
 * \return \c VL_TRUE if successful, \c VL_FALSE otherwise.
 *
 * \sa vlAtomicPtrCompareExchangeStrongExplicit
 */
#define vlAtomicPtrCompareExchangeStrong(atPtr, atPtrExpected, valPtr)\
    vlAtomicPtrCompareExchangeStrongExplicit(atPtr, atPtrExpected, valPtr, VL_MEMORY_ORDER_SEQ_CST, VL_MEMORY_ORDER_SEQ_CST)

/**
 * \brief Convenience macro for weak atomic tagged pointer CAS with sequential consistency.
 *
 * \param atPtr        Pointer to atomic tagged pointer.
 * \param atPtrExpected Expected pointer/tag (updated on failure).
 * \param valPtr       New pointer value.
 * \return \c VL_TRUE if successful, \c VL_FALSE otherwise.
 *
 * \sa vlAtomicPtrCompareExchangeWeakExplicit
 */
#define vlAtomicPtrCompareExchangeWeak(atPtr, atPtrExpected, valPtr)\
    vlAtomicPtrCompareExchangeWeakExplicit(atPtr, atPtrExpected, valPtr, VL_MEMORY_ORDER_SEQ_CST, VL_MEMORY_ORDER_SEQ_CST)

#endif //VL_ATOMIC_PTR_H
