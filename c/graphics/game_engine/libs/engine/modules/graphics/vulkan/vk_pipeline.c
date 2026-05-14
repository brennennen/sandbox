/**
 *
 * https://docs.vulkan.org/spec/latest/chapters/pipelines.html
 */

#include "vk_pipeline.h"

#include <stdio.h>
#include <stdlib.h>

#include "engine/core/logger.h"
#include "engine/modules/graphics/graphics_types.h"

VkShaderModule vk_create_shader_module(VkDevice device, const char* path) {
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
        .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode    = code,
    };

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, NULL, &shader_module) != VK_SUCCESS) {
        log_error("vulkan: failed to create shader module from: %s", path);
        free(code);
        return VK_NULL_HANDLE;
    }

    free(code);
    return shader_module;
}

static VkPipeline create_skybox_pipeline(graphics_t* graphics) {
    VkShaderModule vert_mod = vk_create_shader_module(
        graphics->core.device, "shaders/skybox.vert.spv"
    );
    VkShaderModule frag_mod = vk_create_shader_module(
        graphics->core.device, "shaders/skybox.frag.spv"
    );

    VkPipelineShaderStageCreateInfo stages[2] = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_mod,
            .pName  = "main",
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_mod,
            .pName  = "main",
        }
    };

    // bufferless rendering
    VkPipelineVertexInputStateCreateInfo vertex_input = {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions    = NULL,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions    = NULL
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // 36 vertices = 12 triangles
        .primitiveRestartEnable = VK_FALSE
    };

    // no culling, we are inside the cube
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth   = 1.0f,
        .cullMode    = VK_CULL_MODE_NONE,
        .frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &color_blend_attachment,
    };

    // don't write to depth, and use LESS_OR_EQUAL
    VkPipelineDepthStencilStateCreateInfo depth_stencil = {
        .sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable  = VK_TRUE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL,
    };

    VkPipelineRenderingCreateInfo rendering_info = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &graphics->display.format,
        .depthAttachmentFormat   = VK_FORMAT_D32_SFLOAT,
    };

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamic_info = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates    = dynamic_states,
    };

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &rendering_info,
        .stageCount          = 2,
        .pStages             = stages,
        .pVertexInputState   = &vertex_input,
        .pInputAssemblyState = &input_assembly,
        .pViewportState =
            &(VkPipelineViewportStateCreateInfo){
                .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount  = 1,
            },
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depth_stencil,
        .pColorBlendState    = &color_blending,
        .pDynamicState       = &dynamic_info,
        .layout              = graphics->pipelines.layout,
    };

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(
            graphics->core.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create skybox pipeline");
        return VK_NULL_HANDLE;
    }

    vkDestroyShaderModule(graphics->core.device, vert_mod, NULL);
    vkDestroyShaderModule(graphics->core.device, frag_mod, NULL);
    return pipeline;
}

static VkPipeline create_pipeline_internal(
    graphics_t*         graphics,
    VkPrimitiveTopology topology,
    VkPolygonMode       polygon_mode,
    VkCullModeFlags     cull_mode,
    const char*         vert_path,
    const char*         frag_path
) {

    VkShaderModule vert_mod = vk_create_shader_module(graphics->core.device, vert_path);
    VkShaderModule frag_mod = vk_create_shader_module(graphics->core.device, frag_path);

    VkPipelineShaderStageCreateInfo stages[2] = {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_mod,
            .pName  = "main",
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_mod,
            .pName  = "main",
        }
    };

    VkVertexInputBindingDescription binding_desc = {
        .binding = 0, .stride = sizeof(vertex_t), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription attribute_descriptions[] = {
        // Location, Binding, Format, Offset
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, pos)},
        {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, color)},
        {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, uv)},
        {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex_t, normal)},
        {4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, tangent)}
    };

    VkPipelineVertexInputStateCreateInfo vertex_input = {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions    = &binding_desc,
        .vertexAttributeDescriptionCount = sizeof(attribute_descriptions) /
                                           sizeof(attribute_descriptions[0]),
        .pVertexAttributeDescriptions = attribute_descriptions
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = topology,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = polygon_mode,
        .lineWidth   = 1.0f,
        .cullMode    = cull_mode,
        .frontFace   = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };

    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
    };

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &color_blend_attachment,
    };

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {
        .sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable  = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp   = VK_COMPARE_OP_LESS,
    };

    VkPipelineRenderingCreateInfo rendering_info = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount    = 1,
        .pColorAttachmentFormats = &graphics->display.format,
        .depthAttachmentFormat   = VK_FORMAT_D32_SFLOAT,
    };

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_info = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates    = dynamic_states,
    };

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &rendering_info,
        .stageCount          = 2,
        .pStages             = stages,
        .pVertexInputState   = &vertex_input,
        .pInputAssemblyState = &input_assembly,
        .pViewportState =
            &(VkPipelineViewportStateCreateInfo){
                .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .scissorCount  = 1,
            },
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depth_stencil,
        .pColorBlendState    = &color_blending,
        .pDynamicState       = &dynamic_info,
        .layout              = graphics->pipelines.layout,
    };

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(
            graphics->core.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create pipeline with topology %d", topology);
        return VK_NULL_HANDLE;
    }

    vkDestroyShaderModule(graphics->core.device, vert_mod, NULL);
    vkDestroyShaderModule(graphics->core.device, frag_mod, NULL);
    return pipeline;
}

static bool init_pipeline_layouts(graphics_t* graphics) {
    VkDescriptorSetLayoutBinding global_binding = {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    VkDescriptorSetLayoutCreateInfo global_info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings    = &global_binding,
    };
    if (vkCreateDescriptorSetLayout(
            graphics->core.device, &global_info, NULL, &graphics->pipelines.global_set_layout
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create global descriptor set layout");
        return false;
    }

    VkDescriptorSetLayoutBinding albedo_binding = {
        .binding         = 0,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding normal_binding = {
        .binding         = 1,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding ao_metallic_roughness_binding = {
        .binding         = 2,
        .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    VkDescriptorSetLayoutBinding object_bindings[] = {
        albedo_binding, normal_binding, ao_metallic_roughness_binding
    };

    VkDescriptorSetLayoutCreateInfo object_info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 3,
        .pBindings    = object_bindings,
    };

    if (vkCreateDescriptorSetLayout(
            graphics->core.device, &object_info, NULL, &graphics->pipelines.object_set_layout
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create object descriptor set layout");
        return false;
    }

    VkDescriptorSetLayout layouts[] = {
        graphics->pipelines.global_set_layout,
        graphics->pipelines.object_set_layout,
    };

    VkPushConstantRange push_constant = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset     = 0,
        .size       = sizeof(push_constants_t),
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 2,
        .pSetLayouts            = layouts,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &push_constant,
    };

    if (vkCreatePipelineLayout(
            graphics->core.device, &pipeline_layout_info, NULL, &graphics->pipelines.layout
        ) != VK_SUCCESS) {
        log_error("vulkan: failed to create pipeline layout");
        return false;
    }

    return true;
}

bool vk_create_graphics_pipeline(graphics_t* graphics) {
    if (!init_pipeline_layouts(graphics)) {
        return false;
    }

    graphics->pipelines.forward_lit = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        "shaders/core/mesh.vert.spv",
        "shaders/core/pbr.frag.spv"
    );
    graphics->pipelines.skybox = create_skybox_pipeline(graphics);
    graphics->pipelines.line   = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_BACK_BIT,
        "shaders/core/line.vert.spv",
        "shaders/core/line.frag.spv"
    );
    graphics->pipelines.debug_forward_lit = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        "shaders/core/mesh.vert.spv",
        "shaders/core/debug_pbr.frag.spv"
    );
    graphics->pipelines.debug_wireframe = create_pipeline_internal(
        graphics,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_POLYGON_MODE_LINE,
        VK_CULL_MODE_NONE,
        "shaders/core/mesh.vert.spv",
        "shaders/core/debug_wireframe.frag.spv"
    );

    return (
        graphics->pipelines.forward_lit != VK_NULL_HANDLE &&
        graphics->pipelines.skybox != VK_NULL_HANDLE &&
        graphics->pipelines.debug_wireframe != VK_NULL_HANDLE &&
        graphics->pipelines.debug_forward_lit != VK_NULL_HANDLE &&
        graphics->pipelines.line != VK_NULL_HANDLE
    );
}

void vk_destroy_graphics_pipeline(graphics_t* graphics) {
    if (graphics->pipelines.forward_lit) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.forward_lit, NULL);
    }
    if (graphics->pipelines.transparent) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.transparent, NULL);
    }
    if (graphics->pipelines.skybox) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.skybox, NULL);
    }
    if (graphics->pipelines.line) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.line, NULL);
    }
    if (graphics->pipelines.debug_forward_lit) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.debug_forward_lit, NULL);
    }
    if (graphics->pipelines.debug_wireframe) {
        vkDestroyPipeline(graphics->core.device, graphics->pipelines.debug_wireframe, NULL);
    }

    if (graphics->pipelines.layout) {
        vkDestroyPipelineLayout(graphics->core.device, graphics->pipelines.layout, NULL);
    }
    if (graphics->pipelines.global_set_layout) {
        vkDestroyDescriptorSetLayout(
            graphics->core.device, graphics->pipelines.global_set_layout, NULL
        );
    }
    if (graphics->pipelines.object_set_layout) {
        vkDestroyDescriptorSetLayout(
            graphics->core.device, graphics->pipelines.object_set_layout, NULL
        );
    }
}
