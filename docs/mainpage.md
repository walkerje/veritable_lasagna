# Landing Page
![](vl_logo.svg)
#### Thanks for taking a second to read the docs!
## What is this project?

Veritable Lasagna (VL) is a data structures and algorithms library written for C11. It provides a variety of
memory management utilities, data structures, and algorithms.

##### The mission statement for this project is as follows:

-   Veritable Lasagna aims to be an approachable open-source toolbox project that favors unconventional design while offering flexibility, accessible documentation, portability, and a concise codebase.

Let's break it down.
- Flexibility
  - There are very few type-dependent algorithms.
  - Due to the layered nature of the memory management and data structures, many underlying
    mechanisms are not obfuscated away but rather explicitly exposed.
- Accessible Documentation
  - Regular generated documentation is supplemented with a written [manual](./manual).
  - Rendered graphs are interspersed throughout the documentation, as well as interactive call/caller graphs on a per-function basis.
  - Most functions are tagged with a note denoting their complexity in Big-O notation.
- Portability
  - Care has been taken to offer compatibility for system targets with unconventional limitations.
    - Independent "largest" and "smallest" integer type resolution (See `vl_(i/u)large_t` and `vl_(i/u)small_t`)
    - Compilers with high-precision floating point types. (Intel's 128-bit floats for example, see `vl_float_highp_t` type)
    - Routinely tested on MSVC/GCC for Windows 11, and GCC for >= Ubuntu 22.04 LTS.
- Concise Codebase
  - Follows the "one family of functions, one include file" paradigm.
  - Strict adherence to a consistent naming convention modeled after the OpenGL API.

## Structures

At the time of writing, the following memory management schemes are available:
- Plain block of memory, aligned and unaligned, `vl_memory`.
- Memory buffer with dynamic size (automatic reallocation), `vl_buffer`.
- Pool buffers of fixed-sized blocks of memory, `vl_linearpool` and `vl_fixedpool`
- [Arena allocation](https://en.wikipedia.org/wiki/Region-based_memory_management) of variably-size blocks of memory from a dynamically sized buffer, `vl_arena`

... alongside the following conventional data structures:
- Hash Table `vl_hashtable`
- Buffer `vl_buffer`
- Unique Ordered Set `vl_set`
- Doubly Linked List `vl_linked_list`
- Queue `vl_queue`
- Deque `vl_deque`
- Stack `vl_stack`

Conventional data structures are dependent on one or more of the memory management schemes.
See the dependency graph below.

![Structure Hierarchy](vl_struct_graph.svg)

## How is this project structured?

CMake is used by this project to support cross-platform compilation and the accompanying test suites.

![Build Process](vl_build_graph.svg)