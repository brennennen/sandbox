
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_span.h"

/**
 *
 */
string_span_t span_from_cstr(const char* cstr) {
    if (!cstr) {
        return (string_span_t){NULL, 0};
    }
    return (string_span_t){cstr, strlen(cstr)};
}

/**
 *
 */
string_span_t span_init(const char* data, size_t length) { return (string_span_t){data, length}; }

/**
 *
 */
string_span_t span_slice(string_span_t span, size_t start, size_t end) {
    if (start > span.length) {
        start = span.length;
    }
    if (end > span.length) {
        end = span.length;
    }
    if (start > end) {
        return (string_span_t){span.data + start, 0};
    }
    return (string_span_t){span.data + start, end - start};
}

/**
 *
 */
bool span_equals_cstr(string_span_t span, const char* str) {
    if (!span.data || !str) {
        return false;
    }
    size_t len = strlen(str);
    if (span.length != len) {
        return false;
    }
    return strncmp(span.data, str, span.length) == 0;
}

/**
 *
 */
void span_advance(string_span_t* span, size_t n) {
    if (n > span->length) {
        n = span->length;
    }
    span->data += n;
    span->length -= n;
}

static bool is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

string_span_t span_trim(string_span_t span) {
    // leading
    while (span.length > 0 && is_whitespace(span.data[0])) {
        span.data++;
        span.length--;
    }
    // trailing
    while (span.length > 0 && is_whitespace(span.data[span.length - 1])) {
        span.length--;
    }
    return span;
}

int span_find_char(string_span_t span, char c) {
    for (size_t i = 0; i < span.length; i++) {
        if (span.data[i] == c) {
            return (int)i;
        }
    }
    return -1;
}

bool span_consume_until(string_span_t* span, char delimiter) {
    int idx = span_find_char(*span, delimiter);
    if (idx == -1) {
        return false;
    }
    span_advance(span, idx + 1);
    return true;
}

bool span_to_int(string_span_t span, int* out_val) {
    if (span.length == 0) {
        return false;
    }
    int    result   = 0;
    bool   negative = false;
    size_t i        = 0;
    if (span.data[0] == '-') {
        negative = true;
        i++;
    }
    for (; i < span.length; i++) {
        char c = span.data[i];
        if (c >= '0' && c <= '9') {
            result = (result * 10) + (c - '0');
        } else {
            return false;
        }
    }
    *out_val = negative ? -result : result;
    return true;
}

bool span_to_float(string_span_t span, float* out_val) {
    if (span.length == 0 || span.length >= 32) {
        return false;
    }
    char buffer[32];
    memcpy(buffer, span.data, span.length);
    buffer[span.length] = '\0';
    char* endptr;
    *out_val = strtof(buffer, &endptr);
    return endptr != buffer;
}

char* span_to_arena_cstr(string_span_t span, arena_t* arena) {
    char* cstr = (char*)arena_push(arena, span.length + 1);
    memcpy(cstr, span.data, span.length);
    cstr[span.length] = '\0';
    return cstr;
}

void span_print(string_span_t span) { printf("'%.*s'\n", (int)span.length, span.data); }

bool arena_read_file_to_span(arena_t* arena, const char* filepath, string_span_t* out_span) {
    *out_span = (string_span_t){0};

    FILE* f = fopen(filepath, "rb");
    if (!f) {
        return false;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    if (length <= 0) {
        fclose(f);
        return false;
    }
    fseek(f, 0, SEEK_SET);

    char* buffer = (char*)arena_push(arena, length + 1);
    if (!buffer) {
        fclose(f);
        return false;
    }

    size_t bytes_read  = fread(buffer, 1, length, f);
    buffer[bytes_read] = '\0';
    fclose(f);

    out_span->data   = buffer;
    out_span->length = bytes_read;

    return true;
}
