\page man_utils Utilities & Algorithms

Veritable Lasagna includes a variety of helper modules for common tasks like math, bit manipulation, random number generation, and more.

## Table of Contents
- [Algorithms and Math (vl_algo)](#algorithms-and-math-vl_algo)
- [Random Number Generation (vl_rand)](#random-number-generation-vl_rand)
- [Hashing (vl_hash)](#hashing-vl_hash)
- [Half-Precision Floats (vl_half)](#half-precision-floats-vl_half)
- [Nibble Operations (vl_nibble)](#nibble-operations-vl_nibble)
- [ANSI Terminal (vl_ansi_term)](#ansi-terminal-vl_ansi_term)
- [Dynamic Library Loading (vl_dynlib)](#dynamic-library-loading-vl_dynlib)
- [Comparison Functions (vl_compare)](#comparison-functions-vl_compare)

## Algorithms and Math ( vl_algo )

### Description
The `vl_algo.h` header provides a collection of macros and functions for bitwise operations and basic math. It aims to provide efficient, cross-platform implementations of common algorithms.

### Key Features
- **Bit Manipulation:** PopCount, CLZ (Count Leading Zeros), CTZ (Count Trailing Zeros) for 8, 16, 32, and 64-bit integers.
- **Power of Two:** Functions to check if a number is a power of two or to find the next power of two.
- **Math Helpers:** GCD (Greatest Common Divisor), LCM (Least Common Multiple), and overflow-safe arithmetic.
- **Basic Macros:** Standard `VL_MIN`, `VL_MAX`, `VL_ABS`, and `VL_CLAMP` macros.

### Use Cases
- **Low-Level Bit Shifting:** Efficiently calculating bit masks or finding bit positions.
- **Memory Alignment:** Rounding up sizes to the next power of two for optimal allocation.
- **Safe Math:** Performing calculations where integer overflow must be detected.

## Random Number Generation (`vl_rand`)

### Description
The `vl_rand` module provides a fast, seedable pseudo-random number generator (PRNG). It is designed for performance and repeatability.

### Use Cases
- **Simulations:** Generating random inputs for Monte Carlo simulations.
- **Procedural Content:** Creating randomized game levels or textures.
- **Testing:** Generating random data to stress-test data structures or algorithms.

### Basic Usage
```c
#include <vl/vl_rand.h>

void rand_example() {
    vl_rand rng = vlRandInit(); // Initialize with a default seed

    vl_uint32_t r1 = vlRandUInt32(&rng);
    vl_float32_t f1 = vlRandF(&rng); // Random float between 0.0 and 1.0

    // Fill a buffer with random bytes
    uint8_t buffer[64];
    vlRandFill(&rng, buffer, sizeof(buffer));
}
```

## Hashing ( vl_hash )

### Description
The `vl_hash` module provides standard hashing functions and utilities. These are used internally by `vl_hashtable` but are also available for general use.

### Features
- **String Hashing:** Implements the FNV-1A-64 algorithm for robust string keys.
- **Integer Hashing:** Direct bitcast hashes for various integer sizes.
- **Hash Combination:** A macro to safely merge multiple hash values into one, minimizing collisions.

### Use Cases
- **Custom Hash Tables:** Building your own associative containers.
- **Data Integrity:** Quickly checking if a small piece of data has changed.
- **Indexing:** Creating hash-based indices for fast lookup in large datasets.

## Half-Precision Floats ( vl_half )

### Description
Support for 16-bit floating point numbers (FP16). This module provides functions to convert between FP16 and standard 32-bit floats (FP32).
The float16-format implementation is non-native, designed to replicate bit-level patterns of the IEEE 754 standard.

### Use Cases
- **Memory Savings:** Storing large arrays of floats where reduced precision is acceptable (e.g., graphics data).
- **GPU Interoperability:** Passing data to graphics APIs that expect half-precision formats.

## Nibble Operations ( vl_nibble )

### Description
Utilities for working with 4-bit "nibbles" within bytes.

### Use Cases
- **Data Compression:** Storing two 4-bit values in a single byte.
- **Hexadecimal Processing:** Converting between raw bytes and hex characters.

## ANSI Terminal ( vl_ansi_term )

### Description
A simple set of macros and constants for producing colored and styled output in terminals that support ANSI escape codes.

### Use Cases
- **Command-Line Tools:** Improving the readability of console output with colors and bold text.
- **Logging:** Highlighting errors in red and warnings in yellow in the terminal.

## Dynamic Library Loading ( vl_dynlib )

### Description
A cross-platform abstraction for loading dynamic libraries (`.so`, `.dll`, `.dylib`) and retrieving function pointers at runtime.

### Use Cases
- **Plugin Systems:** Loading external modules into an application.
- **Optional Dependencies:** Loading a library only if it is present on the user's system.

## Comparison Functions ( vl_compare )

### Description
A set of standard comparison functions used by ordered data structures like `vl_set`. These functions follow the standard `strcmp` signature (returning <0, 0, or >0).

### Use Cases
- **Sorting:** Providing comparison logic for sorting algorithms.
- **Ordered Collections:** Using `vl_set` with standard types like integers or strings.