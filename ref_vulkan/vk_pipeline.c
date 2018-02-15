/*
Copyright (C) 2018 Victor Coda.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "vk_local.h"

VkPipeline Vk_CreateDefaultPipeline(
    VkPipelineShaderStageCreateInfo vert, 
    VkPipelineShaderStageCreateInfo frag)
{
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkGraphicsPipelineCreateInfo gp;
    VkPipelineShaderStageCreateInfo stages[2];
    VkVertexInputBindingDescription vertex_binding;
    VkVertexInputAttributeDescription vertex_attrib;
    VkPipelineVertexInputStateCreateInfo vertex_input;
    VkPipelineInputAssemblyStateCreateInfo assembly;
    VkPipelineViewportStateCreateInfo viewport;
    VkPipelineRasterizationStateCreateInfo rasterization;
    VkPipelineMultisampleStateCreateInfo multisample;
    VkPipelineDepthStencilStateCreateInfo depth_stencil;
    VkPipelineColorBlendAttachmentState blend_attachment;
    VkPipelineColorBlendStateCreateInfo blend;
    VkPipelineDynamicStateCreateInfo dynamic;
    VkDynamicState dynamic_states[2];

    stages[0] = vert;
    stages[1] = frag;

    vertex_binding.binding = 0;
    vertex_binding.stride = sizeof(vec3_t);
    vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertex_attrib.location = 0;
    vertex_attrib.binding = 0;
    vertex_attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attrib.offset = 0;

    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.pNext = NULL;
    vertex_input.flags = 0;
    vertex_input.vertexBindingDescriptionCount = 1;
    vertex_input.pVertexBindingDescriptions = &vertex_binding;
    vertex_input.vertexAttributeDescriptionCount = 1;
    vertex_input.pVertexAttributeDescriptions = &vertex_attrib;

    assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly.pNext = NULL;
    assembly.flags = 0;
    assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assembly.primitiveRestartEnable = VK_FALSE;

    viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport.pNext = NULL;
    viewport.flags = 0;
    viewport.viewportCount = 1;
    viewport.pViewports = NULL; // Dynamic
    viewport.scissorCount = 1;
    viewport.pScissors = NULL; // Dynamic

    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.pNext = NULL;
    rasterization.flags = 0;
    rasterization.depthClampEnable = VK_FALSE;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.cullMode = VK_CULL_MODE_NONE; // TODO: BACK
    rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization.depthBiasEnable = VK_FALSE;
    rasterization.depthBiasConstantFactor = 0.f;
    rasterization.depthBiasClamp = 0.f;
    rasterization.depthBiasSlopeFactor = 0.f;
    rasterization.lineWidth = 1.f;

    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.pNext = NULL;
    multisample.flags = 0;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.minSampleShading = 0;
    multisample.pSampleMask = NULL;
    multisample.alphaToCoverageEnable = VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;

    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.pNext = NULL;
    depth_stencil.flags = 0;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.front.failOp = VK_STENCIL_OP_KEEP;
    depth_stencil.front.passOp = VK_STENCIL_OP_KEEP;
    depth_stencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
    depth_stencil.front.compareOp = VK_COMPARE_OP_ALWAYS;
    depth_stencil.front.compareMask = 0;
    depth_stencil.front.writeMask = 0;
    depth_stencil.front.reference = 0; 
    depth_stencil.back = depth_stencil.front;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds = 0.f;
    depth_stencil.maxDepthBounds = 1.f;

    blend_attachment.blendEnable = VK_FALSE;
    blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blend.pNext = NULL;
    blend.flags = 0;
    blend.logicOpEnable = VK_FALSE;
    blend.logicOp = VK_LOGIC_OP_CLEAR;
    blend.attachmentCount = 1;
    blend.pAttachments = &blend_attachment;
    blend.blendConstants[0] = 1.f;
    blend.blendConstants[1] = 1.f;
    blend.blendConstants[2] = 1.f;
    blend.blendConstants[3] = 1.f;

    dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic.pNext = NULL;
    dynamic.flags = 0;
    dynamic.dynamicStateCount = 2;
    dynamic_states[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamic_states[1] = VK_DYNAMIC_STATE_SCISSOR;
    dynamic.pDynamicStates = dynamic_states;

    gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gp.pNext = NULL;
    gp.flags = 0;
    gp.stageCount = 2;
    gp.pStages = stages;
    gp.pVertexInputState = &vertex_input;
    gp.pInputAssemblyState = &assembly;
    gp.pTessellationState = NULL;
    gp.pViewportState = &viewport;
    gp.pRasterizationState = &rasterization;
    gp.pMultisampleState = &multisample;
    gp.pDepthStencilState = &depth_stencil;
    gp.pColorBlendState = &blend;
    gp.pDynamicState = &dynamic;
    gp.layout = vk_context.pipeline_layout;
    gp.renderPass = vk_context.renderpass;
    gp.subpass = 0;
    gp.basePipelineIndex = 0;
    gp.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(vk_context.device, VK_NULL_HANDLE, 1, &gp, NULL, &pipeline) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "Failed to create graphics pipeline\n");
    }

    return pipeline;
}
