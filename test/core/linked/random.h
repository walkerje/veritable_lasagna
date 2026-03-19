#ifndef VL_RANDOM_TEST_H
#define VL_RANDOM_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_memory.h>
#include <vl/vl_numtypes.h>

vl_bool_t vlTestRandomVec4f(void);
vl_bool_t vlTestRandomFill(vl_memsize_t size);

#ifdef __cplusplus
}
#endif

#endif //VL_RANDOM_TEST_H
