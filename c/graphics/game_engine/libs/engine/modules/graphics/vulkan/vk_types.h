#ifndef VK_TYPES_H
#define VK_TYPES_H

#include <stdint.h>

#include "engine/modules/graphics/graphics.h"
#include "volk.h"

#include "engine/core/math/math_types.h"
#include "engine/modules/graphics/graphics_types.h"

#define MAX_MESHES 1024
#define MAX_SUBMESHES_PER_MESH 16

#define MAX_TEXTURES 1024

#define FRAMES_IN_FLIGHT 2

typedef struct graphics_t graphics_t;

/**
 * @brief Tracks a sub-allocation of memory within a larger GPU heap.
 * Prevents the engine from making hundreds of expensive OS-level allocations.
 */
typedef struct {
    uint64_t size;
    uint64_t offset;
    void*    mapped_ptr;
} gpu_allocation_t;

/**
 * @brief Represents a 3D model loaded into VRAM.
 * Contains handles to the GPU buffers holding vertex and index data.
 */
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

/**
 * @brief Represents a sampled image loaded into VRAM for use in shaders.
 */
typedef struct {
    VkImage          image;
    VkImageView      view;
    VkSampler        sampler;
    gpu_allocation_t allocation;
    VkDescriptorSet  descriptor_set;
    bool             is_active;
} vk_texture_t;

/**
 * @brief Represents surface properties mapping to multiple textures.
 * Ex: PBR workflow (Albedo + Normal + Roughness/Metallic).
 */
typedef struct {
    texture_handle_t albedo;
    texture_handle_t normal;

    VkDescriptorSet descriptor_set;
    bool            is_active;
} vk_material_t;

/**
 * @brief A generic wrapper coupling a Vulkan buffer with its memory allocation.
 */
typedef struct {
    VkBuffer         buffer;
    gpu_allocation_t allocation;
} gpu_buffer_t;

/**
 * @brief Custom Vulkan memory allocator/heap.
 * Represents a massive block of VRAM that the engine sub-allocates from.
 */
typedef struct {
    VkDeviceMemory memory;
    uint32_t       type_index;
    uint64_t       capacity;
    uint64_t       used;
    uint64_t       offset;
    void*          mapped_data;
} gpu_heap_t;

/**
 * @brief Uniform Buffer Object data structure.
 * This exact memory layout is uploaded to the GPU every frame for camera math.
 */
typedef struct {
    // mat4_t model;
    mat4_t view;
    mat4_t proj;
} ubo_t;

/**
 * @brief Per-frame synchronization and command state.
 * Required for double-buffering (Frames in Flight) to ensure the CPU
 * does not overwrite memory the GPU is actively reading.
 */
typedef struct {
    VkCommandBuffer command_buffer;
    VkSemaphore     image_available_sem;
    VkSemaphore     render_finished_sem;
    VkFence         in_flight_fence;

    VkBuffer         uniform_buffer;
    gpu_allocation_t uniform_alloc;
    VkDescriptorSet  global_descriptor_set;
} vk_frame_data_t;

/**
 * @brief The Core Vulkan Machine context.
 * Contains the foundational handles required to interface with the GPU hardware.
 */
typedef struct {
    VkInstance               instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR             surface;
    VkPhysicalDevice         physical_device;
    VkDevice                 device;
    VkQueue                  graphics_queue;
} vk_core_t;

/**
 * @brief The Display and Render Target context.
 * Manages the Swapchain (the bridge between Vulkan and the OS Windowing system).
 */
typedef struct {
    present_mode_t   abstract_present_mode;
    VkPresentModeKHR present_mode;

    VkSwapchainKHR swapchain;
    VkFormat       format;
    VkExtent2D     extent;
    uint32_t       image_count;
    VkImage*       images;
    VkImageView*   image_views;

    // Depth buffer is tied 1:1 with swapchain sizing
    VkImage          depth_image;
    VkImageView      depth_view;
    gpu_allocation_t depth_alloc;
} vk_display_t;

/**
 * @brief Asset Management context.
 * Holds the giant memory heaps and the arrays for data-oriented handle lookups.
 */
typedef struct {
    gpu_heap_t* vertex_heap;
    gpu_heap_t* device_heap;
    gpu_heap_t* display_heap;

    vk_mesh_t meshes[MAX_MESHES];
    uint32_t  mesh_count;

    vk_texture_t textures[MAX_TEXTURES];
    uint32_t     texture_count;
} vk_assets_t;

/**
 * @brief Shader and Pipeline context.
 * Defines how memory is bound to shaders (Descriptor Layouts) and the actual
 * compiled state machines (Pipelines) used for drawing.
 */
typedef struct {
    VkDescriptorPool      pool;
    VkDescriptorSetLayout global_set_layout;
    VkDescriptorSetLayout object_set_layout;

    VkPipelineLayout layout;
    VkPipeline       graphics;
    VkPipeline       transparent;
    VkPipeline       debug_wireframe;
    VkPipeline       debug_lighting;
    VkPipeline       debug_albedo;
    VkPipeline       debug_normal;
    VkPipeline       debug_vertex_color;
    VkPipeline       line;
} vk_pipelines_t;

/**
 * @brief Immediate Submission / DMA context.
 * Used for blocking, fire-and-forget memory transfers (uploading textures/meshes).
 */
typedef struct {
    VkCommandPool   command_pool;
    VkCommandBuffer command_buffer;
    VkFence         fence;
} vk_transfer_t;

/**
 * @brief The Vulkan PIMPL (Pointer to Implementation) Object.
 * Orchestrates all subsystems. Passed as an opaque pointer to the frontend engine.
 */
struct graphics_t {
    vk_core_t      core;
    vk_display_t   display;
    vk_assets_t    assets;
    vk_pipelines_t pipelines;
    vk_transfer_t  transfer;

    VkDescriptorPool descriptor_pool; // Kept separate for global dynamic allocations

    // Main render loop command generation
    VkCommandPool   command_pool;
    VkCommandBuffer command_buffer; // Pointer to current frame's command buffer

    vk_frame_data_t frames[FRAMES_IN_FLIGHT];
    uint32_t        current_frame; // Toggles between 0 and 1

    gpu_buffer_t grid_buffer;
    uint32_t     grid_vertex_count;
};

#endif
