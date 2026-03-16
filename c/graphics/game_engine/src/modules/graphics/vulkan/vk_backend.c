
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
#include "modules/graphics/graphics_types.h"

#include "vk_gpu_allocator.h"

#include "core/math/mat4_math.h"

#include "modules/graphics/debug/debug_grid.h"

const vertex_t vertices[] = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // Top (Red)
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},  // Bottom Right (Green)
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}  // Bottom Left (Blue)
};

const vertex_t square_vertices[] = {
    // Pos          // Color (RGBA: All White) // UV
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}, // Top-Left
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},  // Top-Right
    {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},   // Bottom-Right
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},  // Bottom-Left
};
// clang-format off
const uint16_t square_indices[] = {
    0, 1, 2,  // Triangle 1: Top-Left -> Top-Right -> Bottom-Right
    2, 3, 0   // Triangle 2: Bottom-Right -> Bottom-Left -> Top-Left
};
// clang-format on

void init_debug_grid(renderer_t* r) {
    r->grid_vertex_count = 44;
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

    vertex_t* data  = (vertex_t*)r->grid_buffer.allocation.mapped_ptr;
    int       index = 0;
    float     size  = 10.0f;
    float     step  = 1.0f;

    for (int i = 0; i <= 10; i++) {
        float pos = -5.0f + (float)i * step;

        // grid grey, highlight the center axes in white
        vec4_t color = {0.3f, 0.3f, 0.3f, 1.0f};
        if (i == 5)
            color = (vec4_t){1.0f, 1.0f, 1.0f, 1.0f};

        // Lines parallel to the X-axis (Varying X, Fixed Y, Z = 0)
        data[index++] = (vertex_t){{-5.0f, pos, 0.0f}, color, {0, 0}};
        data[index++] = (vertex_t){{5.0f, pos, 0.0f}, color, {0, 0}};

        // Lines parallel to the Y-axis (Fixed X, Varying Y, Z = 0)
        data[index++] = (vertex_t){{pos, -5.0f, 0.0f}, color, {0, 0}};
        data[index++] = (vertex_t){{pos, 5.0f, 0.0f}, color, {0, 0}};
    }
}

void update_uniform_buffer(renderer_t* r, mat4_t view) {
    float  aspect = (float)r->swapchain_extent.width / (float)r->swapchain_extent.height;
    mat4_t proj   = mat4_perspective(0.785f, aspect, 0.1f, 100.0f);
    ubo_t  ubo    = {.view = view, .proj = proj};
    memcpy(r->uniform_alloc.mapped_ptr, &ubo, sizeof(ubo));
}

renderer_t* renderer_create(platform_t* platform, int width, int height) {
    renderer_t* r = calloc(1, sizeof(struct renderer_t));
    if (!r) {
        log_error("renderer: failed to allocate memory for renderer_t");
        return NULL;
    }

    if (!vk_create_instance(r, platform)) {
        free(r);
        return NULL;
    }

    if (!platform_create_vulkan_surface(platform, r->instance, &r->surface)) {
        log_error("vulkan: surface error: %s", SDL_GetError());
        renderer_destroy(r);
        return NULL;
    }

    if (!vk_pick_physical_device(r)) {
        renderer_destroy(r);
        return NULL;
    }

    if (!vk_create_logical_device(r)) {
        renderer_destroy(r);
        return NULL;
    }

    r->vertex_heap = gpu_heap_create(
        r,
        1024 * 1024 * 128, // 128MB Staging
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    r->device_heap = gpu_heap_create(
        r,
        1024 * 1024 * 256, // 256MB Device Local
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    if (!r->vertex_heap || !r->device_heap) {
        log_error("vulkan: failed to create GPU memory heaps");
        renderer_destroy(r);
        return NULL;
    }

    VkDeviceSize ubo_size = sizeof(ubo_t);

    VkBufferCreateInfo ubo_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = ubo_size,
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

    if (!vk_create_commands(r)) {
        log_error("vulkan: failed to create command pool");
        renderer_destroy(r);
        return NULL;
    }

    if (!vk_setup_depth_buffer(r, width, height)) {
        renderer_destroy(r);
        return NULL;
    }

    const char* test_image_path = "test.png";
    image_t     img;
    bool        loaded = image_load(test_image_path, &img);

    if (loaded) {
        vk_create_texture(r, &img);
        image_free(&img);
    } else {
        log_warn("vulkan: could not find '%s', using placeholder", test_image_path);
        image_t dummy = image_create_placeholder();
        vk_create_texture(r, &dummy);
        image_free(&dummy);
    }

    r->vertex_buffer = vk_create_static_buffer(
        r, (void*)square_vertices, sizeof(square_vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    );

    r->index_buffer = vk_create_static_buffer(
        r, (void*)square_indices, sizeof(square_indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    );

    if (!r->vertex_buffer || !r->index_buffer) {
        log_error("vulkan: failed to create geometry buffers");
        renderer_destroy(r);
        return NULL;
    }

    if (!vk_create_swapchain(r, width, height)) {
        log_error("vulkan: failed to create swapchain");
        renderer_destroy(r);
        return NULL;
    }

    if (!vk_create_graphics_pipeline(r)) {
        log_error("vulkan: failed to build graphics pipeline");
        renderer_destroy(r);
        return NULL;
    }

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
        renderer_destroy(r);
        return NULL;
    }

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = r->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &r->descriptor_set_layout,
    };
    vkAllocateDescriptorSets(r->device, &alloc_info, &r->descriptor_set);

    VkDescriptorBufferInfo buffer_info = {
        .buffer = r->uniform_buffer,
        .offset = r->uniform_alloc.offset,
        .range  = sizeof(ubo_t),
    };

    VkDescriptorImageInfo image_info = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView   = r->texture_view,
        .sampler     = r->texture_sampler,
    };

    VkWriteDescriptorSet descriptor_writes[2] = {
        {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = r->descriptor_set,
            .dstBinding      = 0,
            .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo     = &buffer_info,
        },
        {
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = r->descriptor_set,
            .dstBinding      = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .pImageInfo      = &image_info,
        }
    };
    vkUpdateDescriptorSets(r->device, 2, descriptor_writes, 0, NULL);

    VkSemaphoreCreateInfo sem_info   = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo     fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    if (vkCreateSemaphore(r->device, &sem_info, NULL, &r->image_available_sem) != VK_SUCCESS ||
        vkCreateSemaphore(r->device, &sem_info, NULL, &r->render_finished_sem) != VK_SUCCESS ||
        vkCreateFence(r->device, &fence_info, NULL, &r->in_flight_fence) != VK_SUCCESS) {
        log_error("vulkan: failed to create sync objects");
        renderer_destroy(r);
        return NULL;
    }

    init_debug_grid(r);

    log_info("renderer: initialization complete");
    return r;
}

void renderer_destroy(renderer_t* r) {
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

void renderer_draw(renderer_t* r, platform_t* platform, mat4_t view) {
    int w;
    int h;

    platform_get_window_size(platform, &w, &h);

    if (w == 0 || h == 0) {
        return;
    }

    vkWaitForFences(r->device, 1, &r->in_flight_fence, VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
        r->device, r->swapchain, UINT64_MAX, r->image_available_sem, VK_NULL_HANDLE, &image_index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        vk_recreate_swapchain(r, w, h);
        return;
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
        nullptr,
        0,
        nullptr,
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
        .clearValue  = {.depthStencil = {1.0f, 0}} // 1.0 is the "farthest" depth
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
        .y        = (float)r->swapchain_extent.height, // Start at the bottom
        .width    = (float)r->swapchain_extent.width,
        .height   = -(float)r->swapchain_extent.height, // Negative height flips the Y axis
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor = {{0, 0}, r->swapchain_extent};
    vkCmdSetViewport(r->command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(r->command_buffer, 0, 1, &scissor);

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

    float  ticks             = platform_get_ticks(platform);
    float  time              = (float)SDL_GetTicks() / 1000.0f;
    mat4_t rx                = mat4_rotate_x(M_PI / 2.0f);
    mat4_t rz                = mat4_rotate_z(time);
    mat4_t t                 = mat4_translate((vec3_t){0.0f, 0.0f, 1.0f});
    mat4_t combined_rotation = mat4_mul(rx, rz);
    mat4_t square_model      = mat4_mul(combined_rotation, t);

    vkCmdPushConstants(
        r->command_buffer,
        r->pipeline_layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(mat4_t),
        &square_model
    );

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(r->command_buffer, 0, 1, &r->vertex_buffer, offsets);
    vkCmdBindIndexBuffer(r->command_buffer, r->index_buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(r->command_buffer, 6, 1, 0, 0, 0);

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

    VkDeviceSize g_offsets[] = {0};
    vkCmdBindVertexBuffers(r->command_buffer, 0, 1, &r->grid_buffer.buffer, g_offsets);
    vkCmdDraw(r->command_buffer, r->grid_vertex_count, 1, 0, 0);

    vkCmdEndRendering(r->command_buffer);

    barrier.oldLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    vkCmdPipelineBarrier(
        r->command_buffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
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
