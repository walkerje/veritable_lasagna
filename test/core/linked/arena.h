#ifndef VL_ARENA_TEST_H
#define VL_ARENA_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

vl_bool_t vlTestArenaGrowth(void);
vl_bool_t vlTestArenaCoalesce(void);

#ifdef __cplusplus
}
#endif

#endif //VL_ARENA_TEST_H
