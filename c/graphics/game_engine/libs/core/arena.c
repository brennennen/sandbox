
#include <stdio.h>
#include <string.h>

#include "libs/core/arena.h"

#define DEFAULT_ALIGNMENT 8

static uintptr_t align_forward(uintptr_t ptr, size_t alignment) {
    uintptr_t a      = (uintptr_t)alignment;
    uintptr_t modulo = ptr & (a - 1);
    if (modulo != 0) {
        ptr += a - modulo;
    }
    return ptr;
}

void arena_init(arena_t* arena, void* buffer, size_t capacity) {
    arena->buffer   = (uint8_t*)buffer;
    arena->capacity = capacity;
    arena->offset   = 0;
}

void* arena_push(arena_t* arena, size_t size) {
    return arena_push_aligned(arena, size, DEFAULT_ALIGNMENT);
}

void* arena_push_aligned(arena_t* arena, size_t size, size_t alignment) {
    uintptr_t curr_ptr    = (uintptr_t)(arena->buffer + arena->offset);
    uintptr_t aligned_ptr = align_forward(curr_ptr, alignment);
    size_t    new_offset  = (size_t)(aligned_ptr - (uintptr_t)arena->buffer) + size;
    if (new_offset > arena->capacity) {
        return NULL;
    }
    void* result  = &arena->buffer[aligned_ptr - (uintptr_t)arena->buffer];
    arena->offset = new_offset;
    return result;
}

void* arena_push_zero(arena_t* arena, size_t size) {
    void* ptr = arena_push(arena, size);
    if (ptr != NULL) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void arena_clear(arena_t* arena) { arena->offset = 0; }

arena_temp_t arena_begin_temp(arena_t* arena) { return arena->offset; }

void arena_end_temp(arena_t* arena, arena_temp_t temp) {
    if (temp <= arena->offset) {
        arena->offset = temp;
    }
}

size_t arena_get_offset_of(arena_t* arena, void* ptr) {
    return (size_t)((uint8_t*)ptr - arena->buffer);
}
