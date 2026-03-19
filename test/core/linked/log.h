#ifndef VL_LOG_TEST_H
#define VL_LOG_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <vl/vl_numtypes.h>

  VL_TEST_API vl_bool_t vlTestLogFileWriteAndFlushBlocks(void);
  VL_TEST_API vl_bool_t vlTestLogRotationCreatesRotatedFiles(void);
  VL_TEST_API vl_bool_t vlTestLogConcurrentProducersFlushComplete(void);
  VL_TEST_API vl_bool_t vlTestLogStdoutCaptureViaRedirect(void);
  VL_TEST_API vl_bool_t vlTestLoggerInstanceWritesToStreamSink(void);
  VL_TEST_API vl_bool_t vlTestLoggerMultipleInstancesRemainIndependent(void);
  VL_TEST_API vl_bool_t vlTestGlobalLoggerCanAddStreamSink(void);

  VL_TEST_API vl_bool_t vlTestLogSimple(void);

#ifdef __cplusplus
}
#endif

#endif //VL_LOG_TEST_H