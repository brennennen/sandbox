# Arena
Region based memory management. Allocate a large region/chunk/pool of memory and manually manage interpretations of that data in the region.

The goal being to minimize malloc/free syscalls, which can cause performance bottlenecks and be difficult to debug. Rather than having 1000 mallocs, with 1000 frees associated with them, if we can get away with it, why not just "batch" these memory allocations into 1 single malloc and 1 single free?

In "lifetime" terms, the standard method is to carefully manage individual lifetimes per object. The arena approach is to "batch" the management.


## main.c
Sandbox program to demo the arena allocator.
For each frame, allocates 128 "greeblies" (some arbitrary data) at the beginning of the frame, does some work with them, and then frees them by the end of the frame.


## Resources
* Wikipedia - https://en.wikipedia.org/wiki/Region-based_memory_management
* todo: find a link to the zillion times andrew kelley talks about it
* blog: https://www.rfleury.com/p/untangling-lifetimes-the-arena-allocator

## Build/Run
```sh
# Build
./build.bash
# Run
./.build/arena_sandbox
```
