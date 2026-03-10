
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "core/logger.h"
#include "core/math/math_types.h"
#include "renderer.h"
#include "vk_commands.h"
#include "vk_devices.h"
#include "vk_pipeline.h"
#include "vk_resources.h"
#include "vk_swapchain.h"
#include "vk_types.h"
#include "volk.h"

#include "modules/assets/image.h"

#include "vk_gpu_allocator.h"

#include "core/math/mat4_math.h"

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

camera_t* renderer_get_camera(renderer_t* r) { return &r->camera; }

renderer_t* renderer_create(SDL_Window* window, int width, int height) {
    renderer_t* r = calloc(1, sizeof(struct renderer_t));
    if (!r) {
        log_error("renderer: failed to allocate memory for renderer_t");
        return NULL;
    }

    r->camera.pos   = (vec3_t){0.0f, 0.0f, 0.0f};
    r->camera.yaw   = 0.0f;
    r->camera.pitch = 0.0f;

    if (!vk_create_instance(r)) {
        free(r);
        return NULL;
    }

    if (!SDL_Vulkan_CreateSurface(window, r->instance, NULL, &r->surface)) {
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

    log_info("renderer: initialization complete");
    return r;
}

void renderer_destroy(renderer_t* r) {
    if (r == NULL)
        return;

    if (r->device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(r->device);
        if (r->vertex_buffer)
            vkDestroyBuffer(r->device, r->vertex_buffer, NULL);
        if (r->index_buffer)
            vkDestroyBuffer(r->device, r->index_buffer, NULL);
        if (r->vertex_heap) {
            vkFreeMemory(r->device, r->vertex_heap->memory, NULL);
            free(r->vertex_heap);
        }
        if (r->device_heap) {
            vkFreeMemory(r->device, r->device_heap->memory, NULL);
            free(r->device_heap);
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

void update_uniform_buffer(renderer_t* r) {
    float  aspect = (float)r->swapchain_extent.width / (float)r->swapchain_extent.height;
    mat4_t proj   = mat4_perspective(0.785f, aspect, 0.1f, 100.0f);
    // proj.data[1][1] *= -1;
    mat4_t view  = mat4_view(r->camera);
    float  time  = (float)SDL_GetTicks() / 1000.0f;
    mat4_t model = mat4_mul(mat4_rotate_y(time), mat4_translate((vec3_t){0.0f, 0.0f, -5.0f}));
    ubo_t  ubo   = {.model = model, .view = view, .proj = proj};
    memcpy(r->uniform_alloc.mapped_ptr, &ubo, sizeof(ubo));
}

void renderer_draw(renderer_t* r, SDL_Window* window) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);

    if (w == 0 || h == 0) {
        return;
    }

    vkWaitForFences(r->device, 1, &r->in_flight_fence, VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
        r->device, r->swapchain, UINT64_MAX, r->image_available_sem, VK_NULL_HANDLE, &image_index
    );

    // Handle Resize/Out of Date
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        vk_recreate_swapchain(r, w, h);
        return;
    }

    update_uniform_buffer(r);

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

    VkRenderingInfo rendering_info = {
        .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea           = {{0, 0}, {r->swapchain_extent.width, r->swapchain_extent.height}},
        .layerCount           = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &color_attachment
    };

    vkCmdBeginRendering(r->command_buffer, &rendering_info);
    vkCmdBindPipeline(r->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->graphics_pipeline);

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

    VkViewport viewport = {
        0.0f, 0.0f, (float)r->swapchain_extent.width, (float)r->swapchain_extent.height, 0.0f, 1.0f
    };
    VkRect2D scissor = {{0, 0}, r->swapchain_extent};
    vkCmdSetViewport(r->command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(r->command_buffer, 0, 1, &scissor);

    float time = (float)SDL_GetTicks() / 1000.0f;

    vkCmdPushConstants(
        r->command_buffer, r->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &time
    );

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(r->command_buffer, 0, 1, &r->vertex_buffer, offsets);
    vkCmdBindIndexBuffer(r->command_buffer, r->index_buffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(r->command_buffer, 6, 1, 0, 0, 0);
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

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        int w, h;
        SDL_GetWindowSize(SDL_GetWindowFromID(1), &w, &h);
        vk_recreate_swapchain(r, w, h);
    }
}
