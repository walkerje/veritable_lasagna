#ifndef VL_DYNLIB_H
#define VL_DYNLIB_H

typedef struct vl_dynlib_* vl_dynlib;

typedef enum {
    VL_DYNLIB_SUCCESS = 0,
    VL_DYNLIB_ERROR_OPEN,
    VL_DYNLIB_ERROR_SYMBOL,
    VL_DYNLIB_ERROR_INVALID_HANDLE
} vl_dynlib_result;

/**
 * \brief Opens a dynamic library
 * \param name Path to the library
 * \param[out] lib Pointer to store the library handle
 * \return Result code indicating success or failure
 */
vl_dynlib_result vlLibraryOpen(const char* name, vl_dynlib* lib);

/**
 * \brief Closes a dynamic library
 * \param[in,out] lib Pointer to the library handle (will be set to NULL after closing)
 * \return Result code indicating success or failure
 */
vl_dynlib_result vlLibraryClose(vl_dynlib* lib);

/**
 * \brief Gets a procedure address from a dynamic library
 * \param lib Pointer to the library handle
 * \param name Name of the procedure to look up
 * \param[out] proc Pointer to store the procedure address
 * \return Result code indicating success or failure
 */
vl_dynlib_result vlLibraryProc(vl_dynlib lib, const char* name, void** proc);

/**
 * \brief Gets the last error message from dynamic loading operations
 * \return Error message string or NULL if no error
 */
const char* vlLibraryError(void);


#endif //VL_DYNLIB_H
