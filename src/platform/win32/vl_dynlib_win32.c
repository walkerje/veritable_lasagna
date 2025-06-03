#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/* Windows implementation of dynamic library functions */

vl_dynlib_result vlLibraryOpen(const char* name, vl_dynlib* lib) {
    if (!lib) return VL_DYNLIB_ERROR_INVALID_HANDLE;

    *lib = (vl_dynlib)LoadLibraryA(name);

    if (!*lib) {
        return VL_DYNLIB_ERROR_OPEN;
    }

    return VL_DYNLIB_SUCCESS;
}

vl_dynlib_result vlLibraryClose(vl_dynlib* lib) {
    if (!lib || !*lib) return VL_DYNLIB_ERROR_INVALID_HANDLE;

    if (!FreeLibrary((HMODULE)*lib)) {
        return VL_DYNLIB_ERROR_OPEN;
    }

    *lib = NULL;
    return VL_DYNLIB_SUCCESS;
}

vl_dynlib_result vlLibraryProc(vl_dynlib lib, const char* name, void** proc) {
    if (!lib || !proc) return VL_DYNLIB_ERROR_INVALID_HANDLE;

    *proc = (void*)GetProcAddress((HMODULE)lib, name);

    if (!*proc) {
        return VL_DYNLIB_ERROR_SYMBOL;
    }

    return VL_DYNLIB_SUCCESS;
}

const char* vlLibraryError(void) {
    static char buffer[256];
    DWORD error = GetLastError();

    if (error == 0) return NULL;

    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer,
        sizeof(buffer),
        NULL
    );

    return buffer;
}