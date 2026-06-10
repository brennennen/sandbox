#pragma once

#include "libs/core/arena.h"

typedef struct {
    const char* data;
    size_t      length;
} string_span_t;

string_span_t span_init(const char* data, size_t length);
string_span_t span_from_cstr(const char* cstr);
string_span_t span_slice(string_span_t span, size_t start, size_t end);
bool          span_equals_cstr(string_span_t span, const char* str);
void          span_advance(string_span_t* span, size_t n);
string_span_t span_trim(string_span_t span);
int           span_find_char(string_span_t span, char c);
bool          span_consume_until(string_span_t* span, char delimiter);
bool          span_to_int(string_span_t span, int* out_val);
bool          span_to_float(string_span_t span, float* out_val);
char*         span_to_arena_cstr(string_span_t span, arena_t* arena);
void          span_print(string_span_t span);
bool arena_read_file_to_span(arena_t* arena, const char* filepath, string_span_t* out_span);
