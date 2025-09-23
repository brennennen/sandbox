/**
 * Region/arena/bump allocator with chaining.
 */

#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct arena_node_s {
    size_t size;
    size_t offset;

    struct arena_node_s* next;
    uint8_t data[]; // "Flexible Array Member", saves 1 extra malloc/free over a standard pointer (uint8_t* data;)
} arena_node_t;

typedef struct {
    int chain_count;
    arena_node_t* head;
} arena_t;

/** Construct the arena allocator with one chain of a specific size (calls malloc). */
arena_t arena_create(size_t size);

/** Clears counters, effectively "free"s all objects allocated so far from the arena. */
void arena_reset(arena_t* arena);

/** Copies an arena to another arena. */
void arena_copy(arena_t* source, arena_t* dest);

/** Frees/destructs all chains of the arena (calls free). */
void arena_free(arena_t* arena);

/**
 * Allocates memory from the arena, creating a new chain if necessary.
 * (should not call malloc in the default case, but can).
 */
void *arena_alloc(arena_t* arena, size_t size);

/** Frees all chains except for the first (calls free). */
void arena_prune(arena_t* arena);

#endif // ARENA_H
