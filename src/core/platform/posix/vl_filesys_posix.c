#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

struct vl_filesys_iter_
{
    vl_filesys* sys;
    vl_pool_idx iterIdx;

    vl_filesys_path* dirPath;

    DIR* dir;
    struct dirent* entry;

    vl_bool_t recursive;
    struct vl_filesys_iter_* stackPrev;
};

static inline vl_bool_t vl_FSIterImplicitSkip(struct dirent** entryPtr, DIR* dir)
{
    while (VL_TRUE)
    {
        struct dirent* entry = *entryPtr;
        // Skip the implicit "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            *entryPtr = readdir(dir);
            if ((*entryPtr) == NULL)
            {
                return VL_FALSE; // Iterator DOES NOT have remaining entries
            }
        }
        else
            break;
    }

    return VL_TRUE; // Iterator has remaining entries
}

static inline vl_bool_t vl_FSIterPop(vl_filesys_iter* iter)
{
    vl_filesys_iter topIter = *iter;

    if (topIter->stackPrev == NULL)
        return VL_FALSE;
    *iter = topIter->stackPrev;
    // Pop the top path from the string...
    vlFSPathParent((*iter)->dirPath, (*iter)->dirPath);

    topIter->stackPrev = NULL;
    topIter->dirPath = NULL;
    vlFSIterDelete(topIter);

    return VL_TRUE;
}

vl_filesys_result vlFSStatPath(const vl_filesys_path* path, vl_filesys_stat* result)
{
    if (path == NULL)
        return VL_FS_ERROR_PATH_INVALID;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return VL_FS_ERROR_PATH_INVALID;
    if (result == NULL)
        return VL_FS_ERROR_IO;
    if (path->sys != result->sys)
        return VL_FS_ERROR_IO;

    vl_filesys* sys = path->sys;

    struct stat statValue;
    const char* pathString = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);

    if (stat(pathString, &statValue) != 0)
    {
        switch (errno)
        {
        case EACCES:
            return VL_FS_ERROR_ACCESS_DENIED;
        case EBADF:
            return VL_FS_ERROR_PATH_INVALID;
        case ENOENT:
            return VL_FS_ERROR_NOT_FOUND;
        default:
            return VL_FS_ERROR_IO;
        }
    }

    // If the result path is different from the input path, copy the input path
    // into the result path Otherwise, leave the result path as-is. The input is
    // the same as the output here.
    if (&result->filePath != path)
        vlFSPathClone(path, &result->filePath);

    // Parse out the path components (base name, extension, and full name)
    vl_FSParsePathComponents(sys, result);

    result->isDirectory = S_ISDIR(statValue.st_mode);
    result->isReadOnly = !(statValue.st_mode & S_IWUSR);

    if (result->isDirectory)
    {
        result->fileSize = 0;
    }
    else
    {
        result->fileSize = (vl_memsize_t)(statValue.st_size);
    }

#ifdef __APPLE__
    result->createTime =
        (vl_ularge_t)(statValue.st_birthtimespec.tv_sec * 1000LL + statValue.st_birthtimespec.tv_nsec / 1000000LL);
#else
    // Many POSIX systems don't track creation time, use mtime as fallback
    result->createTime = (vl_ularge_t)(statValue.st_mtime * 1000LL);
#endif
    // Modification time
    result->modifyTime = (vl_ularge_t)(statValue.st_mtime * 1000LL);
    // Access time
    result->accessTime = (vl_ularge_t)(statValue.st_atime * 1000LL);

    return VL_FS_SUCCESS;
}

vl_filesys_result vlFSStatIter(vl_filesys_iter iter, vl_filesys_stat* result)
{
    if (iter == VL_FS_ITER_INVALID || result == NULL)
        return VL_FS_ERROR_IO;
    else if (iter->sys != result->sys)
        return VL_FS_ERROR_IO;
    else if (iter->entry == NULL)
        return VL_FS_ERROR_IO;

    // Clone the directory path to result file path
    vlFSPathClone(iter->dirPath, &result->filePath);

    // Join the directory path with the file name
    vlFSPathJoin(&result->filePath, &result->filePath, iter->entry->d_name);

    return vlFSStatPath(&result->filePath, result);
}

void vlFSPathAbsolute(vl_filesys_path* path)
{
    // Validate input parameters
    if (path == NULL)
        return;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return;

    // Check if path is already absolute
    if (vlFSPathIsAbsolute(path))
        return;

    vl_filesys* sys = path->sys;

    // Get the current working directory
    char* currentDir = NULL;
    vl_memsize_t cwdSize = VL_KB(1);

    while (currentDir == NULL)
    {
        currentDir = malloc(cwdSize);
        if (currentDir == NULL)
            return; // Memory allocation failed

        if (getcwd(currentDir, cwdSize) == NULL)
        {
            if (errno == ERANGE)
            {
                // Buffer too small, try larger size
                free(currentDir);
                currentDir = NULL;
                cwdSize *= 2;
                continue;
            }
            else
            {
                // Other error occurred
                free(currentDir);
                return;
            }
        }
    }

    // Create a temporary path for the current directory
    vl_filesys_path tempPath = {0};
    tempPath.sys = sys;
    tempPath.pathIndex = VL_POOL_INVALID_IDX;
    tempPath.pathStringPtr = VL_ARENA_NULL;

    // Set the current directory as the base path
    vlFSPathSet(&tempPath, currentDir);
    free(currentDir); // Done with malloc'd buffer

    // Check if we were able to set the path correctly
    if (tempPath.pathStringPtr == VL_ARENA_NULL)
        return; // Failed to allocate memory for the path

    const char* relativePath = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);
    // Join the current directory with the relative path
    vlFSPathJoin(&tempPath, &tempPath, relativePath);

    // Normalize the resulting path to resolve any ".." and "." components
    vlFSPathNormalize(&tempPath);

    // Replace the original path with the absolute path
    vlArenaMemFree(&sys->memory, path->pathStringPtr);
    path->pathStringPtr = tempPath.pathStringPtr;
}

vl_bool_t vlFSPathIsAbsolute(const vl_filesys_path* path)
{
    // Validate input parameters
    if (path == NULL)
        return VL_FALSE;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return VL_FALSE;

    // Get the path string from the arena
    const char* pathString = (const char*)vlArenaMemSample(&path->sys->memory, path->pathStringPtr);

    // Check if path is empty
    if (pathString == NULL || pathString[0] == '\0')
        return VL_FALSE;

    // Check if path starts with the file separator (indicating absolute path)
    return pathString[0] == VL_NATIVE_PATH_SEPARATOR_CHAR;
}

/**
 * Recursively creates all directories in a path
 *
 * @param path Path to create
 * @param mode Permission mode for directories
 * @return VL_FS_SUCCESS if successful, error code otherwise
 */
static vl_filesys_result vl_FSPathMkDirRecursive(const char* path, mode_t mode)
{
    if (!path || *path == '\0')
        return VL_FS_ERROR_PATH_INVALID;

    // Make a copy of the path that we can modify
    char* path_copy = strdup(path);
    if (!path_copy)
        return VL_FS_ERROR_IO;

    // Get the length of the path
    const size_t len = strlen(path_copy);

    // If the path ends with a separator, temporarily remove it
    int removedSlash = 0;
    if (len > 1 && path_copy[len - 1] == '/')
    {
        path_copy[len - 1] = '\0';
        removedSlash = 1;
    }

    // Check if the whole path already exists
    struct stat st;
    if (stat(path_copy, &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            free(path_copy);
            return VL_FS_SUCCESS; // Path already exists as a directory
        }
        else
        {
            free(path_copy);
            return VL_FS_ERROR_PATH_INVALID; // Path exists but is not a directory
        }
    }

    // Find the last separator in the path
    char* last_sep = strrchr(path_copy, '/');
    if (last_sep != NULL)
    {
        // Temporarily terminate the string at the separator
        *last_sep = '\0';

        // If there's a parent directory, create it first (recursively)
        if (last_sep != path_copy)
        { // Skip if it would create an empty path
            vl_filesys_result result = vl_FSPathMkDirRecursive(path_copy, mode);
            if (result != VL_FS_SUCCESS)
            {
                free(path_copy);
                return result;
            }
        }

        // Restore the separator
        *last_sep = '/';
    }

    // Restore the trailing slash if we removed one
    if (removedSlash)
    {
        path_copy[len - 1] = '/';
    }

    // Create the directory
    if (mkdir(path_copy, mode) != 0 && errno != EEXIST)
    {
        vl_filesys_result result;
        switch (errno)
        {
        case EACCES:
        case EPERM:
            result = VL_FS_ERROR_ACCESS_DENIED;
            break;
        case ENOTDIR:
            result = VL_FS_ERROR_PATH_INVALID;
            break;
        default:
            result = VL_FS_ERROR_IO;
        }
        free(path_copy);
        return result;
    }

    free(path_copy);
    return VL_FS_SUCCESS;
}

vl_filesys_result vlFSPathMkDir(const vl_filesys_path* path)
{
    // Validate input parameters
    if (path == NULL)
        return VL_FS_ERROR_PATH_INVALID;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return VL_FS_ERROR_PATH_INVALID;

    // Get the path string from the arena
    const char* pathString = (const char*)vlArenaMemSample(&path->sys->memory, path->pathStringPtr);

    // Check if path is empty
    if (pathString == NULL || pathString[0] == '\0')
        return VL_FS_ERROR_PATH_INVALID;

    // Try simple directory creation first
    if (mkdir(pathString, 0755) == 0)
    {
        return VL_FS_SUCCESS;
    }

    // If the directory already exists, consider it a success
    if (errno == EEXIST)
    {
        struct stat st;
        if (stat(pathString, &st) == 0 && S_ISDIR(st.st_mode))
        {
            return VL_FS_SUCCESS;
        }
        else
        {
            return VL_FS_ERROR_PATH_INVALID; // Path exists but is not a directory
        }
    }

    // If the parent directory doesn't exist, try recursive creation
    if (errno == ENOENT)
    {
        return vl_FSPathMkDirRecursive(pathString, 0755);
    }

    // Otherwise, handle other errors
    switch (errno)
    {
    case EACCES:
    case EPERM:
        return VL_FS_ERROR_ACCESS_DENIED;
    case ENOTDIR:
        return VL_FS_ERROR_PATH_INVALID;
    default:
        return VL_FS_ERROR_IO;
    }
}

vl_filesys_result vlFSPathRemove(const vl_filesys_path* path)
{
    if (path == NULL)
        return VL_FS_ERROR_PATH_INVALID;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return VL_FS_ERROR_PATH_INVALID;

    const char* pathString = (const char*)vlArenaMemSample(&path->sys->memory, path->pathStringPtr);
    const int status = remove(pathString);

    if (status != 0)
    {
        switch (errno)
        {
        case EACCES:
        case EPERM:
        case EROFS:
            return VL_FS_ERROR_ACCESS_DENIED;
        case EBUSY:
            return VL_FS_ERROR_IO; // File or directory busy
        case ENOENT:
            return VL_FS_ERROR_NOT_FOUND;
        case ENOTDIR:
            return VL_FS_ERROR_PATH_INVALID;
        default:
            return VL_FS_ERROR_IO;
        }
    }

    return VL_FS_SUCCESS;
}

vl_bool_t vlFSPathExists(const vl_filesys_path* path)
{
    if (path == NULL)
        return VL_FALSE;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return VL_FALSE;

    struct stat statValue;
    const char* pathString = (const char*)vlArenaMemSample(&path->sys->memory, path->pathStringPtr);
    return (stat(pathString, &statValue) == 0);
}

vl_filesys_iter vlFSIterNew(vl_filesys* sys)
{
    vl_pool_idx iterIdx = vlPoolTake(&sys->iterPool);
    vl_filesys_iter iter = vlPoolSample(&sys->iterPool, iterIdx);

    iter->entry = NULL;
    iter->dir = NULL;
    iter->sys = sys;
    iter->iterIdx = iterIdx;
    iter->dirPath = vlFSPathNew(sys, NULL);

    iter->recursive = VL_FALSE;
    iter->stackPrev = NULL;

    return iter;
}

void vlFSIterDelete(vl_filesys_iter iter)
{
    if (iter == VL_FS_ITER_INVALID)
        return;

    if (iter->dir != NULL)
    {
        closedir(iter->dir);
        iter->dir = NULL;
    }

    if (iter->dirPath != NULL)
    {
        vlFSPathDelete(iter->dirPath);
        iter->dirPath = NULL;
    }

    if (iter->stackPrev != NULL)
        vlFSIterDelete(iter->stackPrev);

    vlPoolReturn(&iter->sys->iterPool, iter->iterIdx);
}

vl_filesys_result vl_FSIterInit(vl_filesys_iter* iter, const vl_filesys_path* path)
{
    if (path == NULL)
        return VL_FS_ERROR_PATH_INVALID;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return VL_FS_ERROR_PATH_INVALID;
    if ((*iter)->sys != path->sys)
        return VL_FS_ERROR_IO;

    { // Delete all existing iterators down the stack
        vl_filesys_iter topIter = *iter;

        while (VL_TRUE)
        {
            if (topIter->dir != NULL)
            {
                closedir(topIter->dir);
            }

            topIter->dir = NULL;
            topIter->entry = NULL;

            vl_filesys_iter oldIter = topIter;

            if (topIter->stackPrev == NULL)
                break;

            topIter = topIter->stackPrev;

            oldIter->stackPrev = NULL;
            oldIter->dirPath = NULL;
            vlFSIterDelete(oldIter);
        }

        *iter = topIter;
    }

    const char* pathString = (const char*)vlArenaMemSample(&path->sys->memory, path->pathStringPtr);

    // Handle error codes from opendir more specifically
    DIR* dir = opendir(pathString);
    if (dir == NULL)
    {
        switch (errno)
        {
        case EACCES:
            return VL_FS_ERROR_ACCESS_DENIED;
        case ENOENT:
            return VL_FS_ERROR_NOT_FOUND;
        case ENOTDIR:
            return VL_FS_ERROR_PATH_INVALID;
        default:
            return VL_FS_ERROR_IO;
        }
    }

    errno = 0;
    struct dirent* entry = readdir(dir);

    if (entry == NULL)
    {
        closedir(dir);
        return (errno != 0) ? VL_FS_ERROR_IO : VL_FS_ERROR_NOT_FOUND;
    }

    if (!vl_FSIterImplicitSkip(&entry, dir))
    {
        closedir(dir);
        return VL_FS_ERROR_NOT_FOUND;
    }

    (*iter)->dir = dir;
    (*iter)->entry = entry;

    return VL_FS_SUCCESS;
}

vl_filesys_result vlFSIterDir(vl_filesys_iter* iter, const vl_filesys_path* path)
{
    { // Initialize the iterator
        const vl_filesys_result initStatus = vl_FSIterInit(iter, path);
        if (initStatus != VL_FS_SUCCESS)
        {
            (*iter)->dir = NULL;
            (*iter)->entry = NULL;
            return initStatus;
        }
    }

    (*iter)->recursive = VL_FALSE;
    (*iter)->stackPrev = NULL;
    vlFSPathClone(path, (*iter)->dirPath);
    return VL_FS_SUCCESS;
}

vl_filesys_result vlFSIterDirRecursive(vl_filesys_iter* iter, const vl_filesys_path* path)
{
    { // Initialize the iterator
        const vl_filesys_result initStatus = vl_FSIterInit(iter, path);
        if (initStatus != VL_FS_SUCCESS)
        {
            (*iter)->dir = NULL;
            (*iter)->entry = NULL;
            return initStatus;
        }
    }
    (*iter)->recursive = VL_TRUE;
    (*iter)->stackPrev = NULL;
    vlFSPathClone(path, (*iter)->dirPath);
    return VL_FS_SUCCESS;
}

// Expected behavior is recursive preorder traversal
vl_bool_t vlFSIterNext(vl_filesys_iter* iter)
{
    if (iter == NULL)
        return VL_FALSE;
    if (*iter == VL_FS_ITER_INVALID)
        return VL_FALSE;

    if ((*iter)->dir == NULL || (*iter)->entry == NULL)
        return VL_FALSE;

    // Check if entry is a directory using a more reliable method
    vl_bool_t isDirectory = VL_FALSE;

    // Some filesystems don't populate d_type, we need to use stat in those cases
    if ((*iter)->entry->d_type == DT_DIR)
    {
        isDirectory = VL_TRUE;
    }
    else if ((*iter)->entry->d_type == DT_UNKNOWN)
    {
        // Need to stat the file to determine if it's a directory
        struct stat statbuf;
        vl_filesys_path tempPath = {0};
        tempPath.sys = (*iter)->sys;
        tempPath.pathIndex = VL_POOL_INVALID_IDX;
        tempPath.pathStringPtr = VL_ARENA_NULL;

        vlFSPathClone((*iter)->dirPath, &tempPath);
        vlFSPathJoin(&tempPath, &tempPath, (*iter)->entry->d_name);

        const char* fullPath = (const char*)vlFSPathString(&tempPath);
        if (stat(fullPath, &statbuf) == 0)
        {
            isDirectory = S_ISDIR(statbuf.st_mode);
        }

        if (tempPath.pathStringPtr != VL_ARENA_NULL)
        {
            vlArenaMemFree(&(*iter)->sys->memory, tempPath.pathStringPtr);
            tempPath.pathStringPtr = VL_ARENA_NULL;
        }
    }

    const vl_bool_t isRecursive = (*iter)->recursive;

    if (isDirectory && isRecursive)
    {
        vl_filesys_iter childIter = vlFSIterNew((*iter)->sys);
        childIter->dirPath = (*iter)->dirPath;
        vlFSPathJoin(childIter->dirPath, childIter->dirPath, (*iter)->entry->d_name);

        // Check if path exists after join
        const char* childPath = (const char*)vlFSPathString(childIter->dirPath);
        if (childPath == NULL)
        {
            childIter->stackPrev = NULL;
            childIter->dirPath = NULL;
            vlFSIterDelete(childIter);
            return VL_FALSE;
        }

        DIR* childDir = opendir(childPath);

        if (childDir != NULL)
        {
            struct dirent* childEntry = readdir(childDir);
            if (childEntry != NULL && vl_FSIterImplicitSkip(&childEntry, childDir))
            {
                // Successfully opened child directory
                childIter->dir = childDir;
                childIter->entry = childEntry;
                childIter->recursive = VL_TRUE;
                childIter->stackPrev = *iter;

                *iter = childIter;

                return VL_TRUE;
            }
            else
            {
                // Directory is empty or failed to skip implicit entries
                closedir(childDir);
                vlFSPathParent(childIter->dirPath, childIter->dirPath);

                childIter->dir = NULL;
                childIter->dirPath = NULL;
                childIter->stackPrev = NULL;
                vlFSIterDelete(childIter);
            }
        }
        else
        {
            // Failed to open directory
            childIter->stackPrev = NULL;
            childIter->dirPath = NULL;
            vlFSIterDelete(childIter);
        }
    }

    // Move to next entry in current directory
    (*iter)->entry = readdir((*iter)->dir);
    if ((*iter)->entry == NULL)
    {
        // End of current directory
        closedir((*iter)->dir);
        (*iter)->dir = NULL;

        while (vl_FSIterPop(iter))
        {
            (*iter)->entry = readdir((*iter)->dir);
            if ((*iter)->entry != NULL)
            {
                return VL_TRUE;
            }
        }

        return VL_FALSE;
    }

    return VL_TRUE;
}
