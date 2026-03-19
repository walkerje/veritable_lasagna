#ifndef VL_MEMORY_TEST_H
#define VL_MEMORY_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

vl_bool_t vlTestMemReverse(void);
vl_bool_t vlTestMemAlign(vl_int_t alignment);
vl_bool_t vlTestMemSort(vl_int_t numArrayLen);

#ifdef __cplusplus
}
#endif

#endif //VL_MEMORY_TEST_H
