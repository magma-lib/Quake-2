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

typedef struct
{
	VkVertexInputBindingDescription bindings[3]; 
	VkVertexInputAttributeDescription attribs[3];
    uint32_t vertexBindingDescriptionCount;
    uint32_t vertexAttributeDescriptionCount;
} vkfmt_t;

/*
=============
Vk_SetBrushVertexFormat
=============
*/
static void Vk_SetBrushVertexFormat(vkfmt_t *vf)
{
	// all attributes are come from single vertex buffer
    vf->bindings[0].binding = 0;
    vf->bindings[0].stride = sizeof(vkpolyvert_t);
    vf->bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // position
    vf->attribs[0].location = 0;
    vf->attribs[0].binding = 0;
    vf->attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vf->attribs[0].offset = offsetof(vkpolyvert_t, pos);
    // uv0
    vf->attribs[1].location = 1;
    vf->attribs[1].binding = 0;
    vf->attribs[1].format = VK_FORMAT_R32G32_SFLOAT;
    vf->attribs[1].offset = offsetof(vkpolyvert_t, s1);
    // uv1
    vf->attribs[2].location = 2;
    vf->attribs[2].binding = 0;
    vf->attribs[2].format = VK_FORMAT_R32G32_SFLOAT;
    vf->attribs[2].offset = offsetof(vkpolyvert_t, s2);

	vf->vertexBindingDescriptionCount = 1;
	vf->vertexAttributeDescriptionCount = 3;
}

/*
=============
Vk_SetAliasVertexFormat
=============
*/
static void Vk_SetAliasVertexFormat(vkfmt_t *vf)
{
	// position
    vf->bindings[0].binding = 1;
    vf->bindings[0].stride = sizeof(float) * 4; // see gl_mesh.c, ln 159
    vf->bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // color
    vf->bindings[1].binding = 2;
    vf->bindings[1].stride = sizeof(float) * 3; 
    vf->bindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // texcoord
    vf->bindings[2].binding = 3;
    vf->bindings[2].stride = sizeof(float) * 2; 
    vf->bindings[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // position
    vf->attribs[0].location = 0;
    vf->attribs[0].binding = 1;
    vf->attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // .w ignored on writing
    vf->attribs[0].offset = 0;
    // color
    vf->attribs[1].location = 1;
    vf->attribs[1].binding = 2;
    vf->attribs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vf->attribs[1].offset = 0;
    // texcoord
    vf->attribs[2].location = 2;
    vf->attribs[2].binding = 3;
    vf->attribs[2].format = VK_FORMAT_R32G32_SFLOAT;
    vf->attribs[2].offset = 0;

	vf->vertexBindingDescriptionCount = 3;
	vf->vertexAttributeDescriptionCount = 3;
}

/*
=============
Vk_SetSpriteVertexFormat
=============
*/
static void Vk_SetSpriteVertexFormat(vkfmt_t *vf)
{
	// TODO:
}

/*
=============
Vk_SetParticleVertexFormat
=============
*/
static void Vk_SetParticleVertexFormat(vkfmt_t *vf)
{
	// See vkdrawvert_t
    vf->bindings[0].binding = 0;
    vf->bindings[0].stride = sizeof(vkparticle_t);
    vf->bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // x, y, z
    vf->attribs[0].location = 0;
    vf->attribs[0].binding = 0;
    vf->attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vf->attribs[0].offset = offsetof(vkparticle_t, pos);
    // color
    vf->attribs[1].location = 1;
    vf->attribs[1].binding = 0;
    vf->attribs[1].format = VK_FORMAT_R8G8B8A8_UNORM;
    vf->attribs[1].offset = offsetof(vkparticle_t, color);

	vf->vertexBindingDescriptionCount = 1;
	vf->vertexAttributeDescriptionCount = 2;
}

/*
=============
Vk_SetDrawVertexFormat
=============
*/
static void Vk_SetDrawVertexFormat(vkfmt_t *vf)
{
    vf->bindings[0].binding = 0;
    vf->bindings[0].stride = sizeof(vkdrawvert_t);
    vf->bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // x, y
    vf->attribs[0].location = 0;
    vf->attribs[0].binding = 0;
    vf->attribs[0].format = VK_FORMAT_R32G32_SFLOAT;
    vf->attribs[0].offset = offsetof(vkdrawvert_t, x);
    // s, t
    vf->attribs[1].location = 1;
    vf->attribs[1].binding = 0;
    vf->attribs[1].format = VK_FORMAT_R32G32_SFLOAT;
    vf->attribs[1].offset = offsetof(vkdrawvert_t, s);

	vf->vertexBindingDescriptionCount = 1;
	vf->vertexAttributeDescriptionCount = 2;
}

/*
=============
Vk_SetFillVertexFormat
=============
*/
static void Vk_SetFillVertexFormat(vkfmt_t *vf)
{
    vf->bindings[0].binding = 0;
    vf->bindings[0].stride = sizeof(float) * 2;
    vf->bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // x, y
    vf->attribs[0].location = 0;
    vf->attribs[0].binding = 0;
    vf->attribs[0].format = VK_FORMAT_R32G32_SFLOAT;
    vf->attribs[0].offset = 0;

	vf->vertexBindingDescriptionCount = 1;
	vf->vertexAttributeDescriptionCount = 1;
}

/*
=============
Vk_SetDebugVertexFormat
=============
*/
static void Vk_SetDebugVertexFormat(vkfmt_t *vf)
{
    vf->bindings[0].binding = 0;
    vf->bindings[0].stride = sizeof(vec3_t);
    vf->bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    // x, y, z
    vf->attribs[0].location = 0;
    vf->attribs[0].binding = 0;
    vf->attribs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vf->attribs[0].offset = 0;

	vf->vertexBindingDescriptionCount = 1;
	vf->vertexAttributeDescriptionCount = 1;
}

/*
=============
Vk_CreatePipeline
=============
*/
VkPipeline Vk_CreatePipeline(VkPipelineShaderStageCreateInfo vert, VkPipelineShaderStageCreateInfo frag, vertex_format_t format, 
	VkPrimitiveTopology topology, VkPolygonMode polygonmode, VkCullModeFlags cullmode, 
	qboolean depthwrite, VkCompareOp depthfunc, blend_t blendmode)
{
	VkPipeline pipeline = VK_NULL_HANDLE;
    VkGraphicsPipelineCreateInfo gp;
    VkPipelineShaderStageCreateInfo stages[2];
    VkPipelineVertexInputStateCreateInfo vertex_input;
    VkPipelineInputAssemblyStateCreateInfo assembly;
    VkPipelineViewportStateCreateInfo viewport;
    VkPipelineRasterizationStateCreateInfo rasterization;
    VkPipelineMultisampleStateCreateInfo multisample;
    VkPipelineDepthStencilStateCreateInfo depth_stencil;
    VkPipelineColorBlendAttachmentState blend_attachment;
    VkPipelineColorBlendStateCreateInfo color_blend;
    VkPipelineDynamicStateCreateInfo dynamic;
    VkDynamicState dynamic_states[2];
	vkfmt_t vf;

    assert(vk_context.pipeline_layout);
    assert(vk_context.renderpass);

    stages[0] = vert;
    stages[1] = frag;

	switch (format) 
	{
	case VF_BRUSH:
		Vk_SetBrushVertexFormat(&vf);
		break;
	case VF_ALIAS:
		Vk_SetAliasVertexFormat(&vf);
		break;
	case VF_SPRITE:
		Vk_SetSpriteVertexFormat(&vf);
		break;
	case VF_PARTICLE:
		Vk_SetParticleVertexFormat(&vf);
		break;
	case VF_DRAW:
		Vk_SetDrawVertexFormat(&vf);		
		break;
	case VF_FILL:
		Vk_SetFillVertexFormat(&vf);
		break;
	case VF_DEBUG:
		Vk_SetDebugVertexFormat(&vf);
		break;
	}

    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.pNext = NULL;
    vertex_input.flags = 0;
	vertex_input.vertexBindingDescriptionCount = vf.vertexBindingDescriptionCount;
	vertex_input.pVertexBindingDescriptions = vf.bindings;
	vertex_input.vertexAttributeDescriptionCount = vf.vertexAttributeDescriptionCount;
    vertex_input.pVertexAttributeDescriptions = vf.attribs;

    assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly.pNext = NULL;
    assembly.flags = 0;
    assembly.topology = topology;
    assembly.primitiveRestartEnable = VK_FALSE;

    viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport.pNext = NULL;
    viewport.flags = 0;
    viewport.viewportCount = 1;
    viewport.pViewports = NULL;	// dynamic
    viewport.scissorCount = 1;
    viewport.pScissors = NULL;	// dynamic

    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.pNext = NULL;
    rasterization.flags = 0;
    rasterization.depthClampEnable = VK_FALSE;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.polygonMode = polygonmode;
    rasterization.cullMode = cullmode;
    rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization.depthBiasEnable = VK_FALSE;
    rasterization.depthBiasConstantFactor = 0.f;
    rasterization.depthBiasClamp = 0.f;
    rasterization.depthBiasSlopeFactor = 0.f;
    rasterization.lineWidth = 1.f;

    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.pNext = NULL;
    multisample.flags = 0;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // should follow global config
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.minSampleShading = 0;
    multisample.pSampleMask = NULL;
    multisample.alphaToCoverageEnable = VK_FALSE;
    multisample.alphaToOneEnable = VK_FALSE;

    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.pNext = NULL;
    depth_stencil.flags = 0;
    depth_stencil.depthTestEnable = (VK_COMPARE_OP_ALWAYS == depthfunc) ? VK_FALSE : VK_TRUE;
    depth_stencil.depthWriteEnable = depthwrite ? VK_TRUE : VK_FALSE;
    depth_stencil.depthCompareOp = depthfunc;
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

	switch (blendmode)
	{
	case BLEND_NONE:
		blend_attachment.blendEnable = VK_FALSE;
		blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		break;

	case BLEND_NORMAL:
		blend_attachment.blendEnable = VK_TRUE;
		blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		break;

	case BLEND_ADD:
		blend_attachment.blendEnable = VK_TRUE;
		blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		break;
	}

	blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;

    color_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend.pNext = NULL;
    color_blend.flags = 0;
    color_blend.logicOpEnable = VK_FALSE;
    color_blend.logicOp = VK_LOGIC_OP_CLEAR;
    color_blend.attachmentCount = 1;
    color_blend.pAttachments = &blend_attachment;
    color_blend.blendConstants[0] = 1.f;
    color_blend.blendConstants[1] = 1.f;
    color_blend.blendConstants[2] = 1.f;
    color_blend.blendConstants[3] = 1.f;

    dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic.pNext = NULL;
    dynamic.flags = 0;
    dynamic.dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]);
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
    gp.pColorBlendState = &color_blend;
    gp.pDynamicState = &dynamic;
    gp.layout = vk_context.pipeline_layout;
    gp.renderPass = vk_context.renderpass;
    gp.subpass = 0;
    gp.basePipelineIndex = 0;
    gp.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(vk_context.device, VK_NULL_HANDLE, 1, &gp, NULL, &pipeline) != VK_SUCCESS)
        ri.Sys_Error(ERR_FATAL, "Failed to create graphics pipeline\n");

    return pipeline;
}