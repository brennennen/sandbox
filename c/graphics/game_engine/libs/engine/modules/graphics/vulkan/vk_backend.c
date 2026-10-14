
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

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

#include "vk_commands.h"
#include "vk_debug.h"
#include "vk_devices.h"
#include "vk_gpu_allocator.h"
#include "vk_passes.h"
#include "vk_pipeline.h"
#include "vk_render_queue.h"
#include "vk_render_target.h"
#include "vk_resources.h"
#include "vk_swapchain.h"
#include "vk_types.h"
#include "volk.h"

void update_uniform_buffer(
    graphics_t*             r,
    graphics_frame_input_t* gfx_frame_input,
    mat4_t                  view,
    mat4_t                  proj,
    vec3_t                  cam_pos,
    mat4_t                  light_space_matrix,
    vec3_t                  sun_direction,
    vec3_t                  sun_color,
    float                   sun_intensity,
    uint32_t                current_frame
) {
    ubo_t ubo = {
        .view               = view,
        .proj               = proj,
        .light_space_matrix = light_space_matrix,
        .camera_pos         = {cam_pos.x, cam_pos.y, cam_pos.z, 1.0f},
        .sun_direction      = {sun_direction.x, sun_direction.y, sun_direction.z, 0.0f},
        .sun_color          = {sun_color.x, sun_color.y, sun_color.z, sun_intensity},
        .roughness_bias     = gfx_frame_input->environment->roughness_bias,
        .metallic_bias      = gfx_frame_input->environment->metallic_bias,
    };

    memcpy(r->frames[current_frame].uniform_alloc.mapped_ptr, &ubo, sizeof(ubo));
}

static bool init_memory_heaps(graphics_t* r) {
    r->assets.vertex_heap = gpu_heap_create(
        r,
        1ULL * 1024 * 1024 * 1024,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    // TODO: check out AMD VMA
    r->assets.device_heap = gpu_heap_create(
        r, 12ULL * 1024 * 1024 * 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    r->assets.display_heap = gpu_heap_create(
        r, 1024 * 1024 * 32, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    if (!r->assets.vertex_heap || !r->assets.device_heap) {
        log_error("vulkan: failed to create GPU memory heaps");
        return false;
    }
    return true;
}

static bool init_uniform_buffer(graphics_t* r) {
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        VkBufferCreateInfo ubo_info = {
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = sizeof(ubo_t),
            .usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        log_info("vkCreateBuffer - uniform_buffer: %d", ubo_info.size);
        vkCreateBuffer(r->core.device, &ubo_info, NULL, &r->frames[i].uniform_buffer);

        VkMemoryRequirements mem_reqs;
        vkGetBufferMemoryRequirements(r->core.device, r->frames[i].uniform_buffer, &mem_reqs);

        r->frames[i].uniform_alloc = gpu_heap_alloc(
            r->assets.vertex_heap, mem_reqs.size, mem_reqs.alignment
        );
        vkBindBufferMemory(
            r->core.device,
            r->frames[i].uniform_buffer,
            r->assets.vertex_heap->memory,
            r->frames[i].uniform_alloc.offset
        );
    }
    return true;
}

static bool init_descriptors(graphics_t* r) {
    VkDescriptorPoolSize pool_sizes[] = {
        {
            .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1024,
        },
        {
            .type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1024,
        },
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 2,
        .pPoolSizes    = pool_sizes,
        .maxSets       = 1024,
    };

    if (vkCreateDescriptorPool(r->core.device, &pool_info, NULL, &r->descriptor_pool) !=
        VK_SUCCESS) {
        log_error("vulkan: failed to create descriptor pool");
        return false;
    }

    VkDescriptorSetLayout global_layouts[FRAMES_IN_FLIGHT];
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        global_layouts[i] = r->pipelines.global_set_layout;
    }

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = r->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &r->pipelines.global_set_layout,
    };

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (vkAllocateDescriptorSets(
                r->core.device, &alloc_info, &r->frames[i].global_descriptor_set
            ) != VK_SUCCESS) {
            return false;
        }

        VkDescriptorBufferInfo buffer_info = {
            .buffer = r->frames[i].uniform_buffer, .offset = 0, .range = sizeof(ubo_t)
        };
        VkWriteDescriptorSet descriptor_write = {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = r->frames[i].global_descriptor_set,
            .dstBinding      = 0,
            .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo     = &buffer_info
        };
        vkUpdateDescriptorSets(r->core.device, 1, &descriptor_write, 0, NULL);
    }
    return true;
}

static bool init_sync_objects(graphics_t* graphics) {
    VkSemaphoreCreateInfo sem_info   = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo     fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(
                graphics->core.device, &sem_info, NULL, &graphics->frames[i].image_available_sem
            ) != VK_SUCCESS ||
            vkCreateSemaphore(
                graphics->core.device, &sem_info, NULL, &graphics->frames[i].render_finished_sem
            ) != VK_SUCCESS ||
            vkCreateFence(
                graphics->core.device, &fence_info, NULL, &graphics->frames[i].in_flight_fence
            ) != VK_SUCCESS) {
            return false;
        }
    }
    graphics->current_frame = 0;
    return true;
}

graphics_t* graphics_create(platform_t* platform, graphics_config_t* config) {
    graphics_t* graphics = calloc(1, sizeof(struct graphics_t));
    if (!graphics) {
        log_error("renderer: failed to allocate memory for graphics_t");
        return NULL;
    }

    if (!vk_create_instance(graphics, platform) ||
        !platform_create_vulkan_surface(
            platform, graphics->core.instance, &graphics->core.surface
        ) ||
        !vk_pick_physical_device(graphics) || !vk_create_logical_device(graphics)) {
        goto init_failed;
    }

    if (!init_memory_heaps(graphics) || !init_uniform_buffer(graphics) ||
        !vk_create_commands(graphics) ||
        !vk_setup_depth_buffer(graphics, config->width, config->height)) {
        goto init_failed;
    }

    graphics->display.abstract_present_mode = config->present_mode;
    if (!vk_create_swapchain(
            graphics, config->width, config->height, config->present_mode, VK_NULL_HANDLE
        ) ||
        !vk_create_graphics_pipeline(graphics) || !init_descriptors(graphics)) {
        goto init_failed;
    }

    VkSemaphoreCreateInfo sem_info = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (int i = 0; i < 3; i++) {
        if (vkCreateSemaphore(
                graphics->core.device, &sem_info, NULL, &graphics->swapchain_render_sems[i]
            ) != VK_SUCCESS) {
            log_error("Failed to create swapchain render semaphores!");
        }
    }

    if (!init_sync_objects(graphics)) {
        goto init_failed;
    }

    init_debug_grid(graphics);
    init_debug_frustum_buffer(graphics);
    init_debug_sun_line_buffer(graphics);

    if (!init_default_textures(graphics)) {
        goto init_failed;
    }

    log_info("renderer: initialization complete");
    return graphics;

init_failed:
    log_error("renderer: initialization aborted due to failure");
    graphics_destroy(graphics);
    return NULL;
}

void graphics_wait_idle(graphics_t* graphics) {
    if (graphics != NULL && graphics->core.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(graphics->core.device);
    }
}

void graphics_destroy(graphics_t* graphics) {
    if (graphics == NULL) {
        return;
    }

    if (graphics->core.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(graphics->core.device);

        for (uint32_t i = 0; i < VK_MAX_MESHES; i++) {
            vk_mesh_t* mesh = &graphics->assets.meshes[i];
            if (mesh->is_active) {
                if (mesh->vertex_buffer)
                    vkDestroyBuffer(graphics->core.device, mesh->vertex_buffer, NULL);
                if (mesh->index_count > 0 && mesh->index_buffer) {
                    vkDestroyBuffer(graphics->core.device, mesh->index_buffer, NULL);
                }
            }
        }

        for (uint32_t i = 0; i < VK_MAX_TEXTURES; i++) {
            vk_texture_t* tex = &graphics->assets.textures[i];
            if (tex->is_active) {
                if (tex->sampler)
                    vkDestroySampler(graphics->core.device, tex->sampler, NULL);
                if (tex->view)
                    vkDestroyImageView(graphics->core.device, tex->view, NULL);
                if (tex->image)
                    vkDestroyImage(graphics->core.device, tex->image, NULL);
            }
        }

        for (uint32_t i = 0; i < VK_MAX_RENDER_TARGETS; i++) {
            vk_render_target_t* rt = &graphics->assets.render_targets[i];
            if (rt->is_active) {
                if (rt->color_attachment.view) {
                    vkDestroySampler(graphics->core.device, rt->color_attachment.sampler, NULL);
                    vkDestroyImageView(graphics->core.device, rt->color_attachment.view, NULL);
                    vkDestroyImage(graphics->core.device, rt->color_attachment.image, NULL);
                }
                if (rt->has_depth && rt->depth_attachment.view) {
                    vkDestroySampler(graphics->core.device, rt->depth_attachment.sampler, NULL);
                    vkDestroyImageView(graphics->core.device, rt->depth_attachment.view, NULL);
                    vkDestroyImage(graphics->core.device, rt->depth_attachment.image, NULL);
                }
            }
        }

        if (graphics->assets.vertex_heap) {
            vkFreeMemory(graphics->core.device, graphics->assets.vertex_heap->memory, NULL);
            free(graphics->assets.vertex_heap);
        }
        if (graphics->assets.device_heap) {
            vkFreeMemory(graphics->core.device, graphics->assets.device_heap->memory, NULL);
            free(graphics->assets.device_heap);
        }
        if (graphics->assets.display_heap) {
            vkFreeMemory(graphics->core.device, graphics->assets.display_heap->memory, NULL);
            free(graphics->assets.display_heap);
        }

        if (graphics->grid_buffer.buffer) {
            vkDestroyBuffer(graphics->core.device, graphics->grid_buffer.buffer, NULL);
        }
        if (graphics->frustum_buffer.buffer) {
            vkDestroyBuffer(graphics->core.device, graphics->frustum_buffer.buffer, NULL);
        }
        if (graphics->sun_line_buffer.buffer) {
            vkDestroyBuffer(graphics->core.device, graphics->sun_line_buffer.buffer, NULL);
        }

        vk_destroy_graphics_pipeline(graphics);
        vk_destroy_commands(graphics);

        if (graphics->descriptor_pool) {
            vkDestroyDescriptorPool(graphics->core.device, graphics->descriptor_pool, NULL);
        }

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            if (graphics->frames[i].image_available_sem) {
                vkDestroySemaphore(
                    graphics->core.device, graphics->frames[i].image_available_sem, NULL
                );
            }
            if (graphics->frames[i].render_finished_sem) {
                vkDestroySemaphore(
                    graphics->core.device, graphics->frames[i].render_finished_sem, NULL
                );
            }
            if (graphics->frames[i].in_flight_fence) {
                vkDestroyFence(graphics->core.device, graphics->frames[i].in_flight_fence, NULL);
            }
            if (graphics->frames[i].uniform_buffer) {
                vkDestroyBuffer(graphics->core.device, graphics->frames[i].uniform_buffer, NULL);
            }
        }

        for (int i = 0; i < 3; i++) {
            if (graphics->swapchain_render_sems[i]) {
                vkDestroySemaphore(graphics->core.device, graphics->swapchain_render_sems[i], NULL);
            }
        }

        if (graphics->display.depth_view) {
            vkDestroyImageView(graphics->core.device, graphics->display.depth_view, NULL);
        }
        if (graphics->display.depth_image) {
            vkDestroyImage(graphics->core.device, graphics->display.depth_image, NULL);
        }

        if (graphics->transfer.command_pool) {
            vkDestroyCommandPool(graphics->core.device, graphics->transfer.command_pool, NULL);
        }

        if (graphics->transfer.fence) {
            vkDestroyFence(graphics->core.device, graphics->transfer.fence, NULL);
        }

        vk_destroy_swapchain(graphics);
        vkDestroyDevice(graphics->core.device, NULL);
    }

    if (graphics->core.instance != VK_NULL_HANDLE) {
        if (graphics->core.surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(graphics->core.instance, graphics->core.surface, NULL);
        }
        vkDestroyInstance(graphics->core.instance, NULL);
    }

    free(graphics);
    log_info("vulkan: renderer destroyed cleanly");
}

present_mode_t graphics_get_present_mode(const graphics_t* graphics) {
    return graphics->display.abstract_present_mode;
}

void graphics_set_present_mode(graphics_t* r, present_mode_t mode) {
    if (r->display.abstract_present_mode == mode) {
        return;
    }
    r->display.abstract_present_mode = mode;
    vk_recreate_swapchain(r, r->display.extent.width, r->display.extent.height);
}

static int32_t begin_frame(
    graphics_t*             r,
    platform_t*             platform,
    mat4_t                  view,
    vec3_t                  cam_pos,
    vec3_t                  sun_direction,
    vec3_t                  sun_color,
    float                   sun_intensity,
    float                   roughness_bias,
    float                   metallic_bias,
    vk_render_target_t*     render_target,
    draw_mode_t             draw_mode,
    graphics_frame_input_t* gfx_frame_input
) {
    int w;
    int h;
    platform_get_window_size(platform, &w, &h);
    if (w == 0 || h == 0) {
        return -1;
    }

    vkWaitForFences(
        r->core.device, 1, &r->frames[r->current_frame].in_flight_fence, VK_TRUE, UINT64_MAX
    );

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
        r->core.device,
        r->display.swapchain,
        UINT64_MAX,
        r->frames[r->current_frame].image_available_sem,
        VK_NULL_HANDLE,
        &image_index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vk_recreate_swapchain(r, w, h);
        if (render_target != NULL) {
            vk_resize_render_target(r, render_target, w, h);
        }
        return -1;
    }

    update_uniform_buffer(
        r,
        gfx_frame_input,
        view,
        gfx_frame_input->proj,
        cam_pos,
        gfx_frame_input->light_space_matrix,
        sun_direction,
        sun_color,
        sun_intensity,
        r->current_frame
    );

    vkResetFences(r->core.device, 1, &r->frames[r->current_frame].in_flight_fence);
    r->command_buffer = r->frames[r->current_frame].command_buffer;
    vkResetCommandBuffer(r->command_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(r->command_buffer, &begin_info);

    return (int32_t)image_index;
}

static void end_frame(graphics_t* r, uint32_t image_index) {
    vkCmdEndRendering(r->command_buffer);

    VkImageMemoryBarrier barrier = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image            = r->display.images[image_index],
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        .srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask    = 0
    };
    vkCmdPipelineBarrier(
        r->command_buffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &barrier
    );

    vkEndCommandBuffer(r->command_buffer);

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo         submit_info   = {
                  .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                  .waitSemaphoreCount   = 1,
                  .pWaitSemaphores      = &r->frames[r->current_frame].image_available_sem,
                  .pWaitDstStageMask    = wait_stages,
                  .commandBufferCount   = 1,
                  .pCommandBuffers      = &r->command_buffer,
                  .signalSemaphoreCount = 1,
                  .pSignalSemaphores    = &r->swapchain_render_sems[image_index],
    };
    vkQueueSubmit(
        r->core.graphics_queue, 1, &submit_info, r->frames[r->current_frame].in_flight_fence
    );

    VkPresentInfoKHR present_info = {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &r->swapchain_render_sems[image_index],
        .swapchainCount     = 1,
        .pSwapchains        = &r->display.swapchain,
        .pImageIndices      = &image_index,
    };
    vkQueuePresentKHR(r->core.graphics_queue, &present_info);

    r->current_frame = (r->current_frame + 1) % FRAMES_IN_FLIGHT;
}

mesh_handle_t graphics_upload_mesh(graphics_t* graphics, mesh_data_t* data) {
    if (graphics->assets.mesh_count >= VK_MAX_MESHES) {
        log_error("Mesh pool exhausted! Cannot upload new mesh.");
        return (mesh_handle_t){.id = UINT32_MAX};
    }

    uint32_t   id      = graphics->assets.mesh_count++;
    vk_mesh_t* vk_mesh = &graphics->assets.meshes[id];

    vk_mesh->vertex_buffer = vk_create_static_buffer(
        graphics,
        data->vertices,
        data->vertex_count * sizeof(vertex_t),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    );
    if (vk_mesh->vertex_buffer == VK_NULL_HANDLE) {
        log_error("vulkan: Failed to allocate vertex buffer for mesh. Aborting.");
        return (mesh_handle_t){.id = UINT32_MAX};
    }

    if (data->index_count > 0) {
        vk_mesh->index_buffer = vk_create_static_buffer(
            graphics,
            data->indices,
            data->index_count * sizeof(uint32_t),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        );
        if (vk_mesh->index_buffer == VK_NULL_HANDLE) {
            log_error("vulkan: Failed to allocate index buffer for mesh. Rolling back.");
            vkDestroyBuffer(graphics->core.device, vk_mesh->vertex_buffer, NULL);
            vk_mesh->vertex_buffer = VK_NULL_HANDLE;
            return (mesh_handle_t){.id = UINT32_MAX};
        }
    } else {
        vk_mesh->index_buffer = VK_NULL_HANDLE;
    }

    vk_mesh->vertex_count    = data->vertex_count;
    vk_mesh->index_count     = data->index_count;
    vk_mesh->bounding_center = data->bounding_center;
    vk_mesh->bounding_radius = data->bounding_radius;
    vk_mesh->is_active       = true;

    log_debug(
        "vulkan: uploaded mesh to pool slot %d (%d vertices, %d indices)",
        id,
        data->vertex_count,
        data->index_count
    );

    return (mesh_handle_t){.id = id};
}

texture_handle_t graphics_upload_texture(graphics_t* r, image_t* img, pak_texture_format_t format) {
    if (r->assets.texture_count >= VK_MAX_TEXTURES) {
        log_error("vulkan: texture pool exhausted!");
        return (texture_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    uint32_t      id  = r->assets.texture_count++;
    vk_texture_t* tex = &r->assets.textures[id];

    if (img->size == 0) {
        log_error("graphics_upload_texture - img->size = 0!");
    }

    if (!vk_create_texture(r, img, tex, format)) {
        log_error("vulkan: failed to create texture for pool slot %d", id);
        return (texture_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    tex->is_active = true;
    log_debug("vulkan: uploaded texture to pool slot %d", id);

    return (texture_handle_t){.id = id};
}

material_handle_t graphics_create_material(
    graphics_t*      graphics,
    texture_handle_t albedo,
    texture_handle_t normal,
    texture_handle_t metallic_roughness,
    bool             is_alpha_masked,
    bool             is_double_sided,
    bool             is_translucent,
    float            metallic_factor,
    float            roughness_factor,
    float            alpha_cutoff
) {
    if (albedo.id == GRAPHICS_INVALID_HANDLE || normal.id == GRAPHICS_INVALID_HANDLE) {
        log_error("vulkan: Cannot create material with invalid texture handles. Skipping.");
        return (material_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    if (graphics->assets.material_count >= VK_MAX_MATERIALS) {
        log_error("vulkan: material pool exhausted!");
        return (material_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    uint32_t       id  = graphics->assets.material_count++;
    vk_material_t* mat = &graphics->assets.materials[id];

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = graphics->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &graphics->pipelines.object_set_layout,
    };

    if (vkAllocateDescriptorSets(graphics->core.device, &alloc_info, &mat->descriptor_set) !=
        VK_SUCCESS) {
        log_error("vulkan: failed to allocate material descriptor set");
        return (material_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    vk_texture_t* albedo_texture                = NULL;
    vk_texture_t* normal_texture                = NULL;
    vk_texture_t* ao_metallic_roughness_texture = NULL;

    if (albedo.id != GRAPHICS_INVALID_HANDLE && albedo.id < VK_MAX_TEXTURES) {
        vk_texture_t* candidate = &graphics->assets.textures[albedo.id];
        if (candidate->is_active && candidate->view != VK_NULL_HANDLE)
            albedo_texture = candidate;
    }
    if (!albedo_texture)
        albedo_texture = &graphics->assets.textures[graphics->assets.default_albedo.id];

    if (normal.id != GRAPHICS_INVALID_HANDLE && normal.id < VK_MAX_TEXTURES) {
        vk_texture_t* candidate = &graphics->assets.textures[normal.id];
        if (candidate->is_active && candidate->view != VK_NULL_HANDLE)
            normal_texture = candidate;
    }
    if (!normal_texture)
        normal_texture = &graphics->assets.textures[graphics->assets.default_normal.id];

    if (metallic_roughness.id != GRAPHICS_INVALID_HANDLE &&
        metallic_roughness.id < VK_MAX_TEXTURES) {
        vk_texture_t* candidate = &graphics->assets.textures[metallic_roughness.id];
        if (candidate->is_active && candidate->view != VK_NULL_HANDLE) {
            ao_metallic_roughness_texture = candidate;
        }
    }
    if (!ao_metallic_roughness_texture) {
        ao_metallic_roughness_texture =
            &graphics->assets.textures[graphics->assets.default_ao_metallic_roughness.id];
    }

    if (albedo_texture->view == VK_NULL_HANDLE || normal_texture->view == VK_NULL_HANDLE ||
        ao_metallic_roughness_texture->view == VK_NULL_HANDLE) {
        log_error(
            "CRITICAL: Material attempted to bind a NULL image view! Default fallback failed."
        );
        return (material_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    VkDescriptorImageInfo albedo_info = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView   = albedo_texture->view,
        .sampler     = albedo_texture->sampler
    };

    VkDescriptorImageInfo normal_info = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView   = normal_texture->view,
        .sampler     = normal_texture->sampler
    };

    VkDescriptorImageInfo ao_metallic_roughness_info = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView   = ao_metallic_roughness_texture->view,
        .sampler     = ao_metallic_roughness_texture->sampler
    };

    VkWriteDescriptorSet writes[3] = {0};

    writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet          = mat->descriptor_set;
    writes[0].dstBinding      = 0;
    writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].descriptorCount = 1;
    writes[0].pImageInfo      = &albedo_info;

    writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet          = mat->descriptor_set;
    writes[1].dstBinding      = 1;
    writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].descriptorCount = 1;
    writes[1].pImageInfo      = &normal_info;

    writes[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet          = mat->descriptor_set;
    writes[2].dstBinding      = 2;
    writes[2].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[2].descriptorCount = 1;
    writes[2].pImageInfo      = &ao_metallic_roughness_info;

    vkUpdateDescriptorSets(graphics->core.device, 3, writes, 0, NULL);

    mat->is_active        = true;
    mat->is_alpha_masked  = is_alpha_masked;
    mat->is_double_sided  = is_double_sided;
    mat->is_translucent   = is_translucent;
    mat->metallic_factor  = metallic_factor;
    mat->roughness_factor = roughness_factor;
    mat->alpha_cutoff     = alpha_cutoff;
    return (material_handle_t){.id = id};
}

void graphics_update_shadow_map_descriptor(
    graphics_t*            graphics,
    render_target_handle_t shadow_target
) {
    if (shadow_target.id == GRAPHICS_INVALID_HANDLE)
        return;

    vk_render_target_t* rt = &graphics->assets.render_targets[shadow_target.id];

    VkDescriptorImageInfo shadow_info = {
        .sampler     = rt->depth_attachment.sampler,
        .imageView   = rt->depth_attachment.view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
        VkWriteDescriptorSet write = {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = graphics->frames[i].global_descriptor_set,
            .dstBinding      = 4, // match the layout binding index
            .dstArrayElement = 0,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo      = &shadow_info
        };
        vkUpdateDescriptorSets(graphics->core.device, 1, &write, 0, NULL);
    }
    log_info("vulkan: Shadow map descriptor bound to Set 0, Binding 4.");
}

void graphics_draw(
    graphics_t*             graphics,
    platform_t*             platform,
    graphics_frame_input_t* gfx_frame_input,
    render_stats_t*         out_render_stats
) {
    assert(
        is_matrix_valid(&gfx_frame_input->view) &&
        "CRASH: NaN detected in View Matrix entering graphics_draw!"
    );
    assert(
        is_matrix_valid(&gfx_frame_input->culling_view_proj) &&
        "CRASH: NaN detected in Culling Matrix!"
    );

    if (out_render_stats) {
        memset(out_render_stats, 0, sizeof(render_stats_t));
    }

    vk_render_target_t* render_target = NULL;
    if (gfx_frame_input->target.id != GRAPHICS_INVALID_HANDLE) {
        render_target = &graphics->assets.render_targets[gfx_frame_input->target.id];
    }

    vk_render_target_t* shadow_render_target = NULL;
    if (gfx_frame_input->shadow_target.id != GRAPHICS_INVALID_HANDLE) {
        shadow_render_target = &graphics->assets.render_targets[gfx_frame_input->shadow_target.id];
    }

    vk_render_target_t* active_target = render_target;
    if (gfx_frame_input->draw_mode == DRAW_MODE_DEBUG_SDR) {
        // active_target = &graphics->assets.render_targets[target.id];
    }

    int32_t image_index = begin_frame(
        graphics,
        platform,
        gfx_frame_input->view,
        gfx_frame_input->camera_pos,
        gfx_frame_input->environment->sun_direction,
        gfx_frame_input->environment->sun_color,
        gfx_frame_input->environment->sun_intensity,
        gfx_frame_input->environment->roughness_bias,
        gfx_frame_input->environment->metallic_bias,
        render_target,
        gfx_frame_input->draw_mode,
        gfx_frame_input
    );

    if (image_index < 0) {
        igRender();
        ImGuiIO* io = igGetIO_Nil();
        if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            igUpdatePlatformWindows();
            igRenderPlatformWindowsDefault(NULL, NULL);
        }
        return;
    }

    shadow_pass(graphics, gfx_frame_input, shadow_render_target, out_render_stats);
    forward_pass(graphics, gfx_frame_input, render_target, image_index, out_render_stats);

    if (gfx_frame_input->draw_mode != DRAW_MODE_DEBUG_SDR) {
        post_process_pass(graphics, render_target, (uint32_t)image_index);
        if (gfx_frame_input->target.id != GRAPHICS_INVALID_HANDLE) {
            render_target = &graphics->assets.render_targets[gfx_frame_input->target.id];
        }
    }
    // log_info(
    //     "Target ID: %u | Executing Bypass: %s", target.id, (render_target != NULL) ? "YES" : "NO"
    // );

    debug_imgui_render(graphics);

    end_frame(graphics, (uint32_t)image_index);
}

void graphics_update_global_environment(
    graphics_t*      graphics,
    texture_handle_t skybox_tex,
    texture_handle_t irradiance_tex,
    texture_handle_t prefiltered_tex
) {
    if (skybox_tex.id == GRAPHICS_INVALID_HANDLE || irradiance_tex.id == GRAPHICS_INVALID_HANDLE ||
        prefiltered_tex.id == GRAPHICS_INVALID_HANDLE) {
        return;
    }

    vk_texture_t* vk_skybox = &graphics->assets.textures[skybox_tex.id];
    vk_texture_t* vk_irr    = &graphics->assets.textures[irradiance_tex.id];
    vk_texture_t* vk_pref   = &graphics->assets.textures[prefiltered_tex.id];

    VkDescriptorImageInfo skybox_info = {
        .sampler     = vk_skybox->sampler,
        .imageView   = vk_skybox->view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkDescriptorImageInfo irradiance_info = {
        .sampler     = vk_irr->sampler,
        .imageView   = vk_irr->view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    VkDescriptorImageInfo prefilter_info = {
        .sampler     = vk_pref->sampler,
        .imageView   = vk_pref->view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
        VkWriteDescriptorSet descriptor_writes[3] = {0};

        descriptor_writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet          = graphics->frames[i].global_descriptor_set;
        descriptor_writes[0].dstBinding      = 1;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pImageInfo      = &skybox_info;

        descriptor_writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet          = graphics->frames[i].global_descriptor_set;
        descriptor_writes[1].dstBinding      = 2;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo      = &irradiance_info;

        descriptor_writes[2].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[2].dstSet          = graphics->frames[i].global_descriptor_set;
        descriptor_writes[2].dstBinding      = 3;
        descriptor_writes[2].dstArrayElement = 0;
        descriptor_writes[2].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[2].descriptorCount = 1;
        descriptor_writes[2].pImageInfo      = &prefilter_info;

        vkUpdateDescriptorSets(graphics->core.device, 3, descriptor_writes, 0, NULL);
    }

    log_info("Global Environment Descriptor Updated with IBL maps.");
}
