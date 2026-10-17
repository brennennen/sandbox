

#include "vk_render_queue.h"
#include "engine\core\frustum.h"
#include "shared/scene_types.h"
#include "vk_pipeline.h"
#include "vk_types.h"

#include "engine/core/frustum.h"
#include "engine/core/logger.h"
#include "engine/core/math/mat4.h"
#include "engine/modules/assets/obj.h"
#include "engine/modules/debug_imgui/debug_imgui.h"
#include "engine/modules/graphics/debug/debug_grid.h"
#include "engine/modules/graphics/graphics.h"
#include "engine/modules/graphics/graphics_types.h"
#include "shared/math_types.h"
#include "shared/scene_types.h"

render_queue_id_t vk_evaluate_material_queue(const vk_material_t* mat) {
    if (mat->is_translucent) {
        return QUEUE_TRANSLUCENT;
    }
    if (mat->is_alpha_masked && mat->is_double_sided) {
        return QUEUE_ALPHA_MASKED_DOUBLE_SIDED;
    }
    if (mat->is_alpha_masked) {
        return QUEUE_ALPHA_MASKED;
    }
    if (mat->is_double_sided) {
        return QUEUE_OPAQUE_DOUBLE_SIDED;
    }

    return QUEUE_OPAQUE;
}

int render_command_compare(const void* a, const void* b) {
    const render_command_t* cmd_a = (const render_command_t*)a;
    const render_command_t* cmd_b = (const render_command_t*)b;

    if (cmd_a->sort_key < cmd_b->sort_key)
        return -1;
    if (cmd_a->sort_key > cmd_b->sort_key)
        return 1;
    return 0;
}

uint64_t vk_generate_sort_key(
    render_queue_id_t queue_id,
    uint16_t          material_id,
    vec3_t            camera_pos,
    vec3_t            object_pos
) {
    // Depth: Front-to-back sorting for early-Z rejection optimization
    float dist_x  = camera_pos.x - object_pos.x;
    float dist_y  = camera_pos.y - object_pos.y;
    float dist_z  = camera_pos.z - object_pos.z;
    float dist_sq = (dist_x * dist_x) + (dist_y * dist_y) + (dist_z * dist_z);

    // IEEE-754 floats cast directly to uint32 maintain sorting order when positive
    uint32_t depth_bits = *(uint32_t*)&dist_sq;

    // Translucency MUST sort back-to-front.
    // Flipping the bits reverses the integer sort, putting furthest objects first!
    if (queue_id == QUEUE_TRANSLUCENT) {
        depth_bits = ~depth_bits;
    }

    uint64_t q_id   = (uint64_t)queue_id;
    uint64_t m_id   = (uint64_t)material_id;
    uint64_t d_bits = (uint64_t)depth_bits;

    // By placing q_id at the highest 16 bits, qsort guarantees that ALL Opaque
    // objects draw before ALL Double-Sided, which draw before ALL Alpha-Masked, etc.
    return (q_id << 48) | (m_id << 32) | d_bits;
}

uint32_t build_render_queue(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    render_command_t*       out_queue,
    uint32_t                max_commands
) {
    frustum_t frustum       = frustum_extract(gfx_frame_input->culling_view_proj);
    uint32_t  command_count = 0;

    for (uint32_t i = 0; i < gfx_frame_input->scene->object_count; i++) {
        render_object_t* obj = &gfx_frame_input->scene->objects[i];

        if (obj->mesh.id == GRAPHICS_INVALID_HANDLE ||
            obj->material.id == GRAPHICS_INVALID_HANDLE) {
            continue;
        }

        vk_mesh_t*     vk_mesh = &graphics->assets.meshes[obj->mesh.id];
        vk_material_t* vk_mat  = &graphics->assets.materials[obj->material.id];

        if (!vk_mesh->is_active || !vk_mat->is_active)
            continue;

        vec3_t obj_pos = mat4_transform_point(obj->transform, vk_mesh->bounding_center);
        float  scale   = sqrtf(
            obj->transform.m[0][0] * obj->transform.m[0][0] +
            obj->transform.m[0][1] * obj->transform.m[0][1] +
            obj->transform.m[0][2] * obj->transform.m[0][2]
        );
        float bounding_radius = vk_mesh->bounding_radius * scale;

        if (!frustum_test_sphere(&frustum, obj_pos, bounding_radius))
            continue;

        render_queue_id_t queue_id = vk_evaluate_material_queue(vk_mat);
        uint64_t          sort_key = vk_generate_sort_key(
            queue_id, obj->material.id, gfx_frame_input->camera_pos, obj_pos
        );

        if (command_count < max_commands) {
            out_queue[command_count].sort_key     = sort_key;
            out_queue[command_count].object_index = i;
            command_count++;
        }
    }
    return command_count;
}

void build_pipeline_map(graphics_t* graphics, draw_mode_t draw_mode, VkPipeline* out_pipeline_map) {
    for (int i = 0; i < QUEUE_MAX_COUNT; i++) {
        out_pipeline_map[i] = VK_NULL_HANDLE;
    }

    if (draw_mode == DRAW_MODE_FORWARD_LIT) {
        out_pipeline_map[QUEUE_OPAQUE]              = graphics->pipelines.forward_lit;
        out_pipeline_map[QUEUE_OPAQUE_DOUBLE_SIDED] = graphics->pipelines.forward_lit_double_sided;
        out_pipeline_map[QUEUE_ALPHA_MASKED]        = graphics->pipelines.forward_lit_alpha_masked;
        out_pipeline_map[QUEUE_ALPHA_MASKED_DOUBLE_SIDED] =
            graphics->pipelines.forward_lit_alpha_masked_double_sided;
        out_pipeline_map[QUEUE_TRANSLUCENT] = graphics->pipelines.transparent;
    } else {
        // Debug modes override all queues with the exact same debug pipeline
        VkPipeline debug_pipe = get_draw_mode_pipeline(&graphics->pipelines, draw_mode);
        for (int q = 0; q < QUEUE_MAX_COUNT; q++) {
            out_pipeline_map[q] = debug_pipe;
        }
    }
}

void execute_render_queue(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    render_command_t*       render_queue,
    uint32_t                command_count,
    VkPipeline*             pipeline_map,
    render_stats_t*         out_render_stats
) {
    VkPipeline   bound_pipeline    = VK_NULL_HANDLE;
    uint32_t     bound_material_id = UINT32_MAX;
    VkDeviceSize offsets[]         = {0};

    // Bind shared global descriptor set once
    vkCmdBindDescriptorSets(
        graphics->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphics->pipelines.layout,
        0,
        1,
        &graphics->frames[graphics->current_frame].global_descriptor_set,
        0,
        NULL
    );

    for (uint32_t i = 0; i < command_count; i++) {
        render_object_t* obj     = &gfx_frame_input->scene->objects[render_queue[i].object_index];
        vk_mesh_t*       vk_mesh = &graphics->assets.meshes[obj->mesh.id];
        vk_material_t*   vk_mat  = &graphics->assets.materials[obj->material.id];

        // Pipeline State Swap
        // TOOD: clean this up
        uint16_t current_queue_id = (render_queue[i].sort_key >> 48) & 0xFFFF;
        if (current_queue_id >= QUEUE_MAX_COUNT)
            current_queue_id = QUEUE_OPAQUE; // Safety fallback

        VkPipeline desired_pipeline = pipeline_map[current_queue_id];
        if (desired_pipeline == VK_NULL_HANDLE)
            desired_pipeline = pipeline_map[QUEUE_OPAQUE];

        if (desired_pipeline != bound_pipeline) {
            vkCmdBindPipeline(
                graphics->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, desired_pipeline
            );
            bound_pipeline = desired_pipeline;
        }

        // Material State Swap
        if (obj->material.id != bound_material_id) {
            vkCmdBindDescriptorSets(
                graphics->command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphics->pipelines.layout,
                1,
                1,
                &vk_mat->descriptor_set,
                0,
                NULL
            );
            bound_material_id = obj->material.id;
        }

        // Push Constants
        push_constants_t push_constants = {
            .transform        = obj->transform,
            .is_alpha_masked  = vk_mat->is_alpha_masked ? 1 : 0,
            .debug_mode       = gfx_frame_input->draw_mode,
            .metallic_factor  = vk_mat->metallic_factor,
            .roughness_factor = vk_mat->roughness_factor,
            .alpha_cutoff     = vk_mat->alpha_cutoff,
        };

        vkCmdPushConstants(
            graphics->command_buffer,
            graphics->pipelines.layout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(push_constants_t),
            &push_constants
        );

        // Draw
        vkCmdBindVertexBuffers(graphics->command_buffer, 0, 1, &vk_mesh->vertex_buffer, offsets);

        uint32_t drawn_triangles = 0;
        if (vk_mesh->index_count > 0) {
            vkCmdBindIndexBuffer(
                graphics->command_buffer, vk_mesh->index_buffer, 0, VK_INDEX_TYPE_UINT32
            );
            vkCmdDrawIndexed(graphics->command_buffer, vk_mesh->index_count, 1, 0, 0, 0);
            drawn_triangles = vk_mesh->index_count / 3;
        } else {
            vkCmdDraw(graphics->command_buffer, vk_mesh->vertex_count, 1, 0, 0);
            drawn_triangles = vk_mesh->vertex_count / 3;
        }

        if (out_render_stats) {
            out_render_stats->forward_pass_drawn_meshes++;
            out_render_stats->forward_pass_draw_calls++;
            out_render_stats->forward_pass_drawn_triangles += drawn_triangles;
        }
    }
}
