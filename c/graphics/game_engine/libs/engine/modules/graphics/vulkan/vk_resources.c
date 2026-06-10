
#include <math.h>
#include <string.h>

#include "engine/core/logger.h"
#include "math.h"
#include "shared/scene_types.h"
#include "vk_gpu_allocator.h"
#include "vk_resources.h"

gpu_allocation_t vk_create_staging_buffer(
    graphics_t*  r,
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
    vkCreateBuffer(r->core.device, &buffer_info, NULL, out_buffer);
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(r->core.device, *out_buffer, &mem_reqs);

    gpu_allocation_t alloc = gpu_heap_alloc(
        r->assets.vertex_heap, mem_reqs.size, mem_reqs.alignment
    );
    vkBindBufferMemory(r->core.device, *out_buffer, r->assets.vertex_heap->memory, alloc.offset);

    if (data) {
        memcpy(alloc.mapped_ptr, data, size);
    }

    return alloc;
}

static VkFormat get_vk_format(pak_texture_format_t pak_format) {
    switch (pak_format) {
    case PAK_TEX_FORMAT_RGBA8_UNORM:
    case PAK_TEX_FORMAT_PNG_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case PAK_TEX_FORMAT_RGBA8_SRGB:
    case PAK_TEX_FORMAT_PNG_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case PAK_TEX_FORMAT_R8_UNORM:
        return VK_FORMAT_R8_UNORM;
    case PAK_TEX_FORMAT_BC7_UNORM:
        return VK_FORMAT_BC7_UNORM_BLOCK;
    case PAK_TEX_FORMAT_BC7_SRGB:
        return VK_FORMAT_BC7_SRGB_BLOCK;
    case PAK_TEX_FORMAT_RGBA32F:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    default:
        log_warn("vulkan: Unknown pak texture format, defaulting to UNORM");
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

bool vk_create_texture(
    graphics_t*          r,
    image_t*             img,
    vk_texture_t*        out_tex,
    pak_texture_format_t format
) {
    log_debug("Vulkan: Creating image with %d mips", img->mip_levels);

    size_t calculated_size = (size_t)img->width * (size_t)img->height * (size_t)img->channels;
    if (calculated_size == 0 || img->size == 0) {
        log_error(
            "CRITICAL: Calculated size is 0! (W:%u, H:%u, CH:%u)",
            img->width,
            img->height,
            img->channels
        );
    }

    size_t scratch_offset = r->assets.vertex_heap->offset;

    VkBuffer         staging_buffer;
    gpu_allocation_t staging_alloc = vk_create_staging_buffer(
        r, img->pixels, img->size, &staging_buffer
    );

    VkFormat tex_format = get_vk_format(format);

    uint32_t mips = (img->mip_levels > 0) ? img->mip_levels : 1;
    // uint32_t mips = 1; // DEBUG hardcode all mips to 1

    VkImageCreateInfo image_info = {
        .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent =
            {
                img->width,
                img->height,
                1,
            },
        .mipLevels     = mips,
        .arrayLayers   = 1,
        .format        = tex_format,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
    };

    if (img->is_cubemap) {
        image_info.flags       = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        image_info.arrayLayers = 6; // 6 faces of the cube
    } else {
        image_info.flags       = 0;
        image_info.arrayLayers = 1;
    }

    vkCreateImage(r->core.device, &image_info, NULL, &out_tex->image);

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(r->core.device, out_tex->image, &mem_reqs);

    if (r->assets.device_heap->offset + mem_reqs.size > r->assets.device_heap->capacity) {
        log_error("CRITICAL: Device Heap Exhausted! Cannot allocate Texture.");
        vkDestroyImage(r->core.device, out_tex->image, NULL);
        vkDestroyBuffer(r->core.device, staging_buffer, NULL);
        r->assets.vertex_heap->offset = scratch_offset;
        return false;
    }

    out_tex->allocation = gpu_heap_alloc(r->assets.device_heap, mem_reqs.size, mem_reqs.alignment);
    vkBindImageMemory(
        r->core.device, out_tex->image, r->assets.device_heap->memory, out_tex->allocation.offset
    );

    vk_transition_image_layout(
        r,
        out_tex->image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        mips,
        img->is_cubemap
    );
    vk_copy_buffer_to_image(
        r, staging_buffer, out_tex->image, img->width, img->height, mips, format, img->is_cubemap
    );
    vk_transition_image_layout(
        r,
        out_tex->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        mips,
        img->is_cubemap
    );

    VkImageViewCreateInfo view_info = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = out_tex->image,
        .viewType         = img->is_cubemap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
        .format           = tex_format,
        .subresourceRange = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = mips,
            .baseArrayLayer = 0,
            .layerCount     = img->is_cubemap ? 6 : 1,
        }
    };
    if (vkCreateImageView(r->core.device, &view_info, NULL, &out_tex->view) != VK_SUCCESS)
        return false;

    VkSamplerCreateInfo sampler_info = {
        .sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter    = VK_FILTER_LINEAR,                      // VK_FILTER_NEAREST,
        .minFilter    = VK_FILTER_LINEAR,                      // VK_FILTER_NEAREST,
        .mipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR,         // VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, // VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, // VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, // VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .borderColor  = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .maxLod       = (float)mips,
        .mipLodBias   = 0.0f,
    };
    vkCreateSampler(r->core.device, &sampler_info, NULL, &out_tex->sampler);

    vkDestroyBuffer(r->core.device, staging_buffer, NULL);
    r->assets.vertex_heap->offset = scratch_offset;
    return true;
}

void vk_transition_image_layout(
    graphics_t*   r,
    VkImage       image,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    uint32_t      mip_levels,
    bool          is_cubemap
) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool        = r->command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(r->core.device, &alloc_info, &cmd);

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
        .subresourceRange    = {
               .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
               .baseMipLevel   = 0,
               .levelCount     = mip_levels,
               .baseArrayLayer = 0,
               .layerCount     = is_cubemap ? 6 : 1,
        },
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
    vkQueueSubmit(r->core.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->core.graphics_queue);
    vkFreeCommandBuffers(r->core.device, r->command_pool, 1, &cmd);
}

static void copy_buffer(graphics_t* r, VkBuffer src, VkBuffer dst, VkDeviceSize size) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool        = r->command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer temp_cmd;
    vkAllocateCommandBuffers(r->core.device, &alloc_info, &temp_cmd);

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

    vkQueueSubmit(r->core.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->core.graphics_queue);

    vkFreeCommandBuffers(r->core.device, r->command_pool, 1, &temp_cmd);
}

void vk_copy_buffer_to_image(
    graphics_t*          r,
    VkBuffer             buffer,
    VkImage              image,
    uint32_t             width,
    uint32_t             height,
    uint32_t             mip_levels,
    pak_texture_format_t format,
    bool                 is_cubemap
) {
    VkCommandBufferAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool        = r->command_pool,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(r->core.device, &alloc_info, &cmd);

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(cmd, &begin_info);

    bool is_bc7 = (format == PAK_TEX_FORMAT_BC7_UNORM || format == PAK_TEX_FORMAT_BC7_SRGB);

    uint32_t layer_count     = is_cubemap ? 6 : 1;
    uint32_t bytes_per_pixel = 4; // Default to RGBA8
    if (format == PAK_TEX_FORMAT_RGBA32F) {
        bytes_per_pixel = 16;
    } else if (format == PAK_TEX_FORMAT_R8_UNORM) {
        bytes_per_pixel = 1;
    }

    VkBufferImageCopy regions[16];
    uint32_t          current_offset = 0;
    uint32_t          mip_w          = width;
    uint32_t          mip_h          = height;

    for (uint32_t i = 0; i < mip_levels; i++) {
        regions[i] = (VkBufferImageCopy){
            .bufferOffset      = current_offset,
            .bufferRowLength   = 0,
            .bufferImageHeight = 0,
            .imageSubresource =
                {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel       = i,
                    .baseArrayLayer = 0,
                    .layerCount     = layer_count,
                },
            .imageOffset =
                {
                    0,
                    0,
                    0,
                },
            .imageExtent = {
                mip_w,
                mip_h,
                1,
            },
        };

        uint32_t mip_layer_size = 0;
        if (is_bc7) {
            uint32_t blocks_x = (mip_w + 3) / 4;
            uint32_t blocks_y = (mip_h + 3) / 4;
            current_offset += blocks_x * blocks_y * 16;
        } else {
            mip_layer_size = mip_w * mip_h * bytes_per_pixel;
        }
        current_offset += mip_layer_size * layer_count;

        mip_w = (mip_w > 1) ? mip_w / 2 : 1;
        mip_h = (mip_h > 1) ? mip_h / 2 : 1;
    }

    vkCmdCopyBufferToImage(
        cmd, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels, regions
    );

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cmd
    };

    vkQueueSubmit(r->core.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(r->core.graphics_queue);

    vkFreeCommandBuffers(r->core.device, r->command_pool, 1, &cmd);
}

VkBuffer vk_create_static_buffer(
    graphics_t*        r,
    void*              data,
    VkDeviceSize       size,
    VkBufferUsageFlags usage
) {
    size_t scratch_offset = r->assets.vertex_heap->offset;

    VkBuffer           staging_buffer;
    VkBufferCreateInfo s_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    };
    // log_info("vkCreateBuffer - staging_buffer: %d", s_info.size);
    vkCreateBuffer(r->core.device, &s_info, NULL, &staging_buffer);

    VkMemoryRequirements s_reqs;
    vkGetBufferMemoryRequirements(r->core.device, staging_buffer, &s_reqs);
    gpu_allocation_t s_alloc = gpu_heap_alloc(r->assets.vertex_heap, s_reqs.size, s_reqs.alignment);

    if (s_alloc.mapped_ptr == NULL) {
        log_error("CRITICAL: Vertex Heap Exhausted!");
        vkDestroyBuffer(r->core.device, staging_buffer, NULL);
        return VK_NULL_HANDLE;
    }

    vkBindBufferMemory(
        r->core.device, staging_buffer, r->assets.vertex_heap->memory, s_alloc.offset
    );

    if (s_alloc.mapped_ptr) {
        memcpy(s_alloc.mapped_ptr, data, size);
    }

    VkBuffer           device_buffer;
    VkBufferCreateInfo d_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size  = size,
        .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };
    // log_info("vkCreateBuffer - device_buffer: %d", d_info.size);
    vkCreateBuffer(r->core.device, &d_info, NULL, &device_buffer);

    VkMemoryRequirements d_reqs;
    vkGetBufferMemoryRequirements(r->core.device, device_buffer, &d_reqs);

    if (r->assets.device_heap->offset + d_reqs.size > r->assets.device_heap->capacity) {
        log_error("CRITICAL: Device Heap Exhausted! Cannot allocate static buffer.");
        vkDestroyBuffer(r->core.device, staging_buffer, NULL);
        vkDestroyBuffer(r->core.device, device_buffer, NULL);
        r->assets.vertex_heap->offset = scratch_offset;
        return VK_NULL_HANDLE;
    }

    gpu_allocation_t d_alloc = gpu_heap_alloc(r->assets.device_heap, d_reqs.size, d_reqs.alignment);
    vkBindBufferMemory(
        r->core.device, device_buffer, r->assets.device_heap->memory, d_alloc.offset
    );

    copy_buffer(r, staging_buffer, device_buffer, size);
    vkDestroyBuffer(r->core.device, staging_buffer, NULL);
    r->assets.vertex_heap->offset = scratch_offset;
    return device_buffer;
}
