#ifndef GRAPHICS_TYPES_H
#define GRAPHICS_TYPES_H

#include <stdint.h>

// #include "engine/core/game_engine.h"
#include "engine/core/math/mat4.h"
// #include "engine/core/scene.h"

#include "shared/math_types.h"

/**
 * What pipeline and shaders to use.
 */
typedef enum {
    DRAW_MODE_FORWARD_LIT,           // standard rendering
    DRAW_MODE_DEBUG_WIREFRAME,       // only render edges, no faces
    DRAW_MODE_DEBUG_ALBEDO,          // render faces unlit with albedo
    DRAW_MODE_DEBUG_LIGHTING,        // render faces with lighting information, but no albedo
    DRAW_MODE_DEBUG_GEOMETRY_NORMAL, // render faces with geometry normal as face color
    DRAW_MODE_DEBUG_TEXTURE_NORMAL,  // render faces with texture normal as face color
    DRAW_MODE_DEBUG_NORMAL,          // combined normal
    DRAW_MODE_DEBUG_TANGENT,
    DRAW_MODE_DEBUG_BITANGENT,
    DRAW_MODE_DEBUG_VERTEX_COLOR, // render the vertex color
    DRAW_MODE_DEBUG_MIPMAPS,
    DRAW_MODE_DEBUG_SPECULAR,
    DRAW_MODE_DEBUG_AO,
    DRAW_MODE_DEBUG_ROUGHNESS,
    DRAW_MODE_DEBUG_METALLIC,
    DRAW_MODE_DEBUG_SDR,

    DRAW_MODE_COUNT,
} draw_mode_t;
extern const char* const draw_mode_names[];

// typedef struct scenes        scene_t;
typedef struct environment_s environment_t;
typedef struct scene_s       scene_t;

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

    vec3_t bounding_center;
    float  bounding_radius;
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

typedef struct {
    uint32_t id;
} render_target_handle_t;

typedef enum {
    RT_FORMAT_DEPTH_ONLY, // Depth only
    RT_FORMAT_STANDARD,   // Maps to 8-bit UNORM (Standard Color)
    RT_FORMAT_HDR,        // Maps to 16-bit SFLOAT (High Dynamic Range)
} render_target_format_t;

typedef struct {
    uint32_t               width;
    uint32_t               height;
    render_target_format_t format;
    bool                   requires_depth; // True if drawing 3D geometry to this target
} render_target_config_t;

typedef struct {
    // Render Targets
    render_target_handle_t target;
    render_target_handle_t shadow_target;

    // Camera & Culling
    mat4_t view;
    mat4_t proj;
    vec3_t camera_pos;
    mat4_t culling_view_proj;
    bool   is_culling_frozen;

    // Scene & Environment
    scene_t*       scene;
    environment_t* environment;
    mat4_t         light_space_matrix;

    // Render State
    draw_mode_t draw_mode;
} graphics_frame_input_t;

#endif
