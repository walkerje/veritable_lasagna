#include <gtest/gtest.h>

extern "C" {
#include "linked/log.h"
}

TEST(log, file_write_and_flush_blocks) {
  EXPECT_TRUE(vlTestLogFileWriteAndFlushBlocks());
}

TEST(log, rotation_creates_rotated_files) {
  EXPECT_TRUE(vlTestLogRotationCreatesRotatedFiles());
}

TEST(log, concurrent_producers_flush_complete) {
  EXPECT_TRUE(vlTestLogConcurrentProducersFlushComplete());
}

TEST(log, stdout_capture_via_redirect) {
  EXPECT_TRUE(vlTestLogStdoutCaptureViaRedirect());
}

TEST(log, logger_instance_writes_to_stream_sink) {
  EXPECT_TRUE(vlTestLoggerInstanceWritesToStreamSink());
}

TEST(log, multiple_logger_instances_remain_independent) {
  EXPECT_TRUE(vlTestLoggerMultipleInstancesRemainIndependent());
}

TEST(log, global_logger_can_add_stream_sink) {
  EXPECT_TRUE(vlTestGlobalLoggerCanAddStreamSink());
}

TEST(log, simple) {
  EXPECT_TRUE(vlTestLogSimple());
}