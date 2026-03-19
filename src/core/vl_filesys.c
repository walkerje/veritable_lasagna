#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#include "vl_filesys.h"
#include <string.h>

/**
 * \brief Parses out the path components of a stat structure's file path.
 * \param sys
 * \param stat
 * \private
 */
static inline void vl_FSParsePathComponents(vl_filesys* sys, vl_filesys_stat* stat)
{
    // Skip if path is NULL
    if (stat->filePath.pathStringPtr == VL_ARENA_NULL)
        return;

    const char* path = (const char*)vlArenaMemSample(&sys->memory, stat->filePath.pathStringPtr);

    // Find the last separator in the path
    const char* lastSep = strrchr(path, VL_NATIVE_PATH_SEPARATOR_CHAR);
    const char* filename = lastSep ? lastSep + 1 : path;

    // Handle dot files properly: skip leading dots when looking for extension
    const char* extensionStart = filename;

    // Skip leading dots to handle hidden files like .gitignore, .git, etc.
    while (*extensionStart == '.')
    {
        extensionStart++;
    }

    // Find the last dot in the filename portion after any leading dots
    const char* lastDot = strrchr(extensionStart, '.');

    // If we found a dot and it's not at the start of extensionStart, it's a real
    // extension
    if (lastDot && lastDot != extensionStart)
    {
        // Adjust lastDot to point to the actual dot in the original filename
        lastDot = filename + (lastDot - extensionStart) + (extensionStart - filename);
    }
    else
    {
        lastDot = NULL; // No extension found
    }

    // Calculate lengths
    const vl_memsize_t fullNameLen = strlen(filename) + 1;
    const vl_memsize_t baseNameLen = lastDot ? (vl_memsize_t)(lastDot - filename) + 1 : fullNameLen;
    const vl_memsize_t extensionLen = lastDot ? strlen(lastDot + 1) + 1 : 0;

    // Do all the block allocation from the arena first
    if (lastDot && extensionLen > 0)
    {
        if (stat->extension != VL_ARENA_NULL)
            vlArenaMemFree(&sys->memory, stat->extension);

        stat->extension = vlArenaMemAlloc(&sys->memory, extensionLen);
    }
    else if (stat->extension != VL_ARENA_NULL)
    {
        vlArenaMemFree(&sys->memory, stat->extension);
        stat->extension = VL_ARENA_NULL;
    }

    if (stat->fullName != VL_ARENA_NULL)
        vlArenaMemFree(&sys->memory, stat->fullName);
    stat->fullName = vlArenaMemAlloc(&sys->memory, fullNameLen);

    if (stat->baseName != VL_ARENA_NULL)
        vlArenaMemFree(&sys->memory, stat->baseName);
    stat->baseName = vlArenaMemAlloc(&sys->memory, baseNameLen);

    // Then, fix our pointers
    path = (const char*)vlArenaMemSample(&sys->memory, stat->filePath.pathStringPtr);
    lastSep = strrchr(path, VL_NATIVE_PATH_SEPARATOR_CHAR);
    filename = lastSep ? lastSep + 1 : path;

    // Recalculate lastDot using the same logic
    extensionStart = filename;
    while (*extensionStart == '.')
    {
        extensionStart++;
    }
    lastDot = strrchr(extensionStart, '.');
    if (lastDot && lastDot != extensionStart)
    {
        lastDot = filename + (lastDot - extensionStart) + (extensionStart - filename);
    }
    else
    {
        lastDot = NULL;
    }

    // Copy the parts of the path into the arena
    char* fullNameStr = (char*)vlArenaMemSample(&sys->memory, stat->fullName);
    char* baseNameStr = (char*)vlArenaMemSample(&sys->memory, stat->baseName);

    strcpy(fullNameStr, filename);
    strncpy(baseNameStr, filename, baseNameLen - 1);

    baseNameStr[baseNameLen - 1] = '\0';

    // Handle extension if it exists
    if (stat->extension != VL_ARENA_NULL)
    {
        char* extensionStr = (char*)vlArenaMemSample(&sys->memory, stat->extension);
        strcpy(extensionStr, lastDot + 1);
    }
}

#ifdef VL_FILESYS_WIN32

#include "platform/win32/vl_filesys_win32.c"

#elif defined VL_FILESYS_POSIX

#include "platform/posix/vl_filesys_posix.c"

#else
#error "Failed to configure platform for filesystem"
#endif

void vlFSInit(vl_filesys* sys)
{
    vlArenaInit(&sys->memory, VL_DEFAULT_MEMORY_SIZE);
    vlPoolInitAligned(&sys->statPool, sizeof(vl_filesys_stat), VL_ALIGNOF(vl_filesys_stat));
    vlPoolInitAligned(&sys->pathPool, sizeof(vl_filesys_path), VL_ALIGNOF(vl_filesys_path));
    vlPoolInitAligned(&sys->iterPool, sizeof(struct vl_filesys_iter_), VL_ALIGNOF(struct vl_filesys_iter_));
}

void vlFSFree(vl_filesys* sys)
{
    vlArenaFree(&sys->memory);
    vlPoolFree(&sys->statPool);
    vlPoolFree(&sys->pathPool);
    vlPoolFree(&sys->iterPool);
}

vl_filesys* vlFSNew(void)
{
    vl_filesys* sys = malloc(sizeof(vl_filesys));
    vlFSInit(sys);
    return sys;
}

void vlFSDelete(vl_filesys* sys)
{
    vlFSFree(sys);
    free(sys);
}

vl_filesys_stat* vlFSStatNew(vl_filesys* sys)
{
    vl_pool_idx statIndex = vlPoolTake(&sys->statPool);
    vl_filesys_stat* stat = vlPoolSample(&sys->statPool, statIndex);

    memset(stat, 0, sizeof(vl_filesys_stat));

    stat->statIndex = statIndex;
    stat->sys = sys;
    stat->filePath.sys = sys;
    stat->filePath.pathIndex = VL_POOL_INVALID_IDX;

    stat->fullName = VL_ARENA_NULL;
    stat->baseName = VL_ARENA_NULL;
    stat->extension = VL_ARENA_NULL;

    return stat;
}

void vlFSStatDelete(vl_filesys_stat* stat)
{
    vl_filesys* sys = stat->sys;
    if (stat->filePath.pathStringPtr != VL_ARENA_NULL)
        vlArenaMemFree(&sys->memory, stat->filePath.pathStringPtr);
    if (stat->baseName != VL_ARENA_NULL)
        vlArenaMemFree(&sys->memory, stat->baseName);
    if (stat->extension != VL_ARENA_NULL)
        vlArenaMemFree(&sys->memory, stat->extension);
    if (stat->fullName != VL_ARENA_NULL)
        vlArenaMemFree(&sys->memory, stat->fullName);

    stat->filePath.pathStringPtr = VL_ARENA_NULL;
    stat->baseName = VL_ARENA_NULL;
    stat->extension = VL_ARENA_NULL;
    stat->fullName = VL_ARENA_NULL;

    vlPoolReturn(&sys->statPool, stat->statIndex);
}

vl_filesys_path* vlFSPathNew(vl_filesys* sys, const char* pathStr)
{
    vl_pool_idx pathIndex = vlPoolTake(&sys->pathPool);
    vl_filesys_path* path = vlPoolSample(&sys->pathPool, pathIndex);
    path->pathIndex = pathIndex;
    path->sys = sys;

    if (pathStr)
    {
        path->pathStringPtr = vlArenaMemAlloc(&sys->memory, strlen(pathStr) + 1);
        strcpy((char*)vlArenaMemSample(&sys->memory, path->pathStringPtr), pathStr);
        vlFSPathNormalize(path);
    }
    else
        path->pathStringPtr = VL_ARENA_NULL;

    return path;
}

void vlFSPathDelete(vl_filesys_path* path)
{
    vl_filesys* sys = path->sys;
    if (path->pathIndex == VL_POOL_INVALID_IDX)
        return;

    if (path->pathStringPtr != VL_ARENA_NULL)
    {
        vlArenaMemFree(&sys->memory, path->pathStringPtr);
        path->pathStringPtr = VL_ARENA_NULL;
    }

    vlPoolReturn(&sys->pathPool, path->pathIndex);
}

vl_filesys_path* vlFSPathClone(const vl_filesys_path* src, vl_filesys_path* dest)
{
    vl_filesys* sys = src->sys;

    if (dest == NULL)
        dest = vlFSPathNew(sys, NULL);

    if (src->pathStringPtr == VL_ARENA_NULL)
    {
        if (dest->pathStringPtr != VL_ARENA_NULL)
        {
            vlArenaMemFree(&sys->memory, dest->pathStringPtr);
            dest->pathStringPtr = VL_ARENA_NULL;
        }
        return dest;
    }

    const vl_memsize_t srcLen = vlArenaMemSize(&sys->memory, src->pathStringPtr);

    if (dest->pathStringPtr != VL_ARENA_NULL)
        vlArenaMemFree(&sys->memory, dest->pathStringPtr);

    dest->pathStringPtr = vlArenaMemAlloc(&sys->memory, srcLen);

    char* destPathString = (char*)vlArenaMemSample(&sys->memory, dest->pathStringPtr);
    const char* srcPathString = (const char*)vlArenaMemSample(&sys->memory, src->pathStringPtr);
    strcpy(destPathString, srcPathString);

    return dest;
}

vl_bool_t vlFSPathEquals(const vl_filesys_path* pathA, const vl_filesys_path* pathB)
{
    if (pathA->sys != pathB->sys)
        return VL_FALSE;

    vl_filesys* sys = pathA->sys;

    if (pathA->pathStringPtr == pathB->pathStringPtr)
        return VL_TRUE;
    if (pathA->pathStringPtr == VL_ARENA_NULL || pathB->pathStringPtr == VL_ARENA_NULL)
        return VL_FALSE;

    const char* aPathStr = (const char*)vlArenaMemSample(&sys->memory, pathA->pathStringPtr);
    const char* bPathStr = (const char*)vlArenaMemSample(&sys->memory, pathB->pathStringPtr);

    return strcmp(aPathStr, bPathStr) == 0;
}

void vlFSPathSet(vl_filesys_path* path, const char* pathStr)
{
    vl_filesys* sys = path->sys;

    if (pathStr && pathStr[0] == '\0')
    {
        if (path->pathStringPtr != VL_ARENA_NULL)
        {
            vlArenaMemFree(&sys->memory, path->pathStringPtr);
            path->pathStringPtr = VL_ARENA_NULL;
        }
        return;
    }

    const vl_memsize_t pathStrLen = strlen(pathStr) + 1;
    if (path->pathStringPtr == VL_ARENA_NULL)
        path->pathStringPtr = vlArenaMemAlloc(&sys->memory, pathStrLen);
    else
        path->pathStringPtr = vlArenaMemRealloc(&sys->memory, path->pathStringPtr, pathStrLen);

    strcpy((char*)vlArenaMemSample(&sys->memory, path->pathStringPtr), pathStr);
    vlFSPathNormalize(path);
}

const vl_transient* vlFSPathString(const vl_filesys_path* path)
{
    vl_filesys* sys = path->sys;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return NULL;

    return vlArenaMemSample(&sys->memory, path->pathStringPtr);
}

void vlFSPathJoin(const vl_filesys_path* base, vl_filesys_path* dest, const char* component)
{
    vl_filesys* sys = dest->sys;

    // If no component is provided, just clone the base path to dest
    if (!component)
    {
        vlFSPathClone(base, dest);
        return;
    }

    // If no base path is provided or its string is null, just set component as
    // path
    if (!base || base->pathStringPtr == VL_ARENA_NULL)
    {
        vlFSPathSet(dest, component);
        return;
    }

    // Get the base path string
    const char* baseStr = (const char*)vlArenaMemSample(&sys->memory, base->pathStringPtr);

    // Calculate sizes
    const vl_memsize_t baseLen = strlen(baseStr);
    const vl_memsize_t componentLen = strlen(component);

    // Check if base path ends with separator
    vl_bool_t hasEndingSeparator = (baseLen > 0 && baseStr[baseLen - 1] == VL_NATIVE_PATH_SEPARATOR_CHAR);
    // Check if component starts with separator
    vl_bool_t hasStartingSeparator = (componentLen > 0 && component[0] == VL_NATIVE_PATH_SEPARATOR_CHAR);

    // Calculate total size needed including null terminator
    vl_memsize_t totalSize = baseLen + componentLen + 1;
    if (!hasEndingSeparator && !hasStartingSeparator)
    {
        totalSize++; // Need space for separator
    }
    if (hasEndingSeparator && hasStartingSeparator)
    {
        totalSize--; // Don't need double separator
    }

    // Allocate memory for the joined path
    const vl_arena_ptr destStrPtr = vlArenaMemAlloc(&sys->memory, totalSize);

    // Get the destination string pointer
    char* destStr = (char*)vlArenaMemSample(&sys->memory, destStrPtr);
    baseStr = (const char*)vlArenaMemSample(&sys->memory, base->pathStringPtr);

    // Copy base path
    strcpy(destStr, baseStr);

    // Add separator if needed
    if (!hasEndingSeparator && !hasStartingSeparator)
    {
        destStr[baseLen] = VL_NATIVE_PATH_SEPARATOR_CHAR;
        destStr[baseLen + 1] = '\0';
    }

    // Append component (skip the first character if it's a separator and base had
    // one)
    if (hasEndingSeparator && hasStartingSeparator)
    {
        strcat(destStr, component + 1);
    }
    else
    {
        strcat(destStr, component);
    }

    if (dest->pathStringPtr != VL_ARENA_NULL)
        vlArenaMemFree(&sys->memory, dest->pathStringPtr);

    dest->pathStringPtr = destStrPtr;
}

void vlFSPathNormalize(vl_filesys_path* path)
{
    vl_filesys* sys = path->sys;
    if (path == NULL)
        return;
    if (path->pathStringPtr == VL_ARENA_NULL)
        return;

    // Allocate temporary buffer for normalized path
    // We'll need at most the same length as the original
    const vl_memsize_t origLen = vlArenaMemSize(&sys->memory, path->pathStringPtr);
    vl_arena_ptr tempPtr = vlArenaMemAlloc(&sys->memory, origLen);

    vl_ularge_t writePos = 0;
    vl_bool_t lastWasSep = VL_FALSE;

    // Get the original path string and the temporary buffer
    const char* origPath = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);
    char* tempPath = (char*)vlArenaMemSample(&sys->memory, tempPtr);

    // Process each byte
    for (vl_memsize_t readPos = 0; readPos < origLen; readPos++)
    {
        char c = origPath[readPos];

        // Handle separator
        if (c == VL_EXOTIC_PATH_SEPARATOR_CHAR || c == VL_NATIVE_PATH_SEPARATOR_CHAR)
        {
            if (!lastWasSep)
            {
                // Only write separator if previous char wasn't one
                tempPath[writePos++] = VL_NATIVE_PATH_SEPARATOR_CHAR;
                lastWasSep = VL_TRUE;
            }
            continue;
        }
        lastWasSep = VL_FALSE;

        // Normal character, just copy it
        tempPath[writePos++] = c;
    }

    // Null terminate.
    tempPath[writePos] = '\0';

    // Now handle '.' and ".." components
    vl_arena_ptr resultPtr = vlArenaMemAlloc(&sys->memory, writePos + 1);
    vl_arena_ptr componentPtr = vlArenaMemAlloc(&sys->memory, sizeof(char*) * (writePos + 1));

    origPath = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);
    tempPath = (char*)vlArenaMemSample(&sys->memory, tempPtr);
    char* result = (char*)vlArenaMemSample(&sys->memory, resultPtr);
    char** components = (char**)vlArenaMemSample(&sys->memory, componentPtr);

    int componentCount = 0;

    // Split into components
    char* component = strtok(tempPath, VL_NATIVE_PATH_SEPARATOR_STR);
    while (component)
    {
        if (strcmp(component, ".") == 0)
        {
            // Skip "." components
        }
        else if (strcmp(component, "..") == 0)
        {
            // Handle ".." by removing previous component if it exists
            if (componentCount > 0)
            {
                componentCount--;
            }
        }
        else
        {
            // Add normal component
            components[componentCount++] = component;
        }
        component = strtok(NULL, VL_NATIVE_PATH_SEPARATOR_STR);
    }

    // Rebuild the path
    writePos = 0;

    // Handle absolute paths
    if (origPath[0] == VL_NATIVE_PATH_SEPARATOR_CHAR)
    {
        result[writePos++] = VL_NATIVE_PATH_SEPARATOR_CHAR;
    }

    // Join components
    for (int i = 0; i < componentCount; i++)
    {
        if (i > 0)
        {
            result[writePos++] = VL_NATIVE_PATH_SEPARATOR_CHAR;
        }
        const vl_memsize_t componentLen = strlen(components[i]);
        memcpy(result + writePos, components[i], componentLen);
        writePos += componentLen;
    }

    // Handle empty path
    if (writePos == 0 && origPath[0] != VL_NATIVE_PATH_SEPARATOR_CHAR)
    {
        result[writePos++] = '.';
    }

    result[writePos] = '\0';

    // Cleanup
    vlArenaMemFree(&sys->memory, tempPtr);
    vlArenaMemFree(&sys->memory, componentPtr);

    // Update the path with a normalized version
    vlArenaMemFree(&sys->memory, path->pathStringPtr);

    path->pathStringPtr = resultPtr;
}

void vlFSPathParent(const vl_filesys_path* path, vl_filesys_path* parentOut)
{
    vl_filesys* sys = path->sys;

    if (!path || !parentOut || path->pathStringPtr == VL_ARENA_NULL)
    {
        return;
    }

    // Get the original path string
    const char* pathStr = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);
    const vl_memsize_t pathLen = strlen(pathStr);

    // Handle root directory case
    if (pathLen == 1 && pathStr[0] == VL_NATIVE_PATH_SEPARATOR_CHAR)
    {
        // Root directory's parent is itself
        vlFSPathClone(path, parentOut);
        return;
    }

    // Find the last separator, skipping any trailing ones
    vl_memsize_t lastSepPos = pathLen - 1;
    while (lastSepPos > 0 && pathStr[lastSepPos] == VL_NATIVE_PATH_SEPARATOR_CHAR)
    {
        lastSepPos--;
    }

    while (lastSepPos > 0 && pathStr[lastSepPos] != VL_NATIVE_PATH_SEPARATOR_CHAR)
    {
        lastSepPos--;
    }

    // Allocate memory for the parent path
    vl_arena_ptr parentStrPtr;
    char* parentStr;

    if (lastSepPos == 0)
    {
        if (pathStr[0] == VL_NATIVE_PATH_SEPARATOR_CHAR)
        {
            // Parent of "/something" is "/"
            parentStrPtr = vlArenaMemAlloc(&sys->memory, 2);
            parentStr = (char*)vlArenaMemSample(&sys->memory, parentStrPtr);
            parentStr[0] = VL_NATIVE_PATH_SEPARATOR_CHAR;
            parentStr[1] = '\0';
        }
        else
        {
            // Parent of "something" is "."
            parentStrPtr = vlArenaMemAlloc(&sys->memory, 2);
            parentStr = (char*)vlArenaMemSample(&sys->memory, parentStrPtr);
            parentStr[0] = '.';
            parentStr[1] = '\0';
        }
    }
    else
    {
        // Regular case - copy up to the last separator
        vl_memsize_t parentLen = lastSepPos;
        // Ensure we keep the leading separator for absolute paths
        if (pathStr[0] == VL_NATIVE_PATH_SEPARATOR_CHAR && lastSepPos == 1)
        {
            parentLen = 1; // Keep just the root separator
        }

        parentStrPtr = vlArenaMemAlloc(&sys->memory, parentLen + 1);
        parentStr = (char*)vlArenaMemSample(&sys->memory, parentStrPtr);
        pathStr = (const char*)vlArenaMemSample(&sys->memory, path->pathStringPtr);

        memcpy(parentStr, pathStr, parentLen);
        parentStr[parentLen] = '\0';
    }

    // Free any existing string in parentOut
    if (parentOut->pathStringPtr != VL_ARENA_NULL)
    {
        vlArenaMemFree(&sys->memory, parentOut->pathStringPtr);
        parentOut->pathStringPtr = VL_ARENA_NULL;
    }

    parentOut->pathStringPtr = parentStrPtr;
}
