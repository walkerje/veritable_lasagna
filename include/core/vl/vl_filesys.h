/**
 * ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
 * ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
 * ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
 *  ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
 *   ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
 * ====---: A Data Structure and Algorithms library for C11.  :---====
 *
 * Copyright 2026 Jesse Walker, released under the MIT license.
 * Git Repository:  https://github.com/walkerje/veritable_lasagna
 * \private
 */

/**
 * \file vl_filesys.h
 * \brief Cross-platform filesystem operations with UTF-8 path handling
 *
 * This module provides a comprehensive filesystem abstraction layer that works
 * consistently across Windows and POSIX systems. All filesystem operations
 * enforce UTF-8 encoding for paths internally, ensuring proper handling of
 * international characters and consistent behavior regardless of the underlying
 * platform's native path encoding.
 *
 * Key Features:
 * - Cross-platform path operations with consistent separator handling
 * - UTF-8 path enforcement for international character support
 * - Memory-efficient design using arena and pool allocators
 * - Directory iteration with recursive traversal support
 * - File status information including timestamps and metadata
 * - Path manipulation utilities (join, normalize, absolute/relative conversion)
 * - Safe path component parsing (basename, extension, full name)
 *
 * Path Handling:
 * All paths are stored and processed as UTF-8 strings internally, with
 * automatic conversion to the platform's native encoding (UTF-16 on Windows,
 * UTF-8 on POSIX) when making system calls. This ensures consistent behavior
 * and proper support for international filenames across all supported
 * platforms. All paths are automatically normalized when assigned to avoid
 * headaches between file separators across platforms. This opinionated choice
 * aims to strike a comfortable balance between ease-of-use between platforms
 * and the ability to get meaningful work done with the API.
 *
 * Memory Management:
 * The filesystem context uses arena allocation for string storage and pool
 * allocation for object instances, providing efficient memory usage and
 * automatic cleanup when the context is freed.
 *
 * Thread Safety:
 * Individual filesystem contexts are not thread-safe. Each thread should use
 * its own filesystem context or external synchronization must be provided.
 *
 * \see vl_filesys for the main filesystem context structure
 * \see vl_filesys_path for path representation and manipulation
 * \see vl_filesys_stat for file status information
 * \see vl_filesys_iter for directory iteration
 */

#ifndef VL_FILESYS_H
#define VL_FILESYS_H

#include "vl_arena.h"

/**
 * \brief Result codes for filesystem operations.
 */
typedef enum
{
    VL_FS_SUCCESS = 0, /** Operation completed successfully */
    VL_FS_ERROR_ACCESS_DENIED, /** Access denied to the file or directory */
    VL_FS_ERROR_PATH_INVALID, /** The provided path is invalid */
    VL_FS_ERROR_NOT_FOUND, /** File or directory not found */
    VL_FS_ERROR_IO /** Generic I/O error */
} vl_filesys_result;

/**
 * \brief Opaque handle for filesystem directory iteration.
 */
typedef struct vl_filesys_iter_* vl_filesys_iter;

#define VL_FS_ITER_INVALID NULL

struct vl_filesys_;

/**
 * \brief Represents a filesystem path within the filesystem context.
 */
typedef struct
{
    struct vl_filesys_* sys; /** Pointer to the filesystem context */

    vl_pool_idx pathIndex; /** Unique pool index for this path */
    vl_arena_ptr pathStringPtr; /** Pointer to the path string in arena memory */
} vl_filesys_path;

/**
 * \brief File status information structure.
 *
 * Contains detailed information about a file or directory, including
 * metadata such as size, timestamps, and parsed path components.
 */
typedef struct
{
    struct vl_filesys_* sys; /** Pointer to the filesystem context */

    vl_pool_idx statIndex; /** Unique pool index for this stat object */
    vl_bool_t isDirectory; /** True if the file is a directory */
    vl_bool_t isReadOnly; /** True if the file is read-only */
    vl_memsize_t fileSize; /** Size of the file in bytes */
    vl_ularge_t createTime; /** File creation timestamp */
    vl_ularge_t modifyTime; /** File modification timestamp */
    vl_ularge_t accessTime; /** File last access timestamp */
    vl_filesys_path filePath; /** Path to the file (will not have a path index) */

    // Assume we're looking at the path "./foo/bar.txt"
    vl_arena_ptr baseName; /** Base name = "bar" */
    vl_arena_ptr extension; /** Extension = "txt" (or VL_ARENA_NULL if no extension) */
    vl_arena_ptr fullName; /** Full name = "bar.txt" */
} vl_filesys_stat;

/**
 * \brief Filesystem context structure.
 *
 * Manages memory allocation for filesystem operations using
 * arena and pool allocators for efficient memory management.
 */
typedef struct vl_filesys_
{
    vl_arena memory; /** Arena allocator for string storage */
    vl_pool statPool; /** Pool allocator for stat instances */
    vl_pool pathPool; /** Pool allocator for path instances */
    vl_pool iterPool; /** Pool allocator for iterator instances */
} vl_filesys;

/**
 * \brief Initializes a filesystem context.
 * \param sys Pointer to the filesystem context to initialize
 */
VL_API void vlFSInit(vl_filesys* sys);

/**
 * \brief Frees resources associated with a filesystem context.
 * \param sys Pointer to the filesystem context to free
 */
VL_API void vlFSFree(vl_filesys* sys);

/**
 * \brief Creates a new filesystem context.
 * \return Pointer to the newly created filesystem context
 */
VL_API vl_filesys* vlFSNew(void);

/**
 * \brief Deletes a filesystem context created with vlFSNew.
 * \param sys Pointer to the filesystem context to delete
 */
VL_API void vlFSDelete(vl_filesys* sys);

/**
 * \brief Creates a new file stat object.
 * \param sys Pointer to the filesystem context
 * \return Pointer to the newly created stat object
 */
VL_API vl_filesys_stat* vlFSStatNew(vl_filesys* sys);

/**
 * \brief Deletes a file stat object.
 * \param stat Pointer to the stat object to delete
 */
VL_API void vlFSStatDelete(vl_filesys_stat* stat);

/**
 * \brief Gets file status information for a given path.
 * \param path Pointer to the path to query
 * \param[out] result Pointer to store the stat information
 * \return Result code indicating success or failure
 */
VL_API vl_filesys_result vlFSStatPath(const vl_filesys_path* path, vl_filesys_stat* result);

/**
 * \brief Gets file status information for the current iterator position.
 * \param iter Directory iterator handle
 * \param[out] result Pointer to store the stat information
 * \return Result code indicating success or failure
 */
VL_API vl_filesys_result vlFSStatIter(vl_filesys_iter iter, vl_filesys_stat* result);

/**
 * \brief Creates a new filesystem path object.
 * \param sys Pointer to the filesystem context
 * \param path String representation of the path
 * \return Pointer to the newly created path object
 */
VL_API vl_filesys_path* vlFSPathNew(vl_filesys* sys, const char* path);

/**
 * \brief Deletes a filesystem path object.
 * \param path Pointer to the path object to delete
 */
VL_API void vlFSPathDelete(vl_filesys_path* path);

/**
 * \brief Clones a filesystem path.
 * \param src Source path to clone
 * \param[out] dest Destination path object
 * \return Pointer to the cloned path object
 */
VL_API vl_filesys_path* vlFSPathClone(const vl_filesys_path* src, vl_filesys_path* dest);

/**
 * \brief Compares two filesystem paths for equality.
 * \param pathA First path to compare
 * \param pathB Second path to compare
 * \return True if paths are equal, false otherwise
 */
VL_API vl_bool_t vlFSPathEquals(const vl_filesys_path* pathA, const vl_filesys_path* pathB);

/**
 * \brief Sets the path string for a filesystem path object.
 * \param path Pointer to the path object
 * \param pathStr String representation of the new path
 */
VL_API void vlFSPathSet(vl_filesys_path* path, const char* pathStr);

/**
 * \brief Gets the string representation of a filesystem path.
 * \param path Pointer to the path object
 * \return Pointer to the path string
 */
VL_API const vl_transient* vlFSPathString(const vl_filesys_path* path);

/**
 * \brief Joins a path component to a base path.
 * \param base Base path to join to
 * \param[out] dest Destination path object for the result
 * \param component Path component to join
 */
VL_API void vlFSPathJoin(const vl_filesys_path* base, vl_filesys_path* dest, const char* component);

/**
 * \brief Normalizes a filesystem path by resolving relative components.
 *
 * This function will normalize the given path with native file path separators.
 *
 * \param path Pointer to the path object to normalize
 */
VL_API void vlFSPathNormalize(vl_filesys_path* path);

/**
 * \brief Gets the parent directory of a filesystem path.
 * \param path Input path
 * \param[out] parentOut Pointer to store the parent path
 */
VL_API void vlFSPathParent(const vl_filesys_path* path, vl_filesys_path* parentOut);

/**
 * \brief Converts a relative path to an absolute path.
 * \param path Pointer to the path object to make absolute
 */
VL_API void vlFSPathAbsolute(vl_filesys_path* path);

/**
 * \brief Checks if a filesystem path is absolute.
 * \param path Pointer to the path object to check
 * \return True if the path is absolute, false otherwise
 */
VL_API vl_bool_t vlFSPathIsAbsolute(const vl_filesys_path* path);

/**
 * \brief Creates a directory at the specified path.
 * \param path Pointer to the path where the directory should be created
 * \return Result code indicating success or failure
 */
VL_API vl_filesys_result vlFSPathMkDir(const vl_filesys_path* path);

/**
 * \brief Removes a file or directory at the specified path.
 * \param path Pointer to the path to remove
 * \return Result code indicating success or failure
 */
VL_API vl_filesys_result vlFSPathRemove(const vl_filesys_path* path);

/**
 * \brief Checks if a file or directory exists at the specified path.
 * \param path Pointer to the path to check
 * \return True if the path exists, false otherwise
 */
VL_API vl_bool_t vlFSPathExists(const vl_filesys_path* path);

/**
 * \brief Creates a new directory iterator.
 * \param sys Pointer to the filesystem context
 * \return Handle to the new iterator or VL_FS_ITER_INVALID on failure
 */
VL_API vl_filesys_iter vlFSIterNew(vl_filesys* sys);

/**
 * \brief Deletes a directory iterator.
 * \param iter Iterator handle to delete
 */
VL_API void vlFSIterDelete(vl_filesys_iter iter);

/**
 * \brief Initializes an iterator for a directory.
 * \param[in,out] iter Pointer to the iterator handle
 * \param path Pointer to the directory path to iterate
 * \return Result code indicating success or failure
 */
VL_API vl_filesys_result vlFSIterDir(vl_filesys_iter* iter, const vl_filesys_path* path);

/**
 * \brief Initializes an iterator for recursive directory traversal.
 * \param[in,out] iterPtr Pointer to the iterator handle
 * \param path Pointer to the directory path to iterate recursively
 * \return Result code indicating success or failure
 */
VL_API vl_filesys_result vlFSIterDirRecursive(vl_filesys_iter* iterPtr, const vl_filesys_path* path);

/**
 * \brief Advances the iterator to the next entry.
 * \param[in,out] iter Pointer to the iterator handle
 * \return True if there is a next entry, false if iteration is complete
 */
VL_API vl_bool_t vlFSIterNext(vl_filesys_iter* iter);

#endif // VL_FILESYS_H
