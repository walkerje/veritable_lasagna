![Veritable Lasagna](docs/image/vl_logo.svg)

![Written in C11](https://img.shields.io/badge/C11-gray?style=for-the-badge&logo=C&logoColor=orange&labelColor=black) 
[![Built with CMake](https://img.shields.io/badge/Built%20with%20CMake-gray?style=for-the-badge&logo=CMake&logoColor=orange&labelColor=black)](https://cmake.org/)
[![Tested with GoogleTest](https://img.shields.io/badge/Testing%20With%20GTest-gray?style=for-the-badge&logo=googlesearchconsole&logoColor=orange&labelColor=black)](https://github.com/google/googletest)
[![Support development](https://img.shields.io/badge/Buy%20Me%20A%20Coffee-ffdd00?style=for-the-badge&logo=buy-me-a-coffee&logoColor=orange&labelColor=black)](https://www.buymeacoffee.com/walkerje)

# v0.10.17 Table of Contents
- [Introduction](#introduction)
  - [Roadmap](#roadmap-to-v100)
  - [Code Samples](#code-samples)
- [Quick Start & Build Guide](#quick-start--building-with-cmake)
  - [Building](#building)
  - [Testing](#building-and-running-tests)
  - [Generating Docs](#generating-documentation)

![Test Status](https://github.com/walkerje/veritable_lasagna/actions/workflows/build_and_test.yml/badge.svg)

------------------
# Introduction

**Veritable Lasagna** **_(VL)_** is a cross-platform library written in C11. It implements a variety of common memory allocators, data structures, and algorithms.
The majority of this interface is loosely modeled on the C++ STL, and many functions might seem familiar. There are no dependencies aside from the standard library.
**_There is absolutely no type safety in this library._** There are no macro templates provided for any structure. 
All operations are performed on arbitrary byte sequences, regardless of type. Moreover, these structures are interdependent on
one another by design.

![Implementation Dependency Graph](docs/image/vl_struct_graph.svg)

This interdependent design is sensitive to regression when the behavior of any structure is modified.
Veritable Lasagna offers a suite of unit tests to verify persistent behavior between updates to help prevent this.

### Roadmap to v1.0.0

This roadmap specifies what it will take to consider this library feature-complete. Following this, additional features
will be released as part of minor versions after however many patches it might take to implement them. A Major release would involve significant
changes to the overall composition of this project. All proposed features are to be implemented in a cross-platform manner and are to  have  test suites developed for them.

- Memory Management
  - Memory blocks with metadata `vl_memory` ✔
  - Memory pools
    - Linear Pool `vl_linearpool` ✔
    - Fixed Pool `vl_fixedpool` ✔
  - Arena Allocator `vl_arena` ✔
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
      - To be made available to `vl_memory` and `vl_linked_list` ✘
    - Unsorted
      - To be made available to `vl_memory` and `vl_linked_list` ✘
    - Implicit to `vl_set` and `vl_hashtable`. ✔
- Async 
  - Primitives
    - Threads ✘
    - Atomic Types ✔
    - Mutex ✘
    - Semaphore ✘
  - Data Structures
    - Lockless Async Memory Pool ✘
    - Lockless Async Queue ✘
    - Async Message Bus ✘
  - Structures
    - Worker Thread Pool ✘
    - Finite State Machine ✘
- Filesystem
  - Directory listing ✘
  - Path handling ✘
  - Data (De)Serialization (BSON) ✘
- Runtime Dynamic Library Handling ✘

### Code Samples

Below are full examples for some common complex data structures.

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

# Quick Start & Building with CMake
Start by cloning this repo to your project directory.

```bash
git clone this_page_url
```

If your project lives on Git, consider making this repo a submodule. This step is optional, of course.

```bash
git submodule add this_page_url
git submodule update --init
```

In your project `CMakeLists.txt`, add this library as a subdirectory.

```CMake
add_subdirectory(veritable_lasagna)
```

Then link the library like so:

```CMake
include_directories(${VL_INCLUDE})

#..............
# add targets
#..............

target_link_libraries(my_target_name ${VL_LIBRARY})
```

### Building

To build the shared and static versions of this library, VL provides CMake options.

| Flag Name        | Type | Default |
|------------------|------|---------|
| VL_BUILD_SHARED  | BOOL | OFF     |
| VL_SHARED_TESTS  | BOOL | OFF     |

When adding this library as a subdirectory to your project, static linking is preferred by default. 
By having `VL_BUILD_SHARED` set to `ON`, you may explicitly link to either `VL_STATIC_TARGET` or `VL_SHARED_TARGET`.
When neither flag is set, the variable `${VL_LIBRARY}` will link directly to the compiled object files. In this case,
there is no single "library file", and is roughly synonymous with static linking.

```CMake
# force shared building
# this can also be set as a CMake command-line argument, e.g,-DVL_BUILD_SHARED=ON
set(VL_BUILD_SHARED ON CACHE BOOL "" FORCE)

add_subdirectory(veritable_lasagna)

#static linking
target_link_libraries(target_a ${VL_LIBRARY_STATIC})
#shared linking
target_link_libraries(target_b ${VL_LIBRARY_SHARED})
```

#### Building and Running Tests

Veritable Lasagna comes equipped with a test suite powered by [GTest](https://github.com/google/googletest)
and [CTest](https://cmake.org/cmake/help/latest/manual/ctest.1.html). An appropriate build  script can be generated using the `VL_BUILD_TESTS` CMake flag. 
They can be executed using `ctest`, a testing utility that comes packaged with most CMake installations.

```bash
cd veritable_lasagna 
mkdir build && cd build
cmake ..
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