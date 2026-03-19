\page man_ds Data Structures

Veritable Lasagna offers a variety of data structures, each designed for high-performance and a clean C API.

## Table of Contents
- [Hash Table (vl_hashtable)](#hash-table-vl_hashtable)
- [Linked List (vl_linked_list)](#linked-list-vl_linked_list)
- [Sets (vl_set)](#sets-vl_set)
- [Deque (vl_deque)](#deque-vl_deque)
- [Stack (vl_stack)](#stack-vl_stack)
- [Queue (vl_queue)](#queue-vl_queue)

## Hash Table ( vl_hashtable )

### Description
The `vl_hashtable` is a flexible, high-performance associative array that maps keys to values. It uses an internal `vl_arena` for efficient memory management and supports custom hash functions and key/value sizes.

### Key Features
- **Dynamic Growth:** Automatically resizes when a load factor threshold is met.
- **Custom Key/Value Sizes:** Keys and values can be of any size.
- **Efficient Memory Use:** Uses an internal `vl_arena` for allocating its elements.

### Use Cases
- **Caching:** Storing frequently accessed data with quick retrieval by key.
- **Symbol Tables:** Managing a collection of named objects in a compiler or interpreter.
- **Unordered Maps:** Any scenario where you need to associate unique keys with specific data.

### Basic Usage
```c
#include <vl/vl_hashtable.h>

void hash_example() {
    vl_hashtable table;
    vlHashTableInit(&table, vlHashMurmur3); // Use standard Murmur3 hash

    const char* key = "myKey";
    int value = 42;

    // Insert key-value pair
    vl_hash_iter iter = vlHashTableInsert(&table, key, strlen(key) + 1, sizeof(int));
    *(int*)vlHashTableSampleValue(&table, iter, NULL) = value;

    // Retrieve value
    vl_hash_iter found = vlHashTableFind(&table, key, strlen(key) + 1);
    if (found != VL_HASHTABLE_ITER_INVALID) {
        int* val = (int*)vlHashTableSampleValue(&table, found, NULL);
        // val is 42
    }

    vlHashTableFree(&table);
}
```

## Linked List ( vl_linked_list )

### Description
A standard doubly linked list implementation. It allows for O(1) insertion and removal at both ends and provides a simple interface for sequential data management.

### Key Features
- **Doubly Linked:** Supports efficient insertions and deletions at both ends.
- **Standard Operations:** Common operations like push, pop, shift, and unshift.

### Use Cases
- **Task Queues:** Managing a list of tasks where items are frequently added to the end and removed from the front.
- **Undo/Redo Buffers:** Storing a sequence of actions that can be traversed forward and backward.
- **Dynamic Collections:** When the number of elements is unknown and frequent reallocations of an array are undesirable.

### Basic Usage
```c
#include <vl/vl_linked_list.h>

void list_example() {
    vl_linked_list list;
    vlLinkedListInit(&list, sizeof(int));

    int val = 10;
    vlLinkedListPushBack(&list, &val);

    vlLinkedListFree(&list);
}
```

## Sets ( vl_set )

### Description
A `vl_set` maintains an ordered collection of unique elements. It is implemented as a Red-Black Tree, providing O(log n) time complexity for insertions, deletions, and lookups.

### Key Features
- **Ordered:** Elements are kept in order according to a comparison function.
- **Unique Elements:** Prevents duplicate entries.
- **Memory Efficient:** Uses a `vl_pool` internally to manage its nodes.

### Use Cases
- **Maintaining Sorted Data:** Keeping a list of scores or timestamps in order.
- **Deduplication:** Ensuring a collection only contains unique items.
- **Efficient Search:** Quickly finding if an element exists in a large collection.

### Basic Usage
```c
#include <vl/vl_set.h>
#include <vl/vl_compare.h>

void set_example() {
    vl_set mySet;
    // Initialize a set of integers using the standard int comparison function
    vlSetInit(&mySet, sizeof(int), vlCompareInt);

    int a = 5, b = 10, c = 5;
    vlSetInsert(&mySet, &a);
    vlSetInsert(&mySet, &b);
    vlSetInsert(&mySet, &c); // Duplicate 5 will not be added

    // Iterate through the set
    VL_SET_FOREACH(&mySet, iter) {
        int* val = (int*)vlSetSample(&mySet, iter);
        // Elements will be printed in order: 5, 10
    }

    vlSetFree(&mySet);
}
```

## Deque ( vl_deque )

### Description
A Double-Ended Queue (Deque) supports efficient insertion and removal from both the front and the back. It is often implemented as a dynamic array of blocks or a circular buffer.

### Key Features
- **Efficient Ends:** O(1) push and pop from both front and back.
- **Dynamic Resizing:** Grows as needed to accommodate more elements.

### Use Cases
- **Sliding Window Algorithms:** Keeping track of a range of elements in a sequence.
- **Work-Stealing Queues:** Where multiple threads can add or remove work from different ends.
- **General Purpose Buffer:** When you need both FIFO and LIFO behaviors.

### Basic Usage
```c
#include <vl/vl_deque.h>

void deque_example() {
    vl_deque deque;
    vlDequeInit(&deque, sizeof(int));

    int a = 1, b = 2;
    vlDequePushBack(&deque, &a);
    vlDequePushFront(&deque, &b);

    int out;
    vlDequePopBack(&deque, &out);  // out = 1
    vlDequePopFront(&deque, &out); // out = 2

    vlDequeFree(&deque);
}
```

## Stack ( vl_stack )

### Description
A Last-In, First-Out (LIFO) data structure. It is a specialized container that provides a restricted interface for adding and removing elements.

### Use Cases
- **Function Call Management:** Simulating a call stack or managing recursion.
- **Expression Parsing:** Evaluating mathematical expressions or balancing brackets.
- **Backtracking:** Storing states to return to during a search.

### Basic Usage
```c
#include <vl/vl_stack.h>

void stack_example() {
    vl_stack stack;
    vlStackInit(&stack, sizeof(int));

    int val = 100;
    vlStackPush(&stack, &val);

    int top;
    vlStackPop(&stack, &top); // top = 100

    vlStackFree(&stack);
}
```

## Queue ( vl_queue )

### Description
A First-In, First-Out (FIFO) data structure. It ensures that the first element added is the first one to be removed.

### Use Cases
- **Message Passing:** Storing messages or events to be processed in order.
- **Breadth-First Search (BFS):** Managing the frontier of nodes during graph traversal.
- **Buffer Management:** Temporarily holding data between two processes of different speeds.

### Basic Usage
```c
#include <vl/vl_queue.h>

void queue_example() {
    vl_queue queue;
    vlQueueInit(&queue, sizeof(int));

    int val = 200;
    vlQueueEnqueue(&queue, &val);

    int out;
    vlQueueDequeue(&queue, &out); // out = 200

    vlQueueFree(&queue);
}
```
