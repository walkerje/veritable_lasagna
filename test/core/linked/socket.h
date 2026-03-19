#ifndef VL_TEST_CORE_LINKED_SOCKET_H
#define VL_TEST_CORE_LINKED_SOCKET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

VL_TEST_API vl_bool_t vlTestSocketCreateAndDelete(void);
VL_TEST_API vl_bool_t vlTestSocketAddressIPv4(void);
VL_TEST_API vl_bool_t vlTestSocketOptions(void);
VL_TEST_API vl_bool_t vlTestSocketAddressConversion(void);
VL_TEST_API vl_bool_t vlTestSocketInvalidArguments(void);
VL_TEST_API vl_bool_t vlTestSocketLoopbackTCP(void);

#ifdef __cplusplus
}
#endif

#endif // VL_TEST_CORE_LINKED_SOCKET_H
