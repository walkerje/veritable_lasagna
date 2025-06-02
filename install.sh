#!/usr/bin/env bash

set -e  # Exit on error

# Default values
INSTALL_DIR="/tmp/vl_install_$$"  # Using $$ (PID) for uniqueness
BUILD_TYPES=""  # Empty means default (Release)
SUDO_CMD=""

# Function to clean up on exit
cleanup() {
    if [ -d "$INSTALL_DIR" ]; then
        echo "Cleaning up temporary files..."
        rm -rf "$INSTALL_DIR"
    fi
}

# Function to handle errors
error_handler() {
    echo "Error occurred on line $1"
    cleanup
    exit 1
}

# Function to build and install for a specific configuration
build_and_install() {
    local build_type=$1
    local build_dir="build_${build_type,,}"  # lowercase
    
    echo "==============================================="
    echo "Building ${build_type} configuration..."
    echo "==============================================="
    
    # Create and enter build directory
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    # Configure
    echo "Configuring ${build_type} build..."
    cmake -DCMAKE_BUILD_TYPE="$build_type" ../..
    
    # Build
    echo "Building ${build_type}..."
    cmake --build . --config "$build_type" -j "$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)"
    
    # Install
    echo "Installing ${build_type}..."
    $SUDO_CMD cmake --install .
    
    cd ..
}

# Set up error handling
trap 'error_handler ${LINENO}' ERR
trap cleanup EXIT

# Print banner
echo "==============================================="
echo "Veritable Lasagna Installer"
echo "==============================================="

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-type=*)
            BUILD_TYPES="${1#*=}"
            ;;
        --all)
            BUILD_TYPES="Debug;Release"
            ;;
        --no-sudo)
            SUDO_CMD=""
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  --build-type=TYPE    Build type(s): Debug and/or Release"
            echo "                       Use semicolon for multiple: Debug;Release"
            echo "  --all               Build both Debug and Release (shorthand)"
            echo "  --no-sudo           Don't use sudo for installation"
            echo "  --help              Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0 --build-type=Debug"
            echo "  $0 --build-type=Debug;Release"
            echo "  $0 --all"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
    shift
done

# Set default build type if none specified
if [ -z "$BUILD_TYPES" ]; then
    BUILD_TYPES="Release"
fi

# Check if sudo is needed (only if we can detect the install path)
if [ -z "$SUDO_CMD" ]; then
    # Try to get CMake's install prefix
    CMAKE_INSTALL_PREFIX=$(cmake -N -L 2>/dev/null | grep CMAKE_INSTALL_PREFIX | cut -d '=' -f2 || echo "")
    if [ -n "$CMAKE_INSTALL_PREFIX" ] && [ ! -w "$CMAKE_INSTALL_PREFIX" ]; then
        if command -v sudo >/dev/null 2>&1; then
            SUDO_CMD="sudo"
        else
            echo "Error: Installation requires root privileges, but sudo is not available"
            exit 1
        fi
    fi
fi

# Check requirements
echo "Checking requirements..."
command -v cmake >/dev/null 2>&1 || { echo "cmake is required but not installed. Aborting." >&2; exit 1; }
command -v git >/dev/null 2>&1 || { echo "git is required but not installed. Aborting." >&2; exit 1; }

# Create temporary directory
echo "Creating temporary directory..."
mkdir -p "$INSTALL_DIR"
cd "$INSTALL_DIR"

# Clone repository
echo "Cloning repository..."
git clone --depth 1 https://github.com/walkerje/veritable_lasagna.git .

# Create builds directory
mkdir -p builds
cd builds

# Build and install each configuration
IFS=';' read -ra BUILD_ARRAY <<< "$BUILD_TYPES"
for build_type in "${BUILD_ARRAY[@]}"; do
    build_and_install "$build_type"
done

echo "==============================================="
echo "Installation completed successfully!"
echo "Installed configurations: $BUILD_TYPES"
echo "Installation path can be found using:"
echo "cmake --system-information | grep CMAKE_INSTALL_PREFIX"
echo "==============================================="

# Cleanup is handled by the trap