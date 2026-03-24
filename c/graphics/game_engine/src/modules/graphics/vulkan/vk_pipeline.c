#include "vk_pipeline.h"

#include <stdio.h>
#include <stdlib.h>

#include "core/logger.h"

#include "modules/graphics/graphics_types.h"

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

static VkPipeline create_pipeline_internal(
    graphics_t*         r,
    VkPrimitiveTopology topology,
    const char*         vert_path,
    const char*         frag_path
) {

    VkShaderModule vert_mod = vk_create_shader_module(r->device, vert_path);
    VkShaderModule frag_mod = vk_create_shader_module(r->device, frag_path);

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

    VkVertexInputAttributeDescription attribute_descriptions[4] = {0};

    // Position
    attribute_descriptions[0].binding  = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset   = offsetof(vertex_t, pos);

    // Color
    attribute_descriptions[1].binding  = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    attribute_descriptions[1].offset   = offsetof(vertex_t, color);

    // UV
    attribute_descriptions[2].binding  = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[2].offset   = offsetof(vertex_t, uv);

    // Normal
    attribute_descriptions[3].binding  = 0;
    attribute_descriptions[3].location = 3;
    attribute_descriptions[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[3].offset   = offsetof(vertex_t, normal);

    VkPipelineVertexInputStateCreateInfo vertex_input = {
        .sType                         = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions    = &binding_desc,
        .vertexAttributeDescriptionCount = 4,
        .pVertexAttributeDescriptions    = attribute_descriptions
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = topology,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType       = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth   = 1.0f,
        .cullMode    = VK_CULL_MODE_BACK_BIT,
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
        .pColorAttachmentFormats = &r->swapchain_format,
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
        .pViewportState      = &(
            VkPipelineViewportStateCreateInfo
        ){.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
               .viewportCount = 1,
               .scissorCount  = 1},
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pDepthStencilState  = &depth_stencil,
        .pColorBlendState    = &color_blending,
        .pDynamicState       = &dynamic_info,
        .layout              = r->pipeline_layout,
    };

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(r->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline) !=
        VK_SUCCESS) {
        log_error("vulkan: failed to create pipeline with topology %d", topology);
        return VK_NULL_HANDLE;
    }

    vkDestroyShaderModule(r->device, vert_mod, NULL);
    vkDestroyShaderModule(r->device, frag_mod, NULL);
    return pipeline;
}

bool vk_create_graphics_pipeline(graphics_t* r) {
    VkDescriptorSetLayoutBinding bindings[2] = {
        {
            .binding         = 0,
            .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
        },
        {
            .binding         = 1,
            .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
        }
    };

    VkDescriptorSetLayoutCreateInfo ds_layout_info = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings    = bindings,
    };

    if (vkCreateDescriptorSetLayout(r->device, &ds_layout_info, NULL, &r->descriptor_set_layout) !=
        VK_SUCCESS) {
        return false;
    }

    VkPushConstantRange push_constant = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset     = 0,
        .size       = sizeof(mat4_t),
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &r->descriptor_set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &push_constant,
    };

    if (vkCreatePipelineLayout(r->device, &pipeline_layout_info, NULL, &r->pipeline_layout) !=
        VK_SUCCESS) {
        return false;
    }

    r->graphics_pipeline = create_pipeline_internal(
        r,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        "shaders/triangle.vert.spv",
        "shaders/triangle.frag.spv"
    );
    r->line_pipeline = create_pipeline_internal(
        r, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, "shaders/unlit.vert.spv", "shaders/unlit.frag.spv"
    );

    return (r->graphics_pipeline != VK_NULL_HANDLE && r->line_pipeline != VK_NULL_HANDLE);
}

void vk_destroy_graphics_pipeline(graphics_t* r) {
    if (r->graphics_pipeline)
        vkDestroyPipeline(r->device, r->graphics_pipeline, NULL);
    if (r->pipeline_layout)
        vkDestroyPipelineLayout(r->device, r->pipeline_layout, NULL);
}
