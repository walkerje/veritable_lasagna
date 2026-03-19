#include "filesys.h"
#include <vl/vl_filesys.h>
#include <stdio.h>
#include <string.h>

vl_bool_t vlTestFilesysDirectoryIteration() {
    vl_filesys *fs = vlFSNew();

    // Create a test directory with some files
    vl_filesys_path *testDir = vlFSPathNew(fs, "./iteration_test");
    vlFSPathMkDir(testDir);

    // Create some test files (this would typically involve actual file creation)
    // For this example, we'll just iterate over an existing directory
    vl_filesys_path *currentDir = vlFSPathNew(fs, ".");

    vl_filesys_iter iter = vlFSIterNew(fs);
    vl_filesys_stat *entryStat = vlFSStatNew(fs);

    vlFSIterDir(&iter, currentDir);
    int fileCount = 0;

    if (iter != VL_FS_ITER_INVALID) {
        do{
            if (vlFSStatIter(iter, entryStat) == VL_FS_SUCCESS) {
                // const vl_transient *entryPath = vlFSPathString(&entryStat->filePath);
                fileCount++;
            }

        }while(vlFSIterNext(&iter));

        vlFSIterDelete(iter);
    }
    vlFSStatDelete(entryStat);

    // Cleanup
    vlFSPathRemove(testDir);
    vlFSPathDelete(testDir);
    vlFSPathDelete(currentDir);
    vlFSDelete(fs);

    return (vl_bool_t)(fileCount > 0);
}

vl_bool_t vlTestFilesysPathManipulation() {
    vl_filesys *fs = vlFSNew();

    // Test path creation and manipulation
    vl_filesys_path *basePath = vlFSPathNew(fs, "./base/directory");
    vl_filesys_path *joinedPath = vlFSPathNew(fs, "");
    vl_filesys_path *normalizedPath = vlFSPathNew(fs, "./base/../base/./directory");

    // Test path joining
    vlFSPathJoin(basePath, joinedPath, "subdirectory/file.txt");
    // const vl_transient *joinedStr = vlFSPathString(joinedPath);

    // Test path normalization
    vlFSPathNormalize(normalizedPath);
    // const vl_transient *normalizedStr = vlFSPathString(normalizedPath);

    // Test path comparison
    vl_bool_t pathsEqual = vlFSPathEquals(basePath, normalizedPath);

    // Test absolute path checking
    // vl_bool_t isAbsolute = vlFSPathIsAbsolute(basePath);

    // Cleanup
    vlFSPathDelete(basePath);
    vlFSPathDelete(joinedPath);
    vlFSPathDelete(normalizedPath);
    vlFSDelete(fs);

    return pathsEqual;
}
