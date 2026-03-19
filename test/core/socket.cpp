#include <gtest/gtest.h>

extern "C" {
#include "linked/socket.h"
}

TEST(socket, CreateAndDelete) {
    EXPECT_TRUE(vlTestSocketCreateAndDelete());
}

TEST(socket, AddressIPv4) {
    EXPECT_TRUE(vlTestSocketAddressIPv4());
}

TEST(socket, Options) {
    EXPECT_TRUE(vlTestSocketOptions());
}

TEST(socket, AddressConversion) {
    EXPECT_TRUE(vlTestSocketAddressConversion());
}

TEST(socket, InvalidArguments) {
    EXPECT_TRUE(vlTestSocketInvalidArguments());
}

TEST(socket, LoopbackTCP) {
    EXPECT_TRUE(vlTestSocketLoopbackTCP());
}
