#ifndef VL_POOL_TEST_H
#define VL_POOL_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_memory.h>
#include <vl/vl_numtypes.h>

vl_bool_t vlTestPoolClear(void);
vl_bool_t vlTestPoolClone(void);
vl_bool_t vlTestPoolElemReturn(void);
vl_bool_t vlTestPoolReserve(void);
vl_bool_t vlTestPoolAlign(void);

#ifdef __cplusplus
}
#endif

#endif //VL_POOL_TEST_H
