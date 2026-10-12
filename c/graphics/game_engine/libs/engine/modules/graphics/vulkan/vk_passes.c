
#include <stdint.h>
#include <stdlib.h>

#include "engine/modules/graphics/graphics.h"
#include "engine/modules/graphics/graphics_types.h"
#include "engine\core\frustum.h"
#include "shared/math_types.h"
#include "shared/scene_types.h"
#include "vk_pipeline.h"
#include "vk_render_queue.h"
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

#include "vk_passes.h"
#include "vk_types.h"

void shadow_pass(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    vk_render_target_t*     shadow_render_target,
    render_stats_t*         out_render_stats
) {
    if (shadow_render_target == NULL || !shadow_render_target->has_depth) {
        return;
    }

    VkCommandBuffer cmd = graphics->command_buffer;

    VkImageMemoryBarrier shadow_write_barrier = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .image            = shadow_render_target->depth_attachment.image,
        .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
        .srcAccessMask    = 0,
        .dstAccessMask    = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    };
    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &shadow_write_barrier
    );

    VkRenderingAttachmentInfo shadow_depth_att = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = shadow_render_target->depth_attachment.view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = {.depthStencil = {1.0f, 0}}
    };
    VkRenderingInfo shadow_info = {
        .sType            = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea       = {{0, 0}, {shadow_render_target->width, shadow_render_target->height}},
        .layerCount       = 1,
        .pDepthAttachment = &shadow_depth_att
    };
    vkCmdBeginRendering(cmd, &shadow_info);

    VkViewport shadow_vp = {
        .x        = 0.0f,
        .y        = (float)shadow_render_target->height,
        .width    = (float)shadow_render_target->width,
        .height   = -(float)shadow_render_target->height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D shadow_sc = {{0, 0}, {shadow_render_target->width, shadow_render_target->height}};
    vkCmdSetViewport(cmd, 0, 1, &shadow_vp);
    vkCmdSetScissor(cmd, 0, 1, &shadow_sc);

    // Bind Pipeline and Globals
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics->pipelines.shadow);
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphics->pipelines.layout,
        0,
        1,
        &graphics->frames[graphics->current_frame].global_descriptor_set,
        0,
        NULL
    );

    // Draw Geometry
    VkDeviceSize offsets[] = {0};
    for (uint32_t i = 0; i < gfx_frame_input->scene->object_count; i++) {
        render_object_t* obj = &gfx_frame_input->scene->objects[i];

        if (obj->mesh.id == GRAPHICS_INVALID_HANDLE)
            continue;

        vk_mesh_t* vk_mesh = &graphics->assets.meshes[obj->mesh.id];
        if (!vk_mesh->is_active)
            continue;

        push_constants_t push = {.transform = obj->transform};

        vkCmdPushConstants(
            cmd,
            graphics->pipelines.layout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(push_constants_t),
            &push
        );

        vkCmdBindVertexBuffers(cmd, 0, 1, &vk_mesh->vertex_buffer, offsets);

        uint32_t shadow_pass_drawn_triangles = 0;
        if (vk_mesh->index_count > 0) {
            vkCmdBindIndexBuffer(cmd, vk_mesh->index_buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, vk_mesh->index_count, 1, 0, 0, 0);
            if (out_render_stats)
                shadow_pass_drawn_triangles = vk_mesh->index_count / 3;
        } else {
            vkCmdDraw(cmd, vk_mesh->vertex_count, 1, 0, 0);
            if (out_render_stats)
                shadow_pass_drawn_triangles = vk_mesh->vertex_count / 3;
        }

        if (out_render_stats) {
            out_render_stats->shadow_pass_drawn_meshes++;
            out_render_stats->shadow_pass_draw_calls++;
            out_render_stats->shadow_pass_drawn_triangles += shadow_pass_drawn_triangles;
        }
    }

    vkCmdEndRendering(cmd);

    // Transition Shadow Map to Readable by the Fragment Shader
    VkImageMemoryBarrier shadow_read_barrier = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .image            = shadow_render_target->depth_attachment.image,
        .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
        .srcAccessMask    = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT
    };
    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &shadow_read_barrier
    );
}

static void forward_pass_begin(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    vk_render_target_t*     render_target,
    uint32_t                image_index
) {
    uint32_t    render_width        = graphics->display.extent.width;
    uint32_t    render_height       = graphics->display.extent.height;
    VkImage     color_image         = graphics->display.images[image_index];
    VkImageView color_view          = graphics->display.image_views[image_index];
    VkImageView depth_view          = graphics->display.depth_view;
    VkImage     current_depth_image = graphics->display.depth_image;

    if (gfx_frame_input->draw_mode != DRAW_MODE_DEBUG_SDR && render_target != NULL) {
        render_width  = render_target->width;
        render_height = render_target->height;
        color_image   = render_target->color_attachment.image;
        color_view    = render_target->color_attachment.view;
        if (render_target->has_depth) {
            depth_view          = render_target->depth_attachment.view;
            current_depth_image = render_target->depth_attachment.image;
        }
    }

    VkImageMemoryBarrier barriers[2]   = {0};
    uint32_t             barrier_count = 0;

    barriers[barrier_count++] = (VkImageMemoryBarrier){
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image            = color_image,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        .srcAccessMask    = 0,
        .dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    if (current_depth_image != VK_NULL_HANDLE) {
        barriers[barrier_count++] = (VkImageMemoryBarrier){
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .image            = current_depth_image,
            .subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1},
            .srcAccessMask    = 0,
            .dstAccessMask    = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
        };
    }

    vkCmdPipelineBarrier(
        graphics->command_buffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        barrier_count,
        barriers
    );

    VkRenderingAttachmentInfo color_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = color_view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = {{{
            gfx_frame_input->environment->ambient_tint.x,
            gfx_frame_input->environment->ambient_tint.y,
            gfx_frame_input->environment->ambient_tint.z,
            1.0f,
        }}}
    };

    VkRenderingAttachmentInfo depth_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = depth_view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue  = {.depthStencil = {1.0f, 0}}
    };

    VkRenderingInfo rendering_info = {
        .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea           = {{0, 0}, {render_width, render_height}},
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &color_attachment,
        .pDepthAttachment     = (current_depth_image != VK_NULL_HANDLE) ? &depth_attachment : NULL
    };

    vkCmdBeginRendering(graphics->command_buffer, &rendering_info);

    VkViewport viewport = {
        .x        = 0.0f,
        .y        = (float)render_height,
        .width    = (float)render_width,
        .height   = -(float)render_height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor = {{0, 0}, {render_width, render_height}};
    vkCmdSetViewport(graphics->command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(graphics->command_buffer, 0, 1, &scissor);
}

static void forward_pass_draw_scene(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    render_stats_t*         out_render_stats
) {
#define MAX_RENDER_COMMANDS 10000
    render_command_t render_queue[MAX_RENDER_COMMANDS];
    uint32_t         command_count = build_render_queue(
        graphics, gfx_frame_input, render_queue, MAX_RENDER_COMMANDS
    );
    qsort(render_queue, command_count, sizeof(render_command_t), render_command_compare);
    VkPipeline pipeline_map[QUEUE_MAX_COUNT];
    build_pipeline_map(graphics, gfx_frame_input->draw_mode, pipeline_map);
    execute_render_queue(
        graphics, gfx_frame_input, render_queue, command_count, pipeline_map, out_render_stats
    );
}

static void forward_pass_draw_extras(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    render_stats_t*         out_render_stats
) {
    VkDeviceSize offsets[] = {0};

    // Draw Skybox
    if (gfx_frame_input->draw_mode != DRAW_MODE_DEBUG_SDR) {
        vkCmdBindPipeline(
            graphics->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics->pipelines.skybox
        );
    }
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

    vkCmdDraw(graphics->command_buffer, 36, 1, 0, 0);
    if (out_render_stats) {
        out_render_stats->forward_pass_draw_calls++;
        out_render_stats->forward_pass_drawn_triangles += 12;
    }

    // Draw Debug Widgets
    if (gfx_frame_input->show_debug_widgets) {
        if (gfx_frame_input->draw_mode != DRAW_MODE_DEBUG_SDR) {
            vkCmdBindPipeline(
                graphics->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics->pipelines.line
            );

            push_constants_t debug_pc = {
                .transform       = mat4_identity(),
                .is_alpha_masked = 0,
            };

            vkCmdPushConstants(
                graphics->command_buffer,
                graphics->pipelines.layout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(push_constants_t),
                &debug_pc
            );

            // Grid
            vkCmdBindVertexBuffers(
                graphics->command_buffer, 0, 1, &graphics->grid_buffer.buffer, offsets
            );
            vkCmdDraw(graphics->command_buffer, graphics->grid_vertex_count, 1, 0, 0);
            if (out_render_stats)
                out_render_stats->forward_pass_draw_calls++;

            // Sun Line
            vkCmdBindVertexBuffers(
                graphics->command_buffer, 0, 1, &graphics->sun_line_buffer.buffer, offsets
            );
            vkCmdDraw(graphics->command_buffer, 2, 1, 0, 0);
            if (out_render_stats)
                out_render_stats->forward_pass_draw_calls++;
        }

        // Frozen Frustum
        if (gfx_frame_input->is_culling_frozen) {
            push_constants_t frustum_pc = {
                .transform       = mat4_identity(),
                .is_alpha_masked = 0,
            };

            vkCmdPushConstants(
                graphics->command_buffer,
                graphics->pipelines.layout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(push_constants_t),
                &frustum_pc
            );

            vkCmdBindVertexBuffers(
                graphics->command_buffer, 0, 1, &graphics->frustum_buffer.buffer, offsets
            );
            vkCmdDraw(graphics->command_buffer, 24, 1, 0, 0);
            if (out_render_stats)
                out_render_stats->forward_pass_draw_calls++;
        }
    }
}

void forward_pass(
    graphics_t*             graphics,
    graphics_frame_input_t* gfx_frame_input,
    vk_render_target_t*     render_target,
    uint32_t                image_index,
    render_stats_t*         out_render_stats
) {
    forward_pass_begin(graphics, gfx_frame_input, render_target, image_index);
    forward_pass_draw_scene(graphics, gfx_frame_input, out_render_stats);
    forward_pass_draw_extras(graphics, gfx_frame_input, out_render_stats);
}

void post_process_pass(
    graphics_t*         graphics,
    vk_render_target_t* render_target,
    uint32_t            image_index
) {
    vkCmdEndRendering(graphics->command_buffer);

    VkImageMemoryBarrier barriers[2] = {
        {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout        = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .image            = render_target->color_attachment.image,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            .srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask    = VK_ACCESS_SHADER_READ_BIT,
        },
        {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image            = graphics->display.images[image_index],
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
            .srcAccessMask    = 0,
            .dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        }
    };

    vkCmdPipelineBarrier(
        graphics->command_buffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        2,
        barriers
    );

    VkRenderingAttachmentInfo swap_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = graphics->display.image_views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
    };

    VkRenderingInfo pp_rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea =
            {
                {0, 0},
                {graphics->display.extent.width, graphics->display.extent.height},
            },
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &swap_attachment,
        .pDepthAttachment     = NULL
    };

    vkCmdBeginRendering(graphics->command_buffer, &pp_rendering_info);

    VkViewport viewport = {
        .x        = 0.0f,
        .y        = (float)graphics->display.extent.height,
        .width    = (float)graphics->display.extent.width,
        .height   = -(float)graphics->display.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor = {{0, 0}, graphics->display.extent};

    vkCmdSetViewport(graphics->command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(graphics->command_buffer, 0, 1, &scissor);

    vkCmdBindPipeline(
        graphics->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics->pipelines.post_process
    );
    vkCmdBindDescriptorSets(
        graphics->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphics->pipelines.post_process_layout,
        0,
        1,
        &render_target->color_attachment.descriptor_set,
        0,
        NULL
    );
    vkCmdDraw(graphics->command_buffer, 3, 1, 0, 0);
}
