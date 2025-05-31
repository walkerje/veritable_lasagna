![Veritable Lasagna](docs/image/vl_logo.svg)

![Written in C11](https://img.shields.io/badge/C11-gray?style=for-the-badge&logo=C&logoColor=orange&labelColor=black)
[![Built with CMake](https://img.shields.io/badge/Built%20with%20CMake-gray?style=for-the-badge&logo=CMake&logoColor=orange&labelColor=black)](https://cmake.org/)
[![Tested with GoogleTest](https://img.shields.io/badge/Testing%20With%20GTest-gray?style=for-the-badge&logo=googlesearchconsole&logoColor=orange&labelColor=black)](https://github.com/google/googletest)
[![Support development](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-ffdd00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=orange&labelColor=black)](https://www.buymeacoffee.com/walkerje)

# v0.13.9 Table of Contents
- [Introduction](#introduction)
  - [Roadmap](#roadmap-to-v100)
  - [Code Samples](#code-samples)
- [Quick Start & Build Guide](#quick-start)
  - [Building](#option-3-manual-build-and-install)
  - [Testing](#building-and-running-tests)
  - [Generating Docs](#generating-documentation)

![Test Status](https://github.com/walkerje/veritable_lasagna/actions/workflows/build_and_test.yml/badge.svg)

------------------
# Introduction

**Veritable Lasagna** **_(VL)_** is a cross-platform library written in C11. It implements a variety of common memory allocators, data structures, and algorithms.
The majority of this interface is loosely modeled on the C++ STL, and many functions might seem familiar. There are no dependencies aside from the standard library.
There are no macro templates provided for any structure. All operations are performed on arbitrary byte sequences, regardless of type. Moreover, these structures are interdependent on
one another by design.

This interdependent design is sensitive to regression when the behavior of any structure is modified.
Veritable Lasagna offers a suite of unit tests to verify persistent behavior between updates to help prevent this.

### Roadmap to v1.0.0

This roadmap specifies what it will take to consider this library feature-complete. Following this, additional features
will be released as part of minor versions after however many patches it might take to implement them. A Major release would involve significant
changes to the overall composition of this project. All proposed features are to be implemented in a cross-platform manner and are to  have  test suites developed for them.

**This library makes no promise to maintain ABI compatibility between versions until reaching version 1.0.0.**
 
- Memory Management
  - Memory blocks with metadata `vl_memory` ✔
  - Memory Pools `vl_pool`, `vl_async_pool` ✔
  - Arena Allocator `vl_arena` ✔
  - Data (De)Serialization `vl_msgpack` ✔
- Data Structures
  - Buffer `vl_buffer` ✔
  - Stack `vl_stack` ✔
  - Queue `vl_queue` ✔
  - Deque `vl_deque` ✔
  - Linked List `vl_linked_list` ✔
  - Ordered Set `vl_set` ✔
  - Hash Table  `vl_hashtable` ✔
- Algorithms
  - Pseudo-random number generator `vl_rand` ✔
  - Hashing `vl_hash` ✔
  - Comparisons `vl_compare` ✔
  - Sorting
    - Available to `vl_memory` and `vl_linked_list` ✔
      - Implicit to `vl_set` ✔
  - Search
    - Sorted
      - `vl_memory` ✔
    - Unsorted
      - `vl_memory` and `vl_linked_list` ✔
    - Implicit to `vl_set` and `vl_hashtable`. ✔
- Async
  - Primitives
    - Threads `vl_thread` ✔
    - Atomic Types `vl_atomic` ✔
    - Mutex `vl_mutex` ✔
    - SRWLock `vl_srwlock` ✔
    - Conditional Variable `vl_condition` ✔
    - Semaphore `vl_semaphore` ✔
  - Data Structures
    - Lockless Async Memory Pool ✔ `vl_async_pool`
    - Lockless Async Queue ✔ `vl_async_queue`
- Filesystem
  - Directory listing ✘
  - Path handling ✘
- Runtime Dynamic Library Handling ✘

### Code Samples

Below are full examples of common data structures.

#### vl_linked_list
```c
#include <stdlib.h>
#include <vl/vl_linked_list.h>


int main(int argc, const char** argv){
    //List of integers.
    vl_list* list = vlListNew(sizeof(int));
    
    for(int i = 0; i < 10; i++){
        const int listValue = i;
        vlListPushBack(list, &listValue);
    }
    
    VL_LIST_FOREACH(list, curIter){
        const int val = *((int*)vlListSample(list, curIter));
        printf("Value: %d", val);
    }
    
    vlListDelete(list);
    return EXIT_SUCCESS;
}

```
#### vl_hashtable
```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vl/vl_hashtable.h>

int main(int argc, const char** argv){
    vl_hashtable* wealthTable = vlHashTableNew(vlHashString);

    const int numEntries = 5;
    const char* keys[] = {
        "McLovin",
        "Supercop",
        "Napoleon",
        "Terminator",
        "Loch Ness Monster"
    };
    const float values[] = {12.05f, 5.84f, 910.63f, 711.42f, 3.50f};

    for(int i = 0; i < numEntries; i++){
        const char* key = keys[i];
        const float value = values[i];
        const int keyLen = strlen(key) + 1; //+1 to preserve null terminator
                                            //not strictly necessary, but somewhat handy.

        //Insert the key and claim the memory for the value.
        const vl_hash_iter iter = vlHashTableInsert(wealthTable, key, keyLen, sizeof(float));
        //Then assign the value to the memory owned by the table.
        *((float*)vlHashTableSampleValue(wealthTable, iter, NULL)) = value;
    }

    VL_HASHTABLE_FOREACH(wealthTable, curIter){
        //size of key and value, in bytes
        size_t keyLen, valLen;
        const char*    key = (const char*)vlHashTableSampleKey(wealthTable, curIter, &keyLen);
        const float    val =  *((float*)vlHashTableSampleValue(wealthTable, curIter, &valLen));

        printf("%s has %.2f$ in the bank!\n", key, val);
        //if we didn't preserve the null terminator, length can be stated explicitly:
        //printf(%.*s has %.2f$ in the bank!", keyLen, key, val);
    }

    vlHashTableDelete(wealthTable);
    return EXIT_SUCCESS;
}
```
#### vl_set
```c
#include <stdlib.h>
#include <stdio.h>
#include <vl/vl_set.h>

int main(int argc, const char** argv){
    const int set_size = 10;
    const int data[] = {6, 2, 9, 1, 3, 0, 4, 7, 5, 8};

    vl_set* set = vlSetNew(sizeof(int), vlCompareInt);

    for(int i = 0; i < set_size; i++)
        vlSetInsert(set, (void*) &(data[i]));

    printf("Sorted %d elements.", vlSetSize(set));
    VL_SET_FOREACH(set, curIter){
        const int value = *((int*)vlSetSample(curIter));
        printf("\t%d", value);
    }

    vlSetDelete(set);
    
    return EXIT_SUCCESS;
}
```

# Quick Start

There are three documented ways to use this library:
1. [Install as Package from Repository](#option-1-recommended-automated-build--install-from-repo)
2. [Embed as a subdirectory](#option-2-embed-as-subdirectory)
3. [Manual build and install](#option-3-manual-build-and-install)

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

## Configuration Options

These are the primary configuration options relevant to the library. Many of these are ubiquitous across CMake,
but they are described here nonetheless.

| Argument            | Type   | Default              | Description                                                                                                                                                                                                                                                       |
|---------------------|--------|----------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `CMAKE_BUILD_TYPE`  | STRING | `Toolchain Specific` | Specifies the build configuration type. Common values: <br>• `Debug` - No optimizations, includes debug info<br>• `Release` - Full optimizations, no debug info<br>• `RelWithDebInfo` - Full optimizations with debug info<br>• `MinSizeRel` - Size optimizations |
| `BUILD_SHARED_LIBS` | BOOL   | `OFF`                | Global flag affecting the how the library is built: <br>• `ON` - Libraries are built as shared/dynamic (DLL/SO)<br>• `OFF` - Libraries are built as static (LIB/A)                                                                                                |
| `BUILD_TESTING`     | BOOL   | `OFF`                | CTest module flag that controls test building:<br>• `ON` - Configure to build tests via CTest and GTest <br>• `OFF` - Skips building tests                                                                                                                        |



#### Building and Running Tests

Veritable Lasagna comes equipped with test suites powered by [GTest](https://github.com/google/googletest)
and [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html). They can be executed using `ctest`, a testing utility
that comes packaged with most CMake installations.

```bash
cd veritable_lasagna 
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
cmake --build .
cd test && ctest
```

#### Generating Documentation

The documentation webpage is generated using [Doxygen](https://www.doxygen.nl/), and requires the Graphviz [dot](https://graphviz.org/doc/info/lang.html)
extension to work properly. A [theme](https://github.com/jothepro/doxygen-awesome-css) is used to make the appearance of the documentation more modern,
and is included as a submodule in this repo. Packages for each should be readily available in most package managers
across a wide variety of platforms.

Doxygen and Graphviz can also be downloaded at the following locations:
- **[Graphviz](https://graphviz.org/download/) _(dot)_**
- **[Doxygen](https://www.doxygen.nl/download.html)**

```bash
cd veritable_lasagna
git submodule update --init #make sure the theme submodule is initialized
cd docs
doxygen
```

[Back to Top](#introduction)