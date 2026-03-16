
#include <math.h>
#include <string.h>

#include "core/logger.h"
#include "modules/assets/image.h"
#include "vk_gpu_allocator.h"
#include "math.h"
#include "vk_resources.h"

gpu_allocation_t vk_create_staging_buffer(
    renderer_t*  r,
    void*        data,
    VkDeviceSize size,
    VkBuffer*    out_buffer
) {
    VkBufferCreateInfo buffer_info = {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = size,
        .usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    vkCreateBuffer(r->device, &buffer_info, NULL, out_buffer);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(r->device, *out_buffer, &mem_reqs);

    gpu_allocation_t alloc = gpu_heap_alloc(r->vertex_heap, mem_reqs.size, mem_reqs.alignment);
    vkBindBufferMemory(r->device, *out_buffer, r->vertex_heap->memory, alloc.offset);

    if (data) {
        memcpy(alloc.mapped_ptr, data, size);
    }

    return alloc;
}

VkCommandBuffer vk_begin_single_time_commands(renderer_t* r) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool        = r->command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(r->device, &alloc_info, &cmd);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd, &begin_info);
    return cmd;
}

void vk_end_single_time_commands(renderer_t* r, VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info = {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &cmd,
    };

    vkQueueSubmit(r->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->graphics_queue);

    vkFreeCommandBuffers(r->device, r->command_pool, 1, &cmd);
}

bool vk_create_texture(renderer_t* r, image_t* img) {
    VkBuffer         staging_buffer;
    gpu_allocation_t staging_alloc = vk_create_staging_buffer(
        r, img->pixels, img->size, &staging_buffer
    );

    VkImageCreateInfo image_info = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .extent        = {img->width, img->height, 1},
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .format        = VK_FORMAT_R8G8B8A8_SRGB,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
    };
    vkCreateImage(r->device, &image_info, NULL, &r->texture_image);

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(r->device, r->texture_image, &mem_reqs);
    r->texture_allocation = gpu_heap_alloc(r->device_heap, mem_reqs.size, mem_reqs.alignment);
    vkBindImageMemory(
        r->device, r->texture_image, r->device_heap->memory, r->texture_allocation.offset
    );

    vk_transition_image_layout(
        r, r->texture_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    vk_copy_buffer_to_image(r, staging_buffer, r->texture_image, img->width, img->height);

    vk_transition_image_layout(
        r,
        r->texture_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    VkImageViewCreateInfo view_info = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = r->texture_image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = VK_FORMAT_R8G8B8A8_SRGB,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };
    if (vkCreateImageView(r->device, &view_info, NULL, &r->texture_view) != VK_SUCCESS) {
        log_error("vulkan: failed to create texture image view");
        return false;
    }

    VkSamplerCreateInfo sampler_info = {
        .sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter    = VK_FILTER_NEAREST,
        .minFilter    = VK_FILTER_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .borderColor  = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    };
    vkCreateSampler(r->device, &sampler_info, NULL, &r->texture_sampler);

    vkDestroyBuffer(r->device, staging_buffer, NULL);
    return true;
}

bool vk_create_dummy_texture(renderer_t* r) {
    uint32_t pixels[] = {
        0xFFFF00FF,
        0xFF000000,
        0xFF000000,
        0xFFFF00FF,
    };
    VkDeviceSize image_size = sizeof(pixels);
    uint32_t     width      = 2;
    uint32_t     height     = 2;

    VkBuffer staging_buffer;

    gpu_allocation_t staging_alloc = vk_create_staging_buffer(
        r, pixels, image_size, &staging_buffer
    );

    VkImageCreateInfo image_info = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .extent        = {width, height, 1},
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .format        = VK_FORMAT_R8G8B8A8_SRGB,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
    };

    vkCreateImage(r->device, &image_info, NULL, &r->texture_image);

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(r->device, r->texture_image, &mem_reqs);
    r->texture_allocation = gpu_heap_alloc(r->device_heap, mem_reqs.size, mem_reqs.alignment);
    vkBindImageMemory(
        r->device, r->texture_image, r->device_heap->memory, r->texture_allocation.offset
    );

    vk_transition_image_layout(
        r, r->texture_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    vk_copy_buffer_to_image(r, staging_buffer, r->texture_image, width, height);

    vk_transition_image_layout(
        r,
        r->texture_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    VkImageViewCreateInfo view_info = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = r->texture_image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = VK_FORMAT_R8G8B8A8_SRGB,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };
    if (vkCreateImageView(r->device, &view_info, NULL, &r->texture_view) != VK_SUCCESS) {
        log_error("vulkan: failed to create texture image view");
        return false;
    }

    VkSamplerCreateInfo sampler_info = {
        .sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter    = VK_FILTER_NEAREST,
        .minFilter    = VK_FILTER_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .borderColor  = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
    };
    vkCreateSampler(r->device, &sampler_info, NULL, &r->texture_sampler);

    vkDestroyBuffer(r->device, staging_buffer, NULL);
    return true;
}

void vk_transition_image_layout(
    renderer_t*   r,
    VkImage       image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool        = r->command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(r->device, &alloc_info, &cmd);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd, &begin_info);

    VkImageMemoryBarrier barrier = {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout           = old_layout,
        .newLayout           = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = image,
        .subresourceRange    = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        source_stage          = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage     = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        source_stage          = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage     = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(cmd, source_stage, destination_stage, 0, 0, NULL, 0, NULL, 1, &barrier);
    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cmd
    };
    vkQueueSubmit(r->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->graphics_queue);
    vkFreeCommandBuffers(r->device, r->command_pool, 1, &cmd);
}

static void copy_buffer(renderer_t* r, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool        = r->command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer temp_cmd;
    vkAllocateCommandBuffers(r->device, &alloc_info, &temp_cmd);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(temp_cmd, &begin_info);
    VkBufferCopy copy_region = {.size = size};
    vkCmdCopyBuffer(temp_cmd, src, dst, 1, &copy_region);
    vkEndCommandBuffer(temp_cmd);

    VkSubmitInfo submit_info = {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &temp_cmd,
    };

    vkQueueSubmit(r->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->graphics_queue);

    vkFreeCommandBuffers(r->device, r->command_pool, 1, &temp_cmd);
}

void vk_copy_buffer_to_image(
    renderer_t* r,
    VkBuffer    buffer,
    VkImage     image,
    uint32_t    width,
    uint32_t    height
) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool        = r->command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(r->device, &alloc_info, &cmd);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd, &begin_info);

    VkBufferImageCopy region = {
        .bufferOffset      = 0,
        .bufferRowLength   = 0,
        .bufferImageHeight = 0,
        .imageSubresource  = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .imageOffset       = {0, 0, 0},
        .imageExtent       = {width, height, 1},
    };

    vkCmdCopyBufferToImage(cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cmd
    };

    vkQueueSubmit(r->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->graphics_queue);

    vkFreeCommandBuffers(r->device, r->command_pool, 1, &cmd);
}

VkBuffer vk_create_static_buffer(
    renderer_t*        r,
    void*              data,
    VkDeviceSize       size,
    VkBufferUsageFlags usage
) {
    VkBuffer           staging_buffer;
    VkBufferCreateInfo s_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    vkCreateBuffer(r->device, &s_info, NULL, &staging_buffer);

    VkMemoryRequirements s_reqs;
    vkGetBufferMemoryRequirements(r->device, staging_buffer, &s_reqs);
    gpu_allocation_t s_alloc = gpu_heap_alloc(r->vertex_heap, s_reqs.size, s_reqs.alignment);
    vkBindBufferMemory(r->device, staging_buffer, r->vertex_heap->memory, s_alloc.offset);

    if (s_alloc.mapped_ptr) {
        memcpy(s_alloc.mapped_ptr, data, size);
    }

    VkBuffer           device_buffer;
    VkBufferCreateInfo d_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = size,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };
    vkCreateBuffer(r->device, &d_info, NULL, &device_buffer);

    VkMemoryRequirements d_reqs;
    vkGetBufferMemoryRequirements(r->device, device_buffer, &d_reqs);
    gpu_allocation_t d_alloc = gpu_heap_alloc(r->device_heap, d_reqs.size, d_reqs.alignment);
    vkBindBufferMemory(r->device, device_buffer, r->device_heap->memory, d_alloc.offset);

    copy_buffer(r, staging_buffer, device_buffer, size);
    vkDestroyBuffer(r->device, staging_buffer, NULL);

    return device_buffer;
}

void vk_transition_depth_layout(renderer_t* r, VkImage image) {
    VkCommandBuffer cmd = vk_begin_single_time_commands(r);

    VkImageMemoryBarrier barrier = {
        .sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .image     = image,
        .subresourceRange =
            {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
             .levelCount = 1,
             .layerCount = 1},
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
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
        &barrier
    );

    vk_end_single_time_commands(r, cmd);
}

bool vk_setup_depth_buffer(renderer_t* r, uint32_t width, uint32_t height) {
    VkImageCreateInfo depth_info = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .extent        = {width, height, 1},
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .format        = VK_FORMAT_D32_SFLOAT,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
    };

    if (vkCreateImage(r->device, &depth_info, NULL, &r->depth_image) != VK_SUCCESS)
        return false;

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(r->device, r->depth_image, &mem_reqs);
    r->depth_alloc = gpu_heap_alloc(r->device_heap, mem_reqs.size, mem_reqs.alignment);
    vkBindImageMemory(r->device, r->depth_image, r->device_heap->memory, r->depth_alloc.offset);
    vk_transition_depth_layout(r, r->depth_image);

    VkImageViewCreateInfo view_info = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = r->depth_image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = VK_FORMAT_D32_SFLOAT,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1
        }
    };

    return vkCreateImageView(r->device, &view_info, NULL, &r->depth_view) == VK_SUCCESS;
}