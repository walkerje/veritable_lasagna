#ifndef VL_QUEUE_TEST_H
#define VL_QUEUE_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

vl_bool_t vlTestQueueGrowth(void);
vl_bool_t vlTestQueueFIFO(void);
vl_bool_t vlTestQueueClone(void);

#ifdef __cplusplus
}
#endif

#endif //VL_QUEUE_TEST_H
