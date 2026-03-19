#include <gtest/gtest.h>

extern "C" {
#include "linked/msgpack.h"
}

TEST(msgpack, round_trip) {
    EXPECT_TRUE(vlTestMsgPackRoundTrip());
}

TEST(msgpack, partial_encode) {
    EXPECT_TRUE(vlTestMsgPackPartialEncode());
}

TEST(msgpack, negative_integers) {
    EXPECT_TRUE(vlTestMsgPackNegativeIntegers());
}

TEST(msgpack, positive_integers) {
    EXPECT_TRUE(vlTestMsgPackPositiveIntegers());
}

TEST(msgpack, string_boundaries) {
    EXPECT_TRUE(vlTestMsgPackStringBoundaries());
}

TEST(msgpack, float_precision) {
    EXPECT_TRUE(vlTestMsgPackFloatPrecision());
}

TEST(msgpack, binary_and_ext) {
    EXPECT_TRUE(vlTestMsgPackBinaryAndExt());
}

TEST(msgpack, encoder_errors) {
    EXPECT_TRUE(vlTestMsgPackEncoderErrors());
}

TEST(msgpack, decoder_eof) {
    EXPECT_TRUE(vlTestMsgPackDecoderEOF());
}

TEST(msgpack, empty_containers) {
    EXPECT_TRUE(vlTestMsgPackEmptyContainers());
}

TEST(msgpack, all_types) {
    EXPECT_TRUE(vlTestMsgPackAllTypes());
}