![Veritable Lasagna](docs/image/vl_logo.svg)

# Veritable Lasagna (v1.0.0)

> A Data Structures & Algorithms Library for C

![Written in C11](https://img.shields.io/badge/C11-gray?style=for-the-badge&logo=C&logoColor=orange&labelColor=black)
[![Built with CMake](https://img.shields.io/badge/Built%20with%20CMake-gray?style=for-the-badge&logo=CMake&logoColor=orange&labelColor=black)](https://cmake.org/)
[![Tested with GoogleTest](https://img.shields.io/badge/Testing%20With%20GTest-gray?style=for-the-badge&logo=googlesearchconsole&logoColor=orange&labelColor=black)](https://github.com/google/googletest)
[![Support development](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-ffdd00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=orange&labelColor=black)](https://www.buymeacoffee.com/walkerje)

![Test Status](https://github.com/walkerje/veritable_lasagna/actions/workflows/build_and_test.yml/badge.svg)

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Code Samples](#code-samples)
- [Quick Start](#quick-start)
  - [Requirements](#requirements)
  - [Installation Options](#installation-options)
    - [Option 1: Conan Package Manager](#option-1-conan-package-manager)
    - [Option 2: Embed as Subdirectory](#option-2-embed-as-subdirectory)
    - [Option 3: Manual Build](#option-3-manual-build-and-install)
  - [Configuration Options](#configuration-options)
- [Testing](#building-and-running-tests)
- [Documentation](#generating-documentation)
- [Contributing](#contributing)
- [License](#license)

---

## Introduction

**Veritable Lasagna** (or **VL** for short) is a cross-platform library written in C11 that provides
efficient implementations of common memory allocators, data structures,
and algorithms.
Key design principles:

- **Minimal Dependencies**: Relies almost entirely on the C standard library.
  - Also depends on POSIX/Win32 for filesystem operations and thread-related functionality.
- **Cross-Platform**: Fully supports POSIX (Linux, macOS) and Win32 systems.
- **Performant**: Implemented in C11 and optimized for speed and memory efficiency.
- **Comprehensive Testing**: A fairly comprehensive test suite ensures consistent behavior across platforms.
- **Stable ABI**: The ABI is guaranteed to remain stable between major version releases.


### A note from the author
    This project started as the culmination of 6 years of personal interest in C programming. It serves first and foremost
    as a basis for my own applications, research, and education. It is a toolbox of my own code gathered from a collection
    of personal projects that will never see the light of day, and documented education resources. It is presented here in
    adherence to a set of design goals to make this a cohesive and comprehensive resource.
    
    This project was a stress-reliever, like what some people find in knitting or cooking. It served as a way to hone
    my skills in C programming, cross-platform development, and problem-solving. I worked on this project while raising my
    family, working overtime at my restaurant job, and working on building my career. I hope you find it as useful
    and enjoyable to use as I found it to write and test.
    
    Don't let life steal your interest in what you do not know, your joy in what you do, nor your strength to distinguish the two.
    -JWS

## Features

Veritable Lasagna provides a robust set of components:

### Memory Management
- ✅ Memory blocks with metadata (`vl_memory`)
- ✅ Memory Pools (`vl_pool`, `vl_async_pool`)
- ✅ Arena Allocator (`vl_arena`)
- ✅ Data (De)serialization (`vl_msgpack`)
- ✅ Extensible Stream API (`vl_stream`)
  - See also `vl_stream_filesys` and `vl_stream_memory`

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
  - ✅ Available for `vl_memory` and `vl_linked_list`
  - ✅ Implicit for `vl_set`
- ✅ Search
  - ✅ Sorted (`vl_memory`)
  - ✅ Unsorted (`vl_memory` and `vl_linked_list`)
  - ✅ Implicit for `vl_set` and `vl_hashtable`
- ✅ Math Components (`vl_algo`)
  - ✅ Least Common Multiple (LCM)
  - ✅ Greatest Common Divisor (GCD)
  - ✅ Population Count
  - ✅ Count Leading/Trailing Zeros

### Async Primitives & Structures
- ✅ Threads (`vl_thread`)
- ✅ Atomic Types (`vl_atomic`)
- ✅ Mutex (`vl_mutex`)
- ✅ SRWLock (`vl_srwlock`)
- ✅ Condition Variable (`vl_condition`)
- ✅ Semaphore (`vl_semaphore`)
- ✅ Lockless Async Memory Pool (`vl_async_pool`)
- ✅ Lockless Async Queue (`vl_async_queue`)

### Filesystem
- ✅ Directory iteration (flat and recursive) (`vl_filesys`)
- ✅ Path handling (`vl_filesys`)
  - UTF-8 enforced across platforms

### Additional Components
- ✅ Runtime Dynamic Library Handling (`vl_dynlib`)
- ✅ SIMD Intrinsics with Runtime Dispatch (`vl_simd`)
  - x86_64: SSE2, AVX2
  - ARM: NEON
- ✅ 16-Bit Floating-Point Support (`vl_half`)
- ✅ 4-bit Integer Support (`vl_nibble`)
- ✅ ANSI Terminal Control Definitions (`vl_ansi_term`)
- ✅ Thread-Safe Logging (`vl_log`)
  - Includes file rotation and log level filtering.
- ✅ Cross-Platform Socket Networking Abstraction (`vl_socket`)
  - ✅ IPv4 & IPv6 Support
  - ✅ TCP (Stream) & UDP (Datagram)
  - ✅ Blocking & Non-blocking I/O

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
    vl_linked_list* list = vlListNew(sizeof(int));

    // Add 10 integers to the list
    for(int i = 0; i < 10; i++) {
        const int listValue = i;
        vlListPushBack(list, &listValue);
    }

    // Iterate through the list and print each value
    VL_LIST_FOREACH(list, curIter) {
        const int val = *((int*)vlListSample(list, curIter));
        printf("Value: %d\n", val);
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
        vl_memsize_t keyLen, valLen;

        // Access the key and value data
        const char* key = (const char*)vlHashTableSampleKey(wealthTable, curIter, &keyLen);
        const float val = *((float*)vlHashTableSampleValue(wealthTable, curIter, &valLen));

        printf("%s has %.2f$ in the bank!\n", key, val);
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
    printf("Sorted %d elements:\n", (int)vlSetSize(set));
    VL_SET_FOREACH(set, curIter) {
        const int value = *((int*)vlSetSample(set, curIter));
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

- A C11-compatible compiler (GCC, Clang, or MSVC)
- CMake 3.22.1 or higher
- [Conan 2.0+](https://conan.io/) (Optional, for package management)
- Doxygen and Graphviz (Optional, for documentation)

## Installation Options

### Option 1: Conan Package Manager (Recommended)

Veritable Lasagna is designed to be easily integrated using [Conan](https://conan.io/). While it's not yet on Conan Center, you can build it locally from the provided recipe.

To build the package and run the integration tests:

```shell
git clone https://github.com/walkerje/veritable_lasagna.git
cd veritable_lasagna
conan create .
```

This will:
1. Export the recipe to your local cache.
2. Build the library from source.
3. Verify the installation using the test package in `test/package`.

To use it in your project, add the following to your `CMakeLists.txt`:

```cmake
find_package(VLasagna REQUIRED)
target_link_libraries(your_target VLasagna::Core)
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

Installing the package to your system is as simple as specifying the `install` target:

```bash
cmake --build . --target install
```

See [Option 1](#option-1-conan-package-manager-recommended) for a snippet on finding the installed package from your `CMakeLists.txt`.

## Configuration Options

These are the primary configuration options relevant to the library. Many of these are ubiquitous across CMake,
but they are described here nonetheless.

| Argument            | Type   | Default              | Description                                                                                                                                                                                                                                                       |
|---------------------|--------|----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `CMAKE_BUILD_TYPE`  | STRING | `Release`            | Specifies the build configuration type. Common values: <br>• `Debug` - No optimizations, includes debug info<br>• `Release` - Full optimizations, no debug info<br>• `RelWithDebInfo` - Full optimizations with debug info<br>• `MinSizeRel` - Size optimizations |
| `BUILD_SHARED_LIBS` | BOOL   | `OFF`                | Global flag affecting how the library is built: <br>• `ON` - Libraries are built as shared/dynamic (DLL/SO)<br>• `OFF` - Libraries are built as static (LIB/A)                                                                                                    |
| `BUILD_TESTING`     | BOOL   | `OFF`                | CTest module flag that controls test building:<br>• `ON` - Configure to build tests via CTest and GoogleTest <br>• `OFF` - Skips building tests                                                                                                                     |
| `VL_STRICT_BUILD`   | BOOL   | `OFF`                | Enables strict compilation. <br>• GCC/Clang: `-Werror -Wall -Wextra -Wpedantic` <br>• MSVC: `/W4 /WX /permissive- /Zc:preprocessor`                                                                                                                               |

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

### Testing & Coverage Build Options (GNU/Clang)
| Option               | Description                        | Default | Purpose                                                                                                                               |
|----------------------|------------------------------------|---------|---------------------------------------------------------------------------------------------------------------------------------------|
| `VL_ENABLE_COVERAGE` | Enables code coverage reporting    | `OFF`   | Generates coverage reports showing which lines of code are executed during tests. Requires [`gcovr`](https://github.com/gcovr/gcovr). |
| `VL_ENABLE_ASAN`     | Enables AddressSanitizer           | `OFF`   | Detects memory errors like buffer overflows, use-after-free, memory leaks                                                             |
| `VL_ENABLE_UBSAN`    | Enables UndefinedBehaviorSanitizer | `OFF`   | Detects undefined behavior like integer overflow, null pointer dereference                                                            |
| `VL_ENABLE_TSAN`     | Enables ThreadSanitizer            | `OFF`   | Detects data races and other threading issues                                                                                         |

Usage example:
``` bash
cmake -DBUILD_TESTING=ON -DVL_ENABLE_ASAN=ON -DVL_ENABLE_COVERAGE=ON
```
Note:
- These options are only available with GNU and Clang compilers
- Only one sanitizer should be enabled at a time (ASAN, TSAN, or UBSAN)
- Coverage reporting works best with Debug builds
- When using sanitizers, it's recommended to build in Debug mode for better error reporting

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

## Contributing

Please see [CONTRIBUTING.md](CONTRIBUTING.md) for details on the code of conduct for this repository.

## License

Veritable Lasagna is available under the MIT License. See the [LICENSE](LICENSE) file for details.

[Back to Top](#table-of-contents)