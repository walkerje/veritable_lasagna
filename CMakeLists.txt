cmake_minimum_required(VERSION 3.22.1)
include(CMakeDependentOption)
include(CMakePackageConfigHelpers)
include(CheckTypeSize)

# Versioning
set(VL_VERSION_MAJOR 0)
set(VL_VERSION_MINOR 10)
set(VL_VERSION_PATCH 18)
set(VL_VERSION ${VL_VERSION_MAJOR}.${VL_VERSION_MINOR}.${VL_VERSION_PATCH})

# Configuration options
option(VL_BUILD_SHARED      "On/Off. If true, configure to build the shared library."       OFF)
option(CMAKE_EXPORT_PACKAGE_REGISTRY "Export build directory (enables external use without install or subdirectory)." OFF)

# These two options are specifically NON MSVC because Visual Studio has its own dedicated project structure, which makes
# copying the shared compile result somewhere else entirely unnecessary.
cmake_dependent_option(VL_SHARED_COPY "On/Off. If true, shared libraries are copied to the root project binary directory after building."
                        OFF "NOT MSVC AND NOT PROJECT_IS_TOP_LEVEL"     ON)
cmake_dependent_option(VL_SHARED_TESTS "On/Off. If true, shared libraries are copied to the test binary directory after building."
                        OFF "NOT MSVC AND PROJECT_IS_TOP_LEVEL"         ON)

# Some relative variables.
set(VL_PROJECT_PATH ${CMAKE_CURRENT_LIST_DIR})
set(VL_CONFIG_HEADER_INCLUDE "${CMAKE_BINARY_DIR}/config/include")
set(VL_CONFIG_HEADER_PATH "${VL_CONFIG_HEADER_INCLUDE}/vl")
string(LENGTH ${VL_PROJECT_PATH} VL_PROJECT_PATH_LEN)
math(EXPR VL_PROJECT_PATH_LEN "${VL_PROJECT_PATH_LEN} + 1")# +1 to omit path separator

project(veritable_lasagna VERSION ${VL_VERSION}
        LANGUAGES C
        DESCRIPTION "Data structures and algorithms."
        HOMEPAGE_URL "https://bitshiftmountain.com")

set(CMAKE_C_STANDARD            11)

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

if(MSVC)
    # Force experimental c11 atomics flag, non negotiable.
    # There is probably a less destructive way to accomplish th
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /experimental:c11atomics")
endif ()

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
# The input path is relative to the calling cmakelists, while
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
set(VL_OBJECT_TARGET    vlasagna_object)
set(VL_STATIC_TARGET    vlasagna)
set(VL_LIBRARY          ${VL_STATIC_TARGET})
set(VL_INSTALL_TARGETS  ${VL_STATIC_TARGET})

add_library(${VL_OBJECT_TARGET} OBJECT ${VL_SOURCES})
target_include_directories  (${VL_OBJECT_TARGET} PRIVATE include/vl ${VL_CONFIG_HEADER_PATH} ${VL_CONFIG_HEADER_INCLUDE})
set_property                (TARGET ${VL_OBJECT_TARGET} PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(${VL_STATIC_TARGET} STATIC $<TARGET_OBJECTS:${VL_OBJECT_TARGET}>)
target_include_directories(${VL_STATIC_TARGET} PRIVATE include/vl ${VL_CONFIG_HEADER_PATH} ${VL_CONFIG_HEADER_INCLUDE})
set_target_properties(${VL_STATIC_TARGET} PROPERTIES OUTPUT_NAME "${VL_STATIC_TARGET}" DEBUG_POSTFIX "d")
add_dependencies(${VL_STATIC_TARGET} ${VL_OBJECT_TARGET})

if(VL_BUILD_SHARED)
    set(VL_SHARED_TARGET vlasagna_shared)
    set(VL_INSTALL_TARGETS ${VL_INSTALL_TARGETS} ${VL_SHARED_TARGET})

    add_library(${VL_SHARED_TARGET} SHARED $<TARGET_OBJECTS:${VL_OBJECT_TARGET}>)
    target_include_directories(${VL_SHARED_TARGET} PRIVATE include/vl ${VL_CONFIG_HEADER_PATH} ${VL_CONFIG_HEADER_INCLUDE})
    set_target_properties(${VL_SHARED_TARGET} PROPERTIES OUTPUT_NAME "${VL_STATIC_TARGET}" DEBUG_POSTFIX "d")
    add_dependencies(${VL_SHARED_TARGET} ${VL_OBJECT_TARGET})

    if(MSVC)
        # MSVC seems to need both the static and shared libraries for linking...
        add_dependencies(${VL_SHARED_TARGET} ${VL_STATIC_TARGET})
    elseif(_WIN32)
        # If not building with MSVC, but still WIN32 (MinGW/Clang/etc), copy to the binary directory.
        set(VL_SHARED_COPY_DIR ${CMAKE_BINARY_DIR})
    else()
        # Otherwise, copy to the 'lib' directory in the build tree.
        set(VL_SHARED_COPY_DIR ${CMAKE_BINARY_DIR}/lib/)
    endif ()

    set(VL_LIBRARY_SHARED ${VL_SHARED_TARGET})
endif ()

export(TARGETS ${VL_INSTALL_TARGETS} NAMESPACE VLasagna:: FILE VLasagnaTargets.cmake)

# If this list is not top-level, this project was included as a subdirectory into another project.
if(NOT PROJECT_IS_TOP_LEVEL)
    set(VL_INCLUDE "${VL_INCLUDE}" PARENT_SCOPE)
    set(VL_LIBRARY "${VL_LIBRARY}" PARENT_SCOPE)

    if(${VL_BUILD_SHARED})
        set(VL_LIBRARY_SHARED ${VL_LIBRARY_SHARED} PARENT_SCOPE)

        if(${VL_SHARED_COPY} AND NOT MSVC)
            # If the shared copy flag is on, copy the shared library to the binary root.
            add_custom_command(TARGET ${VL_LIBRARY_SHARED} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE_NAME:${VL_LIBRARY_SHARED}>
                    ${VL_SHARED_COPY_DIR})
        endif()
    endif()
else()
    add_subdirectory(test)

    if(${VL_SHARED_TESTS} AND ${VL_SHARED_COPY} AND NOT MSVC)
        add_custom_command(TARGET ${VL_LIBRARY_SHARED} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE_NAME:${VL_LIBRARY_SHARED}>
                ${VL_TEST_BINARY_DIR})
    endif ()

    install(TARGETS ${VL_INSTALL_TARGETS} EXPORT VLasagnaTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
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

    configure_package_config_file(VLasagnaConfig.cmake.in VLasagnaConfig.cmake INSTALL_DESTINATION lib/cmake/VLasagna)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VLasagnaConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/VLasagnaConfigVersion.cmake"
            DESTINATION lib/cmake/VLasagna
    )

    export(PACKAGE VLasagna)
endif()
