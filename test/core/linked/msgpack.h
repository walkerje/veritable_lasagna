#ifndef VL_MSGPACK_TEST_H
#define VL_MSGPACK_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

vl_bool_t vlTestMsgPackRoundTrip(void);
vl_bool_t vlTestMsgPackPartialEncode(void);
vl_bool_t vlTestMsgPackNegativeIntegers(void);
vl_bool_t vlTestMsgPackPositiveIntegers(void);
vl_bool_t vlTestMsgPackStringBoundaries(void);
vl_bool_t vlTestMsgPackFloatPrecision(void);
vl_bool_t vlTestMsgPackBinaryAndExt(void);
vl_bool_t vlTestMsgPackEncoderErrors(void);
vl_bool_t vlTestMsgPackDecoderEOF(void);
vl_bool_t vlTestMsgPackEmptyContainers(void);
vl_bool_t vlTestMsgPackAllTypes(void);

#ifdef __cplusplus
}
#endif

#endif //VL_MSGPACK_TEST_H
