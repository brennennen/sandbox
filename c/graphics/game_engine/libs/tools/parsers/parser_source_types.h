#ifndef PARSER_SOURCE_TYPES_H
#define PARSER_SOURCE_TYPES_H

#include <stdint.h>

#include "shared/scene_types.h"

typedef enum {
    TOK_OLD_EOF,
    TOK_OLD_ERROR,
    TOK_OLD_LBRACE,
    TOK_OLD_RBRACE,
    TOK_OLD_IDENTIFIER,
    TOK_OLD_STRING,
    TOK_OLD_NUMBER
} token_type_old_t;

typedef struct {
    token_type_old_t type;
    char             string_value[256];
    float            float_value;
} token_old_t;

typedef struct {
    char     source_file[512];
    uint32_t model_id;
    bool     opt_fast_textures;
    bool     opt_z_up;
} model_src_old_t;

typedef struct {
    bool   is_active;
    char   skybox_path[512];
    vec3_t ambient_tint;
    float  fog_density;
} environment_src_old_t;

typedef struct {
    environment_src_old_t environment;

    model_src_old_t models[256];
    uint32_t        model_count;

    pak_entity_t entities[PAK_MAX_ENTITIES];
    uint32_t     entity_count;
} scene_src_t;

#endif // PARSER_SOURCE_TYPES_H
