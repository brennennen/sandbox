
#include "engine/core/logger.h"
#include "engine/modules/graphics/graphics.h"
#include "engine/modules/graphics/graphics_types.h"
#include "shared/scene_types.h"

#include "vk_gpu_allocator.h"
#include "vk_types.h"

static bool create_render_target_attachment(
    graphics_t*        graphics,
    uint32_t           width,
    uint32_t           height,
    VkFormat           format,
    VkImageUsageFlags  usage_flags,
    VkImageAspectFlags aspect_flags,
    vk_texture_t*      out_tex
) {
    VkImageCreateInfo image_info = {
        .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType     = VK_IMAGE_TYPE_2D,
        .extent.width  = width,
        .extent.height = height,
        .extent.depth  = 1,
        .mipLevels     = 1,
        .arrayLayers   = 1,
        .format        = format,
        .tiling        = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage         = usage_flags,
        .samples       = VK_SAMPLE_COUNT_1_BIT,
        .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
    };

    if (vkCreateImage(graphics->core.device, &image_info, NULL, &out_tex->image) != VK_SUCCESS) {
        log_error("vulkan: failed to create render target image");
        return false;
    }

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(graphics->core.device, out_tex->image, &mem_reqs);
    gpu_allocation_t alloc = gpu_heap_alloc(
        graphics->assets.device_heap, mem_reqs.size, mem_reqs.alignment
    );
    if (alloc.size == 0) {
        log_error("Failed to allocate texture memory from heap!");
    }

    if (vkBindImageMemory(
            graphics->core.device,
            out_tex->image,
            graphics->assets.device_heap->memory,
            alloc.offset
        ) != VK_SUCCESS) {
        log_error("Failed to bind texture memory!");
    }

    VkSamplerCreateInfo sampler_info = {
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter               = VK_FILTER_NEAREST, // VK_FILTER_LINEAR,
        .minFilter               = VK_FILTER_NEAREST, // VK_FILTER_LINEAR,
        .addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .anisotropyEnable        = VK_FALSE,
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST, // VK_SAMPLER_MIPMAP_MODE_LINEAR,
    };

    if (vkCreateSampler(graphics->core.device, &sampler_info, NULL, &out_tex->sampler) !=
        VK_SUCCESS) {
        log_error("vulkan: failed to create render target sampler");
        return false;
    }

    VkImageViewCreateInfo view_info = {
        .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image            = out_tex->image,
        .viewType         = VK_IMAGE_VIEW_TYPE_2D,
        .format           = format,
        .subresourceRange = {
            .aspectMask     = aspect_flags,
            .baseMipLevel   = 0,
            .levelCount     = 1,
            .baseArrayLayer = 0,
            .layerCount     = 1,
        },
    };

    if (vkCreateImageView(graphics->core.device, &view_info, NULL, &out_tex->view) != VK_SUCCESS) {
        log_error("vulkan: failed to create render target image view");
        return false;
    }

    out_tex->is_active = true;
    return true;
}

render_target_handle_t graphics_create_render_target(
    graphics_t*                   graphics,
    const render_target_config_t* render_target_config
) {
    if (graphics->assets.render_target_count >= VK_MAX_RENDER_TARGETS) {
        log_error("vulkan render target pool exhausted");
        return (render_target_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    uint32_t            id            = graphics->assets.render_target_count++;
    vk_render_target_t* render_target = &graphics->assets.render_targets[id];

    render_target->width     = render_target_config->width;
    render_target->height    = render_target_config->height;
    render_target->has_depth = render_target_config->requires_depth;

    VkFormat color_format = (render_target_config->format == RT_FORMAT_HDR)
                                ? VK_FORMAT_R16G16B16A16_SFLOAT
                                : VK_FORMAT_R8G8B8A8_UNORM;

    if (!create_render_target_attachment(
            graphics,
            render_target_config->width,
            render_target_config->height,
            color_format,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT,
            &render_target->color_attachment
        )) {
        return (render_target_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    if (render_target_config->requires_depth) {
        VkFormat depth_format = VK_FORMAT_D32_SFLOAT;

        if (!create_render_target_attachment(
                graphics,
                render_target_config->width,
                render_target_config->height,
                depth_format,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT,
                &render_target->depth_attachment
            )) {
            return (render_target_handle_t){.id = GRAPHICS_INVALID_HANDLE};
        }
    }

    render_target->is_active = true;
    log_debug(
        "vulkan: Created render target %d (%dx%d)",
        id,
        render_target_config->width,
        render_target_config->height
    );

    VkDescriptorSetAllocateInfo alloc_info = {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = graphics->descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts        = &graphics->pipelines.post_process_set_layout,
    };

    if (vkAllocateDescriptorSets(
            graphics->core.device, &alloc_info, &render_target->color_attachment.descriptor_set
        ) != VK_SUCCESS) {
        log_error("vulkan: fialed to allocate post process descriptor set");
        return (render_target_handle_t){.id = GRAPHICS_INVALID_HANDLE};
    }

    if (render_target->color_attachment.view == VK_NULL_HANDLE) {
        log_error("CRITICAL: render_target->color_attachment.view == VK_NULL_HANDLE");
    }
    if (render_target->color_attachment.sampler == VK_NULL_HANDLE) {
        log_error("CRITICAL: render_target->color_attachment.sampler == VK_NULL_HANDLE");
    }

    VkDescriptorImageInfo image_info = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView   = render_target->color_attachment.view,
        .sampler     = render_target->color_attachment.sampler,
    };
    VkWriteDescriptorSet descriptor_write = {
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet          = render_target->color_attachment.descriptor_set,
        .dstBinding      = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo      = &image_info,
    };
    vkUpdateDescriptorSets(graphics->core.device, 1, &descriptor_write, 0, NULL);

    return (render_target_handle_t){.id = id};
}

void graphics_destroy_render_target(graphics_t* graphics, render_target_handle_t handle) {
    if (handle.id == GRAPHICS_INVALID_HANDLE || handle.id >= graphics->assets.render_target_count) {
        return;
    }

    vk_render_target_t* rt = &graphics->assets.render_targets[handle.id];

    if (!rt->is_active) {
        return; // Already destroyed
    }

    if (rt->color_attachment.view) {
        vkDestroyImageView(graphics->core.device, rt->color_attachment.view, NULL);
    }
    if (rt->color_attachment.image) {
        vkDestroyImage(graphics->core.device, rt->color_attachment.image, NULL);
    }
    if (rt->color_attachment.sampler) {
        vkDestroySampler(graphics->core.device, rt->color_attachment.sampler, NULL);
    }

    if (rt->has_depth) {
        if (rt->depth_attachment.view) {
            vkDestroyImageView(graphics->core.device, rt->depth_attachment.view, NULL);
        }
        if (rt->depth_attachment.image) {
            vkDestroyImage(graphics->core.device, rt->depth_attachment.image, NULL);
        }
        if (rt->depth_attachment.sampler) {
            vkDestroySampler(graphics->core.device, rt->depth_attachment.sampler, NULL);
        }
    }

    rt->is_active = false;

    log_debug("vulkan: Destroyed render target %d", handle.id);
}

void vk_resize_render_target(
    graphics_t*         graphics,
    vk_render_target_t* rt,
    uint32_t            width,
    uint32_t            height
) {
    if (!rt || !rt->is_active) {
        return;
    }
    if (rt->width == width && rt->height == height) {
        return;
    }

    vkDestroyImageView(graphics->core.device, rt->color_attachment.view, NULL);
    vkDestroyImage(graphics->core.device, rt->color_attachment.image, NULL);
    vkDestroySampler(graphics->core.device, rt->color_attachment.sampler, NULL);

    if (rt->has_depth) {
        vkDestroyImageView(graphics->core.device, rt->depth_attachment.view, NULL);
        vkDestroyImage(graphics->core.device, rt->depth_attachment.image, NULL);
        vkDestroySampler(graphics->core.device, rt->depth_attachment.sampler, NULL);
    }

    rt->width  = width;
    rt->height = height;

    create_render_target_attachment(
        graphics,
        width,
        height,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &rt->color_attachment
    );

    if (rt->has_depth) {
        create_render_target_attachment(
            graphics,
            width,
            height,
            VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            &rt->depth_attachment
        );
    }

    VkDescriptorImageInfo image_info = {
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .imageView   = rt->color_attachment.view,
        .sampler     = rt->color_attachment.sampler,
    };
    VkWriteDescriptorSet descriptor_write = {
        .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet          = rt->color_attachment.descriptor_set,
        .dstBinding      = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .pImageInfo      = &image_info,
    };
    vkUpdateDescriptorSets(graphics->core.device, 1, &descriptor_write, 0, NULL);
}
