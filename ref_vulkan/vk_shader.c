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

qboolean Vk_LoadShader(const char *filename, const char *entrypoint, qboolean vertex,
    VkPipelineShaderStageCreateInfo *shader)
{
    VkShaderModule module;
    VkShaderModuleCreateInfo module_info;
    byte *buffer;
    int length;

    length = ri.FS_LoadFile(filename, (void **)&buffer);
    if (!buffer)
    {
        ri.Con_Printf(PRINT_ALL, "Couldn't load file %s\n", filename);
        return false;
    }

    module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_info.pNext = NULL;
    module_info.flags = 0;
    module_info.codeSize = length;
    module_info.pCode = (uint32_t *)buffer;
    
    if (vkCreateShaderModule(vk_context.device, &module_info, NULL, &module) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "Couldn't create shader module %s\n", filename);
        return false;
    }

    shader->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader->pNext = NULL;
    shader->flags = 0;
    shader->stage = vertex ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
    shader->module = module;
    shader->pName = entrypoint;
    shader->pSpecializationInfo = NULL;

    ri.FS_FreeFile(buffer);
    return true;
}

void Vk_DestroyShader(VkPipelineShaderStageCreateInfo *shader)
{
    if (shader->module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(vk_context.device, shader->module, NULL);
        shader->module = VK_NULL_HANDLE;
    }

	memset(shader, 0, sizeof(VkPipelineShaderStageCreateInfo));
}
