#pragma once

#include "libs/core/string_span.h"
#include "tools/parsers/parser_source_types.h"

#define SLL_APPEND(first, last, node)                                                              \
    do {                                                                                           \
        if (last) {                                                                                \
            (last)->next = (node);                                                                 \
        } else {                                                                                   \
            (first) = (node);                                                                      \
        }                                                                                          \
        (last) = (node);                                                                           \
    } while (0)

typedef enum {
    TOK_EOF,
    TOK_ERROR,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_IDENTIFIER,
    TOK_STRING,
    TOK_NUMBER
} token_type_t;

typedef struct {
    token_type_t type;
    const char*  start;
    uint32_t     length;
    union {
        float float_value;
    };
} token_t;

token_t get_next_token(string_span_t* cursor);
