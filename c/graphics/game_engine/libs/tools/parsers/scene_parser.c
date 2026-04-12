
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine/core/logger.h"
#include "engine/core/math/mat4_math.h"

#include "gltf_baker.h"
#include "scene_parser.h"

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
    char         string_value[256];
    float        float_value;
} token_t;

static token_t get_next_token(const char** cursor) {
    token_t token = {0};

    while (true) {
        while (isspace(**cursor))
            (*cursor)++;

        if ((*cursor)[0] == '/' && (*cursor)[1] == '/') {
            while (**cursor != '\n' && **cursor != '\0')
                (*cursor)++;
            continue;
        }
        break;
    }

    if (**cursor == '\0') {
        token.type = TOK_EOF;
        return token;
    }
    if (**cursor == '{') {
        token.type = TOK_LBRACE;
        (*cursor)++;
        return token;
    }
    if (**cursor == '}') {
        token.type = TOK_RBRACE;
        (*cursor)++;
        return token;
    }

    if (**cursor == '"') {
        token.type = TOK_STRING;
        (*cursor)++;
        int i = 0;
        while (**cursor != '"' && **cursor != '\0' && i < 255) {
            token.string_value[i++] = *(*cursor)++;
        }
        token.string_value[i] = '\0';
        if (**cursor == '"')
            (*cursor)++;
        return token;
    }

    if (isalpha(**cursor) || **cursor == '_') {
        token.type = TOK_IDENTIFIER;
        int i      = 0;
        while ((isalnum(**cursor) || **cursor == '_') && i < 255) {
            token.string_value[i++] = *(*cursor)++;
        }
        token.string_value[i] = '\0';
        return token;
    }

    if (isdigit(**cursor) || **cursor == '-' || **cursor == '.') {
        token.type = TOK_NUMBER;
        char num_str[64];
        int  i = 0;
        while ((isdigit(**cursor) || **cursor == '-' || **cursor == '.') && i < 63) {
            num_str[i++] = *(*cursor)++;
        }
        num_str[i]        = '\0';
        token.float_value = strtof(num_str, NULL);
        return token;
    }

    token.type = TOK_ERROR;
    return token;
}

char* read_file_to_string(const char* filepath) {
    FILE* f = fopen(filepath, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);
    return buffer;
}

static void parse_include_statement(
    const char**  cursor,
    const char*   base_dir,
    scene_desc_t* out_scene
) {
    token_t path_tok = get_next_token(cursor);
    if (path_tok.type != TOK_STRING) {
        log_error("Expected string after 'Include'");
        return;
    }

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", base_dir, path_tok.string_value);
    log_info("Flattening Included Layer: %s", full_path);

    char* sub_text = read_file_to_string(full_path);
    if (sub_text) {
        parse_scene_file(sub_text, base_dir, out_scene);
        free(sub_text);
    } else {
        log_error("Could not find included file: %s", full_path);
    }
}

static void parse_model_statement(
    const char**  cursor,
    const char*   base_dir,
    scene_desc_t* out_scene
) {
    uint32_t model_id = (uint32_t)get_next_token(cursor).float_value;
    token_t  path_tok = get_next_token(cursor);

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", base_dir, path_tok.string_value);

    bool opt_fast_textures = false;
    bool opt_z_up          = true;

    const char* saved_cursor = *cursor;
    token_t     peek_tok     = get_next_token(cursor);

    if (peek_tok.type == TOK_LBRACE) {
        while (true) {
            token_t inner_tok = get_next_token(cursor);
            if (inner_tok.type == TOK_RBRACE)
                break;

            if (inner_tok.type == TOK_IDENTIFIER) {
                if (strcmp(inner_tok.string_value, "UseFastTextures") == 0) {
                    opt_fast_textures = (strcmp(get_next_token(cursor).string_value, "true") == 0);
                } else if (strcmp(inner_tok.string_value, "ZUp") == 0) {
                    opt_z_up = (strcmp(get_next_token(cursor).string_value, "true") == 0);
                }
            }
        }
    } else {
        // Rewind if it wasn't a brace
        *cursor = saved_cursor;
    }

    log_info(
        "Baking Model %d: %s (FastTex: %d, ZUp: %d)",
        model_id,
        full_path,
        opt_fast_textures,
        opt_z_up
    );
    bake_model(full_path, out_scene, model_id, opt_fast_textures, opt_z_up);
}

static void parse_entity_statement(const char** cursor, scene_desc_t* out_scene) {
    token_t name_tok = get_next_token(cursor);
    get_next_token(cursor); // Consume '{'

    pak_entity_t new_entity = {0};
    new_entity.transform    = mat4_identity();

    vec3_t pos   = {0.0f, 0.0f, 0.0f};
    vec3_t rot   = {0.0f, 0.0f, 0.0f};
    vec3_t scale = {1.0f, 1.0f, 1.0f};

    while (true) {
        token_t inner_tok = get_next_token(cursor);
        if (inner_tok.type == TOK_RBRACE)
            break;

        if (inner_tok.type == TOK_IDENTIFIER) {
            if (strcmp(inner_tok.string_value, "ModelID") == 0) {
                new_entity.model_id = (uint32_t)get_next_token(cursor).float_value;
            } else if (strcmp(inner_tok.string_value, "Position") == 0) {
                pos.x = get_next_token(cursor).float_value;
                pos.y = get_next_token(cursor).float_value;
                pos.z = get_next_token(cursor).float_value;
            } else if (strcmp(inner_tok.string_value, "Rotation") == 0) {
                rot.x = get_next_token(cursor).float_value;
                rot.y = get_next_token(cursor).float_value;
                rot.z = get_next_token(cursor).float_value;
            } else if (strcmp(inner_tok.string_value, "Scale") == 0) {
                scale.x = get_next_token(cursor).float_value;
                scale.y = get_next_token(cursor).float_value;
                scale.z = get_next_token(cursor).float_value;
            }
        }
    }

    float rx = rot.x * (M_PI / 180.0f);
    float ry = rot.y * (M_PI / 180.0f);
    float rz = rot.z * (M_PI / 180.0f);

    mat4_t t_mat = mat4_translate(pos);
    mat4_t s_mat = mat4_scale(scale);

    mat4_t rot_x = mat4_rotate_x(rx);
    mat4_t rot_y = mat4_rotate_y(ry);
    mat4_t rot_z = mat4_rotate_z(rz);

    mat4_t r_mat         = mat4_mul(rot_y, mat4_mul(rot_x, rot_z));
    mat4_t sr_mat        = mat4_mul(s_mat, r_mat);
    new_entity.transform = mat4_mul(sr_mat, t_mat);

    out_scene->entities[out_scene->entity_count++] = new_entity;
    log_info("Baked Entity: %s (Model %d)", name_tok.string_value, new_entity.model_id);
}

bool parse_scene_file(const char* text, const char* base_dir, scene_desc_t* out_scene) {
    const char* cursor = text;

    while (true) {
        token_t next = get_next_token(&cursor);
        if (next.type == TOK_EOF) {
            break;
        }
        if (next.type == TOK_ERROR) {
            log_error("LEXER ERROR: Unexpected character near '%.10s'", cursor);
            return false;
        }
        if (next.type == TOK_IDENTIFIER) {
            if (strcmp(next.string_value, "Include") == 0) {
                parse_include_statement(&cursor, base_dir, out_scene);
            } else if (strcmp(next.string_value, "Model") == 0) {
                parse_model_statement(&cursor, base_dir, out_scene);
            } else if (strcmp(next.string_value, "Entity") == 0) {
                parse_entity_statement(&cursor, out_scene);
            }
        }
    }
    return true;
}
