# Multi-threaded-memory-allocator

## Overview

In this project I implemented a multi-threaded memory allocator that uses some memory
management techniques. The solution has `main.c` contains a main function that tests the allocator.

**Note:** As usual, all code is written in C and run on a Linux machine.

## Allocator Design

### Initialization

The allocator needs to know the total size of memory assumed in this project and the memory
allocation algorithm to be used. This information will be supplied by the following API:

```c
void initialize_allocator(int size, enum allocation_algorithm);
```

In the above function, `size` indicates the contiguous memory chunk size that is assumed for the
rest of the program. Any requests for allocation and deallocation requests (see
[Allocation/deallocation interfaces](#allocationdeallocation-interfaces)) will be served from this
contiguous chunk. I have allocated the memory chunk using `malloc` and have the memory chunk
pre-faulted and its content initialized to 0; The `allocation_algorithm` parameter is an enum (as
shown below) which will determine the algorithm used for allocation in the rest of the program:

```c
enum allocation_algorithm { FIRST_FIT, BEST_FIT, WORST_FIT };
```

`FIRST_FIT` satisfies the allocation request from the first available memory block (from left) that
is at least as large as the requested size. `BEST_FIT` satisfies the allocation request from the
available memory block that at least as large as the requested size and that results in the smallest
remainder fragment. `WORST_FIT` satisfies the allocation request from the available memory block
that at least as large as the requested size and that results in the largest remainder fragment.

### Allocation/deallocation interfaces

The allocation and deallocation requests will be similar to `malloc` and `free` calls in C, except
that they are called `allocate` and `deallocate` with the following signatures:

```c
void *allocate(int size);
void deallocate(void *ptr);
```

As expected, `allocate` returns a pointer to the allocated block of size `size` and `deallocate`
takes a pointer to a chunk of memory as the sole parameter and return it back to the allocator. If
allocation cannot be satisfied, `allocate` returns `NULL`.

### Metadata Management

I have maintained the size of allocated memory block within the block itself. The "header" only contains a single 8-byte word that
denotes the size of the actual allocation (i.e., 8-byte + requested allocation size). For example,
if the request asks for 16 bytes of memory, you should actually allocate 8 + 16 bytes, and use the
first 8-byte to store the size of the total allocation (24 bytes) and return a pointer to the
user-visible 16-byte.

To manage free/allocated space, I have maintain two separate singly linked lists, one for
allocated blocks, and the other for free blocks. When a block gets allocated (using `allocate`), its
metadata (i.e., a pointer to the allocation) must be inserted to the list of allocated blocks.
Similarly when a block gets freed (using `deallocate`), its metadata must be inserted to the list of
free blocks. Note that each linked list node only contains a pointer to the allocated memory
block and a pointer to the next node in the list, because size of the allocation is already recorded
with the memory block itself. The free list must never maintain contiguous free
blocks (as shown in Figure 1), i.e., if two blocks, one of size _m_ and other of size _n_, are
consecutive in the memory chunk, they must become a combined block of size m + n (as shown in Figure
2). This combination must happen when `deallocate` is called.

![](memory1.png)

_Figure 1: Each block is labeled with its size. White indicates free block while allocated blocks
are colored._

![](memory2.png)

_Figure 2: Example with contiguous free blocks. This should never occur as contiguous free blocks
should be merged immediately (as shown in Figure 1)._

### Statistics

Information about the current state of memory can be found by the following API:

```c
void get_statistics(struct Stats*);
int available_memory();
```

`get_statistics(struct Stats*)` populates the detailed statistics in the `struct Stats*` given
as the argument.

`available_memory()` returns the available memory size (same as Free size in `get_statistics()`)

### Multi-threading support

The allocator supports multi-threading, i.e., it allocates and deallocates even with mutliple concurrent threads.


### Uninitialization

In order to avoid memory leaks after using your contiguous allocator, I have implemented a
function that will release any dynamically allocated memory in contiguous allocator.

```c
void destroy_allocator();
```
