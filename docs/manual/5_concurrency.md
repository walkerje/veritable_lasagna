\page man_concurrency Concurrency

Veritable Lasagna provides a robust set of tools for building modern, multi-threaded applications. This includes portable threading, synchronization primitives, thread pools, and lock-free atomic operations.

## Table of Contents
- [Threading & Concurrency (vl_thread)](#threading--concurrency-vl_thread)
- [Sync Primitives](#sync-primitives)
- [Thread Pool (vl_thread_pool)](#thread-pool-vl_thread_pool)
- [Atomic Operations (vl_atomic)](#atomic-operations-vl_atomic)
- [Asynchronous Containers](#asynchronous-containers)

## Threading & Concurrency ( vl_thread )

### Description
The `vl_thread` module provides a portable, lightweight API for creating and managing threads. It abstracts platform-specific threading (like pthreads or Win32 threads) into a clean C interface.

### Use Cases
- **Parallel Processing:** Distributing heavy computations across multiple CPU cores.
- **Background Tasks:** Performing I/O or periodic cleanup without blocking the main execution flow.
- **Asynchronous Execution:** Offloading tasks that might take a variable amount of time.

### Basic Usage
```c
#include <vl/vl_thread.h>

void my_thread_func(void* arg) {
    // Thread logic here
}

void threading_example() {
    vl_thread t = vlThreadNew(my_thread_func, NULL);
    vlThreadJoin(t);
    vlThreadDelete(t);
}
```

## Sync Primitives

### Description
Veritable Lasagna provides a comprehensive set of synchronization primitives to manage concurrent access to shared resources and coordinate thread execution.

### Key Primitives
- **Mutexes (`vl_mutex`):** Standard mutual exclusion to protect critical sections.
- **Semaphores (`vl_semaphore`):** Counting semaphores for resource management and signaling.
- **Condition Variables (`vl_condition`):** Used for thread signaling and waiting for specific states.
- **SRW Locks (`vl_srwlock`):** Slim Reader/Writer locks that allow multiple concurrent readers or a single exclusive writer.

### Use Cases
- **Resource Protection:** Using mutexes to ensure only one thread modifies a shared data structure at a time.
- **Producer-Consumer:** Using semaphores or condition variables to coordinate data transfer between threads.
- **Read-Heavy Data:** Using SRW locks to improve performance when many threads read data but few modify it.

## Thread Pool ( vl_thread_pool )

### Description
The `vl_thread_pool` manages a fixed collection of worker threads that execute tasks from a shared work queue. This avoids the overhead of creating and destroying threads for every small task.

### Use Cases
- **High-Frequency Tasks:** Handling many small, independent units of work efficiently.
- **Server Request Handling:** Processing incoming network requests using a pool of workers.
- **Parallel Algorithms:** Breaking down a large problem into many small tasks.

### Basic Usage
```c
#include <vl/vl_thread_pool.h>

void my_task(void* arg) { /* ... */ }

void pool_example() {
    vl_thread_pool pool;
    vlThreadPoolInit(&pool, 4); // 4 worker threads

    vlThreadPoolEnqueue(&pool, my_task, NULL);

    vlThreadPoolFree(&pool);
}
```

## Atomic Operations ( vl_atomic )

### Description
Low-level atomic primitives provide thread-safe operations on integers and pointers without using heavy locks. They are essential for building lock-free data structures.

### Use Cases
- **Counters:** Incrementing or decrementing shared counters without a mutex.
- **Flags:** Signaling state changes between threads atomically.
- **Lock-Free Algorithms:** Implementing high-performance concurrent containers.

### Basic Usage
```c
#include <vl/vl_atomic.h>

void atomic_example() {
    vl_atomic_int32_t val;
    vlAtomicInit32(&val, 0);

    vlAtomicAdd32(&val, 1);
    vl_int32_t current = vlAtomicLoad32(&val);
}
```

## Asynchronous Containers

### Description
Thread-safe versions of common data structures designed for high-concurrency environments.

### Available Containers
- **Async Pool (`vl_async_pool`):** A thread-safe object pool for frequent allocation/deallocation of uniform objects.
- **Async Queue (`vl_async_queue`):** A lock-free/thread-safe FIFO queue for efficient message passing between threads.
