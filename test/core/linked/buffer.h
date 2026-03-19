#ifndef VL_BUFFER_TEST_H
#define VL_BUFFER_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

vl_bool_t vlTestBufferClear(void);
vl_bool_t vlTestBufferShrinkToFit(void);
vl_bool_t vlTestBufferClone(void);
vl_bool_t vlTestBufferCopy(void);

#ifdef __cplusplus
}
#endif

#endif //VL_BUFFER_TEST_H
