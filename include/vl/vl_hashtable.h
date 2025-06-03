#ifndef VL_HASHTABLE_H
#define VL_HASHTABLE_H

#include "vl_arena.h"
#include "vl_hash.h"

#define VL_HASHTABLE_ITER_INVALID   0

#ifndef VL_HASHTABLE_RESIZE_FACTOR
#define VL_HASHTABLE_RESIZE_FACTOR  0.8
#endif

#ifndef VL_HASHTABLE_DEFAULT_SIZE
#define VL_HASHTABLE_DEFAULT_SIZE   128
#endif

/**
 * This is a convenience macro for iterating over the entirety of a hashtable.
 *
 * \param table table pointer
 * \param trackVar name of the variable used as the iterator.
 */
#define VL_HASHTABLE_FOREACH(table, trackVar) for(vl_hash_iter trackVar = vlHashTableFront(table); trackVar != VL_HASHTABLE_ITER_INVALID; trackVar = vlHashTableNext(table, trackVar))

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
 * - Growth factor: doubles capacity when load factor exceeds VL_HASHTABLE_RESIZE_FACTOR
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

typedef struct {
    vl_memory *table;               //holds mapping from hash values to collision list heads in the arena
    vl_arena data;                  //holds the node key/value data
    vl_hash_function hashFunc;      //hash function; hashes keys

    vl_dsidx_t totalElements;  //total number of mapped elements
} vl_hashtable;

/**
 * \brief Hashtable element header. Not very space efficient.
 * \private
 */
typedef struct {
    vl_uint16_t     keySize;
    vl_uint16_t     valSize;
    vl_hash         keyHash;
    vl_arena_ptr    next;
} vl_hashtable_header;

/**
 * \brief Initializes the specified table.
 * Specifies the hash function to use on keys.
 *
 * The table instance should be freed via vlHashTableFree.
 *
 * \sa vlHashTableFree
 * \param table pointer
 * \param hashFunc hash function pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlHashTableInit(vl_hashtable *table, vl_hash_function hashFunc);

/**
 * \brief Frees the specified table.
 *
 * The table instance should have been initialized via vlHashTableInit.
 *
 * \sa vlHashTableInit
 * \param table pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlHashTableFree(vl_hashtable *table);

/**
 * \brief Allocates on the heap, initializes, and returns a new hash table.
 * Specifies the hash function to use on keys.
 *
 * The created hashtable instance should be deleted via vlHashTableDelete.
 *
 * \sa vlHashTableDelete
 * \param func
 * \par Complexity O(1) constant.
 * \return hashtable pointer.
 */
VL_API vl_hashtable *vlHashTableNew(vl_hash_function func);

/**
 * Deallocates and deinitializes the specified table.
 * The table MUST be initialized via vlHashTableNew.
 * \sa vlHashTableNew
 * \param table pointer
 * \par Complexity O(1) constant.
 */
VL_API void vlHashTableDelete(vl_hashtable *table);

/**
 * \brief Claims a chunk of memory associated with the specified key.
 * If the key already exists, the associated chunk is reallocated to match the specified size.
 *
 * \param table pointer
 * \param key pointer to key data
 * \param keySize size of key data, in bytes
 * \param dataSize size of element data, in bytes
 * \par Complexity O(1) constant.
 * \return iterator to inserted element.
 */
VL_API vl_hash_iter vlHashTableInsert(vl_hashtable *table, const void *key, vl_memsize_t keySize, vl_memsize_t dataSize);

/**
 * Removes the element represented by the specified key.
 * \param table pointer
 * \param key pointer to key data
 * \param keyLen length of key data, in bytes.
 * \par Complexity O(1) constant.
 */
VL_API void vlHashTableRemoveKey(vl_hashtable *table, const void *key, vl_memsize_t keyLen);

/**
 * Removes the hash element represented by the specified iterator.
 * \param table pointer
 * \param iter
 * \par Complexity of O(1) constant.
 */
VL_API void vlHashTableRemoveIter(vl_hashtable *table, vl_hash_iter iter);

/**
 * \brief Clears the specified hash table so it can be used as if it was just created.
 *
 * No data in the underlying virtual arena allocator is touched. Rather, book-keeping
 * variables are reset to their initial state.
 *
 * \param table pointer
 * \par Complexity of O(1) constant.
 */
VL_API void vlHashTableClear(vl_hashtable *table);


/**
 * \brief Clones the specified hashtable to another.
 *
 * Clones the entirety of the src table to the dest table.
 *
 * The 'src' table must be non-null and initialized.
 * The 'dest' table may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' table pointer is null, a new list is created via vlHashTableNew.
 *
 * \sa vlHashTableNew
 * \param src pointer
 * \param dest pointer
 * \return pointer to table that was copied to or created.
 */
VL_API vl_hashtable *vlHashTableClone(const vl_hashtable *table, vl_hashtable *dest);

/**
 * \brief Copies a single element of a hashtable from one table to another.
 *
 * Both tables must have the same hash function, otherwise this is a no-op and will return VL_HASHTABLE_ITER_INVALID.
 *
 * If an element by the key specified by the iterator already exists in the dest table,
 * its element data is overwritten by the contents of the element in the source table.
 *
 * \param src pointer
 * \param iter element to copy
 * \param dest pointer
 * \return iterator to element inserted into dest table, or VL_HASHTABLE_ITER_INVALID.
 */
VL_API vl_hash_iter vlHashTableCopyElement(vl_hashtable *src, vl_hash_iter iter, vl_hashtable *dest);

/**
 * \brief Copies the entirety of one hashtable to another.
 *
 * Both tables must have the same hash function, otherwise this is a no-op.
 * If any elements with matching keys exist between the two maps, they are
 * overwritten by the contents of the element in the source table.
 *
 * \param src pointer
 * \param dest pointer
 * \return total number of elements copied
 */
VL_API int vlHashTableCopy(vl_hashtable *src, vl_hashtable *dest);

/**
 * \brief Reserves memory in the hashtable before requiring it.
 *
 * This function will reserve memory for buckets, element, and key data.
 *
 * This is used to avoid resizing the underlying virtual heap and bucket allocations.
 * Resizing is a costly operation which can noticeably harm the efficiency of the table if done frequently.
 *
 * \param table pointer
 * \param buckets total buckets to reserve (spots in the table)
 * \param heapSize total bytes to reserve for element and key data
 * \par Complexity of O(1) constant.
 */
VL_API void vlHashTableReserve(vl_hashtable *table, vl_memsize_t buckets, vl_memsize_t heapSize);

/**
 * Searches the hashtable for an element with the specified key.
 * \param table pointer
 * \param key
 * \param keySize
 * \par Complexity of O(1) constant.
 * \return found element, or VL_HASHTABLE_ITER_INVALID on failure.
 */
VL_API vl_hash_iter vlHashTableFind(vl_hashtable *table, const void *key, vl_memsize_t keySize);

/**
 * Samples the key of the key-value pair indicated by the specified iterator.
 * \param table pointer
 * \param iter
 * \param size output pointer representing size of the key, in bytes. may be null.
 * \par Complexity of O(1) constant.
 * \return read-only pointer to key
 */
VL_API const vl_transient *vlHashTableSampleKey(vl_hashtable *table, vl_hash_iter iter, /*out*/vl_memsize_t *size);

/**
 * Samples the value of the key-value pair indicated by the specified iterator.
 * \param table pointer
 * \param iter
 * \param size output pointer representing the size of the value, in bytes. may be null.
 * \par Complexity of O(1) constant.
 * \return read-write pointer to value
 */
VL_API vl_transient *vlHashTableSampleValue(vl_hashtable *table, vl_hash_iter iter, /*out*/vl_memsize_t *size);

/**
 * Returns the "first" iterator for the specified table.
 * \param table pointer
 * \par Complexity of O(1) constant.
 * \return iterator to some "first" element, or VL_HASHTABLE_ITER_INVALID if none exists.
 */
VL_API vl_hash_iter vlHashTableFront(vl_hashtable *table);

/**
 * Returns the "next" iterator relative to the specified iterator.
 * \param table pointer
 * \param iter
 * \par Complexity of O(1) constant.
 * \return iterator to some "next" element, or VL_HASHTABLE_ITER_INVALID if none exists.
 */
VL_API vl_hash_iter vlHashTableNext(vl_hashtable *table, vl_hash_iter iter);

//notice "back" and "prev" functions are omitted here.
//considering there is no well-defined order that should be expected from this structure,
//it shouldn't make much difference to iterate forward or backward, so long as all
//elements are encompassed in the iteration.

//that said, it can often be observed that iteration occurs in the reverse order of
//insertion *up until* the first resize of the table.
//this is due to how the underlying arena behaves, slicing memory off the end of the first free block of memory.

#endif //VL_HASHTABLE_H