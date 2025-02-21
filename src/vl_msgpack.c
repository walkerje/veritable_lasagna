#include "vl_msgpack.h"

typedef struct vl_msgpack_element_{
    //Whichever one is used depends on the type of the parent node.
    //In the case of the root node, both are unused.
    union{
        vl_dsoffs_t     ordinal;
        vl_dsoffs_t     string;
    } key;

//    union{
//          Type-dependent information?
//    };

    vl_msgpack_type type;

    vl_msgpack_iter parent;
    vl_msgpack_iter nextSibling;
    vl_msgpack_iter prevSibling;
} vl_msgpack_element;

//Current working thought is to enforce a hierarchy in the hashtable
//by combining the hash of the parent node.
//MessagePacks are serialized in big-endian, thus all encoded/decoded
//data might require byte-level swapping. Best to leave it be in the DOM though.