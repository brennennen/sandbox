
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


#ifndef ARENA_MALLOC
    #include <stdlib.h>
    #define ARENA_MALLOC malloc
#endif // ARENA_MALLOC

#ifndef ARENA_FREE
    #include <stdlib.h>
    #define ARENA_FREE free
#endif // ARENA_FREE

#ifndef ARENA_MEMCPY
    #include <string.h>
    #define ARENA_MEMCPY memcpy
#endif // ARENA_MEMCPY

#include "arena.h"

#define ARENA_ALIGNMENT 8

static size_t align_up(size_t size) {
    return(size + (ARENA_ALIGNMENT - 1)) & ~(ARENA_ALIGNMENT - 1);
}

static arena_node_t* arena_node_create(size_t size) {
    arena_node_t* arena_node = (arena_node_t*) ARENA_MALLOC(sizeof(arena_node_t) + size);
    if (!arena_node) {
        return(NULL);
    }
    arena_node->next = NULL;
    arena_node->size = size;
    arena_node->offset = 0;
    return(arena_node);
}

arena_t arena_create(size_t size) {
    arena_t arena = {0};
    arena.head = arena_node_create(size);
    arena.chain_count = 1;
    return(arena);
}

void arena_reset(arena_t* arena) {
    arena_node_t* node = arena->head;
    while(node) {
        node->offset = 0;
        node = node->next;
    }
}

void arena_free(arena_t* arena) {
    arena_node_t* node = arena->head;
    while(node) {
        arena_node_t* next = node->next;
        free(node);
        node = next;
    }
}

void *arena_alloc(arena_t* arena, size_t size) {
    if (!arena || !arena->head) {
        return(NULL);
    }

    size = align_up(size);

    arena_node_t* node = arena->head;
    if (node->size - node->offset >= size) {
        void* ptr = node->data + node->offset;
        node->offset += size;
        return(ptr);
    }

    size_t new_capacity = node->size * 2 > size ? node->size * 2 : size;
    arena_node_t* new_node = arena_node_create(new_capacity);
    arena->chain_count += 1;
    if (!new_node) {
        return(NULL);
    }

    new_node->next = arena->head;
    arena->head = new_node;

    void *ptr = new_node->data;
    new_node->offset += size;
    return(ptr);
}

void arena_prune(arena_t* arena) {
    arena_node_t* current = arena->head;
    arena_node_t* last_node = NULL;

    while (current->next != NULL) {
        current = current->next;
    }
    last_node = current;

    current = arena->head;
    while(current != last_node) {
        arena_node_t* next = current->next;
        ARENA_FREE(current);
        arena->chain_count -= 1;
        current = next;
    }
    arena->head = last_node;
}
