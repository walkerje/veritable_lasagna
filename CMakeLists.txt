cmake_minimum_required(VERSION 3.22.1)

# Versioning
set(VL_VERSION_MAJOR 0)
set(VL_VERSION_MINOR 13)
set(VL_VERSION_PATCH 9)
set(VL_VERSION ${VL_VERSION_MAJOR}.${VL_VERSION_MINOR}.${VL_VERSION_PATCH})

# Some relative variables.
set(VL_PROJECT_PATH ${CMAKE_CURRENT_LIST_DIR})
set(VL_CONFIG_HEADER_INCLUDE "${CMAKE_BINARY_DIR}/config/include/")
set(VL_CONFIG_HEADER_PATH "${VL_CONFIG_HEADER_INCLUDE}vl")
string(LENGTH ${VL_PROJECT_PATH} VL_PROJECT_PATH_LEN)
math(EXPR VL_PROJECT_PATH_LEN "${VL_PROJECT_PATH_LEN} + 1")# +1 to omit path separator

project(veritable_lasagna VERSION ${VL_VERSION}
        LANGUAGES C
        DESCRIPTION "Data structures and algorithms."
        HOMEPAGE_URL "https://bitshiftmountain.com")


set(CMAKE_C_STANDARD            11)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CheckTypeSize)

# Primitive type resolution.
# This section is dense and awkward.
# Its purpose is to select explicit sizes purely from built-in language types.
# This offers less dependence on the C standard, allowing the configuration of all integer and floating-point types,
# with their own expectations (explicit IEEE-754 format) and documentation (128-bit floats exist on some platforms).

set(VL_PRIMITIVE_INTEGER_TYPES char short int long "long long")
set(VL_PRIMITIVE_INTEGER_TYPES_NAMES CHAR SHORT INT LONG LONG_LONG)

set(VL_ITER 0)
set(VL_INT_STATUS OFF)
foreach (VL_TYPE_ITER ${VL_PRIMITIVE_INTEGER_TYPES})
    list(GET VL_PRIMITIVE_INTEGER_TYPES_NAMES ${VL_ITER} VL_CURRENT_NAME)
    check_type_size("${VL_TYPE_ITER}" VL_SIZEOF_${VL_CURRENT_NAME} BUILTIN_TYPES_ONLY LANGUAGE C)
    MATH(EXPR VL_ITER "${VL_ITER} + 1")
endforeach ()

macro(vl_define_int_primitive size)
    set(VL_ITER 0)
    set(VL_INT_STATUS OFF)
    MATH(EXPR VL_INT_BITS "${size} * 8")
    foreach (VL_TYPE_ITER ${VL_PRIMITIVE_INTEGER_TYPES})
        list(GET VL_PRIMITIVE_INTEGER_TYPES_NAMES ${VL_ITER} VL_CURRENT_NAME)
        if(${VL_SIZEOF_${VL_CURRENT_NAME}} EQUAL ${size})
            set(VL_I${VL_INT_BITS}_T "${VL_TYPE_ITER}")
            set(VL_U${VL_INT_BITS}_T "unsigned ${VL_TYPE_ITER}")
            set(VL_INT_STATUS ON)
            break ()
        endif ()
        MATH(EXPR VL_ITER "${VL_ITER} + 1")
    endforeach ()

    if(NOT ${VL_INT_STATUS})
        message(WARNING "Failed to find integer type on target platform for ${VL_INT_BITS}-bit width.")
    endif ()
endmacro()

# Try to define 1, 2, 4, and 8 byte wide integer types.
vl_define_int_primitive(1)
vl_define_int_primitive(2)
vl_define_int_primitive(4)
vl_define_int_primitive(8)

# Define VL_INT and VL_UINT, starting with 32-bit types and working down.
if(DEFINED VL_I32_T)
    set(VL_INT_T  "${VL_I32_T}")
    set(VL_UINT_T "${VL_U32_T}")
    set(VL_SIZEOF_STANDARD_INT 4)
elseif (DEFINED VL_I16_T)
    set(VL_INT_T  "${VL_I16_T}")
    set(VL_UINT_T "${VL_U16_T}")
    set(VL_SIZEOF_STANDARD_INT 2)
else ()
    set(VL_INT  "${VL_I8_T}")
    set(VL_UINT "${VL_U8_T}")
    set(VL_SIZEOF_STANDARD_INT 1)
    message(WARNING "VL_INT resolved to 8-bit integers!")
endif ()

if(DEFINED VL_I8_T)
    set(VL_ISMALL_T "${VL_I8_T}")
    set(VL_USMALL_T "${VL_U8_T}")
elseif(DEFINED VL_I16_T)
    set(VL_ISMALL_T "${VL_I16_T}")
    set(VL_USMALL_T "${VL_U16_T}")
else()
    message(FATAL_ERROR "Failed to determine smallest integer types.")
endif ()

if(DEFINED VL_I64_T)
    set(VL_ILARGE_T "${VL_I64_T}")
    set(VL_ULARGE_T "${VL_U64_T}")
elseif(DEFINED VL_I32_T)
    set(VL_ILARGE_T "${VL_I32_T}")
    set(VL_ULARGE_T "${VL_U32_T}")
else ()
    message(FATAL_ERROR "Failed to determine largest integer types.")
endif ()

set(VL_IPTR_T "${VL_ILARGE_T}")
set(VL_UPTR_T "${VL_ULARGE_T}")

# Now define floating point primitives.
set(VL_PRIMITIVE_FLOAT_TYPES float double "long double")

check_type_size("float"             VL_SIZEOF_FLOAT         BUILTIN_TYPES_ONLY LANGUAGE C)
check_type_size("double"            VL_SIZEOF_DOUBLE        BUILTIN_TYPES_ONLY LANGUAGE C)
check_type_size("long double"       VL_SIZEOF_LONG_DOUBLE   BUILTIN_TYPES_ONLY LANGUAGE C)

if(${VL_SIZEOF_FLOAT} EQUAL 4)
    set(VL_F32_T "float")
elseif (${VL_SIZEOF_DOUBLE} EQUAL 4)
    set(VL_F32_T "double")
else ()
    message(FATAL_ERROR "Failed to find a 32-bit floating point type.")
endif ()

if (${VL_SIZEOF_DOUBLE} EQUAL 8)
    set(VL_F64_T "double")
elseif (${VL_SIZEOF_LONG_DOUBLE} EQUAL 8)
    set(VL_F64_T "long double")
else ()
    message(WARNING "Failed to find 64-bit floating point type.")
endif ()

if(${VL_SIZEOF_LONG_DOUBLE} GREATER 8)
    set(VL_FHIGHP_T "long double")
    # Some modern Intel platforms support 16-byte (128-bit) floating point types, offering even more precision.
    message(STATUS "Found high-precision floating point type spanning ${VL_SIZEOF_LONG_DOUBLE} bytes for VL_FHIGHP_T.")
elseif (DEFINED VL_F64_T)
    set(VL_FHIGHP_T "double")
elseif (DEFINED VL_F32_T)
    set(VL_FHIGHP_T "float")
endif ()

set(VL_STRUCTURE_OFFSET_T       "${VL_ULARGE_T}")
set(VL_STRUCTURE_INDEX_T        "${VL_UINT_T}")
set(VL_MEMORY_T                 "${VL_USMALL_T}")
set(VL_MEMORY_SIZE_T            "${VL_ULARGE_T}")

string(REPEAT "FF" ${VL_SIZEOF_STANDARD_INT} VL_STRUCTURE_INDEX_MAX)
set(VL_STRUCTURE_INDEX_MAX "0x${VL_STRUCTURE_INDEX_MAX}")

if(CMAKE_C_BYTE_ORDER EQUAL BIG_ENDIAN)
    set(VL_BYTE_ORDER 1)
else ()
    set(VL_BYTE_ORDER 0)
endif ()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VL_BUILD_TYPE DEBUG)
else()
    set(VL_BUILD_TYPE RELEASE)
endif()

# Platform configuration.

if(NOT WIN32)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
endif ()

find_package(Threads REQUIRED)

if(${CMAKE_USE_WIN32_THREADS_INIT})
    set(VL_THREADS_WIN32 1)
elseif (${CMAKE_USE_PTHREADS_INIT})
    set(VL_THREADS_PTHREAD 1)
endif ()

set(VL_SYSTEM_LIBS Threads::Threads)

if(NOT MSVC)
    set(VL_SYSTEM_LIBS ${VL_SYSTEM_LIBS} atomic)
endif ()

if(MSVC)
    set(VL_COMPILE_OPTIONS ${VL_COMPILE_OPTIONS} /experimental:c11atomics)
endif ()

# Macro definitions.

# This macro will add a source file to a list of sources.
# It will print the path relative to the project path.
macro(vl_add_source filepath)
    set(VL_SOURCE_ABSPATH "${CMAKE_CURRENT_LIST_DIR}/${filepath}")
    string(SUBSTRING ${VL_SOURCE_ABSPATH} ${VL_PROJECT_PATH_LEN} -1 VL_SOURCE_RELPATH)
    message(STATUS "    ${VL_SOURCE_RELPATH}")
    set(VL_SOURCES_LOCAL ${VL_SOURCES_LOCAL} ${VL_SOURCE_RELPATH})
    set(VL_SOURCES ${VL_SOURCES} ${VL_SOURCES_LOCAL} PARENT_SCOPE)
endmacro()

# This function will configure a header file and output it to the specified location.
# The input path is relative to the calling cmakelists.
macro(vl_configure_header filepath outname)
    set(VL_HEADER_ABSPATH "${CMAKE_CURRENT_LIST_DIR}/${filepath}")
    string(SUBSTRING ${VL_HEADER_ABSPATH} ${VL_PROJECT_PATH_LEN} -1 VL_HEADER_RELPATH)
    string(SUBSTRING ${VL_CONFIG_HEADER_PATH} ${VL_PROJECT_PATH_LEN} -1 VL_HEADER_OUT_RELPATH)
    message(STATUS "    ${VL_HEADER_RELPATH} (Configured --> \"${VL_HEADER_OUT_RELPATH}/${outname}\")")
    configure_file(${VL_HEADER_ABSPATH} ${VL_CONFIG_HEADER_PATH}/${outname} @ONLY)
endmacro()

message(STATUS "Gathering sources...")
add_subdirectory(src)

set(VL_INCLUDE          "${CMAKE_CURRENT_LIST_DIR}/include/" "${VL_CONFIG_HEADER_INCLUDE}")
set(VL_CORE_TARGET      Core)
set(VL_INSTALL_TARGETS  ${VL_CORE_TARGET})

add_library                 (${VL_CORE_TARGET} ${VL_SOURCES})

target_include_directories  (${VL_CORE_TARGET} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vl>
        $<INSTALL_INTERFACE:include>
        PRIVATE ${VL_CONFIG_HEADER_PATH} ${VL_CONFIG_HEADER_INCLUDE})

set_target_properties       (${VL_CORE_TARGET} PROPERTIES
        OUTPUT_NAME "vl_core"
        DEBUG_POSTFIX "d")
target_compile_options      (${VL_CORE_TARGET} PUBLIC "${VL_COMPILE_OPTIONS}")
target_link_libraries       (${VL_CORE_TARGET} PUBLIC "${VL_SYSTEM_LIBS}")

add_library(VLasagna::Core ALIAS  ${VL_CORE_TARGET})

if(BUILD_SHARED_LIBS)
    target_compile_definitions(${VL_CORE_TARGET}
            PRIVATE VL_BUILDING_SHARED
            INTERFACE VL_USING_SHARED
    )

    set_target_properties(${VL_CORE_TARGET} PROPERTIES
            C_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
            POSITION_INDEPENDENT_CODE ON
    )
endif()

export(TARGETS ${VL_INSTALL_TARGETS} NAMESPACE VLasagna:: FILE VLasagnaTargets.cmake)

if(BUILD_TESTING)
    add_subdirectory(test)
endif ()

if(MSVC AND BUILD_SHARED_LIBS)
    # Ensure debug info is installed.
    install(FILES $<TARGET_PDB_FILE:${VL_CORE_TARGET}>
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            OPTIONAL)
endif()

install(TARGETS ${VL_INSTALL_TARGETS} EXPORT VLasagnaTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${VL_INCLUDE} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT VLasagnaTargets
        FILE VLasagnaTargets.cmake
        NAMESPACE VLasagna::
        DESTINATION lib/cmake/vlasagna
)

write_basic_package_version_file(
        VLasagnaConfigVersion.cmake
        VERSION ${VL_VERSION}
        COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(VLasagnaConfig.cmake.in VLasagnaConfig.cmake INSTALL_DESTINATION lib/cmake/vlasagna)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VLasagnaConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/VLasagnaConfigVersion.cmake"
        DESTINATION lib/cmake/vlasagna
)

export(PACKAGE VLasagna)