#ifndef VK_TYPES_H
#define VK_TYPES_H

#include "vk_gpu_allocator.h" // So it knows what gpu_heap_t is
#include "volk.h"

#include "core/math/math_types.h"

typedef struct {
    float pos[3];
    float color[4];
    float uv[2];
} vertex_t;

/**
 * gpu heap PIMPL impl
 */
struct gpu_heap_t {
    VkDeviceMemory memory;
    uint32_t       type_index;
    uint64_t       capacity;
    uint64_t       used;
    uint64_t       offset;
    void*          mapped_data;
};

typedef struct {
    mat4_t model;
    mat4_t view;
    mat4_t proj;
} ubo_t;

/**
 * Vulkan PIMPL impl
 */
struct renderer_t {
    VkInstance     instance;
    VkSurfaceKHR   surface;
    VkSwapchainKHR swapchain;

    VkBuffer         uniform_buffer;
    gpu_allocation_t uniform_alloc;

    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool      descriptor_pool;
    VkDescriptorSet       descriptor_set;

    uint32_t     swapchain_image_count;
    VkImage*     swapchain_images;
    VkImageView* swapchain_image_views;

    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice         physical_device;
    VkDevice                 device;
    VkQueue                  graphics_queue;

    VkCommandPool   command_pool;
    VkCommandBuffer command_buffer;

    VkPipeline       graphics_pipeline;
    VkPipeline       transparent_pipeline;
    VkPipelineLayout pipeline_layout;

    VkSemaphore image_available_sem;
    VkSemaphore render_finished_sem;
    VkFence     in_flight_fence;

    // VkDeviceMemory vertex_buffer_memory;
    gpu_heap_t*      vertex_heap;
    gpu_heap_t*      device_heap;
    VkBuffer         vertex_buffer;
    gpu_allocation_t vertex_alloc;

    VkBuffer         index_buffer;
    gpu_allocation_t index_alloc;

    VkFormat   swapchain_format;
    VkExtent2D swapchain_extent;

    VkImage          texture_image;
    gpu_allocation_t texture_allocation;
    VkImageView      texture_view;
    VkSampler        texture_sampler;

    VkImage          depth_image;
    VkImageView      depth_view;
    gpu_allocation_t depth_alloc;

    camera_t camera;
};

#endif
