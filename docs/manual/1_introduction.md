\page man_intro Introduction

Welcome to the **Veritable Lasagna** manual!

Veritable Lasagna is a modern, modular, and portable toolkit for C programmers, offering high-quality data structures and algorithms with a strong focus on efficiency and clarity. Designed for use with C11, this library aims to fill gaps left by the standard library, providing powerful primitives commonly found in higher-level languages, but with a clean and approachable interface tailored for C projects of any size.

## Table of Contents
- [Key Features](#key-features)
- [Philosophy](#philosophy)
- [When to Use Veritable Lasagna](#when-to-use-veritable-lasagna)
- [About This Manual](#about-this-manual)
- [Author's Note](#a-note-from-the-author)

## Key Features
- **Comprehensive Data Structures:**
Includes dynamic arrays, stacks, queues, deques, linked lists, sets, and hash tables—some with both standard and thread-safe variants.
- **Advanced Memory Management:**
Provides arenas and pools that improve allocation speed and deterministic resource management.
- **Threading and Concurrency:**
Supplies atomic types, mutexes, and lock-free asynchronous containers for building robust multithreaded applications.
- **Serialization Support:**
Integrates MessagePack for compact, fast object serialization and deserialization.
- **Modern C:**
Written in clear, standards-compliant C11 with a focus on low dependencies and portability.
- **MIT Licensed:**
Freely available for both commercial and personal use.

## Philosophy
Veritable Lasagna is designed to be practical and unintrusive. Whether you are developing embedded firmware, a new desktop tool, or server infrastructure, this library helps you avoid “reinventing the wheel” for common programming tasks—without tying your project to complex external dependencies or requiring C++.

**Key goals:**
- Favor explicit control and zero-overhead abstractions.
- Support both classic procedural and modern idiomatic C styles.

## When to Use Veritable Lasagna
Use Veritable Lasagna if you need:
- Rapid prototyping
- Tools for multithreaded programming without external runtime
- Cross-platform system targets

## About This Manual
This manual is structured to help you get productive quickly:
1. **Introduction:** Overview and philosophy
2. **Getting Started:** Installation, dependencies, and integration
3. **Memory Management:** Arenas, buffers, and pools
4. **Data Structures:** Linked lists, hash tables, sets, deques, etc.
5. **Concurrency:** Threads, sync primitives, and lock-free containers
6. **Advanced Features:** SIMD, I/O, Filesystem, and Serialization
7. **Utilities:** Math, random numbers, and bitwise algorithms


Let's get started!
