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
#include <assert.h>
#include "../ref_vulkan/vk_local.h"
#include "vkw_win.h"
#include "winquake.h"

qboolean VKimp_InitSwapchain(void);

vkwstate_t vkw_state;

extern cvar_t *vid_fullscreen;
extern cvar_t *vid_ref;

/*
** VID_CreateWindow
*/
#define	WINDOW_CLASS_NAME	"vkQuake 2"

qboolean VID_CreateWindow(int width, int height, qboolean fullscreen)
{
    WNDCLASS		wc;
    RECT			r;
    cvar_t			*vid_xpos, *vid_ypos;
    int				stylebits;
    int				x, y, w, h;
    int				exstyle;

    /* Register the frame class */
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)vkw_state.wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = vkw_state.hInstance;
    wc.hIcon = 0;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (void *)COLOR_GRAYTEXT;
    wc.lpszMenuName = 0;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass(&wc))
        ri.Sys_Error(ERR_FATAL, "Couldn't register window class");

    if (fullscreen)
    {
        exstyle = WS_EX_TOPMOST;
        stylebits = WS_POPUP | WS_VISIBLE;
    }
    else
    {
        exstyle = 0;
        stylebits = WINDOW_STYLE;
    }

    r.left = 0;
    r.top = 0;
    r.right = width;
    r.bottom = height;

    AdjustWindowRect(&r, stylebits, FALSE);

    w = r.right - r.left;
    h = r.bottom - r.top;

    if (fullscreen)
    {
        x = 0;
        y = 0;
    }
    else
    {
        vid_xpos = ri.Cvar_Get("vid_xpos", "0", 0);
        vid_ypos = ri.Cvar_Get("vid_ypos", "0", 0);
        x = vid_xpos->value;
        y = vid_ypos->value;
    }

    vkw_state.hWnd = CreateWindowEx(
        exstyle,
        WINDOW_CLASS_NAME,
        "Quake 2",
        stylebits,
        x, y, w, h,
        NULL,
        NULL,
        vkw_state.hInstance,
        NULL);

    if (!vkw_state.hWnd)
        ri.Sys_Error(ERR_FATAL, "Couldn't create window");

    ShowWindow(vkw_state.hWnd, SW_SHOW);
    UpdateWindow(vkw_state.hWnd);

    // init all the Vulkan stuff for the window
    if (!VKimp_InitSwapchain())
    {
        ri.Con_Printf(PRINT_ALL, "VID_CreateWindow() - VKimp_InitVK failed\n");
        return false;
    }

    SetForegroundWindow(vkw_state.hWnd);
    SetFocus(vkw_state.hWnd);

    // let the sound and input subsystems know about the new window
    ri.Vid_NewWindow(width, height);

    return true;
}

/*
** VKimp_SetMode
*/
rserr_t VKimp_SetMode(int *pwidth, int *pheight, int mode, qboolean fullscreen)
{
    int width, height;
    const char *win_fs[] = { "W", "FS" };

    ri.Con_Printf(PRINT_ALL, "Initializing Vulkan display\n");

    ri.Con_Printf(PRINT_ALL, "...setting mode %d:", mode);

    if (!ri.Vid_GetModeInfo(&width, &height, mode))
    {
        ri.Con_Printf(PRINT_ALL, " invalid mode\n");
        return rserr_invalid_mode;
    }

    ri.Con_Printf(PRINT_ALL, " %d %d %s\n", width, height, win_fs[fullscreen]);

    // destroy the existing window
    if (vkw_state.hWnd)
    {
        VKimp_Shutdown();
    }

    // do a CDS if needed
    if (fullscreen)
    {
        DEVMODE dm;

        ri.Con_Printf(PRINT_ALL, "...attempting fullscreen\n");

        memset(&dm, 0, sizeof(dm));

        dm.dmSize = sizeof(dm);

        dm.dmPelsWidth = width;
        dm.dmPelsHeight = height;
        dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

        if (vk_bitdepth->value != 0)
        {
            dm.dmBitsPerPel = vk_bitdepth->value;
            dm.dmFields |= DM_BITSPERPEL;
            ri.Con_Printf(PRINT_ALL, "...using gl_bitdepth of %d\n", (int)vk_bitdepth->value);
        }
        else
        {
            HDC hdc = GetDC(NULL);
            int bitspixel = GetDeviceCaps(hdc, BITSPIXEL);

            ri.Con_Printf(PRINT_ALL, "...using desktop display depth of %d\n", bitspixel);

            ReleaseDC(0, hdc);
        }

        ri.Con_Printf(PRINT_ALL, "...calling CDS: ");
        if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
        {
            *pwidth = width;
            *pheight = height;

            vk_state.fullscreen = true;

            ri.Con_Printf(PRINT_ALL, "ok\n");

            if (!VID_CreateWindow(width, height, true))
                return rserr_invalid_mode;

            return rserr_ok;
        }
        else
        {
            *pwidth = width;
            *pheight = height;

            ri.Con_Printf(PRINT_ALL, "failed\n");

            ri.Con_Printf(PRINT_ALL, "...calling CDS assuming dual monitors:");

            dm.dmPelsWidth = width * 2;
            dm.dmPelsHeight = height;
            dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

            if (vk_bitdepth->value != 0)
            {
                dm.dmBitsPerPel = vk_bitdepth->value;
                dm.dmFields |= DM_BITSPERPEL;
            }

            /*
            ** our first CDS failed, so maybe we're running on some weird dual monitor
            ** system
            */
            if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            {
                ri.Con_Printf(PRINT_ALL, " failed\n");

                ri.Con_Printf(PRINT_ALL, "...setting windowed mode\n");

                ChangeDisplaySettings(0, 0);

                *pwidth = width;
                *pheight = height;
                vk_state.fullscreen = false;
                if (!VID_CreateWindow(width, height, false))
                    return rserr_invalid_mode;
                return rserr_invalid_fullscreen;
            }
            else
            {
                ri.Con_Printf(PRINT_ALL, " ok\n");
                if (!VID_CreateWindow(width, height, true))
                    return rserr_invalid_mode;

                vk_state.fullscreen = true;
                return rserr_ok;
            }
        }
    }
    else
    {
        ri.Con_Printf(PRINT_ALL, "...setting windowed mode\n");

        ChangeDisplaySettings(0, 0);

        *pwidth = width;
        *pheight = height;
        vk_state.fullscreen = false;
        if (!VID_CreateWindow(width, height, false))
            return rserr_invalid_mode;
    }

    return rserr_ok;
}

/*
** VKimp_Shutdown
**
** Destroys swapchain, VK_KHR_win32_surface and window. 
** The state structure is also nulled out.
**
*/
void VKimp_Shutdown(void)
{
    if (vkw_state.swapchain)
    {
        free(vkw_state.swap_images);
        vkDestroySwapchainKHR(vk_context.device, vkw_state.swapchain, NULL);
        vkw_state.swapchain = VK_NULL_HANDLE;
    }

    if (vkw_state.surface)
    {
        vkDestroySurfaceKHR(vk_context.instance, vkw_state.surface, NULL);
        vkw_state.surface = VK_NULL_HANDLE;
    }

    if (vkw_state.hWnd)
    {
        DestroyWindow(vkw_state.hWnd);
        vkw_state.hWnd = NULL;
    }

    if (vkw_state.log_fp)
    {
        fclose(vkw_state.log_fp);
        vkw_state.log_fp = 0;
    }

    UnregisterClass(WINDOW_CLASS_NAME, vkw_state.hInstance);

    if (vk_state.fullscreen)
    {
        ChangeDisplaySettings(0, 0);
        vk_state.fullscreen = false;
    }
}

/*
** VKimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of Vulkan. Under Win32 this means usage of VK_KHR_win32_surface extension.
*/
qboolean VKimp_Init(void *hinstance, void *wndproc)
{
    vkw_state.hInstance = (HINSTANCE)hinstance;
    vkw_state.wndproc = wndproc;

    return true;
}

qboolean VKimp_InitSwapchain(void)
{
    VkWin32SurfaceCreateInfoKHR surface;
    VkSurfaceCapabilitiesKHR caps;
    VkSwapchainCreateInfoKHR swapchain;
    VkBool32 supported = VK_FALSE;
    uint32_t image_count = 0;

    surface.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface.pNext = NULL;
    surface.flags = 0;
    surface.hinstance = vkw_state.hInstance;
    surface.hwnd = vkw_state.hWnd;

    if (vkCreateWin32SurfaceKHR(vk_context.instance, &surface, NULL, &vkw_state.surface) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "VKimp_Init() - failed to create Win32 surface\n");
        return false;
    }

    vkGetPhysicalDeviceSurfaceSupportKHR(vk_context.phys_device, 0, vkw_state.surface, &supported);
    if (!supported)
    {
        vkDestroySurfaceKHR(vk_context.instance, vkw_state.surface, NULL);
        vkw_state.surface = VK_NULL_HANDLE;
        ri.Con_Printf(PRINT_ALL, "VKimp_Init() - Win32 surface not supported\n");
        return false;
    }

    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_context.phys_device, vkw_state.surface, &caps) != VK_SUCCESS)
    {
        vkDestroySurfaceKHR(vk_context.instance, vkw_state.surface, NULL);
        vkw_state.surface = VK_NULL_HANDLE;
        ri.Con_Printf(PRINT_ALL, "VKimp_Init() - couldn't retrieve Win32 surface capabilities\n");
        return false;
    }

    swapchain.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain.pNext = NULL;
    swapchain.flags = 0;
    swapchain.surface = vkw_state.surface;
    swapchain.minImageCount = 2;
    swapchain.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    swapchain.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchain.imageExtent = caps.currentExtent;
    swapchain.imageArrayLayers = 1;
    swapchain.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain.queueFamilyIndexCount = 0;
    swapchain.pQueueFamilyIndices = NULL;
    swapchain.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchain.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    swapchain.clipped = VK_TRUE;
    swapchain.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(vk_context.device, &swapchain, NULL, &vkw_state.swapchain) != VK_SUCCESS)
    {
        vkDestroySurfaceKHR(vk_context.instance, vkw_state.surface, NULL);
        vkw_state.surface = VK_NULL_HANDLE;
        ri.Con_Printf(PRINT_ALL, "VKimp_Init() - couldn't initialize window swap chain\n");
        return false;
    }

    vkGetSwapchainImagesKHR(vk_context.device, vkw_state.swapchain, &image_count, NULL);
    vkw_state.swap_images = malloc(sizeof(VkImage) * image_count);
    if (vkGetSwapchainImagesKHR(vk_context.device, vkw_state.swapchain, &image_count, vkw_state.swap_images) != VK_SUCCESS)
    {
        vkDestroySwapchainKHR(vk_context.device, vkw_state.swapchain, NULL);
        vkDestroySurfaceKHR(vk_context.instance, vkw_state.surface, NULL);
        vkw_state.swapchain = VK_NULL_HANDLE;
        vkw_state.surface = VK_NULL_HANDLE;
        ri.Con_Printf(PRINT_ALL, "VKimp_Init() - couldn't retrieve swap chain images\n");
        return false;
    }

    vk_context.swap_images = vkw_state.swap_images;

    return true;
}

/*
** VKimp_BeginFrame
*/
void VKimp_BeginFrame(float camera_separation)
{
    VkCommandBufferBeginInfo begin_info;

    if (vkAcquireNextImageKHR(vk_context.device, vkw_state.swapchain, UINT64_MAX,
        vk_context.present,
        VK_NULL_HANDLE,
        &vkw_state.image_index) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "VKimp_BeginFrame() - couldn't acquire next image\n");
    }

    vkWaitForFences(vk_context.device, 1, &vk_context.fences[vkw_state.image_index], VK_TRUE, UINT64_MAX);
    vkResetFences(vk_context.device, 1, &vk_context.fences[vkw_state.image_index]);

    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.pNext = NULL;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = NULL;

    vkResetCommandBuffer(vk_context.cmdbuffer, 0);
    vkBeginCommandBuffer(vk_context.cmdbuffer, &begin_info);

    // set current framebuffer
    vk_context.curr_framebuffer = vk_context.framebuffers[vkw_state.image_index];
}

/*
** VKimp_EndFrame
**
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a VKimp
** function and instead do a call to VKimp_SwapBuffers.
*/
void VKimp_EndFrame(void)
{
    const VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info;
    VkPresentInfoKHR present_info;
    VkResult res;

    vkCmdEndRenderPass(vk_context.cmdbuffer);
    res = vkEndCommandBuffer(vk_context.cmdbuffer);
    if (res != VK_SUCCESS)
        ri.Sys_Error(ERR_FATAL, "VKimp_EndFrame() - couldn't end command buffer\n");

    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = NULL;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &vk_context.present; // Wait for present to complete
    submit_info.pWaitDstStageMask = &wait_stage_mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk_context.cmdbuffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &vk_context.render; // Signal on render complete
    
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = NULL;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &vk_context.render; // Wait for render to complete
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vkw_state.swapchain;
    present_info.pImageIndices = &vkw_state.image_index;
    present_info.pResults = NULL;

    res = vkQueueSubmit(vk_context.queue, 1, &submit_info, vk_context.fences[vkw_state.image_index]);
    if (res != VK_SUCCESS)
        ri.Sys_Error(ERR_FATAL, "VKimp_EndFrame() - failed to submit command buffer\n");
    res = vkQueuePresentKHR(vk_context.queue, &present_info);
    if (res != VK_SUCCESS)
        ri.Sys_Error(ERR_FATAL, "VKimp_EndFrame() - swap chain present failed\n");

    vkDeviceWaitIdle(vk_context.device);
}

/*
** VKimp_AppActivate
*/
void VKimp_AppActivate(qboolean active)
{
    if (active)
    {
        SetForegroundWindow(vkw_state.hWnd);
        ShowWindow(vkw_state.hWnd, SW_RESTORE);
    }
    else
    {
        if (vid_fullscreen->value)
            ShowWindow(vkw_state.hWnd, SW_MINIMIZE);
    }
}
