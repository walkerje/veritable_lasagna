#include "vl_hash.h"
#include "vl_memory.h"

vl_hash vlHashString(const void* data, vl_memsize_t dataSize){
    //FNV1a-64
#ifdef VL_I64_T
    vl_hash hashCode = 0xcbf29ce484222325L;

    for(vl_ularge_t i = 0; i < dataSize; ++i){
        hashCode ^= *((const vl_int8_t*)data + i);
        hashCode *= 0x100000001b3L;
    }

    return hashCode;
#else
    vl_hash hashCode = 0x811c9dc5u;

    for(vl_memsize_t i = 0; i < dataSize; ++i){
        hashCode ^= *((const vl_int8_t*)data + i);
        hashCode *= 0x1000193u;
    }

    return hashCode;
#endif
}

vl_hash vlHash8(const void* data, vl_memsize_t s){
    return *((const vl_uint8_t*)data);
}

vl_hash vlHash16(const void* data, vl_memsize_t s){
    return *((const vl_uint16_t*)data);
}

vl_hash vlHash32(const void* data, vl_memsize_t s){
    return *((const vl_uint32_t*)data);
}

vl_hash vlHash64(const void* data, vl_memsize_t s){
    return *((const vl_uint64_t*)data);
}