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
#ifdef _WIN32
#    define NOMINMAX
#    include <windows.h>
#endif

#include <stdio.h>
#include <math.h>

#ifdef _WIN32
#   define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

#include "../client/ref.h"

#define	REF_VERSION	"VK 0.01"

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2


typedef struct
{
    unsigned		width, height;			// coordinates from main game
} viddef_t;

extern	viddef_t	vid;


/*

skins will be outline flood filled and mip mapped
pics and sprites with alpha will be outline flood filled
pic won't be mip mapped

model skin
sprite frame
wall texture
pic

*/

typedef enum
{
    it_skin,
    it_sprite,
    it_wall,
    it_pic,
    it_sky
} imagetype_t;

typedef struct image_s
{
    char	name[MAX_QPATH];			// game path, including extension
    imagetype_t	type;
    int		width, height;				// source image
    int		upload_width, upload_height;	// after power of two and picmip
    int		registration_sequence;		// 0 = free
    struct msurface_s	*texturechain;	// for sort-by-texture world drawing
    int		texnum;						// gl texture binding
    float	sl, tl, sh, th;				// 0,0 - 1,1 unless part of the scrap
    qboolean	scrap;
    qboolean	has_alpha;

    qboolean paletted;
} image_t;

#define	TEXNUM_LIGHTMAPS	1024
#define	TEXNUM_SCRAPS		1152
#define	TEXNUM_IMAGES		1153

#define		MAX_VKTEXTURES	1024

//===================================================================

typedef enum
{
    rserr_ok,

    rserr_invalid_fullscreen,
    rserr_invalid_mode,

    rserr_unknown
} rserr_t;

void VK_BeginRendering(int *x, int *y, int *width, int *height);
void VK_EndRendering(void);

void VK_SetDefaultState(void);
void VK_UpdateSwapInterval(void);

typedef struct
{
    float	x, y, z;
    float	s, t;
    float	r, g, b;
} vkvert_t;


#define	MAX_LBM_HEIGHT		480

#define BACKFACE_EPSILON	0.01


//====================================================

extern	image_t		vktextures[MAX_VKTEXTURES];
extern	int			numvktextures;


extern	image_t		*r_notexture;
extern	image_t		*r_particletexture;
extern	entity_t	*currententity;
extern	int			r_visframecount;
extern	int			r_framecount;
extern	cplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys;


//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_newrefdef;
extern	int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern	cvar_t	*r_norefresh;
extern	cvar_t	*r_lefthand;
extern	cvar_t	*r_drawentities;
extern	cvar_t	*r_drawworld;
extern	cvar_t	*r_speeds;
extern	cvar_t	*r_fullbright;
extern	cvar_t	*r_novis;
extern	cvar_t	*r_nocull;
extern	cvar_t	*r_lerpmodels;

extern	cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

extern cvar_t	*vk_particle_min_size;
extern cvar_t	*vk_particle_max_size;
extern cvar_t	*vk_particle_size;
extern cvar_t	*vk_particle_att_a;
extern cvar_t	*vk_particle_att_b;
extern cvar_t	*vk_particle_att_c;

extern	cvar_t	*vk_nosubimage;
extern	cvar_t	*vk_bitdepth;
extern	cvar_t	*vk_mode;
extern	cvar_t	*vk_log;
extern	cvar_t	*vk_lightmap;
extern	cvar_t	*vk_shadows;
extern	cvar_t	*vk_dynamic;
extern  cvar_t  *vk_monolightmap;
extern	cvar_t	*vk_nobind;
extern	cvar_t	*vk_round_down;
extern	cvar_t	*vk_picmip;
extern	cvar_t	*vk_skymip;
extern	cvar_t	*vk_showtris;
extern	cvar_t	*vk_finish;
extern	cvar_t	*vk_ztrick;
extern	cvar_t	*vk_clear;
extern	cvar_t	*vk_cull;
extern	cvar_t	*vk_poly;
extern	cvar_t	*vk_texsort;
extern	cvar_t	*vk_polyblend;
extern	cvar_t	*vk_flashblend;
extern	cvar_t	*vk_lightmaptype;
extern	cvar_t	*vk_modulate;
extern	cvar_t	*vk_playermip;
extern	cvar_t	*vk_drawbuffer;
extern  cvar_t  *vk_driver;
extern	cvar_t	*vk_swapinterval;
extern	cvar_t	*vk_texturemode;
extern	cvar_t	*vk_texturealphamode;
extern	cvar_t	*vk_texturesolidmode;
extern  cvar_t  *vk_saturatelighting;
extern  cvar_t  *vk_lockpvs;

extern	cvar_t	*vid_fullscreen;
extern	cvar_t	*vid_gamma;

extern	cvar_t		*intensity;

extern	int		c_visible_lightmaps;
extern	int		c_visible_textures;

extern	float	r_world_matrix[16];

extern	unsigned	d_8to24table[256];

extern	int		registration_sequence;

//====================================================================

extern	unsigned	d_8to24table[256];

extern	int		registration_sequence;


void V_AddBlend(float r, float g, float b, float a, float *v_blend);

int 	R_Init(void *hinstance, void *hWnd);
void	R_Shutdown(void);

void R_RenderView(refdef_t *fd);
void R_DrawAliasModel(entity_t *e);
void R_DrawBrushModel(entity_t *e);
void R_DrawSpriteModel(entity_t *e);
void R_DrawBeam(entity_t *e);
void R_DrawWorld(void);
void R_RenderDlights(void);
void R_DrawAlphaSurfaces(void);
void R_InitParticleTexture(void);
void Draw_InitLocal(void);
qboolean R_CullBox(vec3_t mins, vec3_t maxs);
void R_RotateForEntity(entity_t *e);
void R_MarkLeaves(void);

void R_ClearSkyBox(void);
void R_DrawSkyBox(void);

void	Draw_GetPicSize(int *w, int *h, char *name);
void	Draw_Pic(int x, int y, char *name);
void	Draw_StretchPic(int x, int y, int w, int h, char *name);
void	Draw_Char(int x, int y, int c);
void	Draw_TileClear(int x, int y, int w, int h, char *name);
void	Draw_Fill(int x, int y, int w, int h, int c);
void	Draw_FadeScreen(void);
void	Draw_StretchRaw(int x, int y, int w, int h, int cols, int rows, byte *data);

void	R_BeginFrame(float camera_separation);
void	R_SwapBuffers(int);
void	R_SetPalette(const unsigned char *palette);

struct image_s *R_RegisterSkin(char *name);

typedef struct
{
    VkImage         image;
    VkImageView     view;
    VkDeviceMemory  memory;
} vkimage_t;

typedef struct
{
    VkInstance          instance;
    VkPhysicalDevice    phys_device;
    VkDevice            device;
    VkQueue             queue, transfer_queue;

    VkSemaphore         present, render;
    VkFence             fences[2];

    VkExtent2D          extent;
    vkimage_t           front;
    vkimage_t           back;
    vkimage_t           depth_stencil;
    VkRenderPass        renderpass;
    VkFramebuffer       framebuffers[2];
    VkImage             *swap_images;

    VkDebugReportCallbackEXT debug_report_callback;
} vkcontext_t;

typedef struct
{
    int         renderer;
} vkconfig_t;

typedef struct
{
    qboolean fullscreen;

    float camera_separation;
    qboolean stereo_enabled;

    int     prev_mode;
} vkstate_t;

extern vkcontext_t vk_context;
extern vkconfig_t  vk_config;
extern vkstate_t   vk_state;

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/

extern	refimport_t	ri;


/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void		VKimp_BeginFrame(float camera_separation);
void		VKimp_EndFrame(void);
int 		VKimp_Init(void *hinstance, void *hWnd);
void		VKimp_Shutdown(void);
int     	VKimp_SetMode(int *pwidth, int *pheight, int mode, qboolean fullscreen);
void		VKimp_AppActivate(qboolean active);
void		VKimp_EnableLogging(qboolean enable);
void		VKimp_LogNewFrame(void);
