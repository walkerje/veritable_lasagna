![Veritable Lasagna](docs/image/vl_logo.svg)

# Veritable Lasagna (v0.13.10)

> A Data Structures & Algorithms Library for C

![Written in C11](https://img.shields.io/badge/C11-gray?style=for-the-badge&logo=C&logoColor=orange&labelColor=black)
[![Built with CMake](https://img.shields.io/badge/Built%20with%20CMake-gray?style=for-the-badge&logo=CMake&logoColor=orange&labelColor=black)](https://cmake.org/)
[![Tested with GoogleTest](https://img.shields.io/badge/Testing%20With%20GTest-gray?style=for-the-badge&logo=googlesearchconsole&logoColor=orange&labelColor=black)](https://github.com/google/googletest)
[![Support development](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-ffdd00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=orange&labelColor=black)](https://www.buymeacoffee.com/walkerje)

![Test Status](https://github.com/walkerje/veritable_lasagna/actions/workflows/build_and_test.yml/badge.svg)

## Table of Contents
- [Overview](#introduction)
- [Features](#features)
- [Roadmap](#roadmap-to-v100)
- [Installation](#quick-start)
  - [Requirements](#requirements)
  - [Option 1: Package Installation](#option-1-recommended-automated-build--install-from-repo)
  - [Option 2: Embed as Subdirectory](#option-2-embed-as-subdirectory)
  - [Option 3: Manual Build](#option-3-manual-build-and-install)
  - [Configuration Options](#configuration-options)
- [Usage Examples](#code-samples)
- [Testing](#building-and-running-tests)
- [Documentation](#generating-documentation)
- [License](#license)

---

## Introduction

**Veritable Lasagna** (or **VL** for short) is a cross-platform library written in C11 that provides
efficient implementations of common memory allocators, data structures,
and algorithms. The API is inspired by the C++ Standard Template Library (STL), making it intuitive
for developers familiar with C++, while maintaining pure C compatibility.

Key design principles:

- **Minimum Dependencies**: Relies almost entirely on the C standard library
- **No Macro Templates**: Clean API without macro-based generic programming
- **Comprehensive Testing**: Rigorous test suite ensures consistent behavior

## Features

Veritable Lasagna provides a robust set of components:

## Roadmap to v1.0.0

This roadmap outlines what is needed for Veritable Lasagna to be considered feature-complete. After v1.0.0, new features will be released in minor versions, while major releases will introduce significant architectural changes.

> A Data Structures & Algorithms Library for C

All features must be cross-platform and include comprehensive test suites.

### Memory Management
- ✅ Memory blocks with metadata (`vl_memory`)
- ✅ Memory Pools (`vl_pool`, `vl_async_pool`)
- ✅ Arena Allocator (`vl_arena`)
- ✅ Data (De)Serialization (`vl_msgpack`)

### Data Structures
- ✅ Buffer (`vl_buffer`)
- ✅ Stack (`vl_stack`)
- ✅ Queue (`vl_queue`)
- ✅ Deque (`vl_deque`)
- ✅ Linked List (`vl_linked_list`)
- ✅ Ordered Set (`vl_set`)
- ✅ Hash Table (`vl_hashtable`)

### Algorithms
- ✅ Pseudo-random number generator (`vl_rand`)
- ✅ Hashing (`vl_hash`)
- ✅ Comparisons (`vl_compare`)
- ✅ Sorting
  - ✅ Available to `vl_memory` and `vl_linked_list`
  - ✅ Implicit to `vl_set`
- ✅ Search
  - ✅ Sorted (`vl_memory`)
  - ✅ Unsorted (`vl_memory` and `vl_linked_list`)
  - ✅ Implicit to `vl_set` and `vl_hashtable`

### Async
- Primitives
  - ✅ Threads (`vl_thread`)
  - ✅ Atomic Types (`vl_atomic`)
  - ✅ Mutex (`vl_mutex`)
  - ✅ SRWLock (`vl_srwlock`)
  - ✅ Conditional Variable (`vl_condition`)
  - ✅ Semaphore (`vl_semaphore`)
- Data Structures
  - ✅ Lockless Async Memory Pool (`vl_async_pool`)
  - ✅ Lockless Async Queue (`vl_async_queue`)

### Filesystem
- ❌ Directory listing
- ❌ Path handling

### Other
- ❌ Runtime Dynamic Library Handling

## Code Samples

The following examples demonstrate how to use some of the most common data structures in Veritable Lasagna.

### Linked List Example

Create, populate, and iterate through a linked list of integers:

```c
#include <stdlib.h>
#include <stdio.h>
#include <vl/vl_linked_list.h>

int main(int argc, const char** argv) {
    // Create a new list of integers
    vl_list* list = vlListNew(sizeof(int));

    // Add 10 integers to the list
    for(int i = 0; i < 10; i++) {
        const int listValue = i;
        vlListPushBack(list, &listValue);
    }

    // Iterate through the list and print each value
    VL_LIST_FOREACH(list, curIter) {
        const int val = *((int*)vlListSample(list, curIter));
        printf("Value: %d\n", val);  // Added newline
    }

    // Clean up
    vlListDelete(list);
    return EXIT_SUCCESS;
}
```
### Hash Table Example

Create a hash table mapping character names to their bank balances:

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vl/vl_hashtable.h>

int main(int argc, const char** argv) {
    // Create a new hash table using string hashing
    vl_hashtable* wealthTable = vlHashTableNew(vlHashString);

    // Sample data
    const int numEntries = 5;
    const char* keys[] = {
        "McLovin",
        "Supercop",
        "Napoleon",
        "Terminator",
        "Loch Ness Monster"
    };
    const float values[] = {12.05f, 5.84f, 910.63f, 711.42f, 3.50f};

    // Populate the hash table
    for(int i = 0; i < numEntries; i++) {
        const char* key = keys[i];
        const float value = values[i];
        const int keyLen = strlen(key) + 1; // +1 to preserve null terminator
                                            // not strictly necessary, but somewhat handy

        // Insert the key and claim memory for the value
        const vl_hash_iter iter = vlHashTableInsert(wealthTable, key, keyLen, sizeof(float));

        // Assign the value to the memory owned by the table
        *((float*)vlHashTableSampleValue(wealthTable, iter, NULL)) = value;
    }

    // Iterate through all entries and print them
    VL_HASHTABLE_FOREACH(wealthTable, curIter) {
        // Get key and value sizes in bytes
        size_t keyLen, valLen;

        // Access the key and value data
        const char* key = (const char*)vlHashTableSampleKey(wealthTable, curIter, &keyLen);
        const float val = *((float*)vlHashTableSampleValue(wealthTable, curIter, &valLen));

        printf("%s has %.2f$ in the bank!\n", key, val);
        // If we didn't preserve the null terminator, length can be stated explicitly:
        // printf("%.*s has %.2f$ in the bank!", (int)keyLen, key, val);
    }

    // Clean up
    vlHashTableDelete(wealthTable);
    return EXIT_SUCCESS;
}
```
### Ordered Set Example

Create a set that automatically sorts integers:

```c
#include <stdlib.h>
#include <stdio.h>
#include <vl/vl_set.h>

int main(int argc, const char** argv) {
    // Sample data - unsorted integers
    const int set_size = 10;
    const int data[] = {6, 2, 9, 1, 3, 0, 4, 7, 5, 8};

    // Create a new set of integers using the integer comparison function
    vl_set* set = vlSetNew(sizeof(int), vlCompareInt);

    // Insert all values into the set (they will be automatically ordered)
    for(int i = 0; i < set_size; i++) {
        vlSetInsert(set, (void*)&(data[i]));
    }

    // Print the size and all values in the set (in sorted order)
    printf("Sorted %d elements:\n", vlSetSize(set));
    VL_SET_FOREACH(set, curIter) {
        const int value = *((int*)vlSetSample(curIter));
        printf("\t%d\n", value);
    }

    // Clean up
    vlSetDelete(set);
    
    return EXIT_SUCCESS;
}
```

# Quick Start

## Requirements

To build and use Veritable Lasagna, you'll need:

- A C11-compatible compiler (GCC, Clang, MSVC, etc.)
- CMake 3.22.1 or higher
- For testing: GoogleTest
- For documentation: Doxygen and Graphviz

## Installation Options

There are three ways to incorporate Veritable Lasagna into your project:

## Option 1 (Recommended): Automated Build & Install from Repo

### Install with [vcpkg](https://vcpkg.io/) (Cross-Platform)
```shell
git clone https://github.com/walkerje/veritable_lasagna.git
vcpkg install --overlay-ports=.\veritable_lasagna\vcpkg veritable-lasagna
```

### Bash (Linux/MSYS/Cygwin/etc) (See [install.sh](install.sh) first!)
```shell
wget -O - https://raw.githubusercontent.com/walkerje/veritable_lasagna/refs/heads/main/install.sh | bash
```

You can reference the installation in your project by using `find_package` in your own
`CMakeLists.txt`.

```CMake
# 
# Your project setup...
#

find_package(VLasagna REQUIRED)

#
# Target setup...
#

target_link_libraries(my_target_name VLasagna::Core)
```

## Option 2: Embed as Subdirectory

Start by cloning this project into your project directory or adding it as a git submodule.

* Submodule:
  ```bash
  git submodule add https://github.com/walkerje/veritable_lasagna.git
  git submodule update --init
  ```
* Clone:
  ```bash
  git clone https://github.com/walkerje/veritable_lasagna.git
  ```

Then, somewhere in your own `CMakeLists.txt`, have the following:

```CMake
# 
# Your project setup...
#

add_subdirectory(veritable_lasagna)

#
# Target setup...
#

target_link_libraries(my_target_name VLasagna::Core)
```

## Option 3: Manual Build and Install

Start by cloning this repo to your disk and moving into its directory.

```bash
git clone https://github.com/walkerje/veritable_lasagna.git
cd veritable_lasagna
```

Now create a build directory and use CMake to configure the build.

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

Finally, build the library.

```bash
cmake --build .
```

Installing this package to your system is as simple specifying the `install` target.

```bash
cmake --build . --target install
```

See [Option 1](#option-1-recommended-automated-build--install-from-repo) for a snippet on finding the installed package
from your `CMakeLists.txt`

## Configuration Options

These are the primary configuration options relevant to the library. Many of these are ubiquitous across CMake,
but they are described here nonetheless.

| Argument            | Type   | Default              | Description                                                                                                                                                                                                                                                       |
|---------------------|--------|----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `CMAKE_BUILD_TYPE`  | STRING | `Toolchain Specific` | Specifies the build configuration type. Common values: <br>• `Debug` - No optimizations, includes debug info<br>• `Release` - Full optimizations, no debug info<br>• `RelWithDebInfo` - Full optimizations with debug info<br>• `MinSizeRel` - Size optimizations |
| `BUILD_SHARED_LIBS` | BOOL   | `OFF`                | Global flag affecting the how the library is built: <br>• `ON` - Libraries are built as shared/dynamic (DLL/SO)<br>• `OFF` - Libraries are built as static (LIB/A)                                                                                                |
| `BUILD_TESTING`     | BOOL   | `OFF`                | CTest module flag that controls test building:<br>• `ON` - Configure to build tests via CTest and GTest <br>• `OFF` - Skips building tests                                                                                                                        |



## Building and Running Tests

Veritable Lasagna includes comprehensive test suites powered by [GoogleTest](https://github.com/google/googletest) and [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html).

To build and run the tests:

```bash
# Clone the repository if you haven't already
git clone https://github.com/walkerje/veritable_lasagna.git
cd veritable_lasagna 

# Create build directory and configure with testing enabled
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..

# Build the library and tests
cmake --build .

# Run the tests
cd test && ctest
```

The test results will show you which tests passed and failed, helping ensure the library works correctly on your system.

## Generating Documentation

The project documentation is generated using [Doxygen](https://www.doxygen.nl/) with [Graphviz](https://graphviz.org/) integration for diagrams. The documentation uses a [modern theme](https://github.com/jothepro/doxygen-awesome-css) that's included as a submodule.

### Prerequisites

- **Doxygen** - Documentation generator ([download](https://www.doxygen.nl/download.html))
- **Graphviz** - Graph visualization software ([download](https://graphviz.org/download/))

To generate the documentation:

```bash
# Clone the repository if you haven't already
git clone https://github.com/walkerje/veritable_lasagna.git
cd veritable_lasagna

# Initialize the documentation theme submodule
git submodule update --init

# Generate the documentation
cd docs
doxygen
```

The generated documentation will be available in the `docs/html` directory. Open `index.html` in your browser to view it.

## License

Veritable Lasagna is available under the MIT License. See the [LICENSE](LICENSE) file for details.

[Back to Top](#veritable-lasagna-v0.13.10)