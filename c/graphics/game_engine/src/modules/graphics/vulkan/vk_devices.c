#include <stdlib.h>

#include "core/logger.h"
#include "platform/platform.h"
#include "vk_types.h"
#include "volk.h"

#include "vk_devices.h"

queue_family_indices_t find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface) {
    queue_family_indices_t indices = {.has_graphics = false};

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, families);

    for (uint32_t i = 0; i < queue_family_count; i++) {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present_support) {
            indices.graphics_family = i;
            indices.has_graphics    = true;
            break;
        }
    }

    return indices;
}

static bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);

    queue_family_indices_t indices = find_queue_families(device, surface);

    return indices.has_graphics &&
           device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

bool vk_create_instance(graphics_t* r, platform_t* platform) {
    if (volkInitialize() != VK_SUCCESS) {
        log_error("vulkan: could not find a Vulkan loader");
        return false;
    }

    uint32_t           extensions_count = 0;
    const char* const* extensions = platform_get_vulkan_extensions(platform, &extensions_count);
    // const char* const* sdl_exts      = SDL_Vulkan_GetInstanceExtensions(&sdl_ext_count);

    VkApplicationInfo app_info = {
        .sType            = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "C23 Game Engine",
        .apiVersion       = VK_API_VERSION_1_3,
    };

    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};

    VkInstanceCreateInfo create_info = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &app_info,
        .enabledExtensionCount   = extensions_count,
        .ppEnabledExtensionNames = extensions,
#ifdef DEBUG
        .enabledLayerCount   = 1,
        .ppEnabledLayerNames = layers,
#else
        .enabledLayerCount = 0,
#endif
    };

    if (vkCreateInstance(&create_info, NULL, &r->instance) != VK_SUCCESS) {
        log_error("vulkan: failed to create instance");
        return false;
    }

    volkLoadInstance(r->instance);
    log_info("vulkan: instance created");
    return true;
}

bool vk_pick_physical_device(graphics_t* r) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(r->instance, &device_count, NULL);

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

    // Fallback to the first available device if no discrete GPU is found
    r->physical_device = devices[0];
    return r->physical_device != VK_NULL_HANDLE;
}

bool vk_create_logical_device(graphics_t* r) {
    queue_family_indices_t indices = find_queue_families(r->physical_device, r->surface);

    float                   queue_priority    = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info = {
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = indices.graphics_family,
        .queueCount       = 1,
        .pQueuePriorities = &queue_priority,
    };

    VkPhysicalDeviceVulkan13Features features13 = {
        .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .dynamicRendering = VK_TRUE,
    };

    const char* device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo create_info = {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = &features13,
        .queueCreateInfoCount    = 1,
        .pQueueCreateInfos       = &queue_create_info,
        .enabledExtensionCount   = 1,
        .ppEnabledExtensionNames = device_extensions,
    };

    if (vkCreateDevice(r->physical_device, &create_info, NULL, &r->device) != VK_SUCCESS) {
        log_error("vulkan: failed to create logical device");
        return false;
    }

    volkLoadDevice(r->device);
    vkGetDeviceQueue(r->device, indices.graphics_family, 0, &r->graphics_queue);

    log_info("vulkan: logical device and graphics queue ready");
    return true;
}
