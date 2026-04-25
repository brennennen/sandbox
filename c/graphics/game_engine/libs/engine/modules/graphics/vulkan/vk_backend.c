
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "engine/core/logger.h"
#include "engine/core/math/mat4_math.h"
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

void update_uniform_buffer(graphics_t* r, mat4_t view, uint32_t current_frame) {
    float  aspect = (float)r->display.extent.width / (float)r->display.extent.height;
    mat4_t proj   = mat4_perspective(0.785f, aspect, 0.1f, 100.0f);
    ubo_t  ubo    = {.view = view, .proj = proj};
    memcpy(r->frames[current_frame].uniform_alloc.mapped_ptr, &ubo, sizeof(ubo));
}

static bool init_memory_heaps(graphics_t* r) {
    r->assets.vertex_heap = gpu_heap_create(
        r,
        1ULL * 1024 * 1024 * 1024,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    // TODO: change to lower value! set to 10GB for large/sponza scene, check out AMD VMA.
    r->assets.device_heap = gpu_heap_create(
        r, 10ULL * 1024 * 1024 * 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
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

static int32_t begin_frame(graphics_t* r, platform_t* platform, mat4_t view) {
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

    update_uniform_buffer(r, view, r->current_frame);
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

    vk_mesh->vertex_count = data->vertex_count;
    vk_mesh->index_count  = data->index_count;
    vk_mesh->is_active    = true;

    log_info(
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
    log_info("vulkan: uploaded texture to pool slot %d", id);

    return (texture_handle_t){.id = id};
}

material_handle_t graphics_create_material(
    graphics_t*      r,
    texture_handle_t albedo,
    texture_handle_t normal
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

    vk_texture_t* a_tex = &r->assets.textures[albedo.id];
    vk_texture_t* n_tex = &r->assets.textures[normal.id];

    VkDescriptorImageInfo albedo_info = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView   = a_tex->view,
        .sampler     = a_tex->sampler
    };

    VkDescriptorImageInfo normal_info = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView   = n_tex->view,
        .sampler     = n_tex->sampler
    };

    VkWriteDescriptorSet writes[2] = {0};

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

    vkUpdateDescriptorSets(r->core.device, 2, writes, 0, NULL);

    mat->is_active = true;
    return (material_handle_t){.id = id};
}

static VkPipeline get_draw_mode_pipeline(vk_pipelines_t* pipelines, draw_mode_t draw_mode) {
    switch (draw_mode) {
    case DRAW_MODE_LIT:
        return pipelines->graphics;
    case DRAW_MODE_DEBUG_WIREFRAME:
        return pipelines->debug_wireframe;
    case DRAW_MODE_DEBUG_LIGHTING:
        return pipelines->debug_lighting;
    case DRAW_MODE_DEBUG_ALBEDO:
        return pipelines->debug_albedo;
    case DRAW_MODE_DEBUG_GEOMETRY_NORMAL:
        return pipelines->debug_geometry_normal;
    case DRAW_MODE_DEBUG_TEXTURE_NORMAL:
        return pipelines->debug_texture_normal;
    case DRAW_MODE_DEBUG_NORMAL:
        return pipelines->debug_normal;
    case DRAW_MODE_DEBUG_TANGENT:
        return pipelines->debug_tangent;
    case DRAW_MODE_DEBUG_BITANGENT:
        return pipelines->debug_bitangent;
    case DRAW_MODE_DEBUG_VERTEX_COLOR:
        return pipelines->debug_vertex_color;
    default:
        return pipelines->graphics;
    }
}

void graphics_draw(
    graphics_t*      r,
    platform_t*      platform,
    mat4_t           view,
    draw_mode_t      draw_mode,
    render_object_t* objects,
    uint32_t         object_count
) {
    int32_t image_index = begin_frame(r, platform, view);
    if (image_index < 0) {
        return;
    }

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

        vkCmdPushConstants(
            r->command_buffer,
            r->pipelines.layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(mat4_t),
            &obj->transform
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

    vkCmdBindPipeline(r->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->pipelines.line);

    mat4_t identity = mat4_identity();
    vkCmdPushConstants(
        r->command_buffer,
        r->pipelines.layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(mat4_t),
        &identity
    );

    vkCmdBindVertexBuffers(r->command_buffer, 0, 1, &r->grid_buffer.buffer, offsets);
    vkCmdDraw(r->command_buffer, r->grid_vertex_count, 1, 0, 0);

    end_frame(r, (uint32_t)image_index);
}
