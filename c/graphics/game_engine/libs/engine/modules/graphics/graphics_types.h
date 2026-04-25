#ifndef GRAPHICS_TYPES_H
#define GRAPHICS_TYPES_H

#include <stdint.h>

// #include "engine/core/math/math_types.h"

#include "shared/math_types.h"

typedef struct {
    float u;
    float v;
} uv_elements_t;

typedef union {
    float data[2];
    struct {
        float u;
        float v;
    };
} uv_t;

typedef struct {
    float r;
    float g;
    float b;
    float a;
} color_t;

typedef struct {
    vec3_t pos;
    vec4_t color;
    uv_t   uv;
    vec3_t normal;
    vec4_t tangent;
} vertex_t;

typedef struct {
    vertex_t* vertices;
    uint32_t  vertex_count;

    uint32_t* indices;
    uint32_t  index_count;
} mesh_data_t;

typedef struct {
    uint32_t id;
} mesh_handle_t;

typedef struct {
    uint32_t id;
} texture_handle_t;

typedef struct {
    uint32_t id;
} material_handle_t;

typedef struct {
    uint32_t          first_index;
    uint32_t          index_count;
    material_handle_t material;
} submesh_t;

typedef struct {
    mesh_handle_t mesh;
    // texture_handle_t  texture;
    // texture_handle_t  normal_texture;
    material_handle_t material;
    mat4_t            transform;
} render_object_t;

#endif
