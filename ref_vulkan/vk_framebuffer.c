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

qboolean VK_CreateRenderPass()
{
    VkAttachmentDescription attachments[2];
    VkAttachmentReference color_ref, depth_ref;
    VkSubpassDescription subpass_desc;
    VkRenderPassCreateInfo renderpass_info;

    // front and back color attachment
    attachments[0].flags = 0;
    attachments[0].format = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // depth/stencil attachment
    attachments[1].flags = 0;
    attachments[1].format = VK_FORMAT_D32_SFLOAT;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    depth_ref.attachment = 1;
    depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass_desc.flags = 0;
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.inputAttachmentCount = 0;
    subpass_desc.pInputAttachments = NULL;
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &color_ref;
    subpass_desc.pResolveAttachments = NULL;
    subpass_desc.pDepthStencilAttachment = &depth_ref;
    subpass_desc.preserveAttachmentCount = 0;
    subpass_desc.pPreserveAttachments = NULL;

    renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_info.pNext = NULL;
    renderpass_info.flags = 0;
    renderpass_info.attachmentCount = 2;
    renderpass_info.pAttachments = attachments;
    renderpass_info.subpassCount = 1;
    renderpass_info.pSubpasses = &subpass_desc;
    renderpass_info.dependencyCount = 0;
    renderpass_info.pDependencies = NULL;

    if (vkCreateRenderPass(vk_context.device, &renderpass_info, NULL, &vk_context.renderpass) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "Failed to create render pass\n");
        return false;
    }

    return true;
}

void VK_DestroyRenderPass()
{
    if (vk_context.renderpass)
    {
        vkDestroyRenderPass(vk_context.device, vk_context.renderpass, NULL);
        vk_context.renderpass = VK_NULL_HANDLE;
    }
}

void VK_DestroyFramebuffer();

qboolean VK_CreateFramebuffer()
{
    VkImageCreateInfo info;
    VkImageViewCreateInfo view_info;
    VkMemoryRequirements mem_req;
    VkMemoryAllocateInfo mem;
    VkImageView attachments[2];
    VkFramebufferCreateInfo fb_info;

    // depth image
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = NULL;
    info.flags = 0;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = VK_FORMAT_D32_SFLOAT;
    info.extent.width = vk_context.extent.width;
    info.extent.height = vk_context.extent.height;
    info.extent.depth = 1;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;
    info.pQueueFamilyIndices = NULL;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // depth image view
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.flags = 0;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = info.format;
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    mem.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem.pNext = NULL;
    mem.memoryTypeIndex = 0;

    if (vkCreateImage(vk_context.device, &info, NULL, &vk_context.depth_stencil.image) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "Failed to create depth buffer image\n");
        return false;
    }

    vkGetImageMemoryRequirements(vk_context.device, vk_context.depth_stencil.image, &mem_req);
    mem.allocationSize = mem_req.size;
    if (vkAllocateMemory(vk_context.device, &mem, NULL, &vk_context.depth_stencil.memory) != VK_SUCCESS)
    {
        VK_DestroyFramebuffer();
        ri.Con_Printf(PRINT_ALL, "Failed to allocate depth buffer memory\n");
        return false;
    }

    vkBindImageMemory(vk_context.device,
        vk_context.depth_stencil.image,
        vk_context.depth_stencil.memory, 0);

    view_info.image = vk_context.depth_stencil.image;
    if (vkCreateImageView(vk_context.device, &view_info, NULL, &vk_context.depth_stencil.view) != VK_SUCCESS)
    {
        VK_DestroyFramebuffer();
        ri.Con_Printf(PRINT_ALL, "Failed to create depth buffer view\n");
        return false;
    }

    // front and back swapchain image views
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.flags = 0;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_info.components.r = VK_COMPONENT_SWIZZLE_R;
    view_info.components.g = VK_COMPONENT_SWIZZLE_G;
    view_info.components.b = VK_COMPONENT_SWIZZLE_B;
    view_info.components.a = VK_COMPONENT_SWIZZLE_A;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    // create views of swapchain images
    view_info.image = vk_context.swap_images[0];
    vkCreateImageView(vk_context.device, &view_info, NULL, &vk_context.front.view);
    view_info.image = vk_context.swap_images[1];
    vkCreateImageView(vk_context.device, &view_info, NULL, &vk_context.back.view);

    attachments[0] = vk_context.front.view;
    attachments[1] = vk_context.depth_stencil.view;

    // framebuffer
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = NULL;
    fb_info.flags = 0;
    fb_info.renderPass = vk_context.renderpass;
    fb_info.attachmentCount = 2;
    fb_info.pAttachments = attachments;
    fb_info.width = vk_context.extent.width;
    fb_info.height = vk_context.extent.height;
    fb_info.layers = 1;

    if (vkCreateFramebuffer(vk_context.device, &fb_info, NULL, &vk_context.framebuffers[0]) != VK_SUCCESS)
    {
        VK_DestroyFramebuffer();
        ri.Con_Printf(PRINT_ALL, "Failed to create framebuffer\n");
        return false;
    }

    attachments[0] = vk_context.back.view;
    if (vkCreateFramebuffer(vk_context.device, &fb_info, NULL, &vk_context.framebuffers[1]) != VK_SUCCESS)
    {
        VK_DestroyFramebuffer();
        ri.Con_Printf(PRINT_ALL, "Failed to create framebuffer\n");
        return false;
    }

    return true;
}

void VK_DestroyFramebuffer()
{
    if (vk_context.front.view || vk_context.back.view)
    {
        vkDestroyImageView(vk_context.device, vk_context.front.view, NULL);
        vkDestroyImageView(vk_context.device, vk_context.back.view, NULL);
        vk_context.front.view = VK_NULL_HANDLE;
        vk_context.back.view = VK_NULL_HANDLE;
    }

    if (vk_context.depth_stencil.memory)
    {
        vkFreeMemory(vk_context.device, vk_context.depth_stencil.memory, NULL);
        vk_context.depth_stencil.memory = VK_NULL_HANDLE;
    }

    if (vk_context.depth_stencil.view)
    {
        vkDestroyImageView(vk_context.device, vk_context.depth_stencil.view, NULL);
        vk_context.depth_stencil.view = VK_NULL_HANDLE;
    }

    if (vk_context.depth_stencil.image)
    {
        vkDestroyImage(vk_context.device, vk_context.depth_stencil.image, NULL);
        vk_context.depth_stencil.image = VK_NULL_HANDLE;
    }
}
