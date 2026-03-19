\page man_getting_started Getting Started

This guide covers the basics of getting Veritable Lasagna into your project, including repository location, dependencies, and various integration options.

## Table of Contents
- [GitHub Repository](#github-repository)
- [Requirements](#requirements)
- [Installation Options](#installation-options)
  - [Conan Package Manager](#1-conan-package-manager)
  - [Embed as Subdirectory](#2-embed-as-subdirectory)
  - [Manual Build and Install](#3-manual-build-and-install)

## GitHub Repository
The official source code for Veritable Lasagna is hosted on GitHub:
[https://github.com/walkerje/veritable_lasagna.git](https://github.com/walkerje/veritable_lasagna.git)

You can clone the repository using:
```bash
git clone https://github.com/walkerje/veritable_lasagna.git
```

## Requirements
To build and use Veritable Lasagna, ensure your environment meets these requirements:
- **C Compiler:** A C11-compatible compiler (GCC, Clang, MSVC, etc.)
- **CMake:** Version 3.22.1 or higher
- **Conan:** Version 2.0+ (Optional, for package management)
- **GoogleTest:** Required for building and running the test suite
- **Doxygen & Graphviz:** Required for generating the full documentation

## Installation Options

### 1. Conan Package Manager
Veritable Lasagna can be integrated using [Conan](https://conan.io/). To build the package and run the built-in test recipe:
```bash
cd veritable_lasagna
conan create .
```
In your own `CMakeLists.txt`, you can then use:
```cmake
find_package(VLasagna REQUIRED)
target_link_libraries(your_target VLasagna::Core)
```

### 2. Embed as Subdirectory
The easiest way to integrate Veritable Lasagna without external package managers is to add it as a subdirectory or a git submodule.
```bash
git submodule add https://github.com/walkerje/veritable_lasagna.git
```
Then, in your `CMakeLists.txt`:
```cmake
add_subdirectory(veritable_lasagna)
target_link_libraries(your_target VLasagna::Core)
```

### 3. Manual Build and Install
For a standard system-wide installation:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
cmake --build . --target install
```
After installation, you can use `find_package(VLasagna REQUIRED)` as shown in the Conan section.

## Verification
To verify your installation, you can build and run the included tests:
```bash
cmake -DBUILD_TESTING=ON ..
cmake --build .
cd test && ctest
```
