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

#ifndef VL_HASHTABLE_H
#define VL_HASHTABLE_H

#include "vl_arena.h"
#include "vl_hash.h"

#define VL_HASHTABLE_ITER_INVALID 0

#ifndef VL_HASHTABLE_RESIZE_FACTOR
#define VL_HASHTABLE_RESIZE_FACTOR 0.8
#endif

#ifndef VL_HASHTABLE_DEFAULT_SIZE
#define VL_HASHTABLE_DEFAULT_SIZE 128
#endif

/**
 * This is a convenience macro for iterating over the entirety of a hashtable.
 *
 * \param table table pointer
 * \param trackVar name of the variable used as the iterator.
 */
#define VL_HASHTABLE_FOREACH(table, trackVar)                                                                          \
    for (vl_hash_iter trackVar = vlHashTableFront(table); trackVar != VL_HASHTABLE_ITER_INVALID;                       \
         trackVar = vlHashTableNext(table, trackVar))

typedef vl_arena_ptr vl_hash_iter;

/**
 * \brief A dynamically-sized hash table with variable-sized keys and values.
 *
 * A generic hash table implementation that supports:
 * - Variable-sized keys and values
 * - Custom hash functions
 * - Efficient iteration (only visits occupied entries)
 * - Automatic growth when load factor exceeds threshold
 *
 * Implementation details:
 * - Uses separate chaining for collision resolution
 * - Built on arena allocator for efficient memory management
 * - Non-stable pointers: element addresses may change on insertion/resize
 * - Growth factor: doubles capacity when load factor exceeds
 * VL_HASHTABLE_RESIZE_FACTOR
 *
 * Performance characteristics:
 * - Find/Insert/Delete: O(1) average case
 * - Iteration: O(n) where n is number of elements
 * - Growth: O(n) when resizing is triggered
 * - Memory: O(n + m) where n is number of elements and m is number of buckets
 *
 * Memory considerations:
 * - Elements are stored contiguously in arena memory
 * - No memory is wasted on empty bucket iteration
 * - Each element has overhead for key size, value size, and chain pointer
 *
 * Usage notes:
 * - Do not store pointers to elements long-term as they may become invalid
 * - Use iterators returned by operations to track elements
 * - Always check iterator validity before use
 *
 * \sa vl_arena For details about the underlying memory management
 * \sa VL_HASHTABLE_RESIZE_FACTOR For load factor threshold configuration
 */

typedef struct
{
    vl_memory* table; // holds mapping from hash values to collision list heads
                      // in the arena
    vl_arena data; // holds the node key/value data
    vl_hash_function hashFunc; // hash function; hashes keys

    vl_dsidx_t totalElements; // total number of mapped elements
} vl_hashtable;

/**
 * \brief Hashtable element header. Not very space efficient.
 * \private
 */
typedef struct
{
    vl_uint16_t keySize;
    vl_uint16_t valSize;
    vl_hash keyHash;
    vl_arena_ptr next;
} vl_hashtable_header;

/**
 * \brief Initializes the specified table with a hash function.
 *
 * ## Contract
 * - **Ownership**: The caller maintains ownership of the `table` struct. The function initializes the internal arena
 * and bucket table.
 * - **Lifetime**: The table is valid until `vlHashTableFree` or `vlHashTableDelete`.
 * - **Thread Safety**: Not thread-safe. Concurrent access must be synchronized.
 * - **Nullability**: `table` must not be `NULL`. `hashFunc` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an already initialized table without first calling `vlHashTableFree` (causes memory
 * leak).
 * - **Memory Allocation Expectations**: Allocates initial bucket array and initializes the internal arena.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlHashTableFree
 * \param table pointer
 * \param hashFunc hash function pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlHashTableInit(vl_hashtable* table, vl_hash_function hashFunc);

/**
 * \brief De-initializes and frees the internal resources of the specified table.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal bucket array and the data arena. Does NOT release the `table`
 * struct itself.
 * - **Lifetime**: The table and all its iterators become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `table` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double free.
 * - **Memory Allocation Expectations**: Deallocates internal memory via `vlMemFree`.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlHashTableInit
 * \param table pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlHashTableFree(vl_hashtable* table);

/**
 * \brief Allocates on the heap, initializes, and returns a new hash table instance.
 *
 * ## Contract
 * - **Ownership**: The caller owns the returned `vl_hashtable` pointer and is responsible for calling
 * `vlHashTableDelete`.
 * - **Lifetime**: The table is valid until `vlHashTableDelete`.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Returns `NULL` if heap allocation for the table struct fails.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: Allocates memory for the `vl_hashtable` struct and its internal resources.
 * - **Return-value Semantics**: Returns a pointer to the newly allocated and initialized table, or `NULL`.
 *
 * \sa vlHashTableDelete
 * \param func
 * \par Complexity O(1) constant.
 * \return hashtable pointer.
 */
VL_API vl_hashtable* vlHashTableNew(vl_hash_function func);

/**
 * \brief De-initializes and deletes the specified table and its resources.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal bucket array, data arena, and the `vl_hashtable` struct.
 * - **Lifetime**: The table pointer and all its iterators become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: Safe to call if `table` is `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Double deletion.
 * - **Memory Allocation Expectations**: Deallocates internal resources and the table struct.
 * - **Return-value Semantics**: None (void).
 *
 * \sa vlHashTableNew
 * \param table pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlHashTableDelete(vl_hashtable* table);

/**
 * \brief Claims a chunk of memory associated with the specified key.
 * If the key already exists, the associated chunk is reallocated to match the
 * specified size.
 *
 * ## Contract
 * - **Ownership**: The hashtable maintains copies of the key and allocates space for the data. The caller maintains
 * ownership of their input `key` and `dataSize` is the amount of memory reserved for the caller to own within the
 * table.
 * - **Lifetime**: The returned iterator is valid until the element is removed, the table is cleared, or the table is
 * destroyed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `table` must not be `NULL`. `key` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_HASHTABLE_ITER_INVALID` if node allocation or expansion fails.
 * - **Undefined Behavior**: Passing an uninitialized table.
 * - **Memory Allocation Expectations**: May trigger expansion of the underlying bucket array and the internal data
 * arena.
 * - **Return-value Semantics**: Returns a `vl_hash_iter` handle to the element, or `VL_HASHTABLE_ITER_INVALID` on
 * failure.
 *
 * \param table pointer
 * \param key pointer to key data
 * \param keySize size of key data, in bytes
 * \param dataSize size of element data, in bytes
 * \par Complexity O(1) constant.
 * \return iterator to inserted element.
 */
VL_API vl_hash_iter vlHashTableInsert(vl_hashtable* table, const void* key, vl_memsize_t keySize,
                                      vl_memsize_t dataSize);

/**
 * \brief Removes the element represented by the specified key.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal key copy and data block.
 * - **Lifetime**: Iterators to the removed element become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `table` must not be `NULL`. `key` must not be `NULL`.
 * - **Error Conditions**: None (no-op if key is not found).
 * - **Undefined Behavior**: Passing an uninitialized table.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param table pointer
 * \param key pointer to key data
 * \param keyLen length of key data, in bytes.
 * \par Complexity O(1) constant.
 */
VL_API void vlHashTableRemoveKey(vl_hashtable* table, const void* key, vl_memsize_t keyLen);

/**
 * \brief Removes the hash element represented by the specified iterator.
 *
 * ## Contract
 * - **Ownership**: Releases ownership of the internal key copy and data block.
 * - **Lifetime**: The passed iterator becomes invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `table` must not be `NULL`. `iter` should not be `VL_HASHTABLE_ITER_INVALID`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator or an iterator from a different table.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param table pointer
 * \param iter
 * \par Complexity of O(1) constant.
 */
VL_API void vlHashTableRemoveIter(vl_hashtable* table, vl_hash_iter iter);

/**
 * \brief Clears the specified hash table so it can be used as if it was just
 * created.
 *
 * No data in the underlying virtual arena allocator is touched. Rather,
 * book-keeping variables are reset to their initial state.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: All previously returned iterators and sampled pointers become invalid.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `table` must not be `NULL`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized table.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: None (void).
 *
 * \param table pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlHashTableClear(vl_hashtable* table);

/**
 * \brief Clones the specified hashtable to another.
 *
 * Clones the entirety of the src table to the dest table, including all elements and their data.
 *
 * The 'src' table must be non-null and initialized.
 * The 'dest' table may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' table pointer is null, a new list is created via
 * vlHashTableNew.
 *
 * ## Contract
 * - **Ownership**: If `dest` is `NULL`, the caller owns the returned `vl_hashtable`. If `dest` is provided, ownership
 * remains with the caller.
 * - **Lifetime**: The cloned table remains valid until deleted or freed.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` must not be `NULL`. `dest` can be `NULL`.
 * - **Error Conditions**: Returns `NULL` on allocation failure.
 * - **Undefined Behavior**: Passing an uninitialized table.
 * - **Memory Allocation Expectations**: May allocate a new table struct and expansion of its internal resources.
 * - **Return-value Semantics**: Returns the pointer to the cloned table, or `NULL` on failure.
 *
 * \sa vlHashTableNew
 * \param src pointer
 * \param dest pointer
 * \return pointer to table that was copied to or created.
 */
VL_API vl_hashtable* vlHashTableClone(const vl_hashtable* table, vl_hashtable* dest);

/**
 * \brief Copies a single element of a hashtable from one table to another.
 *
 * Both tables must have the same hash function, otherwise this is a no-op and
 * will return VL_HASHTABLE_ITER_INVALID.
 *
 * If an element by the key specified by the iterator already exists in the dest
 * table, its element data is overwritten by the contents of the element in the
 * source table.
 *
 * ## Contract
 * - **Ownership**: `dest` maintains copies of the key and data.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` and `dest` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_HASHTABLE_ITER_INVALID` if hash functions don't match or if insertion fails.
 * - **Undefined Behavior**: Passing an invalid iterator.
 * - **Memory Allocation Expectations**: May trigger expansion of the destination table and its arena.
 * - **Return-value Semantics**: Returns the `vl_hash_iter` of the element in the destination table, or
 * `VL_HASHTABLE_ITER_INVALID` on failure.
 *
 * \param src pointer
 * \param iter element to copy
 * \param dest pointer
 * \return iterator to element inserted into dest table, or
 * VL_HASHTABLE_ITER_INVALID.
 */
VL_API vl_hash_iter vlHashTableCopyElement(vl_hashtable* src, vl_hash_iter iter, vl_hashtable* dest);

/**
 * \brief Copies the entirety of one hashtable to another.
 *
 * Both tables must have the same hash function, otherwise this is a no-op.
 * If any elements with matching keys exist between the two maps, they are
 * overwritten by the contents of the element in the source table.
 *
 * ## Contract
 * - **Ownership**: `dest` maintains copies of all keys and data.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `src` and `dest` must not be `NULL`.
 * - **Error Conditions**: Returns 0 if hash functions don't match.
 * - **Undefined Behavior**: None.
 * - **Memory Allocation Expectations**: May trigger expansion of the destination table and its arena.
 * - **Return-value Semantics**: Returns the total number of elements successfully copied.
 *
 * \param src pointer
 * \param dest pointer
 * \return total number of elements copied
 */
VL_API int vlHashTableCopy(vl_hashtable* src, vl_hashtable* dest);

/**
 * \brief Reserves memory in the hashtable before requiring it.
 *
 * This function will reserve memory for buckets, element, and key data.
 *
 * This is used to avoid resizing the underlying virtual heap and bucket
 * allocations. Resizing is a costly operation which can noticeably harm the
 * efficiency of the table if done frequently.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Not thread-safe.
 * - **Nullability**: `table` must not be `NULL`.
 * - **Error Conditions**: Allocation failure during reservation.
 * - **Undefined Behavior**: Passing an uninitialized table.
 * - **Memory Allocation Expectations**: Triggers reallocation of the bucket array and the internal data arena.
 * - **Return-value Semantics**: None (void).
 *
 * \param table pointer
 * \param buckets total buckets to reserve (spots in the table)
 * \param heapSize total bytes to reserve for element and key data
 * \par Complexity of O(1) constant.
 */
VL_API void vlHashTableReserve(vl_hashtable* table, vl_memsize_t buckets, vl_memsize_t heapSize);

/**
 * \brief Searches the hashtable for an element with the specified key.
 *
 * ## Contract
 * - **Ownership**: Unchanged.
 * - **Lifetime**: Unchanged.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: `table` must not be `NULL`. `key` must not be `NULL`.
 * - **Error Conditions**: Returns `VL_HASHTABLE_ITER_INVALID` if the key is not found.
 * - **Undefined Behavior**: Passing an uninitialized table.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a `vl_hash_iter` to the found element, or `VL_HASHTABLE_ITER_INVALID` if not
 * found.
 *
 * \param table pointer
 * \param key
 * \param keySize
 * \par Complexity of O(1) constant.
 * \return found element, or VL_HASHTABLE_ITER_INVALID on failure.
 */
VL_API vl_hash_iter vlHashTableFind(vl_hashtable* table, const void* key, vl_memsize_t keySize);

/**
 * \brief Samples the key of the key-value pair indicated by the specified iterator.
 *
 * ## Contract
 * - **Ownership**: Ownership remains with the table. The caller must not modify or free the returned pointer.
 * - **Lifetime**: The returned pointer is valid until the element is removed or the table is reallocated/destroyed.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: Returns `NULL` if `iter` is `VL_HASHTABLE_ITER_INVALID`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a read-only pointer to the key data.
 *
 * \param table pointer
 * \param iter
 * \param size output pointer representing size of the key, in bytes. may be
 * null.
 * \par Complexity of O(1) constant.
 * \return read-only pointer to key
 */
VL_API const vl_transient* vlHashTableSampleKey(vl_hashtable* table, vl_hash_iter iter,
                                                /*out*/ vl_memsize_t* size);

/**
 * \brief Samples the value of the key-value pair indicated by the specified iterator.
 *
 * ## Contract
 * - **Ownership**: Ownership remains with the table. The caller can modify the data at the returned pointer but must
 * not free it.
 * - **Lifetime**: The returned pointer is valid until the element is removed or the table is reallocated/destroyed.
 * - **Thread Safety**: Safe for concurrent reads if no thread is writing. Not thread-safe for concurrent writes to the
 * same value.
 * - **Nullability**: Returns `NULL` if `iter` is `VL_HASHTABLE_ITER_INVALID`.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a read-write pointer to the value data.
 *
 * \param table pointer
 * \param iter
 * \param size output pointer representing the size of the value, in bytes. may
 * be null.
 * \par Complexity of O(1) constant.
 * \return read-write pointer to value
 */
VL_API vl_transient* vlHashTableSampleValue(vl_hashtable* table, vl_hash_iter iter,
                                            /*out*/ vl_memsize_t* size);

/**
 * \brief Returns the "first" iterator for the specified table.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Valid until the element is removed or the table is reallocated/destroyed.
 * - **Thread Safety**: Safe for concurrent reads.
 * - **Nullability**: Returns `VL_HASHTABLE_ITER_INVALID` if the table is empty.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an uninitialized table.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns a `vl_hash_iter` to some "first" element in the table.
 *
 * \param table pointer
 * \par Complexity of O(1) constant.
 * \return iterator to some "first" element, or VL_HASHTABLE_ITER_INVALID if
 * none exists.
 */
VL_API vl_hash_iter vlHashTableFront(vl_hashtable* table);

/**
 * \brief Returns the "next" iterator relative to the specified iterator.
 *
 * ## Contract
 * - **Ownership**: None.
 * - **Lifetime**: Same as `vlHashTableFront`.
 * - **Thread Safety**: Same as `vlHashTableFront`.
 * - **Nullability**: Returns `VL_HASHTABLE_ITER_INVALID` if no more elements exist.
 * - **Error Conditions**: None.
 * - **Undefined Behavior**: Passing an invalid iterator.
 * - **Memory Allocation Expectations**: None.
 * - **Return-value Semantics**: Returns the next `vl_hash_iter` in the sequence.
 *
 * \param table pointer
 * \param iter
 * \par Complexity of O(1) constant.
 * \return iterator to some "next" element, or VL_HASHTABLE_ITER_INVALID if none
 * exists.
 */
VL_API vl_hash_iter vlHashTableNext(vl_hashtable* table, vl_hash_iter iter);

// notice "back" and "prev" functions are omitted here.
// considering there is no well-defined order that should be expected from this
// structure, it shouldn't make much difference to iterate forward or backward,
// so long as all elements are encompassed in the iteration.

// that said, it can often be observed that iteration occurs in the reverse
// order of insertion *up until* the first resize of the table. this is due to
// how the underlying arena behaves, slicing memory off the end of the first
// free block of memory.

#endif // VL_HASHTABLE_H
