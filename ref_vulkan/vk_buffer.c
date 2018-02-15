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

static qboolean Vk_CreateBuffer(VkBufferUsageFlags usage, VkDeviceSize size, vkbuffer_t *buffer)
{
    VkBufferCreateInfo buf_info;
    VkMemoryAllocateInfo mem_alloc;
    VkMemoryRequirements mem_req;

    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.flags = 0;
    buf_info.size = size;
    buf_info.usage = usage;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;

    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = size;
    mem_alloc.memoryTypeIndex = 1; // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT

    if (vkCreateBuffer(vk_context.device, &buf_info, NULL, &buffer->buffer) == VK_SUCCESS)
    {
        vkGetBufferMemoryRequirements(vk_context.device, buffer->buffer, &mem_req);
        if (vkAllocateMemory(vk_context.device, &mem_alloc, NULL, &buffer->memory) == VK_SUCCESS)
        {
            vkBindBufferMemory(vk_context.device, buffer->buffer, buffer->memory, 0);
            buffer->size = size;
            buffer->usage = usage;
            return true;
        }
        else
        {
            vkDestroyBuffer(vk_context.device, buffer->buffer, NULL);
        }
    }

    return false;
}

qboolean Vk_CreateVertexBuffer(VkDeviceSize size, vkbuffer_t *buffer)
{
    return Vk_CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size, buffer);
}

qboolean Vk_CreateIndexBuffer(VkDeviceSize size, vkbuffer_t *buffer)
{
    return Vk_CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size, buffer);
}

void Vk_DestroyBuffer(vkbuffer_t *buffer)
{
    if (buffer->memory)
        vkFreeMemory(vk_context.device, buffer->memory, NULL);
    if (buffer->buffer)
        vkDestroyBuffer(vk_context.device, buffer->buffer, NULL);

    buffer->buffer = VK_NULL_HANDLE;
    buffer->memory = VK_NULL_HANDLE;
    buffer->size = 0;
    buffer->usage = 0;
}
