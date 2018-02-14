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

void R_Clear(void);

void VK_InitDebugCallback(void);
void VK_DestroyDebugCallback(void);
qboolean VK_CreateRenderPass(void);
void VK_DestroyRenderPass();
qboolean VK_CreateFramebuffer(void);
void VK_DestroyFramebuffer(void);

viddef_t	vid;

refimport_t	ri;

vkcontext_t vk_context;
vkconfig_t  vk_config;
vkstate_t   vk_state;

image_t		*r_notexture;		// use for bad textures
image_t		*r_particletexture;	// little dot for particles

entity_t	*currententity;

cplane_t	frustum[4];

int			r_visframecount;	// bumped when going to a new PVS
int			r_framecount;		// used for dlight push checking

int			c_brush_polys, c_alias_polys;

float		v_blend[4];			// final blending color

//
// view origin
//
vec3_t	vup;
vec3_t	vpn;
vec3_t	vright;
vec3_t	r_origin;

float	r_world_matrix[16];
float	r_base_world_matrix[16];

//
// screen size info
//
refdef_t	r_newrefdef;

int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

cvar_t	*r_norefresh;
cvar_t	*r_drawentities;
cvar_t	*r_drawworld;
cvar_t	*r_speeds;
cvar_t	*r_fullbright;
cvar_t	*r_novis;
cvar_t	*r_nocull;
cvar_t	*r_lerpmodels;
cvar_t	*r_lefthand;

cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

cvar_t	*vk_nosubimage;

cvar_t	*vk_particle_min_size;
cvar_t	*vk_particle_max_size;
cvar_t	*vk_particle_size;
cvar_t	*vk_particle_att_a;
cvar_t	*vk_particle_att_b;
cvar_t	*vk_particle_att_c;

cvar_t	*vk_log;
cvar_t	*vk_bitdepth;
cvar_t	*vk_drawbuffer;
cvar_t  *vk_driver;
cvar_t	*vk_lightmap;
cvar_t	*vk_shadows;
cvar_t	*vk_mode;
cvar_t	*vk_dynamic;
cvar_t  *vk_monolightmap;
cvar_t	*vk_modulate;
cvar_t	*vk_nobind;
cvar_t	*vk_round_down;
cvar_t	*vk_picmip;
cvar_t	*vk_skymip;
cvar_t	*vk_showtris;
cvar_t	*vk_ztrick;
cvar_t	*vk_finish;
cvar_t	*vk_clear;
cvar_t	*vk_cull;
cvar_t	*vk_polyblend;
cvar_t	*vk_flashblend;
cvar_t	*vk_playermip;
cvar_t  *vk_saturatelighting;
cvar_t	*vk_swapinterval;
cvar_t	*vk_texturemode;
cvar_t	*vk_texturealphamode;
cvar_t	*vk_texturesolidmode;
cvar_t	*vk_lockpvs;

cvar_t	*vid_fullscreen;
cvar_t	*vid_gamma;
cvar_t	*vid_ref;


/*
=============================================================

SPRITE MODELS

=============================================================
*/


/*
=================
R_DrawSpriteModel

=================
*/
void R_DrawSpriteModel(entity_t *e)
{
}

//==================================================================================

/*
=============
R_DrawNullModel
=============
*/
void R_DrawNullModel(void)
{
}

/*
=============
R_DrawEntitiesOnList
=============
*/
void R_DrawEntitiesOnList(void)
{
}

/*
===============
R_DrawParticles
===============
*/
void R_DrawParticles(void)
{
}

/*
============
R_PolyBlend
============
*/
void R_PolyBlend(void)
{
}

/*
============
R_SetFrustum
============
*/
void R_SetFrustum(void)
{
}

//=======================================================================

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame(void)
{
}

/*
=============
R_Clear
=============
*/
void R_Clear(void)
{

}

void R_Flash(void)
{
    R_PolyBlend();
}

/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/
void R_RenderView(refdef_t *fd)
{
}

/*
====================
R_SetLightLevel

====================
*/
void R_SetLightLevel(void)
{
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
void R_RenderFrame(refdef_t *fd)
{
}


/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
void R_Register(void)
{
    r_lefthand = ri.Cvar_Get("hand", "0", CVAR_USERINFO | CVAR_ARCHIVE);
    r_norefresh = ri.Cvar_Get("r_norefresh", "0", 0);
    r_fullbright = ri.Cvar_Get("r_fullbright", "0", 0);
    r_drawentities = ri.Cvar_Get("r_drawentities", "1", 0);
    r_drawworld = ri.Cvar_Get("r_drawworld", "1", 0);
    r_novis = ri.Cvar_Get("r_novis", "0", 0);
    r_nocull = ri.Cvar_Get("r_nocull", "0", 0);
    r_lerpmodels = ri.Cvar_Get("r_lerpmodels", "1", 0);
    r_speeds = ri.Cvar_Get("r_speeds", "0", 0);

    r_lightlevel = ri.Cvar_Get("r_lightlevel", "0", 0);

    vk_nosubimage = ri.Cvar_Get("vk_nosubimage", "0", 0);

    vk_particle_min_size = ri.Cvar_Get("vk_particle_min_size", "2", CVAR_ARCHIVE);
    vk_particle_max_size = ri.Cvar_Get("vk_particle_max_size", "40", CVAR_ARCHIVE);
    vk_particle_size = ri.Cvar_Get("vk_particle_size", "40", CVAR_ARCHIVE);
    vk_particle_att_a = ri.Cvar_Get("vk_particle_att_a", "0.01", CVAR_ARCHIVE);
    vk_particle_att_b = ri.Cvar_Get("vk_particle_att_b", "0.0", CVAR_ARCHIVE);
    vk_particle_att_c = ri.Cvar_Get("vk_particle_att_c", "0.01", CVAR_ARCHIVE);

    vk_modulate = ri.Cvar_Get("vk_modulate", "1", CVAR_ARCHIVE);
    vk_log = ri.Cvar_Get("vk_log", "0", 0);
    vk_bitdepth = ri.Cvar_Get("vk_bitdepth", "0", 0);
    vk_mode = ri.Cvar_Get("vk_mode", "3", CVAR_ARCHIVE);
    vk_lightmap = ri.Cvar_Get("vk_lightmap", "0", 0);
    vk_shadows = ri.Cvar_Get("vk_shadows", "0", CVAR_ARCHIVE);
    vk_dynamic = ri.Cvar_Get("vk_dynamic", "1", 0);
    vk_nobind = ri.Cvar_Get("vk_nobind", "0", 0);
    vk_round_down = ri.Cvar_Get("vk_round_down", "1", 0);
    vk_picmip = ri.Cvar_Get("vk_picmip", "0", 0);
    vk_skymip = ri.Cvar_Get("vk_skymip", "0", 0);
    vk_showtris = ri.Cvar_Get("vk_showtris", "0", 0);
    vk_ztrick = ri.Cvar_Get("vk_ztrick", "0", 0);
    vk_finish = ri.Cvar_Get("vk_finish", "0", CVAR_ARCHIVE);
    vk_clear = ri.Cvar_Get("vk_clear", "0", 0);
    vk_cull = ri.Cvar_Get("vk_cull", "1", 0);
    vk_polyblend = ri.Cvar_Get("vk_polyblend", "1", 0);
    vk_flashblend = ri.Cvar_Get("vk_flashblend", "0", 0);
    vk_playermip = ri.Cvar_Get("vk_playermip", "0", 0);
    vk_monolightmap = ri.Cvar_Get("vk_monolightmap", "0", 0);
    //vk_driver = ri.Cvar_Get("vk_driver", "opengl32", CVAR_ARCHIVE);
    vk_texturemode = ri.Cvar_Get("vk_texturemode", "anisotropic", CVAR_ARCHIVE);
    vk_texturealphamode = ri.Cvar_Get("vk_texturealphamode", "default", CVAR_ARCHIVE);
    vk_texturesolidmode = ri.Cvar_Get("vk_texturesolidmode", "default", CVAR_ARCHIVE);
    vk_lockpvs = ri.Cvar_Get("vk_lockpvs", "0", 0);

    vk_drawbuffer = ri.Cvar_Get("vk_drawbuffer", "BACK", 0);
    vk_swapinterval = ri.Cvar_Get("vk_swapinterval", "1", CVAR_ARCHIVE);

    vk_saturatelighting = ri.Cvar_Get("vk_saturatelighting", "0", 0);


    vid_fullscreen = ri.Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);
    vid_gamma = ri.Cvar_Get("vid_gamma", "1.0", CVAR_ARCHIVE);
    vid_ref = ri.Cvar_Get("vid_ref", "soft", CVAR_ARCHIVE);
}

/*
==================
R_SetMode
==================
*/
qboolean R_SetMode(void)
{
    rserr_t err;
    qboolean fullscreen;

    fullscreen = vid_fullscreen->value;

    vid_fullscreen->modified = false;
    vk_mode->modified = false;

    if ((err = VKimp_SetMode(&vid.width, &vid.height, vk_mode->value, fullscreen)) == rserr_ok)
    {
       vk_state.prev_mode = vk_mode->value;
    }
    else
    {
        if (err == rserr_invalid_fullscreen)
        {
            ri.Cvar_SetValue("vid_fullscreen", 0);
            vid_fullscreen->modified = false;
            ri.Con_Printf(PRINT_ALL, "ref_vulkan::R_SetMode() - fullscreen unavailable in this mode\n");
            if ((err = VKimp_SetMode(&vid.width, &vid.height, vk_mode->value, false)) == rserr_ok)
                return true;
        }
        else if (err == rserr_invalid_mode)
        {
            ri.Cvar_SetValue("gl_mode", vk_state.prev_mode);
            vk_mode->modified = false;
            ri.Con_Printf(PRINT_ALL, "ref_vulkan::R_SetMode() - invalid mode\n");
        }

        // try setting it back to something safe
        if ((err = VKimp_SetMode(&vid.width, &vid.height, vk_state.prev_mode, false)) != rserr_ok)
        {
            ri.Con_Printf(PRINT_ALL, "ref_vulkan::R_SetMode() - could not revert to safe mode\n");
            return false;
        }
    }

    return true;
}

/*
===============
R_InitContextObjects
===============
*/
static void R_InitContextObjects()
{
    VkSemaphoreCreateInfo semaphore_info;
    VkFenceCreateInfo fence_info;

    vkGetDeviceQueue(vk_context.device, 0, 0, &vk_context.queue);
    vkGetDeviceQueue(vk_context.device, 2, 0, &vk_context.transfer_queue);

    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_info.pNext = NULL;
    semaphore_info.flags = 0;

    vkCreateSemaphore(vk_context.device, &semaphore_info, NULL, &vk_context.present);
    vkCreateSemaphore(vk_context.device, &semaphore_info, NULL, &vk_context.render);

    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.pNext = NULL;
    fence_info.flags = 0;

    vkCreateFence(vk_context.device, &fence_info, NULL, &vk_context.fence);
}

/*
===============
R_Init
===============
*/
qboolean R_Init(void *hinstance, void *hWnd)
{
    const char *layers[] = {
#ifdef _DEBUG
        "VK_LAYER_LUNARG_standard_validation"
#endif
    };
    const char *instance_extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef _DEBUG
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
    };
    const char *dev_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME
    };

    VkApplicationInfo app_info;
    VkInstanceCreateInfo instance_info;
    VkDeviceCreateInfo dev_info;
    uint32_t phys_device_count = 0;
    VkPhysicalDevice *phys_devices;
    VkPhysicalDeviceFeatures features;
    uint32_t family_count = 0; 
    VkQueueFamilyProperties *family_properties;
    VkDeviceQueueCreateInfo queues[2];
    float queue_priority = 1.f;

    ri.Con_Printf(PRINT_ALL, "ref_vulkan version: "REF_VERSION"\n");

    R_Register();

    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "vkQuake 2";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "idTech2";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION_1_0;
    
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext = NULL;
    instance_info.flags = 0;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledLayerCount = sizeof(layers) / sizeof(layers[0]);
    instance_info.ppEnabledLayerNames = layers;
    instance_info.enabledExtensionCount = sizeof(instance_extensions) / sizeof(instance_extensions[0]);
    instance_info.ppEnabledExtensionNames = instance_extensions;

    if (vkCreateInstance(&instance_info, NULL, &vk_context.instance) != VK_SUCCESS)
    {
        ri.Con_Printf(PRINT_ALL, "R_Init() - failed to create Vulkan instance\n");
        return false;
    }

    VK_InitDebugCallback();

    if (vkEnumeratePhysicalDevices(vk_context.instance, &phys_device_count, NULL) != VK_SUCCESS)
    {
        vkDestroyInstance(vk_context.instance, NULL);
        ri.Con_Printf(PRINT_ALL, "R_Init() - failed to count physical GPU devices\n");
        return false;
    }

    phys_devices = calloc(phys_device_count, sizeof(VkPhysicalDevice));
    if (vkEnumeratePhysicalDevices(vk_context.instance, &phys_device_count, phys_devices) != VK_SUCCESS)
    {
        free(phys_devices);
        vkDestroyInstance(vk_context.instance, NULL);
        ri.Con_Printf(PRINT_ALL, "R_Init() - failed to enumerate physical GPU devices\n");
        return false;
    }

    vk_context.phys_device = phys_devices[0]; // TODO: Cvar for this
    free(phys_devices);

    // Enable widely used GPU features
    memset(&features, 0, sizeof(VkPhysicalDeviceFeatures));
    features.fillModeNonSolid = VK_TRUE;
    features.samplerAnisotropy = VK_TRUE;
    features.textureCompressionBC = VK_TRUE;

    vkGetPhysicalDeviceQueueFamilyProperties(vk_context.phys_device, &family_count, NULL);
    family_properties = calloc(family_count, sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(vk_context.phys_device, &family_count, family_properties);

    queues[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queues[0].pNext = NULL;
    queues[0].flags = 0;
    queues[0].queueFamilyIndex = 0; // graphics
    queues[0].queueCount = 1;
    queues[0].pQueuePriorities = &queue_priority;

    queues[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queues[1].pNext = NULL;
    queues[1].flags = 0;
    queues[1].queueFamilyIndex = 2; // transfer
    queues[1].queueCount = 1;
    queues[1].pQueuePriorities = &queue_priority;

    free(family_properties);

    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.pNext = NULL;
    dev_info.flags = 0;
    dev_info.queueCreateInfoCount = 2;
    dev_info.pQueueCreateInfos = queues;
    dev_info.enabledLayerCount = 0;
    dev_info.ppEnabledLayerNames = NULL;
    dev_info.enabledExtensionCount = sizeof(dev_extensions) / sizeof(dev_extensions[0]);
    dev_info.ppEnabledExtensionNames = dev_extensions;
    dev_info.pEnabledFeatures = &features;

    if (vkCreateDevice(vk_context.phys_device, &dev_info, NULL, &vk_context.device) != VK_SUCCESS)
    {
        vkDestroyInstance(vk_context.instance, NULL);
        ri.Con_Printf(PRINT_ALL, "R_Init() - failed to create logical device\n");
        return false;
    }

    // initialize common Vulkan objects
    R_InitContextObjects();
    
    // initialize OS-specific parts of Vulkan
    if (!VKimp_Init(hinstance, hWnd))
        return false;

    // create the window and set up the context
    if (!R_SetMode())
    {
        ri.Con_Printf(PRINT_ALL, "ref_vulkan::R_Init() - could not R_SetMode()\n");
        return false;
    }

    VK_CreateRenderPass();
    VK_CreateFramebuffer();

    ri.Vid_MenuInit();

    Draw_InitLocal();

    return true;
}

/*
===============
R_Shutdown
===============
*/
void R_Shutdown(void)
{
    VK_DestroyFramebuffer();
    VK_DestroyRenderPass();

    if (vk_context.present != VK_NULL_HANDLE || 
        vk_context.render != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(vk_context.device, vk_context.present, NULL);
        vkDestroySemaphore(vk_context.device, vk_context.render, NULL);
        vk_context.present = VK_NULL_HANDLE;
        vk_context.render = VK_NULL_HANDLE;
    }

    if (vk_context.device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(vk_context.device, NULL);
        vk_context.device = VK_NULL_HANDLE;
    }

    VK_DestroyDebugCallback();

    if (vk_context.instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(vk_context.instance, NULL);
        vk_context.instance = VK_NULL_HANDLE;
    }
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_BeginFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void R_BeginFrame(float camera_separation)
{
    vk_state.camera_separation = camera_separation;

    /*
    ** change modes if necessary
    */
    if (vk_mode->modified || vid_fullscreen->modified)
    {	// FIXME: only restart if CDS is required
        cvar_t	*ref;

        ref = ri.Cvar_Get("vid_ref", "gl", 0);
        ref->modified = true;
    }

    if (vk_log->modified)
    {
        //VKimp_EnableLogging(vk_log->value);
        vk_log->modified = false;
    }

    if (vk_log->value)
    {
        //VKimp_LogNewFrame();
    }

    VKimp_BeginFrame(camera_separation);
}

/*
=============
R_SetPalette
=============
*/
unsigned r_rawpalette[256];

void R_SetPalette(const unsigned char *palette)
{
}

/*
** R_DrawBeam
*/
void R_DrawBeam(entity_t *e)
{
}

//===================================================================


void	R_BeginRegistration(char *map);
struct model_s	*R_RegisterModel(char *name);
struct image_s	*R_RegisterSkin(char *name);
void R_SetSky(char *name, float rotate, vec3_t axis);
void	R_EndRegistration(void);

void	R_RenderFrame(refdef_t *fd);

struct image_s	*Draw_FindPic(char *name);

void	Draw_Pic(int x, int y, char *name);
void	Draw_Char(int x, int y, int c);
void	Draw_TileClear(int x, int y, int w, int h, char *name);
void	Draw_Fill(int x, int y, int w, int h, int c);
void	Draw_FadeScreen(void);

/*
@@@@@@@@@@@@@@@@@@@@@
GetRefAPI

@@@@@@@@@@@@@@@@@@@@@
*/
refexport_t GetRefAPI(refimport_t rimp)
{
    refexport_t	re;

    ri = rimp;

    re.api_version = API_VERSION;

    re.BeginRegistration = R_BeginRegistration;
    re.RegisterModel = R_RegisterModel;
    re.RegisterSkin = R_RegisterSkin;
    re.RegisterPic = Draw_FindPic;
    re.SetSky = R_SetSky;
    re.EndRegistration = R_EndRegistration;

    re.RenderFrame = R_RenderFrame;

    re.DrawGetPicSize = Draw_GetPicSize;
    re.DrawPic = Draw_Pic;
    re.DrawStretchPic = Draw_StretchPic;
    re.DrawChar = Draw_Char;
    re.DrawTileClear = Draw_TileClear;
    re.DrawFill = Draw_Fill;
    re.DrawFadeScreen = Draw_FadeScreen;

    re.DrawStretchRaw = Draw_StretchRaw;

    re.Init = R_Init;
    re.Shutdown = R_Shutdown;

    re.CinematicSetPalette = R_SetPalette;
    re.BeginFrame = R_BeginFrame;
    re.EndFrame = VKimp_EndFrame;

    re.AppActivate = VKimp_AppActivate;

    Swap_Init();

    return re;
}


#ifndef REF_HARD_LINKED
// this is only here so the functions in q_shared.c and q_shwin.c can link
void Sys_Error(char *error, ...)
{
    va_list		argptr;
    char		text[1024];

    va_start(argptr, error);
    vsprintf(text, error, argptr);
    va_end(argptr);

    ri.Sys_Error(ERR_FATAL, "%s", text);
}

void Com_Printf(char *fmt, ...)
{
    va_list		argptr;
    char		text[1024];

    va_start(argptr, fmt);
    vsprintf(text, fmt, argptr);
    va_end(argptr);

    ri.Con_Printf(PRINT_ALL, "%s", text);
}

#endif
