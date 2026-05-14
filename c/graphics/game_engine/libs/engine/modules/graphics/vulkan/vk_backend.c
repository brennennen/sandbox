
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "engine/core/frustum.h"
#include "engine/core/logger.h"
#include "engine/core/math/mat4.h"
#include "engine/modules/assets/image.h"
#include "engine/modules/assets/obj.h"
#include "engine/modules/graphics/debug/debug_grid.h"
#include "engine/modules/graphics/graphics.h"
#include "engine/modules/graphics/graphics_types.h"
#include "shared/math_types.h"
#include "shared/scene_types.h"

#include "vk_commands.h"
#include "vk_devices.h"
#include "vk_pipeline.h"
#include "vk_resources.h"
#include "vk_swapchain.h"
#include "vk_types.h"
#include "volk.h"

#include "vk_gpu_allocator.h"

static bool init_default_textures(graphics_t* r) {
    uint8_t white_pixels[4]  = {255, 255, 255, 255};
    image_t albedo_img       = {.width = 1, .height = 1, .channels = 4, .pixels = white_pixels};
    r->assets.default_albedo = graphics_upload_texture(r, &albedo_img, PAK_TEX_FORMAT_RGBA8_SRGB);

    uint8_t flat_normal_pixels[4] = {128, 128, 255, 255};
    image_t normal_img = {.width = 1, .height = 1, .channels = 4, .pixels = flat_normal_pixels};
    r->assets.default_normal = graphics_upload_texture(r, &normal_img, PAK_TEX_FORMAT_RGBA8_UNORM);

    uint8_t arm_pixels[4] = {255, 255, 255, 255};
    image_t mr_img        = {.width = 1, .height = 1, .channels = 4, .pixels = arm_pixels};
    r->assets.default_ao_metallic_roughness = graphics_upload_texture(
        r, &mr_img, PAK_TEX_FORMAT_RGBA8_UNORM
    );

    if (r->assets.default_albedo.id == GRAPHICS_INVALID_HANDLE ||
        r->assets.default_normal.id == GRAPHICS_INVALID_HANDLE ||
        r->assets.default_ao_metallic_roughness.id == GRAPHICS_INVALID_HANDLE) {
        log_error("vulkan: Failed to initialize default fallback textures.");
        return false;
    }

    log_info("vulkan: Default fallback textures initialized.");
    return true;
}

static void init_debug_grid(graphics_t* r) {
    int   grid_size = 10;
    float grid_step = 1.0f;

    r->grid_vertex_count = debug_grid_vertex_count(grid_size);
    size_t buffer_size   = r->grid_vertex_count * sizeof(vertex_t);

    r->grid_buffer.allocation = gpu_heap_alloc(r->assets.vertex_heap, buffer_size, 16);

    VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = buffer_size,
        .usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    vkCreateBuffer(r->core.device, &buffer_info, NULL, &r->grid_buffer.buffer);
    vkBindBufferMemory(
        r->core.device,
        r->grid_buffer.buffer,
        r->assets.vertex_heap->memory,
        r->grid_buffer.allocation.offset
    );

    vertex_t* mapped_data = (vertex_t*)r->grid_buffer.allocation.mapped_ptr;
    generate_grid(mapped_data, grid_size, grid_step);
}

void init_debug_frustum_buffer(graphics_t* r) {
    size_t buffer_size           = 24 * sizeof(vertex_t);
    r->frustum_buffer.allocation = gpu_heap_alloc(r->assets.vertex_heap, buffer_size, 16);

    VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = buffer_size,
        .usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    vkCreateBuffer(r->core.device, &buffer_info, NULL, &r->frustum_buffer.buffer);
    vkBindBufferMemory(
        r->core.device,
        r->frustum_buffer.buffer,
        r->assets.vertex_heap->memory,
        r->frustum_buffer.allocation.offset
    );
}

void graphics_update_debug_frustum(graphics_t* r, mat4_t inv_vp) {
    // 8 corners of Vulkan's NDC space
    vec4_t ndc[8] = {
        {-1, -1, 0, 1},
        {1, -1, 0, 1},
        {1, 1, 0, 1},
        {-1, 1, 0, 1}, // Near
        {-1, -1, 1, 1},
        {1, -1, 1, 1},
        {1, 1, 1, 1},
        {-1, 1, 1, 1} // Far
    };

    vec3_t corners[8];
    for (int i = 0; i < 8; i++) {
        vec4_t world_pos = mat4_mul_vec4(inv_vp, ndc[i]);
        corners[i]       = (vec3_t){
            world_pos.x / world_pos.w,
            world_pos.y / world_pos.w,
            world_pos.z / world_pos.w,
        };
    }

    vec4_t c = {1.0f, 1.0f, 0.0f, 1.0f}; // yellow
    // clang-format off
    vertex_t lines[24] = {
        // near face
        {.pos = corners[0], .color = c}, {.pos = corners[1], .color = c},
        {.pos = corners[1], .color = c}, {.pos = corners[2], .color = c},
        {.pos = corners[2], .color = c}, {.pos = corners[3], .color = c},
        {.pos = corners[3], .color = c}, {.pos = corners[0], .color = c},
        // far face
        {.pos = corners[4], .color = c}, {.pos = corners[5], .color = c},
        {.pos = corners[5], .color = c}, {.pos = corners[6], .color = c},
        {.pos = corners[6], .color = c}, {.pos = corners[7], .color = c},
        {.pos = corners[7], .color = c}, {.pos = corners[4], .color = c},
        // edges
        {.pos = corners[0], .color = c}, {.pos = corners[4], .color = c},
        {.pos = corners[1], .color = c}, {.pos = corners[5], .color = c},
        {.pos = corners[2], .color = c}, {.pos = corners[6], .color = c},
        {.pos = corners[3], .color = c}, {.pos = corners[7], .color = c},
    };
    // clang-format on

    memcpy(r->frustum_buffer.allocation.mapped_ptr, lines, sizeof(lines));
}

void update_uniform_buffer(graphics_t* r, mat4_t view, vec3_t cam_pos, uint32_t current_frame) {
    float  aspect = (float)r->display.extent.width / (float)r->display.extent.height;
    mat4_t proj   = mat4_perspective(0.785f, aspect, 0.1f, 100.0f);
    ubo_t  ubo    = {
            .view       = view,
            .proj       = proj,
            .camera_pos = {cam_pos.x, cam_pos.y, cam_pos.z, 1.0f},
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
        r, 8ULL * 1024 * 1024 * 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
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

static bool init_sync_objects(graphics_t* r) {
    VkSemaphoreCreateInfo sem_info   = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo     fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(r->core.device, &sem_info, NULL, &r->frames[i].image_available_sem) !=
                VK_SUCCESS ||
            vkCreateSemaphore(r->core.device, &sem_info, NULL, &r->frames[i].render_finished_sem) !=
                VK_SUCCESS ||
            vkCreateFence(r->core.device, &fence_info, NULL, &r->frames[i].in_flight_fence) !=
                VK_SUCCESS) {
            return false;
        }
    }
    r->current_frame = 0;
    return true;
}

graphics_t* graphics_create(platform_t* platform, graphics_config_t* config) {
    graphics_t* r = calloc(1, sizeof(struct graphics_t));
    if (!r) {
        log_error("renderer: failed to allocate memory for graphics_t");
        return NULL;
    }

    if (!vk_create_instance(r, platform) ||
        !platform_create_vulkan_surface(platform, r->core.instance, &r->core.surface) ||
        !vk_pick_physical_device(r) || !vk_create_logical_device(r)) {
        goto init_failed;
    }

    if (!init_memory_heaps(r) || !init_uniform_buffer(r) || !vk_create_commands(r) ||
        !vk_setup_depth_buffer(r, config->width, config->height)) {
        goto init_failed;
    }

    r->display.abstract_present_mode = config->present_mode;
    if (!vk_create_swapchain(
            r, config->width, config->height, config->present_mode, VK_NULL_HANDLE
        ) ||
        !vk_create_graphics_pipeline(r) || !init_descriptors(r)) {
        goto init_failed;
    }

    if (!init_sync_objects(r)) {
        goto init_failed;
    }

    init_debug_grid(r);
    init_debug_frustum_buffer(r);
    if (!init_default_textures(r)) {
        goto init_failed;
    }

    log_info("renderer: initialization complete");
    return r;

init_failed:
    log_error("renderer: initialization aborted due to failure");
    graphics_destroy(r);
    return NULL;
}

void graphics_destroy(graphics_t* r) {
    if (r == NULL)
        return;

    if (r->core.device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(r->core.device);

        if (r->assets.vertex_heap) {
            vkFreeMemory(r->core.device, r->assets.vertex_heap->memory, NULL);
            free(r->assets.vertex_heap);
        }
        if (r->assets.device_heap) {
            vkFreeMemory(r->core.device, r->assets.device_heap->memory, NULL);
            free(r->assets.device_heap);
        }

        if (r->grid_buffer.buffer) {
            vkDestroyBuffer(r->core.device, r->grid_buffer.buffer, NULL);
        }
        if (r->frustum_buffer.buffer) {
            vkDestroyBuffer(r->core.device, r->frustum_buffer.buffer, NULL);
        }

        vk_destroy_graphics_pipeline(r);
        vk_destroy_commands(r);

        if (r->descriptor_pool) {
            vkDestroyDescriptorPool(r->core.device, r->descriptor_pool, NULL);
        }

        for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
            if (r->frames[i].image_available_sem)
                vkDestroySemaphore(r->core.device, r->frames[i].image_available_sem, NULL);
            if (r->frames[i].render_finished_sem)
                vkDestroySemaphore(r->core.device, r->frames[i].render_finished_sem, NULL);
            if (r->frames[i].in_flight_fence)
                vkDestroyFence(r->core.device, r->frames[i].in_flight_fence, NULL);
            if (r->frames[i].uniform_buffer)
                vkDestroyBuffer(r->core.device, r->frames[i].uniform_buffer, NULL);
        }

        if (r->display.depth_view) {
            vkDestroyImageView(r->core.device, r->display.depth_view, NULL);
        }
        if (r->display.depth_image) {
            vkDestroyImage(r->core.device, r->display.depth_image, NULL);
        }

        vk_destroy_swapchain(r);
        vkDestroyDevice(r->core.device, NULL);
    }

    if (r->core.instance != VK_NULL_HANDLE) {
        if (r->core.surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(r->core.instance, r->core.surface, NULL);
        }
        vkDestroyInstance(r->core.instance, NULL);
    }

    free(r);
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

static int32_t begin_frame(graphics_t* r, platform_t* platform, mat4_t view, vec3_t cam_pos) {
    int w;
    int h;
    platform_get_window_size(platform, &w, &h);
    if (w == 0 || h == 0)
        return -1;

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
        return -1;
    }

    update_uniform_buffer(r, view, cam_pos, r->current_frame);
    vkResetFences(r->core.device, 1, &r->frames[r->current_frame].in_flight_fence);
    r->command_buffer = r->frames[r->current_frame].command_buffer;
    vkResetCommandBuffer(r->command_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(r->command_buffer, &begin_info);

    VkImageMemoryBarrier barrier = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image            = r->display.images[image_index],
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        .srcAccessMask    = 0,
        .dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };
    vkCmdPipelineBarrier(
        r->command_buffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &barrier
    );

    VkRenderingAttachmentInfo color_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = r->display.image_views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = {{{0.1f, 0.1f, 0.2f, 1.0f}}}
    };

    VkRenderingAttachmentInfo depth_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = r->display.depth_view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue  = {.depthStencil = {1.0f, 0}}
    };

    VkRenderingInfo rendering_info = {
        .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea           = {{0, 0}, {r->display.extent.width, r->display.extent.height}},
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &color_attachment,
        .pDepthAttachment     = &depth_attachment
    };

    vkCmdBeginRendering(r->command_buffer, &rendering_info);

    VkViewport viewport = {
        .x        = 0.0f,
        .y        = (float)r->display.extent.height,
        .width    = (float)r->display.extent.width,
        .height   = -(float)r->display.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor = {{0, 0}, r->display.extent};
    vkCmdSetViewport(r->command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(r->command_buffer, 0, 1, &scissor);

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
                  .pSignalSemaphores    = &r->frames[r->current_frame].render_finished_sem,
    };
    vkQueueSubmit(
        r->core.graphics_queue, 1, &submit_info, r->frames[r->current_frame].in_flight_fence
    );

    VkPresentInfoKHR present_info = {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &r->frames[r->current_frame].render_finished_sem,
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

    if (!vk_create_texture(r, img, tex, format)) {
        log_error("vulkan: failed to create texture for pool slot %d", id);
        return (texture_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    tex->is_active = true;
    log_debug("vulkan: uploaded texture to pool slot %d", id);

    return (texture_handle_t){.id = id};
}

material_handle_t graphics_create_material(
    graphics_t*      r,
    texture_handle_t albedo,
    texture_handle_t normal,
    texture_handle_t metallic_roughness,
    bool             is_alpha_masked,
    float            metallic_factor,
    float            roughness_factor
) {
    if (albedo.id == GRAPHICS_INVALID_HANDLE || normal.id == GRAPHICS_INVALID_HANDLE) {
        log_error("vulkan: Cannot create material with invalid texture handles. Skipping.");
        return (material_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    if (r->assets.material_count >= VK_MAX_MATERIALS) {
        log_error("vulkan: material pool exhausted!");
        return (material_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    uint32_t       id  = r->assets.material_count++;
    vk_material_t* mat = &r->assets.materials[id];

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = r->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &r->pipelines.object_set_layout,
    };

    if (vkAllocateDescriptorSets(r->core.device, &alloc_info, &mat->descriptor_set) != VK_SUCCESS) {
        log_error("vulkan: failed to allocate material descriptor set");
        return (material_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    vk_texture_t* albedo_texture                = NULL;
    vk_texture_t* normal_texture                = NULL;
    vk_texture_t* ao_metallic_roughness_texture = NULL;

    if (albedo.id != GRAPHICS_INVALID_HANDLE && albedo.id < VK_MAX_TEXTURES) {
        vk_texture_t* candidate = &r->assets.textures[albedo.id];
        if (candidate->is_active && candidate->view != VK_NULL_HANDLE)
            albedo_texture = candidate;
    }
    if (!albedo_texture)
        albedo_texture = &r->assets.textures[r->assets.default_albedo.id];

    if (normal.id != GRAPHICS_INVALID_HANDLE && normal.id < VK_MAX_TEXTURES) {
        vk_texture_t* candidate = &r->assets.textures[normal.id];
        if (candidate->is_active && candidate->view != VK_NULL_HANDLE)
            normal_texture = candidate;
    }
    if (!normal_texture)
        normal_texture = &r->assets.textures[r->assets.default_normal.id];

    if (metallic_roughness.id != GRAPHICS_INVALID_HANDLE &&
        metallic_roughness.id < VK_MAX_TEXTURES) {
        vk_texture_t* candidate = &r->assets.textures[metallic_roughness.id];
        if (candidate->is_active && candidate->view != VK_NULL_HANDLE) {
            ao_metallic_roughness_texture = candidate;
        }
    }
    if (!ao_metallic_roughness_texture) {
        ao_metallic_roughness_texture =
            &r->assets.textures[r->assets.default_ao_metallic_roughness.id];
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

    vkUpdateDescriptorSets(r->core.device, 3, writes, 0, NULL);

    mat->is_active        = true;
    mat->is_alpha_masked  = is_alpha_masked;
    mat->metallic_factor  = metallic_factor;
    mat->roughness_factor = roughness_factor;
    return (material_handle_t){.id = id};
}

static VkPipeline get_draw_mode_pipeline(vk_pipelines_t* pipelines, draw_mode_t draw_mode) {
    switch (draw_mode) {
    case DRAW_MODE_FORWARD_LIT:
        return pipelines->forward_lit;
    case DRAW_MODE_DEBUG_WIREFRAME:
        return pipelines->debug_wireframe;
    case DRAW_MODE_DEBUG_ALBEDO:
    case DRAW_MODE_DEBUG_LIGHTING:
    case DRAW_MODE_DEBUG_GEOMETRY_NORMAL:
    case DRAW_MODE_DEBUG_TEXTURE_NORMAL:
    case DRAW_MODE_DEBUG_NORMAL:
    case DRAW_MODE_DEBUG_TANGENT:
    case DRAW_MODE_DEBUG_BITANGENT:
    case DRAW_MODE_DEBUG_VERTEX_COLOR:
    case DRAW_MODE_DEBUG_MIPMAPS:
    case DRAW_MODE_DEBUG_SPECULAR:
    default:
        return pipelines->debug_forward_lit;
    }
}

static bool is_matrix_valid(mat4_t* m) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (isnan(m->m[i][j]) || isinf(m->m[i][j]))
                return false;
        }
    }
    return true;
}

#include <assert.h>

void graphics_draw(
    graphics_t*      r,
    platform_t*      platform,
    mat4_t           view,
    vec3_t           camera_pos,
    mat4_t           culling_view_proj,
    bool             is_culling_frozen,
    draw_mode_t      draw_mode,
    render_object_t* objects,
    uint32_t         object_count
) {
    assert(is_matrix_valid(&view) && "CRASH: NaN detected in View Matrix entering graphics_draw!");
    assert(is_matrix_valid(&culling_view_proj) && "CRASH: NaN detected in Culling Matrix!");

    int32_t image_index = begin_frame(r, platform, view, camera_pos);
    if (image_index < 0) {
        return;
    }
    float     aspect    = (float)r->display.extent.width / (float)r->display.extent.height;
    mat4_t    proj      = mat4_perspective(0.785f, aspect, 0.1f, 100.0f);
    mat4_t    view_proj = mat4_mul(proj, view);
    frustum_t frustum   = frustum_extract(culling_view_proj);

    VkPipeline current_pipeline = get_draw_mode_pipeline(&r->pipelines, draw_mode);

    vkCmdBindPipeline(r->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, current_pipeline);

    vkCmdBindDescriptorSets(
        r->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        r->pipelines.layout,
        0,
        1,
        &r->frames[r->current_frame].global_descriptor_set,
        0,
        NULL
    );

    int culled_count = 0;

    VkDeviceSize offsets[] = {0};
    for (uint32_t i = 0; i < object_count; i++) {
        render_object_t* obj = &objects[i];

        if (obj->mesh.id == GRAPHICS_INVALID_HANDLE ||
            obj->material.id == GRAPHICS_INVALID_HANDLE) {
            continue;
        }

        vk_mesh_t*     vk_mesh = &r->assets.meshes[obj->mesh.id];
        vk_material_t* vk_mat  = &r->assets.materials[obj->material.id];

        if (!vk_mesh->is_active || !vk_mat->is_active)
            continue;

        vec3_t obj_pos = mat4_transform_point(obj->transform, vk_mesh->bounding_center);
        float  scale   = sqrtf(
            obj->transform.m[0][0] * obj->transform.m[0][0] +
            obj->transform.m[0][1] * obj->transform.m[0][1] +
            obj->transform.m[0][2] * obj->transform.m[0][2]
        );
        float bounding_radius = vk_mesh->bounding_radius * scale;

        if (!frustum_test_sphere(&frustum, obj_pos, bounding_radius)) {
            culled_count++;
            continue;
        }

        push_constants_t push_constants = {
            .transform        = obj->transform,
            .is_alpha_masked  = vk_mat->is_alpha_masked ? 1 : 0,
            .debug_mode       = draw_mode,
            .metallic_factor  = vk_mat->metallic_factor,
            .roughness_factor = vk_mat->roughness_factor,
        };

        assert(is_matrix_valid(&push_constants.transform) && "CRASH: NaN in Object Transform!");
        assert(!isnan(push_constants.metallic_factor) && "CRASH: NaN in Metallic Factor!");
        assert(!isnan(push_constants.roughness_factor) && "CRASH: NaN in Roughness Factor!");

        vkCmdPushConstants(
            r->command_buffer,
            r->pipelines.layout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(push_constants_t),
            &push_constants
        );

        vkCmdBindDescriptorSets(
            r->command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            r->pipelines.layout,
            1,
            1,
            &vk_mat->descriptor_set,
            0,
            NULL
        );

        vkCmdBindVertexBuffers(r->command_buffer, 0, 1, &vk_mesh->vertex_buffer, offsets);

        if (vk_mesh->index_count > 0) {
            vkCmdBindIndexBuffer(r->command_buffer, vk_mesh->index_buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(r->command_buffer, vk_mesh->index_count, 1, 0, 0, 0);
        } else {
            vkCmdDraw(r->command_buffer, vk_mesh->vertex_count, 1, 0, 0);
        }
    }

    // log_info("Rendered: %d | Culled: %d", object_count - culled_count, culled_count);

    vkCmdBindPipeline(r->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipelines.skybox);

    vkCmdBindDescriptorSets(
        r->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        r->pipelines.layout,
        0,
        1,
        &r->frames[r->current_frame].global_descriptor_set,
        0,
        NULL
    );

    vkCmdDraw(r->command_buffer, 36, 1, 0, 0);

    vkCmdBindPipeline(r->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipelines.line);

    push_constants_t grid_pc = {
        .transform       = mat4_identity(),
        .is_alpha_masked = 0,
    };
    mat4_t identity = mat4_identity();
    vkCmdPushConstants(
        r->command_buffer,
        r->pipelines.layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(push_constants_t),
        &grid_pc
    );

    vkCmdBindVertexBuffers(r->command_buffer, 0, 1, &r->grid_buffer.buffer, offsets);
    vkCmdDraw(r->command_buffer, r->grid_vertex_count, 1, 0, 0);

    if (is_culling_frozen) {
        push_constants_t frustum_pc = {
            .transform       = mat4_identity(),
            .is_alpha_masked = 0,
        };

        vkCmdPushConstants(
            r->command_buffer,
            r->pipelines.layout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(push_constants_t),
            &frustum_pc
        );

        vkCmdBindVertexBuffers(r->command_buffer, 0, 1, &r->frustum_buffer.buffer, offsets);
        vkCmdDraw(r->command_buffer, 24, 1, 0, 0);
    }

    end_frame(r, (uint32_t)image_index);
}
