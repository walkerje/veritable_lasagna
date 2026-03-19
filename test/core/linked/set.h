#ifndef VL_SET_TEST_H
#define VL_SET_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

vl_bool_t vlTestSetGrowth(void);
vl_bool_t vlTestSetOrder(void);
vl_bool_t vlTestSetIterate(vl_bool_t reverse);

#ifdef __cplusplus
}
#endif

#endif //VL_SET_TEST_H
