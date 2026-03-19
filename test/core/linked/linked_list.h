#ifndef VL_LINKED_LIST_TEST_H
#define VL_LINKED_LIST_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

vl_bool_t vlTestListGrowth(void);
vl_bool_t vlTestListSort(void);
vl_bool_t vlTestListIterate(vl_bool_t reverse);
vl_bool_t vlTestListInlineInsert(void);

#ifdef __cplusplus
}
#endif

#endif //VL_LINKED_LIST_TEST_H
