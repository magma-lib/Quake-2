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
#include <limits.h>

#ifdef _WIN32
#   define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <vulkan/vulkan.h>

#include "../client/ref.h"

#include "dxmath.h"

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

#include "vk_model.h"

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
extern	model_t		*currentmodel;
extern	int			r_visframecount;
extern	int			r_framecount;
extern	cplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys;


extern	int			vk_filter_min, vk_filter_max;


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

extern	int		vk_lightmap_format;
extern	int		vk_alpha_format;
extern	int		vk_tex_alpha_format;

extern	int		c_visible_lightmaps;
extern	int		c_visible_textures;

extern	float	r_world_matrix[16];

extern	unsigned	d_8to24table[256];

extern	int		registration_sequence;

void R_LightPoint(vec3_t p, vec3_t color);
void R_PushDlights(void);

//====================================================================

extern	model_t	*r_worldmodel;

extern	unsigned	d_8to24table[256];

extern	int		registration_sequence;


void V_AddBlend(float r, float g, float b, float a, float *v_blend);

int 	R_Init(void *hinstance, void *hWnd);
void	R_Shutdown(void);

void R_RenderView(refdef_t *fd);
void R_BeginRenderAliasModels();
void R_EndRenderAliasModels();
void R_DrawAliasModel(entity_t *e);
void R_DrawBrushModel(entity_t *e);
void R_DrawSpriteModel(entity_t *e);
void R_DrawBeam(entity_t *e);
void R_DrawWorld(void);
void R_RenderDlights(void);
void R_DrawAlphaSurfaces(void);
void R_InitParticleTexture(void);
void Draw_InitLocal(void);
void Vk_SubdivideSurface(msurface_t *fa);
qboolean R_CullBox(vec3_t mins, vec3_t maxs);
void R_RotateForEntity(entity_t *e);
void R_MarkLeaves(void);

vkpoly_t *WaterWarpPolyVerts(vkpoly_t *p);
void EmitWaterPolys(msurface_t *fa);
void R_AddSkySurface(msurface_t *fa);
void R_ClearSkyBox(void);
void R_DrawSkyBox(void);
void R_MarkLights(dlight_t *light, int bit, mnode_t *node);

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

void VK_ResampleTexture(unsigned *in, int inwidth, int inheight, unsigned *out, int outwidth, int outheight);

struct image_s *R_RegisterSkin(char *name);

void LoadPCX(char *filename, byte **pic, byte **palette, int *width, int *height);
image_t *VK_LoadPic(char *name, byte *pic, int width, int height, imagetype_t type, int bits);
image_t	*VK_FindImage(char *name, imagetype_t type);
void	VK_TextureMode(char *string);
void	VK_ImageList_f(void);

void	VK_SetTexturePalette(unsigned palette[256]);

void	VK_InitImages(void);
void	VK_ShutdownImages(void);

void	VK_FreeUnusedImages(void);

void VK_TextureAlphaMode(char *string);
void VK_TextureSolidMode(char *string);

typedef struct
{
	VkImage			image;
	VkImageView		view;
	VkDeviceMemory	memory;
} vkimage_t;

typedef struct
{
	vkbuffer_t		perframe;
	vkbuffer_t		perobject;
	XMMATRIX		entities[MAX_ENTITIES];
	uint32_t		offset;
} vkmatrices_t;

typedef struct
{
	VkInstance			instance;
	VkPhysicalDevice	phys_device;
	VkDevice			device;
	VkQueue				queue, transfer_queue;

	VkSemaphore			present, render;
	VkFence				fences[2];

	VkCommandPool		cmdpool;
	VkCommandBuffer		cmdbuffer;

	vkimage_t			front;
	vkimage_t			back;
	vkimage_t			depth_stencil;

	VkRenderPass		renderpass;
	VkFramebuffer		framebuffers[2];
	VkFramebuffer		curr_framebuffer;
	VkImage				*swap_images;

	VkDescriptorPool	dpool;
	VkDescriptorSetLayout   dset_layout;
	VkDescriptorSet		dset;

	VkPipelineLayout	pipeline_layout;
	VkPipeline			pipeline_world;
	VkPipeline			pipeline_brush;
	VkPipeline			pipeline_tri_strip;
	VkPipeline			pipeline_tri_fan;

	VkDebugReportCallbackEXT debug_report_callback;
} vkcontext_t;

typedef struct
{
	VkPipelineShaderStageCreateInfo tnl_alias_v;
	VkPipelineShaderStageCreateInfo tnl_alias_f;
	VkPipelineShaderStageCreateInfo tnl_world_v;
	VkPipelineShaderStageCreateInfo tnl_world_f;
	VkPipelineShaderStageCreateInfo tnl_brush_v;
} vkshaders_t;

typedef struct
{
	int		 renderer;
} vkconfig_t;

typedef struct
{
	float inverse_intensity;
	qboolean fullscreen;

	int	 prev_mode;

	unsigned char *d_16to8table;

	int lightmap_textures;

	int	currenttextures[2];
	int currenttmu;

	float camera_separation;
	qboolean stereo_enabled;

	unsigned char originalRedGammaTable[256];
	unsigned char originalGreenGammaTable[256];
	unsigned char originalBlueGammaTable[256];
} vkstate_t;

extern vkcontext_t vk_context;
extern vkshaders_t vk_shaders;
extern vkconfig_t  vk_config;
extern vkstate_t   vk_state;
extern vkmatrices_t vk_transforms;

/*
====================================================================

VULKAN OBJECTS

====================================================================
*/

void VK_InitDebugCallback(void);
void VK_DestroyDebugCallback(void);
qboolean VK_CreateRenderPass(void);
void VK_DestroyRenderPass();
qboolean VK_CreateFramebuffer(uint32_t width, uint32_t height);
void VK_DestroyFramebuffer(void);

qboolean Vk_LoadShader(const char *filename, const char *entrypoint, qboolean vertex, VkPipelineShaderStageCreateInfo *shader);
void Vk_DestroyShader(VkPipelineShaderStageCreateInfo *shader);
qboolean Vk_DSetSetupLayout();
void Vk_DSetDestroyLayout();
VkPipeline Vk_CreateDefaultPipeline(VkPipelineShaderStageCreateInfo vert, VkPipelineShaderStageCreateInfo frag,
    VkPrimitiveTopology topology);
VkPipeline Vk_CreateWorldPipeline(VkPipelineShaderStageCreateInfo vert, VkPipelineShaderStageCreateInfo frag,
    VkPrimitiveTopology topology);
qboolean Vk_CreateVertexBuffer(VkDeviceSize size, vkbuffer_t *buffer);
qboolean Vk_CreateIndexBuffer(VkDeviceSize size, vkbuffer_t *buffer);
qboolean Vk_CreateUniformBuffer(VkDeviceSize size, vkbuffer_t *buffer);
void Vk_DestroyBuffer(vkbuffer_t *buffer);

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
