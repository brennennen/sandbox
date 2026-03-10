#ifndef VK_PIPELINE_H
#define VK_PIPELINE_H

#include "vk_types.h"

// Compiles the full graphics state machine
bool vk_create_graphics_pipeline(renderer_t* r);

// Cleans up pipeline and layout
void vk_destroy_graphics_pipeline(renderer_t* r);

// Helper for loading SPIR-V from disk
VkShaderModule vk_create_shader_module(VkDevice device, const char* path);

#endif
