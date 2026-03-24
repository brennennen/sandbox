
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "core/logger.h"
#include "core/math/math_types.h"
#include "modules/graphics/graphics.h"
#include "vk_commands.h"
#include "vk_devices.h"
#include "vk_pipeline.h"
#include "vk_resources.h"
#include "vk_swapchain.h"
#include "vk_types.h"
#include "volk.h"

#include "modules/assets/image.h"
#include "modules/assets/obj.h"
#include "modules/graphics/graphics_types.h"

#include "vk_gpu_allocator.h"

#include "core/math/mat4_math.h"

#include "modules/graphics/debug/debug_grid.h"

static void init_debug_grid(graphics_t* r) {
    int   grid_size = 10;
    float grid_step = 1.0f;

    r->grid_vertex_count = debug_grid_vertex_count(grid_size);
    size_t buffer_size   = r->grid_vertex_count * sizeof(vertex_t);

    r->grid_buffer.allocation = gpu_heap_alloc(r->vertex_heap, buffer_size, 16);

    VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = buffer_size,
        .usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    vkCreateBuffer(r->device, &buffer_info, NULL, &r->grid_buffer.buffer);
    vkBindBufferMemory(
        r->device, r->grid_buffer.buffer, r->vertex_heap->memory, r->grid_buffer.allocation.offset
    );

    vertex_t* mapped_data = (vertex_t*)r->grid_buffer.allocation.mapped_ptr;
    generate_grid(mapped_data, grid_size, grid_step);
}

void update_uniform_buffer(graphics_t* r, mat4_t view) {
    float  aspect = (float)r->swapchain_extent.width / (float)r->swapchain_extent.height;
    mat4_t proj   = mat4_perspective(0.785f, aspect, 0.1f, 100.0f);
    ubo_t  ubo    = {.view = view, .proj = proj};
    memcpy(r->uniform_alloc.mapped_ptr, &ubo, sizeof(ubo));
}

static bool init_memory_heaps(graphics_t* r) {
    r->vertex_heap = gpu_heap_create(
        r,
        1024 * 1024 * 128,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );
    r->device_heap = gpu_heap_create(r, 1024 * 1024 * 256, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (!r->vertex_heap || !r->device_heap) {
        log_error("vulkan: failed to create GPU memory heaps");
        return false;
    }
    return true;
}

static bool init_uniform_buffer(graphics_t* r) {
    VkBufferCreateInfo ubo_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = sizeof(ubo_t),
        .usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    vkCreateBuffer(r->device, &ubo_info, NULL, &r->uniform_buffer);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(r->device, r->uniform_buffer, &mem_reqs);

    r->uniform_alloc = gpu_heap_alloc(r->vertex_heap, mem_reqs.size, mem_reqs.alignment);
    vkBindBufferMemory(
        r->device, r->uniform_buffer, r->vertex_heap->memory, r->uniform_alloc.offset
    );

    return true;
}

static bool init_descriptors(graphics_t* r) {
    VkDescriptorPoolSize pool_sizes[] = {
        {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1},
        {.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1},
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 2,
        .pPoolSizes    = pool_sizes,
        .maxSets       = 1
    };

    if (vkCreateDescriptorPool(r->device, &pool_info, NULL, &r->descriptor_pool) != VK_SUCCESS) {
        log_error("vulkan: failed to create descriptor pool");
        return false;
    }

    return true;
}

static bool init_sync_objects(graphics_t* r) {
    VkSemaphoreCreateInfo sem_info   = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo     fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    if (vkCreateSemaphore(r->device, &sem_info, NULL, &r->image_available_sem) != VK_SUCCESS ||
        vkCreateSemaphore(r->device, &sem_info, NULL, &r->render_finished_sem) != VK_SUCCESS ||
        vkCreateFence(r->device, &fence_info, NULL, &r->in_flight_fence) != VK_SUCCESS) {
        log_error("vulkan: failed to create sync objects");
        return false;
    }
    return true;
}

graphics_t* graphics_create(platform_t* platform, int width, int height) {
    graphics_t* r = calloc(1, sizeof(struct graphics_t));
    if (!r) {
        log_error("renderer: failed to allocate memory for graphics_t");
        return NULL;
    }

    if (!vk_create_instance(r, platform) ||
        !platform_create_vulkan_surface(platform, r->instance, &r->surface) ||
        !vk_pick_physical_device(r) || !vk_create_logical_device(r)) {
        goto init_failed;
    }

    if (!init_memory_heaps(r) || !init_uniform_buffer(r) || !vk_create_commands(r) ||
        !vk_setup_depth_buffer(r, width, height)) {
        goto init_failed;
    }

    if (!vk_create_swapchain(r, width, height) || !vk_create_graphics_pipeline(r) ||
        !init_descriptors(r)) {
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

    if (r->device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(r->device);
        if (r->vertex_buffer) {
            vkDestroyBuffer(r->device, r->vertex_buffer, NULL);
        }
        if (r->index_buffer) {
            vkDestroyBuffer(r->device, r->index_buffer, NULL);
        }
        if (r->vertex_heap) {
            vkFreeMemory(r->device, r->vertex_heap->memory, NULL);
            free(r->vertex_heap);
        }
        if (r->device_heap) {
            vkFreeMemory(r->device, r->device_heap->memory, NULL);
            free(r->device_heap);
        }
        if (r->grid_buffer.buffer) {
            vkDestroyBuffer(r->device, r->grid_buffer.buffer, NULL);
        }

        vkDestroyBuffer(r->device, r->model_index_buffer, NULL);

        vk_destroy_graphics_pipeline(r);
        vk_destroy_commands(r);

        if (r->image_available_sem)
            vkDestroySemaphore(r->device, r->image_available_sem, NULL);
        if (r->render_finished_sem)
            vkDestroySemaphore(r->device, r->render_finished_sem, NULL);
        if (r->in_flight_fence)
            vkDestroyFence(r->device, r->in_flight_fence, NULL);

        if (r->depth_view) {
            vkDestroyImageView(r->device, r->depth_view, NULL);
        }
        if (r->depth_image) {
            vkDestroyImage(r->device, r->depth_image, NULL);
        }

        vk_destroy_swapchain(r);
        vkDestroyDevice(r->device, NULL);
    }

    if (r->instance != VK_NULL_HANDLE) {
        if (r->surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(r->instance, r->surface, NULL);
        }
        vkDestroyInstance(r->instance, NULL);
    }

    free(r);
    log_info("vulkan: renderer destroyed cleanly");
}

static int32_t begin_frame(graphics_t* r, platform_t* platform, mat4_t view) {
    int w, h;
    platform_get_window_size(platform, &w, &h);
    if (w == 0 || h == 0)
        return -1;

    vkWaitForFences(r->device, 1, &r->in_flight_fence, VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
        r->device, r->swapchain, UINT64_MAX, r->image_available_sem, VK_NULL_HANDLE, &image_index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vk_recreate_swapchain(r, w, h);
        return -1;
    }

    update_uniform_buffer(r, view);

    vkResetFences(r->device, 1, &r->in_flight_fence);
    vkResetCommandBuffer(r->command_buffer, 0);

    VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(r->command_buffer, &begin_info);

    VkImageMemoryBarrier barrier = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image            = r->swapchain_images[image_index],
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
        .imageView   = r->swapchain_image_views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue  = {{{0.1f, 0.1f, 0.2f, 1.0f}}}
    };

    VkRenderingAttachmentInfo depth_attachment = {
        .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView   = r->depth_view,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue  = {.depthStencil = {1.0f, 0}}
    };

    VkRenderingInfo rendering_info = {
        .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea           = {{0, 0}, {r->swapchain_extent.width, r->swapchain_extent.height}},
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &color_attachment,
        .pDepthAttachment     = &depth_attachment
    };

    vkCmdBeginRendering(r->command_buffer, &rendering_info);

    VkViewport viewport = {
        .x        = 0.0f,
        .y        = (float)r->swapchain_extent.height,
        .width    = (float)r->swapchain_extent.width,
        .height   = -(float)r->swapchain_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor = {{0, 0}, r->swapchain_extent};
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
        .image            = r->swapchain_images[image_index],
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
                  .pWaitSemaphores      = &r->image_available_sem,
                  .pWaitDstStageMask    = wait_stages,
                  .commandBufferCount   = 1,
                  .pCommandBuffers      = &r->command_buffer,
                  .signalSemaphoreCount = 1,
                  .pSignalSemaphores    = &r->render_finished_sem,
    };
    vkQueueSubmit(r->graphics_queue, 1, &submit_info, r->in_flight_fence);

    VkPresentInfoKHR present_info = {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &r->render_finished_sem,
        .swapchainCount     = 1,
        .pSwapchains        = &r->swapchain,
        .pImageIndices      = &image_index,
    };
    vkQueuePresentKHR(r->graphics_queue, &present_info);
}

mesh_handle_t graphics_upload_mesh(graphics_t* graphics, mesh_data_t* data) {
    if (graphics->mesh_count >= MAX_MESHES) {
        log_error("Mesh pool exhausted! Cannot upload new mesh.");
        return (mesh_handle_t){.id = UINT32_MAX};
    }

    uint32_t   id      = graphics->mesh_count++;
    vk_mesh_t* vk_mesh = &graphics->mesh_pool[id];

    vk_mesh->vertex_buffer = vk_create_static_buffer(
        graphics,
        data->vertices,
        data->vertex_count * sizeof(vertex_t),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    );

    if (data->index_count > 0) {
        vk_mesh->index_buffer = vk_create_static_buffer(
            graphics,
            data->indices,
            data->index_count * sizeof(uint16_t),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        );
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

texture_handle_t graphics_upload_texture(graphics_t* r, image_t* img) {
    if (r->texture_count >= MAX_TEXTURES) {
        log_error("vulkan: texture pool exhausted!");
        return (texture_handle_t){.id = UINT32_MAX};
    }

    uint32_t      id  = r->texture_count++;
    vk_texture_t* tex = &r->texture_pool[id];

    if (!vk_create_texture(r, img, tex)) {
        log_error("vulkan: failed to create texture for pool slot %d", id);
        return (texture_handle_t){.id = UINT32_MAX};
    }

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = r->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &r->descriptor_set_layout,
    };

    if (vkAllocateDescriptorSets(r->device, &alloc_info, &tex->descriptor_set) != VK_SUCCESS) {
        log_error("vulkan: failed to allocate descriptor set for texture %d", id);
        return (texture_handle_t){.id = UINT32_MAX};
    }

    VkDescriptorBufferInfo buffer_info = {
        .buffer = r->uniform_buffer, .offset = r->uniform_alloc.offset, .range = sizeof(ubo_t)
    };

    VkDescriptorImageInfo image_info = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView   = tex->view,
        .sampler     = tex->sampler
    };

    VkWriteDescriptorSet descriptor_writes[2] = {
        {.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet          = tex->descriptor_set,
         .dstBinding      = 0,
         .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .descriptorCount = 1,
         .pBufferInfo     = &buffer_info},
        {.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet          = tex->descriptor_set,
         .dstBinding      = 1,
         .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .descriptorCount = 1,
         .pImageInfo      = &image_info}
    };

    vkUpdateDescriptorSets(r->device, 2, descriptor_writes, 0, NULL);

    tex->is_active = true;
    log_info("vulkan: uploaded texture to pool slot %d", id);

    return (texture_handle_t){.id = id};
}

void graphics_draw(
    graphics_t*      r,
    platform_t*      platform,
    mat4_t           view,
    render_object_t* objects,
    uint32_t         object_count
) {
    int32_t image_index = begin_frame(r, platform, view);
    if (image_index < 0) {
        return;
    }

    vkCmdBindDescriptorSets(
        r->command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        r->pipeline_layout,
        0,
        1,
        &r->descriptor_set,
        0,
        NULL
    );
    vkCmdBindPipeline(r->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->graphics_pipeline);

    VkDeviceSize offsets[] = {0};

    for (uint32_t i = 0; i < object_count; i++) {
        render_object_t* obj     = &objects[i];
        vk_mesh_t*       vk_mesh = &r->mesh_pool[obj->mesh.id];
        vk_texture_t*    vk_tex  = &r->texture_pool[obj->texture.id];

        if (!vk_mesh->is_active || !vk_tex->is_active)
            continue;

        vkCmdPushConstants(
            r->command_buffer,
            r->pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(mat4_t),
            &obj->transform
        );

        vkCmdBindDescriptorSets(
            r->command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            r->pipeline_layout,
            0,
            1,
            &vk_tex->descriptor_set,
            0,
            NULL
        );

        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(r->command_buffer, 0, 1, &vk_mesh->vertex_buffer, offsets);

        if (vk_mesh->index_count > 0) {
            vkCmdBindIndexBuffer(r->command_buffer, vk_mesh->index_buffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(r->command_buffer, vk_mesh->index_count, 1, 0, 0, 0);
        } else {
            vkCmdDraw(r->command_buffer, vk_mesh->vertex_count, 1, 0, 0);
        }
    }

    vkCmdBindPipeline(r->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->line_pipeline);
    mat4_t identity = mat4_identity();
    vkCmdPushConstants(
        r->command_buffer,
        r->pipeline_layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(mat4_t),
        &identity
    );

    vkCmdBindVertexBuffers(r->command_buffer, 0, 1, &r->grid_buffer.buffer, offsets);
    vkCmdDraw(r->command_buffer, r->grid_vertex_count, 1, 0, 0);
    end_frame(r, (uint32_t)image_index);
}
