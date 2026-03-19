#ifndef VL_ALGO_TEST_H
#define VL_ALGO_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

vl_bool_t vlTestAlgoBitOperations(void);
vl_bool_t vlTestAlgoPowerOfTwo(void);
vl_bool_t vlTestAlgoGCDLCM(void);
vl_bool_t vlTestAlgoOverflowDetection(void);
vl_bool_t vlTestAlgoMacroUtilities(void);
vl_bool_t vlTestAlgoComprehensiveBit(void);

#ifdef __cplusplus
}
#endif

#endif //VL_ALGO_TEST_H
