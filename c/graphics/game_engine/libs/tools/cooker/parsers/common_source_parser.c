

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/core/logger.h"
#include "engine/core/math/mat4.h"

#include "engine/core/logger.h"

#include "libs/core/string_span.h"

#include "libs/tools/cooker/parsers/common_source_parser.h"

#include "common_source_parser.h"

token_t get_next_token(string_span_t* cursor) {
    token_t token = {0};

    while (cursor->length > 0) {
        while (cursor->length > 0 && isspace(cursor->data[0])) {
            span_advance(cursor, 1);
        }

        if (cursor->length >= 2 && cursor->data[0] == '/' && cursor->data[1] == '/') {
            while (cursor->length > 0 && cursor->data[0] != '\n') {
                span_advance(cursor, 1);
            }
            continue;
        }
        break;
    }

    if (cursor->length == 0) {
        token.type   = TOK_EOF;
        token.start  = NULL;
        token.length = 0;
        return token;
    }

    char c = cursor->data[0];
    if (c == '{') {
        token.type   = TOK_LBRACE;
        token.start  = cursor->data;
        token.length = 1;
        span_advance(cursor, 1);
        return token;
    }
    if (c == '}') {
        token.type   = TOK_RBRACE;
        token.start  = cursor->data;
        token.length = 1;
        span_advance(cursor, 1);
        return token;
    }

    if (c == '"') {
        span_advance(cursor, 1);
        int end_idx = span_find_char(*cursor, '"');

        if (end_idx == -1) {
            token.type   = TOK_ERROR;
            token.start  = cursor->data;
            token.length = (uint32_t)cursor->length;
            return token;
        }

        token.type   = TOK_STRING;
        token.start  = cursor->data;
        token.length = (uint32_t)end_idx;
        span_advance(cursor, end_idx + 1);
        return token;
    }

    if (isalpha(c) || c == '_') {
        size_t len = 0;
        while (len < cursor->length && (isalnum(cursor->data[len]) || cursor->data[len] == '_')) {
            len++;
        }
        token.type   = TOK_IDENTIFIER;
        token.start  = cursor->data;
        token.length = (uint32_t)len;
        span_advance(cursor, len);
        return token;
    }

    if (isdigit(c) || c == '-' || c == '.') {
        size_t len = 0;
        while (len < cursor->length && (isdigit(cursor->data[len]) || cursor->data[len] == '-' ||
                                        cursor->data[len] == '.')) {
            len++;
        }

        token.type   = TOK_NUMBER;
        token.start  = cursor->data;
        token.length = (uint32_t)len;

        string_span_t num_span = span_init(token.start, len);
        span_to_float(num_span, &token.float_value);

        span_advance(cursor, len);
        return token;
    }

    token.type   = TOK_ERROR;
    token.start  = cursor->data;
    token.length = 1;
    span_advance(cursor, 1);
    return token;
}
