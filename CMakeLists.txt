cmake_minimum_required(VERSION 3.22.1)

#     ██    ██ ██       █████  ███████  █████   ██████  ███    ██  █████
#     ██    ██ ██      ██   ██ ██      ██   ██ ██       ████   ██ ██   ██
#     ██    ██ ██      ███████ ███████ ███████ ██   ███ ██ ██  ██ ███████
#      ██  ██  ██      ██   ██      ██ ██   ██ ██    ██ ██  ██ ██ ██   ██
#       ████   ███████ ██   ██ ███████ ██   ██  ██████  ██   ████ ██   ██
# -----------------------------------------------------------------------------
# Project: Veritable Lasagna - A Data Structures & Algorithms Library for C
#
# Git Repository:  https://github.com/walkerje/veritable_lasagna
#

set(VL_VERSION_MAJOR 1)
set(VL_VERSION_MINOR 0)
set(VL_VERSION_PATCH 0)
set(VL_VERSION "${VL_VERSION_MAJOR}.${VL_VERSION_MINOR}.${VL_VERSION_PATCH}")

# Define the main project
project(veritable_lasagna
        VERSION ${VL_VERSION}
        LANGUAGES C
        DESCRIPTION "A Data Structures & Algorithms Library for C"
        HOMEPAGE_URL "https://bitshiftmountain.com")

option(VL_STRICT_BUILD "Strict builds. For GCC/CLang, this adds -Werror -Wall -Wextra -Wpedantic.\
                        For MSVC, This adds /W4 /WX /permissive- /Zc:preprocessor" OFF)

# Some relative variables.
set(VL_PROJECT_PATH ${CMAKE_CURRENT_LIST_DIR})
set(VL_CONFIG_HEADER_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/config/include")
set(VL_CONFIG_HEADER_PATH "${VL_CONFIG_HEADER_INCLUDE}/vl")
string(LENGTH ${VL_PROJECT_PATH} VL_PROJECT_PATH_LEN)
math(EXPR VL_PROJECT_PATH_LEN "${VL_PROJECT_PATH_LEN} + 1")# +1 to omit path separator

# Global list to track configured headers for installation
set(VL_CONFIGURED_HEADERS "" CACHE INTERNAL "List of configured headers")

# Include required CMake modules
include(GNUInstallDirs)                 # Standard installation directories
include(CMakePackageConfigHelpers)      # Package config file generation
include(CheckIncludeFile)               # Compilation header checking
include(CheckLibraryExists)             # Compilation library checking
include(cmake/PrimitiveTypes.cmake)     # Platform-specific type definitions
include(cmake/ComponentHelper.cmake)    # Component helper functions
include(cmake/SIMD.cmake)               # SIMD helper

# Set build type macro for configuration
if(CMAKE_CONFIGURATION_TYPES)
    set(VL_BUILD_TYPE MULTI_CONFIG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VL_BUILD_TYPE DEBUG)
else()
    set(VL_BUILD_TYPE RELEASE)
endif()

# -------------------------------------------------------------------------------
# Platform Configuration
# -------------------------------------------------------------------------------

# Endianness detection
if(CMAKE_C_BYTE_ORDER EQUAL BIG_ENDIAN)
    set(VL_BYTE_ORDER 1)  # Big endian
else()
    set(VL_BYTE_ORDER 0)  # Little endian
endif()

# Threading configuration
if(NOT WIN32)
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
endif()

find_package(Threads REQUIRED)

if(${CMAKE_USE_WIN32_THREADS_INIT})
    set(VL_THREADS_WIN32 1)
elseif(${CMAKE_USE_PTHREADS_INIT})
    set(VL_THREADS_PTHREAD 1)
else ()
    message(ERROR "Failed to configure thread configuration. Found \"${CMAKE_THREAD_LIBS_INIT}\"")
endif ()

# System libraries and compiler-specific options
set(VL_SYSTEM_LIBS Threads::Threads)

if(MSVC)
    set(VL_NATIVE_PATH_SEPARATOR \\\\)
    set(VL_EXOTIC_PATH_SEPARATOR /)
    set(VL_FILESYS_WIN32 1)
    set(VL_SOCKET_WIN32 1)
    set(VL_DYNLIB_WIN32 1)

    set(VL_STRICT_OPTIONS /W4 /WX /permissive- /Zc:preprocessor /wd5105)
    set(VL_COMPILE_OPTIONS ${VL_COMPILE_OPTIONS} /experimental:c11atomics)
    set(VL_SYSTEM_LIBS ${VL_SYSTEM_LIBS} ws2_32)
else()
    set(VL_NATIVE_PATH_SEPARATOR /)
    set(VL_EXOTIC_PATH_SEPARATOR \\\\)
    set(VL_SOCKET_POSIX 1)
    set(VL_FILESYS_POSIX 1)

    set(VL_STRICT_OPTIONS -Werror -Wall -Wextra -Wpedantic)
    set(VL_COMPILE_OPTIONS ${VL_COMPILE_OPTIONS} -mcx16)
    set(VL_SYSTEM_LIBS ${VL_SYSTEM_LIBS} atomic)

    check_include_file(dlfcn.h VL_DYNLIB_POSIX)

    if(VL_DYNLIB_POSIX)
        check_library_exists(dl dlopen "" VL_DYNLIB_POSIX_DLIB)

        if(VL_DYNLIB_POSIX_DLIB)
            set(VL_SYSTEM_LIBS ${VL_SYSTEM_LIBS} ${CMAKE_DL_LIBS})
        endif ()
    else ()
        message(ERROR "Failed to platform runtime dynamic link dependency.")
    endif ()
endif()

# -------------------------------------------------------------------------------
# Helper Macros
# -------------------------------------------------------------------------------

#
# vl_add_source - Add a source file to the build and display relative path
#
# @param filepath Path to the source file relative to the current directory
#
macro(vl_add_source filepath)
    set(VL_SOURCE_ABSPATH "${CMAKE_CURRENT_LIST_DIR}/${filepath}")
    string(SUBSTRING ${VL_SOURCE_ABSPATH} ${VL_PROJECT_PATH_LEN} -1 VL_SOURCE_RELPATH)
    message(STATUS "    ${VL_SOURCE_RELPATH}")
    set(VL_SOURCES_LOCAL ${VL_SOURCES_LOCAL} ${VL_SOURCE_RELPATH})
endmacro()

#
# vl_configure_component_header - Configure a header file for a specific component
#
# @param COMPONENT_NAME Name of the component (e.g., "Core", "Math")
# @param filepath Source header template path relative to current directory
# @param outname  Output filename (will be placed in component's configured headers directory)
#
macro(vl_configure_component_header COMPONENT_NAME filepath outname)
    set(VL_HEADER_ABSPATH "${CMAKE_CURRENT_LIST_DIR}/${filepath}")
    string(SUBSTRING ${VL_HEADER_ABSPATH} ${VL_PROJECT_PATH_LEN} -1 VL_HEADER_RELPATH)

    # Create component-specific output path
    string(TOLOWER ${COMPONENT_NAME} component_dir)
    set(VL_COMPONENT_CONFIG_PATH "${VL_CONFIG_HEADER_INCLUDE}")
    set(VL_COMPONENT_VL_PATH "${VL_COMPONENT_CONFIG_PATH}/vl")

    # Ensure the output directory exists
    file(MAKE_DIRECTORY "${VL_COMPONENT_VL_PATH}")

    # Configure the header
    set(VL_OUTPUT_PATH "${VL_COMPONENT_VL_PATH}/${outname}")
    string(SUBSTRING ${VL_OUTPUT_PATH} ${VL_PROJECT_PATH_LEN} -1 VL_HEADER_OUT_RELPATH)

    message(STATUS "    ${VL_HEADER_RELPATH} (Configured --> \"${VL_HEADER_OUT_RELPATH}\")")
    configure_file(${VL_HEADER_ABSPATH} ${VL_OUTPUT_PATH} @ONLY)

    # Track this configured header for installation
    list(APPEND VL_CONFIGURED_HEADERS "${COMPONENT_NAME}:${VL_OUTPUT_PATH}:vl/${outname}")
    set(VL_CONFIGURED_HEADERS ${VL_CONFIGURED_HEADERS} CACHE INTERNAL "List of configured headers")
endmacro()

macro(vl_configure_header filepath outname)
    set(VL_HEADER_ABSPATH "${CMAKE_CURRENT_LIST_DIR}/${filepath}")
    string(SUBSTRING ${VL_HEADER_ABSPATH} ${VL_PROJECT_PATH_LEN} -1 VL_HEADER_RELPATH)
    string(SUBSTRING ${VL_CONFIG_HEADER_PATH} ${VL_PROJECT_PATH_LEN} -1 VL_HEADER_OUT_RELPATH)
    message(STATUS "    ${VL_HEADER_RELPATH} (Configured --> \"${VL_HEADER_OUT_RELPATH}/${outname}\")")
    configure_file(${VL_HEADER_ABSPATH} ${VL_CONFIG_HEADER_PATH}/${outname} @ONLY)

    # Track for global installation
    list(APPEND VL_CONFIGURED_HEADERS "Global:${VL_CONFIG_HEADER_PATH}/${outname}:vl/${outname}")
    set(VL_CONFIGURED_HEADERS ${VL_CONFIGURED_HEADERS} CACHE INTERNAL "List of configured headers")
endmacro()


# -------------------------------------------------------------------------------
# Component Configuration
# -------------------------------------------------------------------------------

# Initialize component list
set(VL_INSTALL_TARGETS)

message(STATUS "Gathering sources...")

# Build Core component
add_subdirectory(src/core)
vl_add_component(Core VL_CORE_SOURCES "include/core" "vl_core")

if(VL_STRICT_BUILD)
    message(STATUS "Strict compilation is ENABLED: ${VL_STRICT_OPTIONS}")
    target_compile_options(Core PRIVATE ${VL_STRICT_OPTIONS})
endif()

# Export targets for use in build tree without installation
export(TARGETS ${VL_INSTALL_TARGETS} NAMESPACE VLasagna:: FILE VLasagnaTargets.cmake)

# -------------------------------------------------------------------------------
# Testing Configuration
# -------------------------------------------------------------------------------

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

# -------------------------------------------------------------------------------
# Installation and Packaging
# -------------------------------------------------------------------------------

# Is VL the top-level project?
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    vl_install_global_configured_headers()

    vl_install_component(Core "include/core")

    # Install library binaries
    install(TARGETS ${VL_INSTALL_TARGETS} EXPORT VLasagnaTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}      # .so/.dylib files
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}      # .lib/.a files
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}      # .dll files
    )

    # Install CMake targets file
    install(EXPORT VLasagnaTargets
            FILE VLasagnaTargets.cmake
            NAMESPACE VLasagna::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vlasagna
    )

    # Install debug symbols for MSVC shared libraries
    if(MSVC AND BUILD_SHARED_LIBS)
        foreach(target ${VL_INSTALL_TARGETS})
            install(FILES $<TARGET_PDB_FILE:${target}>
                    DESTINATION ${CMAKE_INSTALL_BINDIR}
                    OPTIONAL)
        endforeach()
    endif()

    # Generate and install version information
    write_basic_package_version_file(
            VLasagnaConfigVersion.cmake
            VERSION ${VL_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    # Configure and install CMake package config file
    configure_package_config_file(
            VLasagnaConfig.cmake.in
            VLasagnaConfig.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vlasagna
    )

    # Install CMake configuration files
    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/VLasagnaConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/VLasagnaConfigVersion.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/vlasagna
    )

    # Make package available for CMake's find_package command
    export(PACKAGE VLasagna)

endif()