#include <shlwapi.h>
#include <windows.h>

/**
 * Converts a UTF-8 string to a UTF-16 (wide) string for Windows API calls.
 *
 * \private
 * \param utf8 The UTF-8 string to convert
 * \param utf16 The output UTF-16 string buffer
 * \param utf16Len The length of the output buffer in wchar_t elements
 * \return Number of wchar_t elements written (excluding null terminator), or 0
 * on failure
 */
static int vl_utf8_to_utf16(const char* utf8, wchar_t* utf16, int utf16Len)
{
    if (!utf8 || !utf16 || utf16Len <= 0)
        return 0;

    return MultiByteToWideChar(CP_UTF8, 0, utf8, -1, utf16, utf16Len) - 1;
}

/**
 * Converts a UTF-16 (wide) string to a UTF-8 string for internal use.
 *
 * \private
 * \param utf16 The UTF-16 string to convert
 * \param utf8 The output UTF-8 string buffer
 * \param utf8Len The length of the output buffer in bytes
 * \return Number of bytes written (excluding null terminator), or 0 on failure
 */
static int vl_utf16_to_utf8(const wchar_t* utf16, char* utf8, int utf8Len)
{
    if (!utf16 || !utf8 || utf8Len <= 0)
        return 0;

    return WideCharToMultiByte(CP_UTF8, 0, utf16, -1, utf8, utf8Len, NULL, NULL) - 1;
}

struct vl_filesys_iter_
{
    vl_filesys* sys;
    vl_pool_idx iterIdx;

    vl_filesys_path* dirPath;

    HANDLE findHandle;
    WIN32_FIND_DATAW findData;
    vl_bool_t hasNextEntry;

    vl_bool_t recursive;
    struct vl_filesys_iter_* stackPrev;
};

/**
 * Pops an iterator from the stack during recursive iteration.
 *
 * \param iter Pointer to the iterator pointer
 * \return VL_TRUE if a parent iterator was popped, VL_FALSE if there's no
 * parent
 */
static inline vl_bool_t vl_FSIterPop(vl_filesys_iter* iter)
{
    vl_filesys_iter topIter = *iter;

    if (topIter->stackPrev == NULL)
        return VL_FALSE;
    *iter = topIter->stackPrev;
    // Pop the top path from the string...
    vlFSPathParent((*iter)->dirPath, (*iter)->dirPath);

    topIter->stackPrev = NULL;

    if (topIter->findHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(topIter->findHandle);
        topIter->findHandle = INVALID_HANDLE_VALUE;
    }

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

    // Get the UTF-8 path string
    const char* utf8Path = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);

    // Convert path to UTF-16 for Windows API
    wchar_t widePath[MAX_PATH];
    if (vl_utf8_to_utf16(utf8Path, widePath, MAX_PATH) == 0)
    {
        return VL_FS_ERROR_PATH_INVALID;
    }

    // Use GetFileAttributesExW for file information
    WIN32_FILE_ATTRIBUTE_DATA fileAttrData;
    if (!GetFileAttributesExW(widePath, GetFileExInfoStandard, &fileAttrData))
    {
        DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_ACCESS_DENIED:
            return VL_FS_ERROR_ACCESS_DENIED;
        case ERROR_INVALID_PARAMETER:
        case ERROR_INVALID_NAME:
            return VL_FS_ERROR_PATH_INVALID;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return VL_FS_ERROR_NOT_FOUND;
        default:
            return VL_FS_ERROR_IO;
        }
    }

    // For file times, get a HANDLE to the file
    HANDLE hFile =
        CreateFileW(widePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    // If the result path is different from the input path, copy the input path
    // into the result path
    if (&result->filePath != path)
        vlFSPathClone(path, &result->filePath);

    // Parse out the path components (base name, extension, and full name)
    vl_FSParsePathComponents(sys, result);

    // Set file properties
    result->isDirectory = (fileAttrData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    result->isReadOnly = (fileAttrData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;

    if (result->isDirectory)
    {
        result->fileSize = 0;
    }
    else
    {
        ULARGE_INTEGER fileSize;
        fileSize.LowPart = fileAttrData.nFileSizeLow;
        fileSize.HighPart = fileAttrData.nFileSizeHigh;
        result->fileSize = (vl_memsize_t)fileSize.QuadPart;
    }

    // Convert Windows FILETIME to milliseconds since epoch
    // Windows FILETIME is 100-nanosecond intervals since January 1, 1601 (UTC)
    // We need to convert to milliseconds since January 1, 1970 (epoch)

    // Creation time
    ULARGE_INTEGER createTime;
    createTime.LowPart = fileAttrData.ftCreationTime.dwLowDateTime;
    createTime.HighPart = fileAttrData.ftCreationTime.dwHighDateTime;
    // Subtract Windows epoch (1601-01-01) to Unix epoch (1970-01-01)
    // which is 116444736000000000 in 100-nanosecond intervals
    createTime.QuadPart -= 116444736000000000;
    // Convert 100-nanosecond intervals to milliseconds
    result->createTime = createTime.QuadPart / 10000;

    // Modification time
    ULARGE_INTEGER modifyTime;
    modifyTime.LowPart = fileAttrData.ftLastWriteTime.dwLowDateTime;
    modifyTime.HighPart = fileAttrData.ftLastWriteTime.dwHighDateTime;
    modifyTime.QuadPart -= 116444736000000000;
    result->modifyTime = modifyTime.QuadPart / 10000;

    // Access time
    ULARGE_INTEGER accessTime;
    accessTime.LowPart = fileAttrData.ftLastAccessTime.dwLowDateTime;
    accessTime.HighPart = fileAttrData.ftLastAccessTime.dwHighDateTime;
    accessTime.QuadPart -= 116444736000000000;
    result->accessTime = accessTime.QuadPart / 10000;

    // Close the file handle if it was opened
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

    return VL_FS_SUCCESS;
}

vl_filesys_result vlFSStatIter(vl_filesys_iter iter, vl_filesys_stat* result)
{
    if (iter == VL_FS_ITER_INVALID || result == NULL)
        return VL_FS_ERROR_IO;
    else if (iter->sys != result->sys)
        return VL_FS_ERROR_IO;
    else if (!iter->hasNextEntry)
        return VL_FS_ERROR_IO;

    vl_filesys* sys = iter->sys;

    // Convert the filename from findData (UTF-16) to UTF-8
    char utf8Filename[MAX_PATH];
    if (vl_utf16_to_utf8(iter->findData.cFileName, utf8Filename, MAX_PATH) == 0)
    {
        return VL_FS_ERROR_IO;
    }

    // Clone the directory path to result file path
    vlFSPathClone(iter->dirPath, &result->filePath);

    // Join the directory path with the file name
    vlFSPathJoin(&result->filePath, &result->filePath, utf8Filename);

    // Set file properties from the WIN32_FIND_DATA
    result->isDirectory = (iter->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    result->isReadOnly = (iter->findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;

    if (result->isDirectory)
    {
        result->fileSize = 0;
    }
    else
    {
        ULARGE_INTEGER fileSize;
        fileSize.LowPart = iter->findData.nFileSizeLow;
        fileSize.HighPart = iter->findData.nFileSizeHigh;
        result->fileSize = (vl_memsize_t)fileSize.QuadPart;
    }

    // Parse out the path components (base name, extension, and full name)
    vl_FSParsePathComponents(sys, result);

    // Convert Windows FILETIME to milliseconds since epoch
    // Creation time
    ULARGE_INTEGER createTime;
    createTime.LowPart = iter->findData.ftCreationTime.dwLowDateTime;
    createTime.HighPart = iter->findData.ftCreationTime.dwHighDateTime;
    createTime.QuadPart -= 116444736000000000;
    result->createTime = createTime.QuadPart / 10000;

    // Modification time
    ULARGE_INTEGER modifyTime;
    modifyTime.LowPart = iter->findData.ftLastWriteTime.dwLowDateTime;
    modifyTime.HighPart = iter->findData.ftLastWriteTime.dwHighDateTime;
    modifyTime.QuadPart -= 116444736000000000;
    result->modifyTime = modifyTime.QuadPart / 10000;

    // Access time
    ULARGE_INTEGER accessTime;
    accessTime.LowPart = iter->findData.ftLastAccessTime.dwLowDateTime;
    accessTime.HighPart = iter->findData.ftLastAccessTime.dwHighDateTime;
    accessTime.QuadPart -= 116444736000000000;
    result->accessTime = accessTime.QuadPart / 10000;

    return VL_FS_SUCCESS;
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

    // Get the UTF-8 path string
    const char* utf8Path = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);

    // Convert path to UTF-16 for Windows API
    wchar_t widePath[MAX_PATH];
    if (vl_utf8_to_utf16(utf8Path, widePath, MAX_PATH) == 0)
    {
        return;
    }

    // Get the full path
    wchar_t fullPathW[MAX_PATH];
    if (GetFullPathNameW(widePath, MAX_PATH, fullPathW, NULL) == 0)
    {
        return;
    }

    // Convert back to UTF-8
    char fullPathUtf8[MAX_PATH];
    if (vl_utf16_to_utf8(fullPathW, fullPathUtf8, MAX_PATH) == 0)
    {
        return;
    }

    // Replace backslashes with forward slashes for cross-platform consistency
    char* p = fullPathUtf8;
    while (*p)
    {
        if (*p == '\\')
            *p = '/';
        p++;
    }

    // Update the path
    vlFSPathSet(path, fullPathUtf8);
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

    // In Windows, an absolute path can be:
    // 1. Starting with a drive letter followed by a colon (e.g., "C:/")
    // 2. Starting with two slashes (e.g., "//server/share")
    // 3. Starting with a single slash (converted from "\" in Windows paths)

    // Check for drive letter (e.g., "C:/")
    if (isalpha(pathString[0]) && pathString[1] == ':' && (pathString[2] == '/' || pathString[2] == '\\'))
    {
        return VL_TRUE;
    }

    // Check for UNC path (e.g., "//server/share")
    if ((pathString[0] == '/' || pathString[0] == '\\') && (pathString[1] == '/' || pathString[1] == '\\'))
    {
        return VL_TRUE;
    }

    return VL_FALSE;
}

vl_filesys_result vlFSPathMkDir(const vl_filesys_path* path)
{
    // Validate input parameters
    if (path == NULL)
        return VL_FS_ERROR_PATH_INVALID;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return VL_FS_ERROR_PATH_INVALID;

    vl_filesys* sys = path->sys;

    // Get the UTF-8 path string
    const char* utf8Path = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);

    // Check if path is empty
    if (utf8Path == NULL || utf8Path[0] == '\0')
        return VL_FS_ERROR_PATH_INVALID;

    // Convert path to absolute path first to ensure SHCreateDirectoryExW works
    // correctly
    vl_filesys_path tempPath;
    tempPath.sys = sys;
    tempPath.pathIndex = VL_POOL_INVALID_IDX;
    tempPath.pathStringPtr = VL_ARENA_NULL;

    vlFSPathClone(path, &tempPath);
    vlFSPathAbsolute(&tempPath);
    vlFSPathNormalize(&tempPath);

    // Get the absolute path string
    const char* absolutePath = (const char*)vlArenaMemSample(&sys->memory, tempPath.pathStringPtr);

    // Convert path to UTF-16 for Windows API
    wchar_t widePath[MAX_PATH];
    if (vl_utf8_to_utf16(absolutePath, widePath, MAX_PATH) == 0)
    {
        vlArenaMemFree(&sys->memory, tempPath.pathStringPtr);
        return VL_FS_ERROR_PATH_INVALID;
    }

    // Convert forward slashes to backslashes for Windows
    //    wchar_t* p = widePath;
    //    while (*p) {
    //        if (*p == L'/') *p = L'\\';
    //        p++;
    //    }

    // Check if directory already exists
    DWORD attrs = GetFileAttributesW(widePath);
    if (attrs != INVALID_FILE_ATTRIBUTES)
    {
        vlArenaMemFree(&sys->memory, tempPath.pathStringPtr);
        if (attrs & FILE_ATTRIBUTE_DIRECTORY)
        {
            return VL_FS_SUCCESS; // Directory already exists
        }
        else
        {
            return VL_FS_ERROR_PATH_INVALID; // Path exists but is not a directory
        }
    }

    // Create the directory with SHCreateDirectoryExW which creates all parent
    // directories
    int result = CreateDirectoryExW(NULL, widePath, NULL);

    // Clean up temporary path
    vlArenaMemFree(&sys->memory, tempPath.pathStringPtr);

    if (result == ERROR_SUCCESS)
    {
        return VL_FS_SUCCESS;
    }

    // Handle errors
    switch (result)
    {
    case ERROR_ACCESS_DENIED:
    case ERROR_SHARING_VIOLATION:
        return VL_FS_ERROR_ACCESS_DENIED;
    case ERROR_PATH_NOT_FOUND:
    case ERROR_INVALID_NAME:
        return VL_FS_ERROR_PATH_INVALID;
    case ERROR_ALREADY_EXISTS:
        // Double-check if it's a directory
        attrs = GetFileAttributesW(widePath);
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            return VL_FS_SUCCESS;
        }
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

    vl_filesys* sys = path->sys;

    // Get the UTF-8 path string
    const char* utf8Path = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);

    // Convert path to UTF-16 for Windows API
    wchar_t widePath[MAX_PATH];
    if (vl_utf8_to_utf16(utf8Path, widePath, MAX_PATH) == 0)
    {
        return VL_FS_ERROR_PATH_INVALID;
    }

    // Check if the path is a directory
    DWORD attrs = GetFileAttributesW(widePath);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
        {
            return VL_FS_ERROR_NOT_FOUND;
        }
        return VL_FS_ERROR_IO;
    }

    BOOL success;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        // Remove directory
        success = RemoveDirectoryW(widePath);
    }
    else
    {
        // Remove file
        success = DeleteFileW(widePath);
    }

    if (!success)
    {
        DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
            return VL_FS_ERROR_ACCESS_DENIED;
        case ERROR_DIR_NOT_EMPTY:
            return VL_FS_ERROR_IO; // Directory not empty
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return VL_FS_ERROR_NOT_FOUND;
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

    vl_filesys* sys = path->sys;

    // Get the UTF-8 path string
    const char* utf8Path = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);

    // Convert path to UTF-16 for Windows API
    wchar_t widePath[MAX_PATH];
    if (vl_utf8_to_utf16(utf8Path, widePath, MAX_PATH) == 0)
    {
        return VL_FALSE;
    }

    DWORD attrs = GetFileAttributesW(widePath);
    return (attrs != INVALID_FILE_ATTRIBUTES);
}

vl_filesys_iter vlFSIterNew(vl_filesys* sys)
{
    vl_pool_idx iterIdx = vlPoolTake(&sys->iterPool);
    vl_filesys_iter iter = vlPoolSample(&sys->iterPool, iterIdx);

    iter->findHandle = INVALID_HANDLE_VALUE;
    iter->hasNextEntry = VL_FALSE;
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

    if (iter->findHandle != INVALID_HANDLE_VALUE)
    {
        FindClose(iter->findHandle);
        iter->findHandle = INVALID_HANDLE_VALUE;
    }

    if (iter->dirPath != NULL)
    {
        vlFSPathDelete(iter->dirPath);
        iter->dirPath = NULL;
    }

    if (iter->stackPrev != NULL)
    {
        vlFSIterDelete(iter->stackPrev);
    }
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

    vl_filesys* sys = path->sys;

    { // Delete all existing iterators down the stack
        vl_filesys_iter topIter = *iter;

        while (VL_TRUE)
        {
            if (topIter->findHandle != INVALID_HANDLE_VALUE)
            {
                FindClose(topIter->findHandle);
            }

            topIter->findHandle = INVALID_HANDLE_VALUE;
            topIter->hasNextEntry = VL_FALSE;

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

    // Get the UTF-8 path string
    const char* utf8Path = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);

    // Convert path to UTF-16 for Windows API
    wchar_t widePath[MAX_PATH];
    if (vl_utf8_to_utf16(utf8Path, widePath, MAX_PATH) == 0)
    {
        return VL_FS_ERROR_PATH_INVALID;
    }

    // Append wildcard for FindFirstFile
    wcscat_s(widePath, MAX_PATH, L"/*");

    // Start the find operation
    (*iter)->findHandle = FindFirstFileW(widePath, &(*iter)->findData);
    if ((*iter)->findHandle == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        switch (error)
        {
        case ERROR_ACCESS_DENIED:
            return VL_FS_ERROR_ACCESS_DENIED;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return VL_FS_ERROR_NOT_FOUND;
        case ERROR_INVALID_NAME:
            return VL_FS_ERROR_PATH_INVALID;
        default:
            return VL_FS_ERROR_IO;
        }
    }

    (*iter)->hasNextEntry = VL_TRUE;

    // Skip "."
    while ((*iter)->hasNextEntry &&
           (wcscmp((*iter)->findData.cFileName, L".") == 0 || wcscmp((*iter)->findData.cFileName, L"..") == 0))
    {
        if (!FindNextFileW((*iter)->findHandle, &(*iter)->findData))
        {
            (*iter)->hasNextEntry = VL_FALSE;
            if (GetLastError() != ERROR_NO_MORE_FILES)
            {
                FindClose((*iter)->findHandle);
                (*iter)->findHandle = INVALID_HANDLE_VALUE;
                return VL_FS_ERROR_IO;
            }
        }
    }

    if (!(*iter)->hasNextEntry)
    {
        FindClose((*iter)->findHandle);
        (*iter)->findHandle = INVALID_HANDLE_VALUE;
        return VL_FS_ERROR_NOT_FOUND;
    }

    return VL_FS_SUCCESS;
}

vl_filesys_result vlFSIterDir(vl_filesys_iter* iter, const vl_filesys_path* path)
{
    { // Initialize the iterator
        const vl_filesys_result initStatus = vl_FSIterInit(iter, path);
        if (initStatus != VL_FS_SUCCESS)
        {
            (*iter)->findHandle = INVALID_HANDLE_VALUE;
            (*iter)->hasNextEntry = VL_FALSE;
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
            (*iter)->findHandle = INVALID_HANDLE_VALUE;
            (*iter)->hasNextEntry = VL_FALSE;
            return initStatus;
        }
    }
    (*iter)->recursive = VL_TRUE;
    (*iter)->stackPrev = NULL;
    vlFSPathClone(path, (*iter)->dirPath);
    return VL_FS_SUCCESS;
}

vl_bool_t vlFSIterNext(vl_filesys_iter* iter)
{
    if (iter == NULL)
        return VL_FALSE;
    if (*iter == VL_FS_ITER_INVALID)
        return VL_FALSE;

    if ((*iter)->findHandle == INVALID_HANDLE_VALUE || !(*iter)->hasNextEntry)
        return VL_FALSE;

    // Check if current entry is a directory
    vl_bool_t isDirectory = ((*iter)->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    const vl_bool_t isRecursive = (*iter)->recursive;

    if (isDirectory && isRecursive)
    {
        vl_filesys_iter childIter = vlFSIterNew((*iter)->sys);
        childIter->dirPath = (*iter)->dirPath;

        // Convert the current filename from UTF-16 to UTF-8
        char utf8Filename[MAX_PATH];
        if (vl_utf16_to_utf8((*iter)->findData.cFileName, utf8Filename, MAX_PATH) > 0)
        {
            // Join the path with the current directory name
            vlFSPathJoin(childIter->dirPath, childIter->dirPath, utf8Filename);

            // Try to initialize this new directory iterator
            vl_filesys_result result = vl_FSIterInit(&childIter, childIter->dirPath);
            if (result == VL_FS_SUCCESS)
            {
                // Successfully initialized child directory iterator
                childIter->recursive = VL_TRUE;
                childIter->stackPrev = *iter;

                *iter = childIter;
                return VL_TRUE;
            }
            else
                vlFSPathParent(childIter->dirPath, childIter->dirPath);
        }

        // If we reach here, failed to recurse into directory
        childIter->stackPrev = NULL;
        childIter->dirPath = NULL;
        vlFSIterDelete(childIter);
    }

    // Move to next entry in current directory
    if (!FindNextFileW((*iter)->findHandle, &(*iter)->findData))
    {
        DWORD error = GetLastError();
        if (error != ERROR_NO_MORE_FILES)
        {
            // Error occurred
            FindClose((*iter)->findHandle);
            (*iter)->findHandle = INVALID_HANDLE_VALUE;
            (*iter)->hasNextEntry = VL_FALSE;
            return VL_FALSE;
        }

        // End of directory
        FindClose((*iter)->findHandle);
        (*iter)->findHandle = INVALID_HANDLE_VALUE;
        (*iter)->hasNextEntry = VL_FALSE;

        // Try to pop up to parent directory
        while (vl_FSIterPop(iter))
        {
            // Move to next entry in the parent directory
            if (!FindNextFileW((*iter)->findHandle, &(*iter)->findData))
            {
                DWORD parentError = GetLastError();
                if (parentError != ERROR_NO_MORE_FILES)
                {
                    // Error occurred
                    FindClose((*iter)->findHandle);
                    (*iter)->findHandle = INVALID_HANDLE_VALUE;
                    (*iter)->hasNextEntry = VL_FALSE;
                    return VL_FALSE;
                }
                // End of this directory too, continue popping
                FindClose((*iter)->findHandle);
                (*iter)->findHandle = INVALID_HANDLE_VALUE;
                (*iter)->hasNextEntry = VL_FALSE;
                continue;
            }

            // Skip "." and ".." entries
            while (wcscmp((*iter)->findData.cFileName, L".") == 0 || wcscmp((*iter)->findData.cFileName, L"..") == 0)
            {
                if (!FindNextFileW((*iter)->findHandle, &(*iter)->findData))
                {
                    DWORD skipError = GetLastError();
                    if (skipError != ERROR_NO_MORE_FILES)
                    {
                        // Error occurred
                        FindClose((*iter)->findHandle);
                        (*iter)->findHandle = INVALID_HANDLE_VALUE;
                        (*iter)->hasNextEntry = VL_FALSE;
                        return VL_FALSE;
                    }
                    // End of this directory, break out of skip loop
                    FindClose((*iter)->findHandle);
                    (*iter)->findHandle = INVALID_HANDLE_VALUE;
                    (*iter)->hasNextEntry = VL_FALSE;
                    break;
                }
            }

            // Check if we found a valid entry after skipping
            if ((*iter)->findHandle != INVALID_HANDLE_VALUE)
            {
                (*iter)->hasNextEntry = VL_TRUE;
                return VL_TRUE;
            }
        }

        return VL_FALSE;
    }

    // Skip "." and ".." entries
    while (wcscmp((*iter)->findData.cFileName, L".") == 0 || wcscmp((*iter)->findData.cFileName, L"..") == 0)
    {
        if (!FindNextFileW((*iter)->findHandle, &(*iter)->findData))
        {
            if (GetLastError() == ERROR_NO_MORE_FILES)
            {
                (*iter)->hasNextEntry = VL_FALSE;
                FindClose((*iter)->findHandle);
                (*iter)->findHandle = INVALID_HANDLE_VALUE;
                return VL_FALSE;
            }
            else
            {
                // Error occurred
                (*iter)->hasNextEntry = VL_FALSE;
                FindClose((*iter)->findHandle);
                (*iter)->findHandle = INVALID_HANDLE_VALUE;
                return VL_FALSE;
            }
        }
    }

    return VL_TRUE;
}
