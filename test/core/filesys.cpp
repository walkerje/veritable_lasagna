#include <gtest/gtest.h>

extern "C" {
#include "linked/filesys.h"
}

TEST(filesys, directory_iteration) {
    EXPECT_TRUE(vlTestFilesysDirectoryIteration());
}

TEST(filesys, path_manipulation) {
    EXPECT_TRUE(vlTestFilesysPathManipulation());
}