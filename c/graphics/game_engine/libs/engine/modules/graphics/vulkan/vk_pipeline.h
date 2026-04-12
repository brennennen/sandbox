#ifndef VK_PIPELINE_H
#define VK_PIPELINE_H

#include "vk_types.h"

bool vk_create_graphics_pipeline(graphics_t* r);
bool vk_create_line_pipeline(graphics_t* r);
void vk_destroy_graphics_pipeline(graphics_t* r);
VkShaderModule vk_create_shader_module(VkDevice device, const char* path);

#endif
