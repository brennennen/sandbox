#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t* buffer;
    size_t   capacity;
    size_t   offset;
} arena_t;

typedef size_t arena_temp_t;

void         arena_init(arena_t* arena, void* memory, size_t capacity);
void*        arena_push(arena_t* arena, size_t size);
void*        arena_push_aligned(arena_t* arena, size_t size, size_t alignment);
void*        arena_push_zero(arena_t* arena, size_t size);
void         arena_clear(arena_t* arena);
arena_temp_t arena_begin_temp(arena_t* arena);
void         arena_end_temp(arena_t* arena, arena_temp_t temp);
size_t       arena_get_offset_of(arena_t* arena, void* ptr);

#define arena_push_struct(arena, type) (type*)arena_push_zero(arena, sizeof(type))
#define arena_push_array(arena, type, count) (type*)arena_push_zero(arena, sizeof(type) * (count))
