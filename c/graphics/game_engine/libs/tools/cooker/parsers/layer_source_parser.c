
#include <stdio.h>
#include <string.h>

#include "engine/core/logger.h"

#include "libs/core/string_span.h"

#include "layer_source_parser.h"

#include "common_source_parser.h"

static bool parse_model_statement(string_span_t* cursor, model_src_t* model) {
    log_info("parse_model_statement");
    token_t id_tok   = get_next_token(cursor);
    token_t path_tok = get_next_token(cursor);
    token_t brace    = get_next_token(cursor); // '{'

    model->id   = (int)id_tok.float_value;
    model->path = span_init(path_tok.start, path_tok.length);

    // log_info("  -> Model %d: %s", model->id, model->path);
    log_info("Model %u: %.*s", model->id, (int)model->path.length, model->path.data);

    while (true) {
        token_t next = get_next_token(cursor);

        if (next.type == TOK_EOF || next.type == TOK_ERROR) {
            return false;
        }

        if (next.type == TOK_IDENTIFIER) {
            string_span_t token_span = span_init(next.start, next.length);
            if (span_equals_cstr(token_span, "UseFastTextures")) {
                token_t       val_tok  = get_next_token(cursor);
                string_span_t val_span = span_init(val_tok.start, val_tok.length);
                model->fast_textures   = span_equals_cstr(val_span, "true");
                log_info("  -> UseFastTextures: %d", (int)model->fast_textures);
            } else if (span_equals_cstr(token_span, "ZUp")) {
                token_t       val_tok  = get_next_token(cursor);
                string_span_t val_span = span_init(val_tok.start, val_tok.length);
                model->z_up            = span_equals_cstr(val_span, "true");
                log_info("  -> ZUp: %d", (int)model->z_up);
            } else {
                log_info("unrecognized model keyword: '%.*s'", (int)next.length, next.start);
            }
        } else if (next.type == TOK_RBRACE) {
            log_info(" model end");
            break;
        }
    }
    return true;
}

static bool parse_entity_statement(string_span_t* cursor, entity_src_t* entity) {
    log_info("parse_entity_statement");

    token_t name_tok = get_next_token(cursor);
    token_t brace    = get_next_token(cursor); // '{'

    entity->name = span_init(name_tok.start, name_tok.length);

    log_info("  -> Entity: %.*s", (int)entity->name.length, entity->name.data);

    while (true) {
        token_t next = get_next_token(cursor);

        if (next.type == TOK_EOF || next.type == TOK_ERROR) {
            return false;
        }

        if (next.type == TOK_IDENTIFIER) {
            string_span_t token_span = span_init(next.start, next.length);
            if (span_equals_cstr(token_span, "ModelID")) {
                token_t id_tok   = get_next_token(cursor);
                entity->model_id = (int)id_tok.float_value;
                log_info("  -> model_id: %d", (int)entity->model_id);
            } else if (span_equals_cstr(token_span, "Position")) {
                entity->position.x = get_next_token(cursor).float_value;
                entity->position.y = get_next_token(cursor).float_value;
                entity->position.z = get_next_token(cursor).float_value;
                log_info(
                    "  -> pos: %f, %f, %f",
                    entity->position.x,
                    entity->position.y,
                    entity->position.z
                );
            } else if (span_equals_cstr(token_span, "Rotation")) {
                entity->rotation.x = get_next_token(cursor).float_value;
                entity->rotation.y = get_next_token(cursor).float_value;
                entity->rotation.z = get_next_token(cursor).float_value;
                log_info(
                    "  -> rot: %f, %f, %f",
                    entity->rotation.x,
                    entity->rotation.y,
                    entity->rotation.z
                );
            } else if (span_equals_cstr(token_span, "Scale")) {
                entity->scale.x = get_next_token(cursor).float_value;
                entity->scale.y = get_next_token(cursor).float_value;
                entity->scale.z = get_next_token(cursor).float_value;
                log_info(
                    "  -> scale: %f, %f, %f", entity->scale.x, entity->scale.y, entity->scale.z
                );
            } else {
                log_info("unrecognized entity keyword: '%.*s'", (int)next.length, next.start);
            }
        } else if (next.type == TOK_RBRACE) {
            log_info(" entity end");
            break;
        }
    }
    return true;
}

bool parse_layer_source(
    arena_t*      arena,
    string_span_t text,
    string_span_t base_dir,
    layer_src_t*  out_layer_src
) {
    log_info("parse_layer_source");
    string_span_t cursor = text;

    model_src_t*  last_model  = NULL;
    entity_src_t* last_entity = NULL;

    while (true) {
        token_t next = get_next_token(&cursor);
        if (next.type == TOK_EOF) {
            break;
        }
        if (next.type == TOK_ERROR) {
            int err_len = next.length > 10 ? 10 : (int)next.length;
            log_error("LAYER LEXER ERROR: Unexpected character near '%.*s'", err_len, next.start);
            return false;
        }
        if (next.type == TOK_IDENTIFIER) {
            string_span_t token_span = span_init(next.start, next.length);
            if (span_equals_cstr(token_span, "Layer")) {
                log_info("Parsing Layer block...");
                token_t name_tok    = get_next_token(&cursor);
                out_layer_src->name = span_init(name_tok.start, name_tok.length);
                log_info(
                    "  -> Layer Name: %.*s",
                    (int)out_layer_src->name.length,
                    out_layer_src->name.data
                );

                token_t brace_tok = get_next_token(&cursor); // {
                continue;
            } else if (span_equals_cstr(token_span, "Model")) {
                model_src_t* new_model = arena_push_struct(arena, model_src_t);
                out_layer_src->model_count++;
                SLL_APPEND(out_layer_src->models, last_model, new_model);
                if (!parse_model_statement(&cursor, new_model)) {
                    return false;
                }
            } else if (span_equals_cstr(token_span, "Entity")) {
                entity_src_t* new_entity = arena_push_struct(arena, entity_src_t);
                out_layer_src->entity_count++;
                SLL_APPEND(out_layer_src->entities, last_entity, new_entity);
                if (!parse_entity_statement(&cursor, new_entity)) {
                    return false;
                }
            } else {
                log_info("unrecognized layer keyword: '%.*s'", (int)next.length, next.start);
            }
        } else if (next.type == TOK_RBRACE) { // }
            log_info("End of Layer block");
        }
    }

    return true;
}
