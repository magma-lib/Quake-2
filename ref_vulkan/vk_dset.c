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

void Vk_DSetUpdateUniformBuffer(const vkbuffer_t *buffer, uint32_t binding, VkDescriptorSetLayoutBinding layout_binding)
{
    VkDescriptorBufferInfo uniform_buffer_info;
    VkWriteDescriptorSet write_dset;

    uniform_buffer_info.buffer = buffer->buffer;
    uniform_buffer_info.offset = 0;
    uniform_buffer_info.range = VK_WHOLE_SIZE;

    write_dset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_dset.pNext = NULL;
    write_dset.dstSet = vk_context.dset;
    write_dset.dstBinding = binding;
    write_dset.dstArrayElement = 0;
    write_dset.descriptorCount = layout_binding.descriptorCount;
    write_dset.descriptorType = layout_binding.descriptorType;
    write_dset.pImageInfo = NULL;
    write_dset.pBufferInfo = &uniform_buffer_info;
    write_dset.pTexelBufferView = NULL;

    vkUpdateDescriptorSets(vk_context.device, 1, &write_dset, 0, NULL);
}

qboolean Vk_DSetSetupLayout()
{
    VkDescriptorPoolSize pool_sizes[1];
    VkDescriptorSetLayoutBinding bindings[1];
    VkDescriptorPoolCreateInfo pool_info;
    VkDescriptorSetLayoutCreateInfo layout_info;
    VkDescriptorSetAllocateInfo alloc_info;
    
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = 1;

    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.pNext = NULL;
    pool_info.flags = 0;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
    pool_info.pPoolSizes = pool_sizes;
    
    if (vkCreateDescriptorPool(vk_context.device, &pool_info, NULL, &vk_context.dpool) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "Couldn't create descriptor pool\n");
        return false;
    }

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bindings[0].pImmutableSamplers = NULL;

    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.pNext = NULL;
    layout_info.flags = 0;
    layout_info.bindingCount = sizeof(bindings) / sizeof(bindings[0]);
    layout_info.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(vk_context.device, &layout_info, NULL, &vk_context.dset_layout) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "Couldn't create descriptor set layout\n");
        return false;
    }

    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.descriptorPool = vk_context.dpool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &vk_context.dset_layout;

    if (vkAllocateDescriptorSets(vk_context.device, &alloc_info, &vk_context.dset) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "Couldn't allocate descriptor set\n");
        return false;
    }

    if (!Vk_CreateUniformBuffer(sizeof(float) * 4 * 4, &vk_context.per_object))
    {
        ri.Con_Printf(PRINT_ALL, "Couldn't create per-object uniform buffer\n");
        return false;
    }

    Vk_DSetUpdateUniformBuffer(&vk_context.per_object, 0, bindings[0]);
    return true;
}
