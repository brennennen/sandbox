/*
 * initialization:
 * * Instance & Volk: Load the Vulkan driver and API entry points.
 * * Surface: Create a bridge between Vulkan and the SDL3 window.
 * * Physical Device: Select the actual GPU hardware.
 * * Logical Device: Create a software interface to that GPU (setting up queues).
 * * Swapchain: Setup the buffer images that will eventually be shown on screen.
 * * Pipeline: Compile shaders and set up the fixed-function GPU state.
 */

#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "logger.h"
#include "renderer.h"
#include "volk.h"

struct renderer_t {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;

    uint32_t swapchain_image_count;
    VkImage* swapchain_images;
    VkImageView* swapchain_image_views;

    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;

    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;

    VkPipeline graphics_pipeline;
    VkPipelineLayout pipeline_layout;

    VkSemaphore image_available_sem;  // Signal: "Image is ready to be drawn to"
    VkSemaphore render_finished_sem;  // Signal: "Rendering is done, ready to show"
    VkFence in_flight_fence;          // Signal: "GPU is finished with this command buffer"
};

typedef struct {
    uint32_t graphics_family;
    bool has_graphics;
} queue_family_indices_t;

static bool create_graphics_pipeline(renderer_t* r);
static VkShaderModule create_shader_module(VkDevice device, const char* path);

static queue_family_indices_t find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
    queue_family_indices_t indices = {.has_graphics = false};

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    VkQueueFamilyProperties families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, families);

    for (uint32_t i = 0; i < queue_family_count; i++) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present_support) {
            indices.graphics_family = i;
            indices.has_graphics = true;
            break;
        }
    }

    return indices;
}

static bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    queue_family_indices_t indices = find_queue_families(device, surface);
    return indices.has_graphics
           && device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

static bool pick_physical_device(renderer_t* r) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(r->instance, &device_count, nullptr);

    if (device_count == 0) {
        log_error("vulkan: failed to find GPUs with Vulkan support");
        return false;
    }

    VkPhysicalDevice devices[device_count];
    vkEnumeratePhysicalDevices(r->instance, &device_count, devices);

    for (uint32_t i = 0; i < device_count; i++) {
        if (is_device_suitable(devices[i], r->surface)) {
            r->physical_device = devices[i];
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(r->physical_device, &props);
            log_info("vulkan: selected GPU: %s", props.deviceName);
            return true;
        }
    }
    // Default fallback
    r->physical_device = devices[0];
    return r->physical_device != VK_NULL_HANDLE;
}

static bool create_instance(renderer_t* r) {
    // use volk to dynamically load vulkan
    if (volkInitialize() != VK_SUCCESS) {
        log_error("vulkan: could not find a Vulkan loader");
        return false;
    }

    uint32_t sdl_ext_count = 0;
    const char* const* sdl_exts = SDL_Vulkan_GetInstanceExtensions(&sdl_ext_count);

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "C23 Game Engine",
        .apiVersion = VK_API_VERSION_1_4,
    };

    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = sdl_ext_count,
        .ppEnabledExtensionNames = sdl_exts,
#ifdef DEBUG
        // Validation layers act like a "debug mode" for Vulkan.
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = layers,
#else
        .enabledLayerCount = 0,
#endif
    };

    if (vkCreateInstance(&create_info, nullptr, &r->instance) != VK_SUCCESS) {
        log_error("vulkan: failed to create instance");
        return false;
    }

    volkLoadInstance(r->instance);
    log_info("vulkan: instance created");
    return true;
}

static bool create_logical_device(renderer_t* r) {
    queue_family_indices_t indices = find_queue_families(r->physical_device, r->surface);

    float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = indices.graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority,
    };

    VkPhysicalDeviceVulkan13Features features13 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .dynamicRendering = VK_TRUE,
    };
    const char* device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features13,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_create_info,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = device_extensions,
    };

    if (vkCreateDevice(r->physical_device, &create_info, nullptr, &r->device) != VK_SUCCESS) {
        log_error("vulkan: failed to create logical device");
        return false;
    }

    volkLoadDevice(r->device);
    vkGetDeviceQueue(r->device, indices.graphics_family, 0, &r->graphics_queue);

    log_info("vulkan: logical device and graphics queue ready");
    return true;
}

static bool create_command_pool(renderer_t* r) {
    queue_family_indices_t indices = find_queue_families(r->physical_device, r->surface);

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphics_family,
    };

    if (vkCreateCommandPool(r->device, &pool_info, nullptr, &r->command_pool) != VK_SUCCESS) {
        log_error("vulkan: failed to create command pool");
        return false;
    }

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = r->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    return vkAllocateCommandBuffers(r->device, &alloc_info, &r->command_buffer) == VK_SUCCESS;
}

static bool create_swapchain(renderer_t* r, int width, int height) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(r->physical_device, r->surface, &capabilities);
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(r->physical_device, r->surface, &format_count, nullptr);
    VkSurfaceFormatKHR formats[format_count];
    vkGetPhysicalDeviceSurfaceFormatsKHR(r->physical_device, r->surface, &format_count, formats);
    VkSurfaceFormatKHR selected_format = formats[0];
    for (uint32_t i = 0; i < format_count; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
            && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selected_format = formats[i];
            break;
        }
    }

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = r->surface,
        .minImageCount = 3,
        .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = {(uint32_t)width, (uint32_t)height},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
    };

    if (vkCreateSwapchainKHR(r->device, &create_info, nullptr, &r->swapchain) != VK_SUCCESS) {
        log_error("vulkan: failed to create swapchain");
        return false;
    }

    vkGetSwapchainImagesKHR(r->device, r->swapchain, &r->swapchain_image_count, nullptr);
    r->swapchain_images = malloc(sizeof(VkImage) * r->swapchain_image_count);
    vkGetSwapchainImagesKHR(
        r->device, r->swapchain, &r->swapchain_image_count, r->swapchain_images
    );

    r->swapchain_image_views = malloc(sizeof(VkImageView) * r->swapchain_image_count);
    for (uint32_t i = 0; i < r->swapchain_image_count; i++) {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = r->swapchain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1
            },
        };
        vkCreateImageView(r->device, &view_info, nullptr, &r->swapchain_image_views[i]);
    }

    log_info("vulkan: swapchain and image views created");
    return true;
}

renderer_t* renderer_create(SDL_Window* window, int width, int height) {
    renderer_t* r = malloc(sizeof(struct renderer_t));
    if (!r) return nullptr;
    memset(r, 0, sizeof(struct renderer_t));

    // Execution of the initialization sequence
    if (!create_instance(r)) {
        free(r);
        return nullptr;
    }
    // SDL provides a helper to create the Vulkan Surface for different OSs
    if (!SDL_Vulkan_CreateSurface(window, r->instance, nullptr, &r->surface)) {
        log_error("vulkan: surface error: %s", SDL_GetError());
        renderer_destroy(r);
        return nullptr;
    }

    if (!pick_physical_device(r)) {
        renderer_destroy(r);
        return nullptr;
    }
    if (!create_logical_device(r)) {
        renderer_destroy(r);
        return nullptr;
    }
    if (!create_command_pool(r)) {
        renderer_destroy(r);
        return nullptr;
    }
    if (!create_swapchain(r, width, height)) {
        renderer_destroy(r);
        return nullptr;
    }
    if (!create_graphics_pipeline(r)) {
        renderer_destroy(r);
        return nullptr;
    }

    // Sync objects to prevent the CPU from outrunning the GPU.
    VkSemaphoreCreateInfo sem_info = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    if (vkCreateSemaphore(r->device, &sem_info, nullptr, &r->image_available_sem) != VK_SUCCESS
        || vkCreateSemaphore(r->device, &sem_info, nullptr, &r->render_finished_sem) != VK_SUCCESS
        || vkCreateFence(r->device, &fence_info, nullptr, &r->in_flight_fence) != VK_SUCCESS) {
        log_error("vulkan: failed to create sync objects");
        renderer_destroy(r);
        return nullptr;
    }

    return r;
}

void renderer_destroy(renderer_t* r) {
    if (r == nullptr) return;

    if (r->device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(r->device);
        // Wait for the GPU to finish all work before we start deleting things
        vkDestroyPipeline(r->device, r->graphics_pipeline, nullptr);
        vkDestroyPipelineLayout(r->device, r->pipeline_layout, nullptr);

        if (r->command_pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(r->device, r->command_pool, nullptr);
        }

        if (r->swapchain_image_views != nullptr) {
            for (uint32_t i = 0; i < r->swapchain_image_count; i++) {
                vkDestroyImageView(r->device, r->swapchain_image_views[i], nullptr);
            }
            free(r->swapchain_image_views);
        }

        if (r->swapchain_images != nullptr) free(r->swapchain_images);

        if (r->swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(r->device, r->swapchain, nullptr);
        }

        vkDestroyDevice(r->device, nullptr);
    }

    if (r->instance != VK_NULL_HANDLE) {
        if (r->surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(r->instance, r->surface, nullptr);
        }
        vkDestroyInstance(r->instance, nullptr);
    }

    free(r);
    log_info("vulkan: renderer destroyed cleanly");
}

void renderer_draw(renderer_t* r) {
    // Wait for the GPU to finish the previous frame
    vkWaitForFences(r->device, 1, &r->in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(r->device, 1, &r->in_flight_fence);
    // Get an image from the swapchain
    uint32_t image_index;
    vkAcquireNextImageKHR(
        r->device, r->swapchain, UINT64_MAX, r->image_available_sem, VK_NULL_HANDLE, &image_index
    );

    vkResetCommandBuffer(r->command_buffer, 0);
    VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(r->command_buffer, &begin_info);

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = r->swapchain_images[image_index],
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };
    vkCmdPipelineBarrier(
        r->command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier
    );
    // Start Rendering (Dynamic Rendering)
    VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = r->swapchain_image_views[image_index],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {{{0.1f, 0.1f, 0.2f, 1.0f}}}
    };

    VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, {800, 600}},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment
    };

    vkCmdBeginRendering(r->command_buffer, &rendering_info);
    vkCmdBindPipeline(r->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, r->graphics_pipeline);
    // set viewport and scissor dynamically so we can resize the window easily later
    VkViewport viewport = {0.0f, 0.0f, 800.0f, 600.0f, 0.0f, 1.0f};
    vkCmdSetViewport(r->command_buffer, 0, 1, &viewport);
    VkRect2D scissor = {{0, 0}, {800, 600}};
    vkCmdSetScissor(r->command_buffer, 0, 1, &scissor);
    // trigger the draw call for our triangle
    vkCmdDraw(r->command_buffer, 3, 1, 0, 0);

    vkCmdEndRendering(r->command_buffer);
    // Transition image layout again: "Color Attachment" -> "Present" (send to monitor)
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    vkCmdPipelineBarrier(
        r->command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier
    );

    vkEndCommandBuffer(r->command_buffer);
    // Submit the work to the GPU
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &r->image_available_sem,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &r->command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &r->render_finished_sem,
    };

    vkQueueSubmit(r->graphics_queue, 1, &submit_info, r->in_flight_fence);
    // Present the finished image to the screen
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &r->render_finished_sem,
        .swapchainCount = 1,
        .pSwapchains = &r->swapchain,
        .pImageIndices = &image_index,
    };

    vkQueuePresentKHR(r->graphics_queue, &present_info);
}

// Helper to load SPIR-V shader files from disk
static VkShaderModule create_shader_module(VkDevice device, const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        log_error("vulkan: failed to open shader file: %s", path);
        return VK_NULL_HANDLE;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    uint32_t* code = malloc(size);
    fread(code, 1, size, file);
    fclose(file);

    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = code,
    };

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
        log_error("vulkan: failed to create shader module from: %s", path);
        free(code);
        return VK_NULL_HANDLE;
    }

    free(code);
    return shader_module;
}

static bool create_graphics_pipeline(renderer_t* r) {
    VkShaderModule vert_mod = create_shader_module(r->device, "shaders/triangle.vert.spv");
    VkShaderModule frag_mod = create_shader_module(r->device, "shaders/triangle.frag.spv");

    VkPipelineShaderStageCreateInfo stages[2] = {
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_VERTEX_BIT,
         .module = vert_mod,
         .pName = "main"},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
         .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
         .module = frag_mod,
         .pName = "main"}
    };

    VkPipelineVertexInputStateCreateInfo vertex_input = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
    };
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };
    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE
    };
    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };
    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE
    };
    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment
    };
    // Dynamic states allow us to change viewport/scissor without rebuilding the whole pipeline
    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_states
    };
    VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };
    vkCreatePipelineLayout(r->device, &layout_info, nullptr, &r->pipeline_layout);
    // Part of Dynamic Rendering: we tell the pipeline what image format we'll be drawing to.
    VkPipelineRenderingCreateInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = (VkFormat[]){VK_FORMAT_B8G8R8A8_SRGB}
    };
    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &rendering_info,
        .stageCount = 2,
        .pStages = stages,
        .pVertexInputState = &vertex_input,
        .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_info,
        .layout = r->pipeline_layout
    };
    vkCreateGraphicsPipelines(
        r->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &r->graphics_pipeline
    );
    // Clean up temporary shader modules
    vkDestroyShaderModule(r->device, vert_mod, nullptr);
    vkDestroyShaderModule(r->device, frag_mod, nullptr);
    return true;
}
