#ifndef VK_TYPES_H
#define VK_TYPES_H

#include "volk.h"

#include "core/camera.h"
#include "core/math/math_types.h"
#include "modules/graphics/graphics_types.h"

#define MAX_MESHES 1024
#define MAX_SUBMESHES_PER_MESH 16

#define MAX_TEXTURES 1024

typedef struct graphics_t graphics_t;

typedef struct {
    uint64_t size;
    uint64_t offset;
    void*    mapped_ptr;
} gpu_allocation_t;

typedef struct {
    VkBuffer         vertex_buffer;
    gpu_allocation_t vertex_alloc;
    VkBuffer         index_buffer;
    gpu_allocation_t index_alloc;
    uint32_t         vertex_count;
    uint32_t         index_count;

    // submesh_t submeshes[MAX_SUBMESHES_PER_MESH];
    // uint32_t  submesh_count;

    bool is_active;
} vk_mesh_t;

typedef struct {
    VkImage          image;
    VkImageView      view;
    VkSampler        sampler;
    gpu_allocation_t allocation;
    VkDescriptorSet  descriptor_set;
    bool             is_active;
} vk_texture_t;

typedef struct {
    texture_handle_t albedo;
    texture_handle_t normal;

    VkDescriptorSet descriptor_set;
    bool            is_active;
} vk_material_t;

typedef struct {
    VkBuffer         buffer;
    gpu_allocation_t allocation;
} gpu_buffer_t;

/**
 * gpu heap PIMPL impl
 */
typedef struct {
    VkDeviceMemory memory;
    uint32_t       type_index;
    uint64_t       capacity;
    uint64_t       used;
    uint64_t       offset;
    void*          mapped_data;
} gpu_heap_t;

typedef struct {
    // mat4_t model;
    mat4_t view;
    mat4_t proj;
} ubo_t;

/**
 * Vulkan PIMPL impl
 */
struct graphics_t {
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

    // square
    // VkBuffer         square_vertex_buffer;
    // gpu_allocation_t square_vertex_alloc;
    // VkBuffer         square_index_buffer;
    // gpu_allocation_t square_index_alloc;

    // objs
    // VkBuffer         model_vertex_buffer;
    // gpu_allocation_t model_vertex_alloc;
    // uint32_t         model_vertex_count;

    VkBuffer model_index_buffer;
    uint32_t model_index_count;

    VkFormat   swapchain_format;
    VkExtent2D swapchain_extent;

    VkImage          texture_image;
    gpu_allocation_t texture_allocation;
    VkImageView      texture_view;
    VkSampler        texture_sampler;

    VkImage          depth_image;
    VkImageView      depth_view;
    gpu_allocation_t depth_alloc;

    gpu_buffer_t grid_buffer;
    VkPipeline   line_pipeline;
    uint32_t     grid_vertex_count;

    vk_mesh_t mesh_pool[MAX_MESHES];
    uint32_t  mesh_count;

    vk_texture_t texture_pool[MAX_TEXTURES];
    uint32_t     texture_count;
};

#endif
