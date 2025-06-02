cmake_minimum_required(VERSION 3.22.1)

# Project: Veritable Lasagna - A Data Structures & Algorithms Library for C
# ------------------------------------------------------------------------------

# Read metadata from vcpkg.json to ensure version consistency
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json" VL_VCPKG_JSON_CONTENTS)
string(JSON VL_VCPKG_VERSION GET ${VL_VCPKG_JSON_CONTENTS} version)
string(JSON VL_VCPKG_DESCRIPTION GET ${VL_VCPKG_JSON_CONTENTS} description)
string(JSON VL_VCPKG_HOMEPAGE GET ${VL_VCPKG_JSON_CONTENTS} homepage)

# Installs a git hook to .git that automatically propagates version changes
include(cmake/InstallHooks.cmake)

# Parse version components
string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" VERSION_MATCHES ${VL_VCPKG_VERSION})
set(VL_VERSION_MAJOR ${CMAKE_MATCH_1})
set(VL_VERSION_MINOR ${CMAKE_MATCH_2})
set(VL_VERSION_PATCH ${CMAKE_MATCH_3})
set(VL_VERSION ${VL_VCPKG_VERSION})

# Some relative variables.
set(VL_PROJECT_PATH ${CMAKE_CURRENT_LIST_DIR})
set(VL_CONFIG_HEADER_INCLUDE "${CMAKE_BINARY_DIR}/config/include/")
set(VL_CONFIG_HEADER_PATH "${VL_CONFIG_HEADER_INCLUDE}vl")
string(LENGTH ${VL_PROJECT_PATH} VL_PROJECT_PATH_LEN)
math(EXPR VL_PROJECT_PATH_LEN "${VL_PROJECT_PATH_LEN} + 1")# +1 to omit path separator

# Define the main project
project(veritable_lasagna 
        VERSION ${VL_VERSION}
        LANGUAGES C
        DESCRIPTION "${VL_VCPKG_DESCRIPTION}"
        HOMEPAGE_URL "${VL_VCPKG_HOMEPAGE}")

# Include required CMake modules
include(GNUInstallDirs)            # Standard installation directories
include(CMakePackageConfigHelpers) # Package config file generation
include(cmake/PrimitiveTypes.cmake) # Platform-specific type definitions

# Set build type macro for configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
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
endif()

# System libraries and compiler-specific options
set(VL_SYSTEM_LIBS Threads::Threads)

if(MSVC)
    set(VL_COMPILE_OPTIONS ${VL_COMPILE_OPTIONS} /experimental:c11atomics)
else()
    set(VL_SYSTEM_LIBS ${VL_SYSTEM_LIBS} atomic)
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
    set(VL_SOURCES ${VL_SOURCES} ${VL_SOURCES_LOCAL} PARENT_SCOPE)
endmacro()

#
# vl_configure_header - Configure a header file with template variables
#
# @param filepath Source header template path relative to current directory
# @param outname  Output filename (will be placed in configured headers directory)
#
macro(vl_configure_header filepath outname)
    set(VL_HEADER_ABSPATH "${CMAKE_CURRENT_LIST_DIR}/${filepath}")
    string(SUBSTRING ${VL_HEADER_ABSPATH} ${VL_PROJECT_PATH_LEN} -1 VL_HEADER_RELPATH)
    string(SUBSTRING ${VL_CONFIG_HEADER_PATH} ${VL_PROJECT_PATH_LEN} -1 VL_HEADER_OUT_RELPATH)
    message(STATUS "    ${VL_HEADER_RELPATH} (Configured --> \"${VL_HEADER_OUT_RELPATH}/${outname}\")")
    configure_file(${VL_HEADER_ABSPATH} ${VL_CONFIG_HEADER_PATH}/${outname} @ONLY)
endmacro()

# -------------------------------------------------------------------------------
# Source Collection and Target Configuration
# -------------------------------------------------------------------------------

message(STATUS "Gathering sources...")
add_subdirectory(src)

# Configure paths and targets
set(VL_INCLUDE          "${CMAKE_CURRENT_LIST_DIR}/include/" "${VL_CONFIG_HEADER_INCLUDE}")
set(VL_CORE_TARGET      Core)
set(VL_INSTALL_TARGETS  ${VL_CORE_TARGET})

# Create the main library target
add_library(${VL_CORE_TARGET} ${VL_SOURCES})

# Configure include directories
target_include_directories(${VL_CORE_TARGET} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/vl>
    $<INSTALL_INTERFACE:include>
    PRIVATE ${VL_CONFIG_HEADER_PATH} ${VL_CONFIG_HEADER_INCLUDE})

# Set library properties
set_target_properties(${VL_CORE_TARGET} PROPERTIES
    OUTPUT_NAME   "vl_core"  # Actual library filename
    DEBUG_POSTFIX "d"        # Append 'd' to Debug builds
)

# Configure compile options and dependencies
target_compile_features(${VL_CORE_TARGET} PUBLIC c_std_11)
target_compile_options(${VL_CORE_TARGET} PUBLIC "${VL_COMPILE_OPTIONS}")
target_link_libraries(${VL_CORE_TARGET} PUBLIC "${VL_SYSTEM_LIBS}")

# Create an alias for use in this project
add_library(VLasagna::Core ALIAS ${VL_CORE_TARGET})

# -------------------------------------------------------------------------------
# Shared Library Configuration (when BUILD_SHARED_LIBS is ON)
# -------------------------------------------------------------------------------

if(BUILD_SHARED_LIBS)
    # Configure visibility and symbol export for shared libraries
    target_compile_definitions(${VL_CORE_TARGET}
        PRIVATE VL_BUILDING_SHARED  # Used internally when building the library
        INTERFACE VL_USING_SHARED   # Used by consumers of the shared library
    )

    # Configure visibility settings for shared libraries
    set_target_properties(${VL_CORE_TARGET} PROPERTIES
        C_VISIBILITY_PRESET hidden       # Hide symbols by default
        VISIBILITY_INLINES_HIDDEN ON      # Hide inline functions
        POSITION_INDEPENDENT_CODE ON      # Generate position-independent code
    )
endif()

# Export targets for use in build tree without installation
export(TARGETS ${VL_INSTALL_TARGETS} NAMESPACE VLasagna:: FILE VLasagnaTargets.cmake)

# -------------------------------------------------------------------------------
# Testing Configuration
# -------------------------------------------------------------------------------

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

# Install debug symbols for MSVC shared libraries
if(MSVC AND BUILD_SHARED_LIBS)
    install(FILES $<TARGET_PDB_FILE:${VL_CORE_TARGET}>
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            OPTIONAL)
endif()

# -------------------------------------------------------------------------------
# Installation and Packaging
# -------------------------------------------------------------------------------

# Install library binaries
install(TARGETS ${VL_INSTALL_TARGETS} EXPORT VLasagnaTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}      # .so/.dylib files
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}      # .lib/.a files
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}      # .dll files
)

# Install header files
install(DIRECTORY ${VL_INCLUDE} 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install CMake targets file
install(EXPORT VLasagnaTargets
    FILE VLasagnaTargets.cmake
    NAMESPACE VLasagna::
    DESTINATION lib/cmake/vlasagna
)

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
    INSTALL_DESTINATION lib/cmake/vlasagna
)

# Install CMake configuration files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/VLasagnaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VLasagnaConfigVersion.cmake"
    DESTINATION lib/cmake/vlasagna
)

# Make package available for CMake's find_package command
export(PACKAGE VLasagna)