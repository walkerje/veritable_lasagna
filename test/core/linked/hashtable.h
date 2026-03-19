#ifndef VL_HASHTABLE_TEST_H
#define VL_HASHTABLE_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_memory.h>
#include <vl/vl_numtypes.h>

vl_bool_t vlTestHashTableCollision(void);
vl_bool_t vlTestHashTableRemove(void);
vl_bool_t vlTestHashTableInsert(vl_uint32_t set_size, vl_bool_t reserved);
vl_bool_t vlTestHashTableIterate(vl_int_t set_size, vl_int_t rounds, vl_bool_t reserved);
vl_bool_t vlTestHashTableRealWorld(void);

#ifdef __cplusplus
}
#endif

#endif //VL_HASHTABLE_TEST_H
