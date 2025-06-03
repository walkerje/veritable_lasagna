#include <dlfcn.h>
#include <stdlib.h>

/* POSIX implementation of dynamic library functions */

vl_dynlib_result vlLibraryOpen(const char* name, vl_dynlib* lib) {
    if (!lib) return VL_DYNLIB_ERROR_INVALID_HANDLE;

    /* Clear any existing error */
    dlerror();

    *lib = dlopen(name, RTLD_LAZY | RTLD_LOCAL);

    if (!*lib) {
        return VL_DYNLIB_ERROR_OPEN;
    }

    return VL_DYNLIB_SUCCESS;
}

vl_dynlib_result vlLibraryClose(vl_dynlib* lib) {
    if (!lib || !*lib) return VL_DYNLIB_ERROR_INVALID_HANDLE;

    if (dlclose(*lib) != 0) {
        return VL_DYNLIB_ERROR_OPEN;
    }

    *lib = NULL;
    return VL_DYNLIB_SUCCESS;
}

vl_dynlib_result vlLibraryProc(vl_dynlib lib, const char* name, void** proc) {
    if (!lib || !proc) return VL_DYNLIB_ERROR_INVALID_HANDLE;

    /* Clear any existing error */
    dlerror();

    *proc = dlsym(lib, name);

    /* Check if dlsym failed */
    const char* error = dlerror();
    if (error != NULL) {
        return VL_DYNLIB_ERROR_SYMBOL;
    }

    return VL_DYNLIB_SUCCESS;
}

const char* vlLibraryError(void) {
    return dlerror();
}
