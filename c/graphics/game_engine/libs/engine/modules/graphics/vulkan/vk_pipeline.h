#pragma once

#include "vk_types.h"

bool           vk_create_graphics_pipeline(graphics_t* graphics);
bool           vk_create_line_pipeline(graphics_t* graphics);
void           vk_destroy_graphics_pipeline(graphics_t* graphics);
VkShaderModule vk_create_shader_module(VkDevice device, const char* path);
