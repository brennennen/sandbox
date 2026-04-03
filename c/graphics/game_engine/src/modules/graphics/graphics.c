
#include "graphics.h"
// #include "math3d.h"
// #include "vulkan/vk_backend.h"

const char* const draw_mode_names[] = {
    [DRAW_MODE_LIT]             = "LIT",
    [DRAW_MODE_DEBUG_WIREFRAME] = "DEBUG_WIREFRAME",
    [DRAW_MODE_DEBUG_LIGHTING]  = "DEBUG_LIGHTING_ONLY",
    [DRAW_MODE_DEBUG_ALBEDO]    = "DEBUG_ALBEDO",
    [DRAW_MODE_DEBUG_NORMAL]    = "DEBUG_NORMAL"
};

// void renderer_submit_mesh(
//     uint32_t mesh_id,
//     mat4_t   world_from_model,
//     mat4_t   view_from_world,
//     mat4_t   projection_from_view
// ) {
//     // https://youtu.be/CXEgyA2aGTM?t=3341
//     /*
//     mat4 projection;
//     mat4 view;
//     mat4 model;

//     vec4 pos;

//     mat4 project_from_view;
//     mat4 view_from_world;
//     mat4 world_from_model;

//     vect4 model_project = projection_from_view * view_from_world * world_from_model *
//     model_position;
//     */
//     // dominoes: [view_from_WORLD] * [WORLD_from_model] -> [view_from_model]
//     mat4_t view_from_model       = mat4_mul(view_from_world, world_from_model);
//     mat4_t projection_from_model = mat4_mul(projection_from_view, view_from_model);
//     vk_backend_push_draw_command(mesh_id, projection_from_model);
// }
