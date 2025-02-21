#ifndef VL_MSGPACK_H
#define VL_MSGPACK_H

#include "vl_hashtable.h"
#include "vl_arena.h"
#include "vl_buffer.h"

#define VL_MSGPACK_ITER_INVALID VL_HASHTABLE_ITER_INVALID

typedef vl_hash_iter vl_msgpack_iter;

typedef enum vl_msgpack_type_{

    VL_MSGPACK_POS_FIXINT   = 0x00,//Leftmost bit is 0. Encoded in remaining 7 bits.
    VL_MSGPACK_FIXMAP       = 0x80,//Leftmost 4 bits are 0b1000.
    VL_MSGPACK_FIXARRAY     = 0x90,//Leftmost 4 bits are 0b1001.
    VL_MSGPACK_FIXSTR       = 0xA0,//Leftmost 3 bits are 0b101.
    VL_MSGPACK_NEG_FIXINT   = 0xE0,//Leftmost 3 bits are 0b111.

    VL_MSGPACK_NIL          = 0xC0,
    VL_MSGPACK_FALSE        = 0xC2,
    VL_MSGPACK_TRUE         = 0xC3,
    VL_MSGPACK_BIN8         = 0xC4,
    VL_MSGPACK_BIN16        = 0xC5,
    VL_MSGPACK_BIN32        = 0xC6,
    VL_MSGPACK_EXT8         = 0xC7,
    VL_MSGPACK_EXT16        = 0xC8,
    VL_MSGPACK_EXT32        = 0xC9,
    VL_MSGPACK_FLOAT32      = 0xCA,
    VL_MSGPACK_FLOAT64      = 0xCB,
    VL_MSGPACK_UINT8        = 0xCC,
    VL_MSGPACK_UINT16       = 0xCD,
    VL_MSGPACK_UINT32       = 0xCE,
    VL_MSGPACK_UINT64       = 0xCF,
    VL_MSGPACK_INT8         = 0xD0,
    VL_MSGPACK_INT16        = 0xD1,
    VL_MSGPACK_INT32        = 0xD2,
    VL_MSGPACK_INT64        = 0xD3,
    VL_MSGPACK_FIXEXT1      = 0xD4,
    VL_MSGPACK_FIXEXT2      = 0xD5,
    VL_MSGPACK_FIXEXT4      = 0xD6,
    VL_MSGPACK_FIXEXT8      = 0xD7,
    VL_MSGPACK_FIXEXT16     = 0xD8,
    VL_MSGPACK_STR8         = 0xD9,
    VL_MSGPACK_STR16        = 0xDA,
    VL_MSGPACK_STR32        = 0xDB,
    VL_MSGPACK_ARRAY16      = 0xDC,
    VL_MSGPACK_ARRAY32      = 0xDD,
    VL_MSGPACK_MAP16        = 0xDE,
    VL_MSGPACK_MAP32        = 0xDF,
} vl_msgpack_type;

typedef struct{
    vl_hashtable        dom;//document object model
    vl_msgpack_iter     root;
} vl_msgpack;

void                vlMsgPackInit(vl_msgpack* pack);
void                vlMsgPackFree(vl_msgpack* pack);

vl_msgpack*         vlMsgPackNew();
void                vlMsgPackDelete(vl_msgpack* pack);

vl_msgpack          vlMsgPackClone(vl_msgpack* src, vl_msgpack* dest);

vl_msgpack_iter     vlMsgPackParent(vl_msgpack* pack, vl_msgpack_iter iter);
vl_msgpack_iter     vlMsgPackNextSibling(vl_msgpack* pack, vl_msgpack_iter iter);
vl_msgpack_iter     vlMsgPackPrevSibling(vl_msgpack* pack, vl_msgpack_iter iter);

vl_msgpack_type     vlMsgPackGetType(vl_msgpack* pack, vl_msgpack_iter iter);

vl_msgpack_iter     vlMsgPackGetNamedChild(vl_msgpack* pack, vl_msgpack_iter iter, const char* str);
vl_msgpack_iter     vlMsgPackGetIndexedChild(vl_msgpack* pack, vl_msgpack_iter iter, vl_dsidx_t idx);

vl_transient*       vlMsgPackSample(vl_msgpack* pack, vl_msgpack_iter iter, vl_memsize_t* size);

#ifndef vlMsgPackRoot
#define vlMsgPackRoot(rootPtr) ((rootPtr)->root)
#endif

//TODO: Use callbacks to allow more flexible IO for message packs.

void        vlMsgPackSerialize(vl_msgpack* srcPack, vl_buffer* destBuffer);
vl_bool_t   vlMsgPackDeserialize(vl_msgpack* destPack, const void* srcBuffer, vl_memsize_t srcLength);

#endif //VL_MSGPACK_H
