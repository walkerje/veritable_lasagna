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
 * \brief A dynamically-sized hash table.
 *
 * The vl_hashtable structure represents a HashTable/Hashmap/Unordered map.
 * It is implemented on top of a vl_arena, which holds the underlying node data.
 * A vl_memory* allocation is used to hold mappings between hash nodes and
 * their respective vl_arena_ptr values into the virtual arena allocator.
 *
 * This hashtable resolves collisions through separate chaining.
 *
 * The structure retains a counter that represents the total number of elements it contains.
 * Should the total number of elements become larger or equal to the
 * size of the table * VL_HASHTABLE_RESIZE_FACTOR, the table grows in size by doubling its capacity.
 * The table and collision chains will then be rebuilt according to this new size with a complexity of O(n).
 *
 * This table does not waste time by iterating over empty space.
 * \sa vl_arena
 */
typedef struct{
    vl_memory*              table;          //holds mapping from hash values to collision list heads in the arena
    vl_arena                data;           //holds the node key/value data
    vl_hash_function        hashFunc;       //hash function; hashes keys

    vl_dsidx_t              totalElements;  //total number of mapped elements
} vl_hashtable;

/**
 * \brief Hashtable element header. Not very space efficient.
 * \private
 */
typedef struct{
    vl_memsize_t  keySize;
    vl_memsize_t  valSize;
    vl_hash keyHash;
    vl_arena_ptr next;
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
void            vlHashTableInit(vl_hashtable* table, vl_hash_function hashFunc);

/**
 * \brief Frees the specified table.
 *
 * The table instance should have been initialized via vlHashTableInit.
 *
 * \sa vlHashTableInit
 * \param table pointer
 * \par Complexity O(1) constant.
 */
void            vlHashTableFree(vl_hashtable* table);

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
vl_hashtable*   vlHashTableNew(vl_hash_function func);

/**
 * Deallocates and deinitializes the specified table.
 * The table MUST be initialized via vlHashTableNew.
 * \sa vlHashTableNew
 * \param table pointer
 * \par Complexity O(1) constant.
 */
void            vlHashTableDelete(vl_hashtable* table);

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
vl_hash_iter    vlHashTableInsert(vl_hashtable* table, const void* key, vl_memsize_t keySize, vl_memsize_t dataSize);

/**
 * Removes the element represented by the specified key.
 * \param table pointer
 * \param key pointer to key data
 * \param keyLen length of key data, in bytes.
 * \par Complexity O(1) constant.
 */
void            vlHashTableRemoveKey(vl_hashtable* table, const void* key, vl_memsize_t keyLen);

/**
 * Removes the hash element represented by the specified iterator.
 * \param table pointer
 * \param iter
 * \par Complexity of O(1) constant.
 */
void            vlHashTableRemoveIter(vl_hashtable* table, vl_hash_iter iter);

/**
 * \brief Clears the specified hash table so it can be used as if it was just created.
 *
 * No data in the underlying virtual arena allocator is touched. Rather, book-keeping
 * variables are reset to their initial state.
 *
 * \param table pointer
 * \par Complexity of O(1) constant.
 */
void            vlHashTableClear(vl_hashtable* table);


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
vl_hashtable*   vlHashTableClone(const vl_hashtable* table, vl_hashtable* dest);

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
vl_hash_iter vlHashTableCopyElement(vl_hashtable* src, vl_hash_iter iter, vl_hashtable* dest);

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
int             vlHashTableCopy(vl_hashtable* src, vl_hashtable* dest);

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
void            vlHashTableReserve(vl_hashtable* table, vl_memsize_t buckets, vl_memsize_t heapSize);

/**
 * Searches the hashtable for an element with the specified key.
 * \param table pointer
 * \param key
 * \param keySize
 * \par Complexity of O(1) constant.
 * \return found element, or VL_HASHTABLE_ITER_INVALID on failure.
 */
vl_hash_iter    vlHashTableFind(vl_hashtable* table, const void* key, vl_memsize_t keySize);

/**
 * Samples the key of the key-value pair indicated by the specified iterator.
 * \param table pointer
 * \param iter
 * \param size output pointer representing size of the key, in bytes. may be null.
 * \par Complexity of O(1) constant.
 * \return read-only pointer to key
 */
const vl_transient*     vlHashTableSampleKey(vl_hashtable* table, vl_hash_iter iter, /*out*/vl_memsize_t* size);

/**
 * Samples the value of the key-value pair indicated by the specified iterator.
 * \param table pointer
 * \param iter
 * \param size output pointer representing the size of the value, in bytes. may be null.
 * \par Complexity of O(1) constant.
 * \return read-write pointer to value
 */
vl_transient*           vlHashTableSampleValue(vl_hashtable* table, vl_hash_iter iter, /*out*/vl_memsize_t* size);

/**
 * Returns the "first" iterator for the specified table.
 * \param table pointer
 * \par Complexity of O(1) constant.
 * \return iterator to some "first" element, or VL_HASHTABLE_ITER_INVALID if none exists.
 */
vl_hash_iter    vlHashTableFront(vl_hashtable* table);

/**
 * Returns the "next" iterator relative to the specified iterator.
 * \param table pointer
 * \param iter
 * \par Complexity of O(1) constant.
 * \return iterator to some "next" element, or VL_HASHTABLE_ITER_INVALID if none exists.
 */
vl_hash_iter    vlHashTableNext(vl_hashtable* table, vl_hash_iter iter);

//notice "back" and "prev" functions are omitted here.
//considering there is no well-defined order that should be expected from this structure,
//it shouldn't make much difference to iterate forward or backward, so long as all
//elements are encompassed in the iteration.

//that said, it can often be observed that iteration occurs in the reverse order of
//insertion *up until* the first resize of the table.
//this is due to how the underlying arena behaves, slicing memory off the end of the first free block of memory.

#endif //VL_HASHTABLE_H