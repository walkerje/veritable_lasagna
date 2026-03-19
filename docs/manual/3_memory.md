\page man_mem Memory Management

Veritable Lasagna provides several memory management primitives designed for efficiency and control. These are especially useful in performance-critical C applications where standard `malloc` and `free` might introduce fragmentation or overhead.

## Table of Contents
- [Arena Allocator (vl_arena)](#arena-allocator-vl_arena)
- [Buffer (vl_buffer)](#buffer-vl_buffer)
- [Pool Allocator (vl_pool)](#pool-allocator-vl_pool)

## Arena Allocator ( vl_arena )

### Description
The `vl_arena` is a region-based allocator that manages memory as a large, contiguous block. It is designed for scenarios where many small allocations are made and then freed collectively or in a specific order. The allocator uses an "Offset-Ordered First Fit Backwards Allocation" strategy to minimize fragmentation and allow for efficient coalescing of adjacent free blocks.

### Key Features
- **Fast Allocation:** Extremely quick allocation from a pre-allocated block.
- **Minimal Fragmentation:** Uses an "Offset-Ordered First Fit Backwards Allocation" strategy.
- **Coalescing:** Automatically merges adjacent free blocks.
- **Automatic Growth:** The arena can grow (by doubling capacity) if it runs out of space.

### Use Cases
- **Task-Based Memory:** Allocating all memory needed for a single frame in a game or a single request in a server, then clearing it all at once.
- **Hierarchical Data:** Building complex structures like trees or graphs where all nodes share the same lifetime.
- **Performance Critical Loops:** Avoiding `malloc` overhead in tight loops by pre-allocating an arena.

### Basic Usage
```c
#include <vl/vl_arena.h>

void example() {
    vl_arena arena;
    vlArenaInit(&arena, 1024); // 1KB initial size

    // Allocate memory (returns a stable offset)
    vl_arena_ptr p1 = vlArenaMemAlloc(&arena, 100);
    
    // Convert offset to a pointer for use (pointer may become invalid if arena grows)
    void* raw = vlArenaMemSample(&arena, p1);

    // Free specific allocation
    vlArenaMemFree(&arena, p1);

    // Clear all allocations at once
    vlArenaClear(&arena);

    vlArenaFree(&arena);
}
```

### Important Note on Pointers
Since `vl_arena` can grow, it may reallocate its internal buffer. This means standard C pointers to arena memory can become invalid. Always use `vl_arena_ptr` for long-term storage and call `vlArenaMemSample` only when you need the immediate raw pointer.

## Buffer ( vl_buffer )

### Description
The `vl_buffer` type is a dynamic, resizable byte array with an integrated seek head, similar to a file stream but in memory. It provides a convenient way to build up or parse binary data sequences.

### Key Features
- **Dynamic Growth:** Automatically resizes as you write data.
- **Seekable:** Maintain a current position for reading and writing.
- **Alignment Support:** Can be initialized with specific memory alignment.

### Use Cases
- **Serialization:** Building a binary message for network transmission or file storage.
- **Parsing:** Reading through a byte stream by seeking to specific offsets.
- **String Building:** Efficiently concatenating many strings or byte sequences.

### Basic Usage
```c
#include <vl/vl_buffer.h>

void buffer_example() {
    vl_buffer buffer;
    vlBufferInit(&buffer);

    int data = 42;
    vlBufferWrite(&buffer, sizeof(int), &data);

    vlBufferSeekBegin(&buffer);
    int read_data;
    vlBufferRead(&buffer, sizeof(int), &read_data);

    vlBufferFree(&buffer);
}
```

## Pool Allocator ( vl_pool )

### Description
The Pool allocator is designed for fast, fixed-size memory allocations. It manages a collection of uniform slots, which is ideal for situations where you need to create and destroy many objects of the same type. It uses a free list to achieve constant time O(1) allocation and deallocation.

### Key Features
- **Fixed-Size Elements:** Each pool is initialized to handle elements of a specific size.
- **Fast Allocation:** Provides O(1) allocation and deallocation from a free list.
- **Stable References:** Uses persistent indices (`vl_pool_idx`) to refer to elements, even if the pool's internal memory buffer is moved.
- **Automatic Growth:** Similar to arenas, pools can double their capacity when full.

### Use Cases
- **Object Management:** Managing entities in a game engine or nodes in a custom data structure.
- **Resource Pooling:** Keeping a set of pre-allocated buffers or state objects that are reused frequently.
- **Cache-Friendly Allocation:** Keeping objects of the same type close together in memory to improve CPU cache performance.

### Basic Usage
```c
#include <vl/vl_pool.h>

typedef struct {
    int x, y;
} MyPoint;

void pool_example() {
    vl_pool pool;
    vlPoolInit(&pool, sizeof(MyPoint));

    // Take an element from the pool (returns an index)
    vl_pool_idx idx = vlPoolTake(&pool);
    
    // Get a raw pointer to the element (may become invalid if pool grows)
    MyPoint* p = (MyPoint*)vlPoolSample(&pool, idx);
    p->x = 10;
    p->y = 20;

    // Return element to the pool
    vlPoolReturn(&pool, idx);

    vlPoolFree(&pool);
}
```
