#ifndef VL_MSGPACK_H
#define VL_MSGPACK_H

#include "vl_hashtable.h"
#include "vl_arena.h"
#include "vl_buffer.h"
#include <string.h>

#define VL_MSGPACK_ITER_INVALID     VL_HASHTABLE_ITER_INVALID
#define VL_MSGPACK_EXT_NONE         -127

#define VL_MSGPACK_FOREACH_CHILD(packPtr, parentIter, childIterSymbol)                  \
    for(vl_msgpack_iter childIterSymbol = vlMsgPackFirstChild((packPtr), (parentIter)); \
    childIterSymbol != VL_MSGPACK_ITER_INVALID;                                         \
    childIterSymbol = vlMsgPackNextSibling((packPtr), (childIterSymbol)))

typedef vl_hash_iter vl_msgpack_iter;

/**
 * \brief All MessagePack Types
 */
typedef enum vl_msgpack_type_ {
    /**
     * \brief MessagePack type equivalent to NIL. Has no data associated with it.
     */
    VL_MSGPACK_NIL,
    /**
     * \brief MessagePack Boolean. Either True (1) or False (0).
     */
    VL_MSGPACK_BOOL,
    /**
     * \brief MessagePack Signed Integer. Implemented as vl_ilarge_t.
     */
    VL_MSGPACK_INT,
    /**
     * \brief MessagePack Unsigned Integer. Implemented as vl_ularge_t.
     */
    VL_MSGPACK_UINT,
    /**
     * \brief MessagePack 32-bit Float. Implemented as vl_float32_t.
     */
    VL_MSGPACK_FLOAT32,
    /**
     * \brief MessagePack 64-bit Float. Implemented as vl_float64_t.
     */
    VL_MSGPACK_FLOAT64,
    /**
     * \brief MessagePack String. Must have UTF-8 encoding.
     */
    VL_MSGPACK_STRING,
    /**
     * \brief MessagePack Binary. An arbitrary sequence of bytes with a well-defined length.
     */
    VL_MSGPACK_BINARY,
    /**
     * \brief MessagePack Array. A sequence of dynamically-typed nodes of a well-determined length.
     */
    VL_MSGPACK_ARRAY,

    /**
     * \brief MessagePack Map. A collection of node key-value pairs of an arbitrary size.
     */
    VL_MSGPACK_MAP,

    /**
     * \brief MessagePack Extension Type. Has a type identifier in range of 0...127 and arbitrary data.
     */
    VL_MSGPACK_EXT
} vl_msgpack_type;

/**
 * \brief MessagePack Document Object Model
 *
 * An in-memory, hierarchical representation of a complete MessagePack.
 *
 * This structure represents an N-Tree with parent-child relationships that accurately
 * represents a MessagePack structure. It is implemented using a hierarchical hash table for
 * nodes and parent-child relationships, using separate arena allocator for node values.
 * Nodes in this structure are dynamically typed.
 *
 * The design of this structure is NOT completely compliant with the MessagePack spec.
 * It requires the two following rules to be enforced:
 *  1. Elements in a Map must ALWAYS have String keys (e.g, named elements).
 *  2. Elements in an Array must ALWAYS have Integer keys (e.g, indexed elements), although their keys are implicit.
 * This design choice simplifies the implementation by disallowing complex structures
 * such as Maps and Arrays to act as Map keys, keeping it in line with formats comparable to JSON.
 *
 * All strings in and out of this structure, keys or values, are assumed to be UTF-8 encoded.
 *
 * \sa https://github.com/msgpack/msgpack/blob/master/spec.md
 */
typedef struct vl_msgpack_ {
    vl_hashtable nodes;      //hierarchy
    vl_arena values;     //data
    vl_msgpack_iter root;       //root node
} vl_msgpack;

/**
 * \brief Initializes the specified MessagePack DOM.
 *
 * \sa vlMsgPackInit
 * \param pack pointer to DOM
 */
VL_API void vlMsgPackInit(vl_msgpack *pack);

/**
 * \brief Frees the specified MessagePack DOM.
 *
 * The specified MessagePack must have been initialized via vlMsgPackInit before calling this function.
 *
 * \param pack pointer to DOM
 */
VL_API void vlMsgPackFree(vl_msgpack *pack);

/**
 * \brief Allocates and initializes a MessagePack DOM.
 *
 * The returned pointer is to be deleted via vlMsgPackDelete.
 *
 * \sa vlMsgPackDelete
 * \return pointer to DOM
 */
VL_API vl_msgpack *vlMsgPackNew();

/**
 * \brief Deletes the specified MessagePack DOM.
 *
 * The specified MessagePack must have been initialized via vlMsgPackNew before calling this function.
 *
 * \sa vlMsgPackNew
 * \param pack pointer to DOM
 */
VL_API void vlMsgPackDelete(vl_msgpack *pack);

/**
 * \brief Clears the MessagePack DOM, resetting it for reuse.
 *
 * This function resets the MessagePack DOM instance without deallocating memory.
 * Previously allocated memory remains available for reuse, improving performance
 * when repeatedly decoding into the same structure.
 *
 * \param pack The MessagePack DOM instance to clear.
 */
VL_API void vlMsgPackClear(vl_msgpack *pack);


/**
 * \brief Clones the specified MessagePack DOM to another.
 *
 * Clones the entirety of the src MessagePack to the dest MessagePack.
 *
 * The 'src' DOM must be non-null and initialized.
 * The 'dest' DOM may be null, but if it is not null it must be initialized.
 *
 * If the 'dest' table pointer is null, a new DOM is created via vlMsgPackNew.
 *
 * \sa vlMsgPackNew
 * \param src source DOM pointer
 * \param dest destination DOM pointer
 * \return pointer to the MessagePack that was copied to or created.
 */
VL_API vl_msgpack *vlMsgPackClone(vl_msgpack *src, vl_msgpack *dest);

/**
 * \brief Retrieves the parent node of a given node in the MessagePack DOM.
 *
 * \param pack The MessagePack DOM instance.
 * \param iter The iterator pointing to the current node.
 * \return An iterator to the parent node, or an invalid iterator if the node has no parent.
 */
VL_API vl_msgpack_iter vlMsgPackParent(vl_msgpack *pack, vl_msgpack_iter iter);

/**
 * \brief Retrieves the total number of children of a given node.
 *
 * \param pack The MessagePack DOM instance.
 * \param iter The iterator pointing to the current node.
 * \return The number of child nodes.
 */
VL_API vl_dsidx_t vlMsgPackTotalChildren(vl_msgpack *pack, vl_msgpack_iter iter);

/**
 * \brief Retrieves the first child of a given node.
 *
 * \param pack The MessagePack DOM instance.
 * \param iter The iterator pointing to the current node.
 * \return An iterator to the first child node, or an invalid iterator if there are no children.
 */
VL_API vl_msgpack_iter vlMsgPackFirstChild(vl_msgpack *pack, vl_msgpack_iter iter);

/**
 * \brief Retrieves the next sibling of a given node.
 *
 * \param pack The MessagePack DOM instance.
 * \param iter The iterator pointing to the current node.
 * \return An iterator to the next sibling node, or an invalid iterator if there is no next sibling.
 */
VL_API vl_msgpack_iter vlMsgPackNextSibling(vl_msgpack *pack, vl_msgpack_iter iter);

/**
 * \brief Retrieves the previous sibling of a given node.
 *
 * \param pack The MessagePack DOM instance.
 * \param iter The iterator pointing to the current node.
 * \return An iterator to the previous sibling node, or an invalid iterator if there is no previous sibling.
 */
VL_API vl_msgpack_iter vlMsgPackPrevSibling(vl_msgpack *pack, vl_msgpack_iter iter);

/**
 * \brief Retrieves the type of a given node in the MessagePack DOM.
 *
 * \param pack The MessagePack DOM instance.
 * \param iter The iterator pointing to the current node.
 * \return The MessagePack type of the node.
 */
VL_API vl_msgpack_type vlMsgPackType(vl_msgpack *pack, vl_msgpack_iter iter);

/**
 * \brief Retrieves the extension type of a MessagePack EXT node.
 *
 * \param pack The MessagePack DOM instance.
 * \param iter The iterator pointing to the current node.
 * \return The extension type value, or an undefined result if the node is not an EXT type.
 */
VL_API vl_int8_t vlMsgPackExtType(vl_msgpack *pack, vl_msgpack_iter iter);

/**
 * \brief Inserts a new element into the MessagePack DOM with an extended type.
 *
 * This function inserts an element of the specified MessagePack type and optional
 * extended type under a given parent node. The key and value are copied into the
 * internal storage, allowing external memory to be safely freed after insertion.
 *
 * \param pack     The MessagePack DOM instance.
 * \param type     The type of the inserted element.
 * \param subType  The extended type (used for MessagePack ext types).
 * \param parent   The parent node where the element is inserted.
 * \param keyPtr   Pointer to the key data.
 * \param keyLen   Length of the key.
 * \param dataPtr  Pointer to the value data.
 * \param dataLen  Length of the value.
 * \return An iterator to the newly inserted element.
 */
VL_API vl_msgpack_iter vlMsgPackInsertExt(
        vl_msgpack *pack,
        vl_msgpack_type type,
        vl_int8_t subType,
        vl_msgpack_iter parent,
        const void *keyPtr,
        vl_memsize_t keyLen,
        const void *dataPtr,
        vl_memsize_t dataLen
);

/**
 * \brief Inserts a new element into the MessagePack DOM.
 *
 * This function is a wrapper around `vlMsgPackInsertExt`, inserting an element
 * without an extended type (normal MessagePack types only).
 *
 * \param pack    The MessagePack DOM instance.
 * \param type    The type of the inserted element.
 * \param parent  The parent node where the element is inserted.
 * \param keyPtr  Pointer to the key data.
 * \param keyLen  Length of the key.
 * \param dataPtr Pointer to the value data.
 * \param dataLen Length of the value.
 * \return An iterator to the newly inserted element.
 */
static inline vl_msgpack_iter vlMsgPackInsert(
        vl_msgpack *pack,
        vl_msgpack_type type,
        vl_msgpack_iter parent,
        const void *keyPtr,
        vl_memsize_t keyLen,
        const void *dataPtr,
        vl_memsize_t dataLen
) {
    return vlMsgPackInsertExt(pack, type, VL_MSGPACK_EXT_NONE, parent, keyPtr, keyLen, dataPtr, dataLen);
}

/**
 * \brief Removes an element from the MessagePack DOM.
 *
 * This function removes the specified element and all its children from the DOM.
 * Memory may be reused depending on the allocator's behavior.
 *
 * \param pack  The MessagePack DOM instance.
 * \param iter  Iterator pointing to the element to be removed.
 */
VL_API void vlMsgPackRemove(vl_msgpack *pack, vl_msgpack_iter iter);

/**
 * \brief Finds a child element by key.
 *
 * This function searches for a child under the given parent node with a matching key.
 *
 * \param pack    The MessagePack DOM instance.
 * \param parent  The parent node to search in.
 * \param key     Pointer to the key data.
 * \param keyLen  Length of the key.
 * \return An iterator to the matching child or an invalid iterator if not found.
 */
VL_API vl_msgpack_iter vlMsgPackFindChild(
        vl_msgpack *pack,
        vl_msgpack_iter parent,
        const void *key,
        vl_memsize_t keyLen
);

/**
 * \brief Finds a child element by name.
 *
 * This function is a wrapper around `vlMsgPackFindChild`, searching for a child
 * using a null-terminated, UTF-8-encoded string as the key.
 * The parent must be a map.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node to search in.
 * \param name   Null-terminated key string.
 * \return An iterator to the matching child or an invalid iterator if not found.
 */
static inline vl_msgpack_iter vlMsgPackFindChildNamed(
        vl_msgpack *pack,
        vl_msgpack_iter parent,
        const char *name
) {
    return vlMsgPackFindChild(pack, parent, name, strlen(name));
}

/**
 * \brief Finds a child element by index.
 *
 * This function retrieves the indexed child from a parent container.
 * The parent must be an array.
 *
 * \param pack   The MessagePack DOM instance.
 * \param iter   The parent node.
 * \param idx    The zero-based index of the child.
 * \return An iterator to the indexed child or an invalid iterator if out of bounds.
 */
VL_API vl_msgpack_iter vlMsgPackFindChildIndexed(
        vl_msgpack *pack,
        vl_msgpack_iter iter,
        vl_dsidx_t idx
);

/**
 * \brief Retrieves the key associated with an element.
 *
 * This function returns a pointer to the key data of the given element.
 * The key may move if modifications occur in the DOM, so users should not
 * store this pointer for long-term access.
 *
 * \param pack  The MessagePack DOM instance.
 * \param iter  The element iterator.
 * \param size  Pointer to receive the key size (optional, can be NULL).
 * \return A transient pointer to the key data.
 */
VL_API const vl_transient *vlMsgPackSampleKey(
        vl_msgpack *pack,
        vl_msgpack_iter iter,
        vl_memsize_t *size
);

/**
 * \brief Retrieves the key of an indexed map element.
 *
 * This function assumes the key is stored as an integer and returns it directly.
 *
 * \param pack  The MessagePack DOM instance.
 * \param iter  The element iterator.
 * \return The integer index of the key.
 */
static inline vl_dsidx_t vlMsgPackSampleKeyIndex(
        vl_msgpack *pack,
        vl_msgpack_iter iter
) {
    return *((const vl_dsidx_t *) vlMsgPackSampleKey(pack, iter, NULL));
}

/**
 * \brief Retrieves the value associated with an element.
 *
 * This function returns a pointer to the value data of the given element.
 * The value may move if modifications occur in the DOM, so users should not
 * store this pointer for long-term access.
 *
 * \param pack  The MessagePack DOM instance.
 * \param iter  The element iterator.
 * \param size  Pointer to receive the value size (optional, can be NULL).
 * \return A transient pointer to the value data.
 */
VL_API vl_transient *vlMsgPackSampleValue(
        vl_msgpack *pack,
        vl_msgpack_iter iter,
        vl_memsize_t *size
);

#ifndef vlMsgPackRoot
/**
 * \brief Macro to retrieve the root of the MessagePack DOM.
 *
 * This macro provides access to the root node of the DOM.
 * The root node is always a map.
 *
 * \param packPtr  A pointer to the MessagePack DOM instance.
 * \return The root iterator of the MessagePack DOM.
 */
#define vlMsgPackRoot(packPtr) ((packPtr)->root)
#endif

/**
 * \brief Sets a named map under a parent node.
 *
 * This function inserts a new map element with a given name (key) under the
 * specified parent node.
 * The parent must be a map.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the map is inserted.
 * \param key    The name (key) of the new map.
 * \return An iterator to the newly inserted map.
 */
static inline vl_msgpack_iter vlMsgPackSetMapNamed(vl_msgpack *pack, vl_msgpack_iter parent, const char *key) {
    return vlMsgPackInsert(pack, VL_MSGPACK_MAP, parent, key, strlen(key), NULL, 0);
}

/**
 * \brief Sets an indexed map under a parent node.
 *
 * This function inserts a new map element with an integer index under the
 * specified parent node.
 * The parent must be an array.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the map is inserted.
 * \param idx    The index of the new map.
 * \return An iterator to the newly inserted map.
 */
static inline vl_msgpack_iter vlMsgPackSetMapIndexed(vl_msgpack *pack, vl_msgpack_iter parent, vl_dsidx_t idx) {
    return vlMsgPackInsert(pack, VL_MSGPACK_MAP, parent, &idx, sizeof(vl_dsidx_t), NULL, 0);
}

/**
 * \brief Sets a named array under a parent node.
 *
 * This function inserts a new array element with a given name (key) and length
 * under the specified parent node.
 * The parent must be a map.
 *
 * \param pack    The MessagePack DOM instance.
 * \param parent  The parent node where the array is inserted.
 * \param arrayLen The length of the array.
 * \param key     The name (key) of the new array.
 * \return An iterator to the newly inserted array.
 */
static inline vl_msgpack_iter
vlMsgPackSetArrayNamed(vl_msgpack *pack, vl_msgpack_iter parent, vl_dsidx_t arrayLen, const char *key) {
    return vlMsgPackInsert(pack, VL_MSGPACK_ARRAY, parent, key, strlen(key), &arrayLen, sizeof(vl_dsidx_t));
}

/**
 * \brief Sets an indexed array under a parent node.
 *
 * This function inserts a new array element with an integer index and length
 * under the specified parent node.
 * The parent must be an array.
 *
 * \param pack    The MessagePack DOM instance.
 * \param parent  The parent node where the array is inserted.
 * \param arrayLen The length of the array.
 * \param idx     The index of the new array.
 * \return An iterator to the newly inserted array.
 */
static inline vl_msgpack_iter
vlMsgPackSetArrayIndexed(vl_msgpack *pack, vl_msgpack_iter parent, vl_dsidx_t arrayLen, vl_dsidx_t idx) {
    return vlMsgPackInsert(pack, VL_MSGPACK_ARRAY, parent, &idx, sizeof(vl_dsidx_t), &arrayLen, sizeof(vl_dsidx_t));
}

/**
 * \brief Sets a named boolean value under a parent node.
 *
 * This function inserts a new boolean value with a given name (key) under the
 * specified parent node.
 * The parent must be a map.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the boolean value is inserted.
 * \param value  The boolean value to insert.
 * \param key    The name (key) of the new boolean value.
 * \return An iterator to the newly inserted boolean value.
 */
static inline vl_msgpack_iter
vlMsgPackSetBoolNamed(vl_msgpack *pack, vl_msgpack_iter parent, vl_bool_t value, const char *key) {
    return vlMsgPackInsert(pack, VL_MSGPACK_BOOL, parent, key, strlen(key), &value, sizeof(vl_bool_t));
}

/**
 * \brief Sets an indexed boolean value under a parent node.
 *
 * This function inserts a new boolean value with an integer index under the
 * specified parent node.
 * The parent must be an array.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the boolean value is inserted.
 * \param value  The boolean value to insert.
 * \param idx    The index of the new boolean value.
 * \return An iterator to the newly inserted boolean value.
 */
static inline vl_msgpack_iter
vlMsgPackSetBoolIndexed(vl_msgpack *pack, vl_msgpack_iter parent, vl_bool_t value, vl_dsidx_t idx) {
    return vlMsgPackInsert(pack, VL_MSGPACK_BOOL, parent, &idx, sizeof(vl_dsidx_t), &value, sizeof(vl_bool_t));
}

/**
 * \brief Sets a named integer value under a parent node.
 *
 * This function inserts a new integer value with a given name (key) under the
 * specified parent node.
 * The parent must be a map.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the integer value is inserted.
 * \param value  The integer value to insert.
 * \param key    The name (key) of the new integer value.
 * \return An iterator to the newly inserted integer value.
 */
static inline vl_msgpack_iter
vlMsgPackSetIntNamed(vl_msgpack *pack, vl_msgpack_iter parent, vl_ilarge_t value, const char *key) {
    return vlMsgPackInsert(pack, VL_MSGPACK_INT, parent, key, strlen(key), &value, sizeof(vl_ilarge_t));
}

/**
 * \brief Sets an indexed integer value under a parent node.
 *
 * This function inserts a new integer value with an integer index under the
 * specified parent node.
 * The parent must be an array.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the integer value is inserted.
 * \param value  The integer value to insert.
 * \param idx    The index of the new integer value.
 * \return An iterator to the newly inserted integer value.
 */
static inline vl_msgpack_iter
vlMsgPackSetIntIndexed(vl_msgpack *pack, vl_msgpack_iter parent, vl_ilarge_t value, vl_dsidx_t idx) {
    return vlMsgPackInsert(pack, VL_MSGPACK_INT, parent, &idx, sizeof(vl_dsidx_t), &value, sizeof(vl_ilarge_t));
}

/**
 * \brief Sets a named unsigned integer value under a parent node.
 *
 * This function inserts a new unsigned integer value with a given name (key)
 * under the specified parent node.
 * The parent must be a map.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the unsigned integer value is inserted.
 * \param value  The unsigned integer value to insert.
 * \param key    The name (key) of the new unsigned integer value.
 * \return An iterator to the newly inserted unsigned integer value.
 */
static inline vl_msgpack_iter
vlMsgPackSetUIntNamed(vl_msgpack *pack, vl_msgpack_iter parent, vl_ularge_t value, const char *key) {
    return vlMsgPackInsert(pack, VL_MSGPACK_UINT, parent, key, strlen(key), &value, sizeof(vl_ularge_t));
}

/**
 * \brief Sets an indexed unsigned integer value under a parent node.
 *
 * This function inserts a new unsigned integer value with an integer index under
 * the specified parent node.
 * The parent must be an array.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the unsigned integer value is inserted.
 * \param value  The unsigned integer value to insert.
 * \param idx    The index of the new unsigned integer value.
 * \return An iterator to the newly inserted unsigned integer value.
 */
static inline vl_msgpack_iter
vlMsgPackSetUIntIndexed(vl_msgpack *pack, vl_msgpack_iter parent, vl_ularge_t value, vl_dsidx_t idx) {
    return vlMsgPackInsert(pack, VL_MSGPACK_UINT, parent, &idx, sizeof(vl_dsidx_t), &value, sizeof(vl_ularge_t));
}

/**
 * \brief Sets a named 32-bit floating point value under a parent node.
 *
 * This function inserts a new 32-bit floating point value with a given name (key)
 * under the specified parent node.
 * The parent must be a map.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the 32-bit float value is inserted.
 * \param value  The 32-bit float value to insert.
 * \param key    The name (key) of the new 32-bit float value.
 * \return An iterator to the newly inserted 32-bit float value.
 */
static inline vl_msgpack_iter
vlMsgPackSetFloat32Named(vl_msgpack *pack, vl_msgpack_iter parent, vl_float32_t value, const char *key) {
    return vlMsgPackInsert(pack, VL_MSGPACK_FLOAT32, parent, key, strlen(key), &value, sizeof(vl_float32_t));
}

/**
 * \brief Sets an indexed 32-bit floating point value under a parent node.
 *
 * This function inserts a new 32-bit floating point value with an integer index
 * under the specified parent node.
 * The parent must be an array.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the 32-bit float value is inserted.
 * \param value  The 32-bit float value to insert.
 * \param idx    The index of the new 32-bit float value.
 * \return An iterator to the newly inserted 32-bit float value.
 */
static inline vl_msgpack_iter
vlMsgPackSetFloat32Indexed(vl_msgpack *pack, vl_msgpack_iter parent, vl_float32_t value, vl_dsidx_t idx) {
    return vlMsgPackInsert(pack, VL_MSGPACK_FLOAT32, parent, &idx, sizeof(vl_dsidx_t), &value, sizeof(vl_float32_t));
}

/**
 * \brief Sets a named 64-bit floating point value under a parent node.
 *
 * This function inserts a new 64-bit floating point value with a given name (key)
 * under the specified parent node.
 * The parent must be a map.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the 64-bit float value is inserted.
 * \param value  The 64-bit float value to insert.
 * \param key    The name (key) of the new 64-bit float value.
 * \return An iterator to the newly inserted 64-bit float value.
 */
static inline vl_msgpack_iter
vlMsgPackSetFloat64Named(vl_msgpack *pack, vl_msgpack_iter parent, vl_float64_t value, const char *key) {
    return vlMsgPackInsert(pack, VL_MSGPACK_FLOAT64, parent, key, strlen(key), &value, sizeof(vl_float64_t));
}

/**
 * \brief Sets an indexed 64-bit floating point value under a parent node.
 *
 * This function inserts a new 64-bit floating point value with an integer index
 * under the specified parent node.
 * The parent must be an array.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the 64-bit float value is inserted.
 * \param value  The 64-bit float value to insert.
 * \param idx    The index of the new 64-bit float value.
 * \return An iterator to the newly inserted 64-bit float value.
 */
static inline vl_msgpack_iter
vlMsgPackSetFloat64Indexed(vl_msgpack *pack, vl_msgpack_iter parent, vl_float64_t value, vl_dsidx_t idx) {
    return vlMsgPackInsert(pack, VL_MSGPACK_FLOAT64, parent, &idx, sizeof(vl_dsidx_t), &value, sizeof(vl_float64_t));
}

/**
 * \brief Sets a named string value under a parent node.
 *
 * This function inserts a new string value with a given name (key) under the
 * specified parent node.
 * The parent must be a map.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the string value is inserted.
 * \param value  The string value to insert.
 * \param key    The name (key) of the new string value.
 * \return An iterator to the newly inserted string value.
 */
static inline vl_msgpack_iter
vlMsgPackSetStringNamed(vl_msgpack *pack, vl_msgpack_iter parent, const char *value, const char *key) {
    return vlMsgPackInsert(pack, VL_MSGPACK_STRING, parent, key, strlen(key), value, strlen(value));
}

/**
 * \brief Sets an indexed string value under a parent node.
 *
 * This function inserts a new string value with an integer index under the
 * specified parent node.
 * The parent must be an array.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the string value is inserted.
 * \param value  The string value to insert.
 * \param idx    The index of the new string value.
 * \return An iterator to the newly inserted string value.
 */
static inline vl_msgpack_iter
vlMsgPackSetStringIndexed(vl_msgpack *pack, vl_msgpack_iter parent, const char *value, vl_dsidx_t idx) {
    return vlMsgPackInsert(pack, VL_MSGPACK_STRING, parent, &idx, sizeof(vl_dsidx_t), value, strlen(value));
}

/**
 * \brief Sets a named binary value under a parent node.
 *
 * This function inserts a new binary value with a given name (key) under the
 * specified parent node.
 * The parent must be a map.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the binary value is inserted.
 * \param value  The binary value to insert.
 * \param valLen The length of the binary value.
 * \param key    The name (key) of the new binary value.
 * \return An iterator to the newly inserted binary value.
 */
static inline vl_msgpack_iter
vlMsgPackSetBinaryNamed(vl_msgpack *pack, vl_msgpack_iter parent, const void *value, vl_memsize_t valLen,
                        const char *key) {
    return vlMsgPackInsert(pack, VL_MSGPACK_BINARY, parent, key, strlen(key), value, valLen);
}

/**
 * \brief Sets an indexed binary value under a parent node.
 *
 * This function inserts a new binary value with an integer index under the
 * specified parent node.
 * The parent must be an array.
 *
 * \param pack   The MessagePack DOM instance.
 * \param parent The parent node where the binary value is inserted.
 * \param value  The binary value to insert.
 * \param valLen The length of the binary value.
 * \param idx    The index of the new binary value.
 * \return An iterator to the newly inserted binary value.
 */
static inline vl_msgpack_iter
vlMsgPackSetBinaryIndexed(vl_msgpack *pack, vl_msgpack_iter parent, const void *value, vl_memsize_t valLen,
                          vl_dsidx_t idx) {
    return vlMsgPackInsert(pack, VL_MSGPACK_BINARY, parent, &idx, sizeof(vl_dsidx_t), value, valLen);
}

/**
 * \brief Gets the boolean value from a MessagePack element.
 *
 * This function retrieves the boolean value from a given element iterator. If
 * the iterator is invalid or the element type does not match, a default value
 * is returned.
 *
 * \param pack          The MessagePack DOM instance.
 * \param iter          The iterator to the element.
 * \param defaultValue  The default value to return if the iterator is invalid or type mismatch.
 * \return The boolean value stored in the element or the default value.
 */
static inline vl_bool_t vlMsgPackGetBool(vl_msgpack *pack, vl_msgpack_iter iter, vl_bool_t defaultValue) {
    if (iter == VL_MSGPACK_ITER_INVALID)
        return defaultValue;
    if ((vlMsgPackType(pack, iter) != VL_MSGPACK_BOOL))
        return defaultValue;

    return *((const vl_bool_t *) vlMsgPackSampleValue(pack, iter, NULL));
}

/**
 * \brief Gets the signed integer value from a MessagePack element.
 *
 * This function retrieves the signed integer value from a given element iterator.
 * If the iterator is invalid or the element type does not match, a default value
 * is returned.
 *
 * \param pack          The MessagePack DOM instance.
 * \param iter          The iterator to the element.
 * \param defaultValue  The default value to return if the iterator is invalid or type mismatch.
 * \return The signed integer value stored in the element or the default value.
 */
static inline vl_ilarge_t vlMsgPackGetInt(vl_msgpack *pack, vl_msgpack_iter iter, vl_ilarge_t defaultValue) {
    if (iter == VL_MSGPACK_ITER_INVALID)
        return defaultValue;
    if ((vlMsgPackType(pack, iter) != VL_MSGPACK_INT))
        return defaultValue;

    return *((const vl_ilarge_t *) vlMsgPackSampleValue(pack, iter, NULL));
}

/**
 * \brief Gets the unsigned integer value from a MessagePack element.
 *
 * This function retrieves the unsigned integer value from a given element iterator.
 * If the iterator is invalid or the element type does not match, a default value
 * is returned.
 *
 * \param pack          The MessagePack DOM instance.
 * \param iter          The iterator to the element.
 * \param defaultValue  The default value to return if the iterator is invalid or type mismatch.
 * \return The unsigned integer value stored in the element or the default value.
 */
static inline vl_ilarge_t vlMsgPackGetUInt(vl_msgpack *pack, vl_msgpack_iter iter, vl_ularge_t defaultValue) {
    if (iter == VL_MSGPACK_ITER_INVALID)
        return defaultValue;
    if ((vlMsgPackType(pack, iter) != VL_MSGPACK_UINT))
        return defaultValue;

    return *((const vl_ularge_t *) vlMsgPackSampleValue(pack, iter, NULL));
}

/**
 * \brief Gets the 32-bit floating point value from a MessagePack element.
 *
 * This function retrieves the 32-bit floating point value from a given element iterator.
 * If the iterator is invalid or the element type does not match, a default value
 * is returned.
 *
 * \param pack          The MessagePack DOM instance.
 * \param iter          The iterator to the element.
 * \param defaultValue  The default value to return if the iterator is invalid or type mismatch.
 * \return The 32-bit floating point value stored in the element or the default value.
 */
static inline vl_float32_t vlMsgPackGetFloat32(vl_msgpack *pack, vl_msgpack_iter iter, vl_float32_t defaultValue) {
    if (iter == VL_MSGPACK_ITER_INVALID)
        return defaultValue;
    if (vlMsgPackType(pack, iter) != VL_MSGPACK_FLOAT32)
        return defaultValue;

    return *((const vl_float32_t *) vlMsgPackSampleValue(pack, iter, NULL));
}

/**
 * \brief Gets the 64-bit floating point value from a MessagePack element.
 *
 * This function retrieves the 64-bit floating point value from a given element iterator.
 * If the iterator is invalid or the element type does not match, a default value
 * is returned.
 *
 * \param pack          The MessagePack DOM instance.
 * \param iter          The iterator to the element.
 * \param defaultValue  The default value to return if the iterator is invalid or type mismatch.
 * \return The 64-bit floating point value stored in the element or the default value.
 */
static inline vl_float64_t vlMsgPackGetFloat64(vl_msgpack *pack, vl_msgpack_iter iter, vl_float64_t defaultValue) {
    if (iter == VL_MSGPACK_ITER_INVALID)
        return defaultValue;
    if (vlMsgPackType(pack, iter) != VL_MSGPACK_FLOAT64)
        return defaultValue;

    return *((const vl_float64_t *) vlMsgPackSampleValue(pack, iter, NULL));
}

#endif //VL_MSGPACK_H
