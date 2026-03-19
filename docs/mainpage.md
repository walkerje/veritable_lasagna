# Veritable Lasagna Documentation
![](vl_logo.svg)

### Welcome to the Veritable Lasagna (VL) Documentation!

Veritable Lasagna is a high-performance, cross-platform data structures and algorithms library written in C11. Designed with a focus on efficiency, portability, and a clean API, VL provides a robust foundation for C applications ranging from embedded systems to high-performance computing.

---

## 🎯 Mission Statement

**Veritable Lasagna aims to be an approachable open-source toolbox that offers flexibility, accessible documentation, portability, and a concise codebase.**

### Key Design Pillars:
- **Flexibility**: Layered architecture that exposes underlying mechanisms without unnecessary obfuscation.
- **Accessible Documentation**: Comprehensive in-source documentation with Big-O complexity notes and interactive call graphs.
- **Portability**: Rigorous support for POSIX and Win32 systems, with consistent behavior across GCC, Clang, and MSVC.
- **Concise Codebase**: Follows a strict "one family of functions, one include file" paradigm with an intuitive, OpenGL-inspired naming convention.

---

## 🛠️ Feature Overview

Veritable Lasagna is organized into several core modules, each providing specialized functionality:
 
### 🧠 Memory Management & Streams
Efficient allocation strategies and data streaming:
- **`vl_memory`**: Basic memory blocks with alignment support and metadata.
- **`vl_buffer`**: Dynamically resizing memory buffers.
- **`vl_pool`**: Fixed-size block allocator for high-frequency allocations.
- **`vl_arena`**: Region-based memory management for grouped allocations.
- **`vl_stream`**: Extensible streaming API with `vl_stream_filesys` and `vl_stream_memory` implementations.

### 🏗️ Data Structures
A collection of standard and specialized containers:
- **`vl_hashtable`**: Fast, generic hash map implementation.
- **`vl_set`**: Unique ordered set.
- **`vl_linked_list`**: Doubly linked list.
- **`vl_queue` / `vl_deque` / `vl_stack`**: Standard FIFO and LIFO structures.
- **`vl_msgpack`**: Full implementation of the MessagePack serialization format.

### ⚡ Async & Concurrency
Modern primitives for multi-threaded applications:
- **`vl_thread` / `vl_thread_pool`**: Thread management and task scheduling.
- **`vl_atomic` / `vl_atomic_ptr`**: Lock-free atomic operations and tagged pointers.
- **Synchronization**: `vl_mutex`, `vl_semaphore`, `vl_condition`, and `vl_srwlock` (Slim Reader/Writer locks).
- **Lock-free Containers**: `vl_async_pool` and `vl_async_queue` for high-concurrency scenarios.

### 📂 Filesystem & OS
Cross-platform abstractions for system operations:
- **`vl_filesys`**: Path manipulation and recursive directory iteration with enforced UTF-8 support.
- **`vl_dynlib`**: Runtime dynamic library loading and symbol resolution.
- **`vl_ansi_term`**: ANSI terminal control and styling utilities.
- **`vl_log`**: Thread-safe, formatted logging system with configurable output sinks.

### 🌐 Networking
Cross-platform socket communication:
- **`vl_socket`**: Unified API for IPv4/IPv6, TCP, and UDP.
- **Support**: Blocking/non-blocking modes, and standard socket options (ReuseAddr, NoDelay, KeepAlive).

### 🧮 Algorithms & Math
Optimized computational utilities:
- **`vl_algo`**: Core algorithms including GCD, LCM, Popcount, and Bit-scanning.
- **`vl_simd`**: Runtime-dispatched SIMD intrinsics for x86 (SSE2, AVX2) and ARM (NEON).
- **`vl_rand`**: High-quality pseudo-random number generation.
- **`vl_half` / `vl_nibble`**: Support for 16-bit floats and 4-bit integers.
- **`vl_hash` / `vl_compare`**: Standardized hashing and comparison interfaces.

---

## 🚀 Getting Started

Veritable Lasagna is designed to be easy to integrate. For a comprehensive guide, please refer to the **[Full Manual](@ref manual)**.

### 📋 Prerequisites
- **Compiler**: C11-compliant (GCC, Clang, MSVC)
- **Build System**: [CMake](https://cmake.org/) (3.22+)
- **OS**: Windows (Win32 API) or Linux/macOS (POSIX)

### 📦 Integration
You can integrate VL into your project using several methods:
- **[Conan](https://conan.io/)**: `conan create .` and use `find_package(VLasagna REQUIRED)`.
- **Subdirectory**: Add via `add_subdirectory(veritable_lasagna)` in your `CMakeLists.txt`.
- **Manual Build**: Standard CMake build and install workflow.

For detailed instructions, see the **[Getting Started Manual](@ref man_getting_started)**.

---

*Happy Coding!*
