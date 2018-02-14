/*
Copyright (C) 1997-2001 Id Software, Inc.
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
// GL_RSURF.C: surface-related refresh code
#include <assert.h>

#include "vk_local.h"

static vec3_t	modelorg;		// relative to viewpoint

msurface_t	*r_alpha_surfaces;

#define DYNAMIC_LIGHT_WIDTH  128
#define DYNAMIC_LIGHT_HEIGHT 128

#define LIGHTMAP_BYTES 4

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128

#define	MAX_LIGHTMAPS	128

int		c_visible_lightmaps;
int		c_visible_textures;

#define VK_LIGHTMAP_FORMAT VK_FORMAT_R8G8B8A8_UNORM

typedef struct
{
    int internal_format;
    int	current_lightmap_texture;

    msurface_t	*lightmap_surfaces[MAX_LIGHTMAPS];

    int			allocated[BLOCK_WIDTH];

    // the lightmap texture data needs to be kept in
    // main memory so texsubimage can update properly
    byte		lightmap_buffer[4 * BLOCK_WIDTH*BLOCK_HEIGHT];
} vklightmapstate_t;

static vklightmapstate_t vk_lms;


static void		LM_InitBlock(void);
static void		LM_UploadBlock(qboolean dynamic);
static qboolean	LM_AllocBlock(int w, int h, int *x, int *y);

extern void R_SetCacheState(msurface_t *surf);
extern void R_BuildLightMap(msurface_t *surf, byte *dest, int stride);


/*
=============================================================================

LIGHTMAP ALLOCATION

=============================================================================
*/

static void LM_InitBlock(void)
{
    memset(vk_lms.allocated, 0, sizeof(vk_lms.allocated));
}

static void LM_UploadBlock(qboolean dynamic)
{
    // TODO: upload to Vulkan image
}

// returns a texture number and the position inside it
static qboolean LM_AllocBlock(int w, int h, int *x, int *y)
{
    int		i, j;
    int		best, best2;

    best = BLOCK_HEIGHT;

    for (i = 0; i<BLOCK_WIDTH - w; i++)
    {
        best2 = 0;

        for (j = 0; j<w; j++)
        {
            if (vk_lms.allocated[i + j] >= best)
                break;
            if (vk_lms.allocated[i + j] > best2)
                best2 = vk_lms.allocated[i + j];
        }
        if (j == w)
        {	// this is a valid spot
            *x = i;
            *y = best = best2;
        }
    }

    if (best + h > BLOCK_HEIGHT)
        return false;

    for (i = 0; i<w; i++)
        vk_lms.allocated[*x + i] = best + h;

    return true;
}

/*
================
VK_BuildPolygonFromSurface
================
*/
void VK_BuildPolygonFromSurface(msurface_t *fa)
{
    int			i, lindex, lnumverts;
    medge_t		*pedges, *r_pedge;
    int			vertpage;
    float		*vec;
    float		s, t;
    vkpoly_t	*poly;
    vec3_t		total;

    // reconstruct the polygon
    pedges = currentmodel->edges;
    lnumverts = fa->numedges;
    vertpage = 0;

    VectorClear(total);
    //
    // draw texture
    //
    poly = Hunk_Alloc(sizeof(vkpoly_t) + (lnumverts - 4) * VERTEXSIZE * sizeof(float));
    poly->next = fa->polys;
    poly->flags = fa->flags;
    fa->polys = poly;
    poly->numverts = lnumverts;

    for (i = 0; i<lnumverts; i++)
    {
        lindex = currentmodel->surfedges[fa->firstedge + i];

        if (lindex > 0)
        {
            r_pedge = &pedges[lindex];
            vec = currentmodel->vertexes[r_pedge->v[0]].position;
        }
        else
        {
            r_pedge = &pedges[-lindex];
            vec = currentmodel->vertexes[r_pedge->v[1]].position;
        }
        s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
        s /= fa->texinfo->image->width;

        t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
        t /= fa->texinfo->image->height;

        VectorAdd(total, vec, total);
        VectorCopy(vec, poly->verts[i]);
        poly->verts[i][3] = s;
        poly->verts[i][4] = t;

        //
        // lightmap texture coordinates
        //
        s = DotProduct(vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
        s -= fa->texturemins[0];
        s += fa->light_s * 16;
        s += 8;
        s /= BLOCK_WIDTH * 16; //fa->texinfo->texture->width;

        t = DotProduct(vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
        t -= fa->texturemins[1];
        t += fa->light_t * 16;
        t += 8;
        t /= BLOCK_HEIGHT * 16; //fa->texinfo->texture->height;

        poly->verts[i][5] = s;
        poly->verts[i][6] = t;
    }

    poly->numverts = lnumverts;

}

/*
========================
VK_CreateSurfaceLightmap
========================
*/
void VK_CreateSurfaceLightmap(msurface_t *surf)
{
    int		smax, tmax;
    byte	*base;

    if (surf->flags & (SURF_DRAWSKY | SURF_DRAWTURB))
        return;

    smax = (surf->extents[0] >> 4) + 1;
    tmax = (surf->extents[1] >> 4) + 1;

    if (!LM_AllocBlock(smax, tmax, &surf->light_s, &surf->light_t))
    {
        LM_UploadBlock(false);
        LM_InitBlock();
        if (!LM_AllocBlock(smax, tmax, &surf->light_s, &surf->light_t))
        {
            ri.Sys_Error(ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed\n", smax, tmax);
        }
    }

    surf->lightmaptexturenum = vk_lms.current_lightmap_texture;

    base = vk_lms.lightmap_buffer;
    base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;

    R_SetCacheState(surf);
    R_BuildLightMap(surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
}


/*
==================
VK_BeginBuildingLightmaps

==================
*/
void VK_BeginBuildingLightmaps(model_t *m)
{
    static lightstyle_t	lightstyles[MAX_LIGHTSTYLES];
    int				i;
    unsigned		dummy[128 * 128];

    memset(vk_lms.allocated, 0, sizeof(vk_lms.allocated));

    r_framecount = 1;		// no dlightcache

    /*
    ** setup the base lightstyles so the lightmaps won't have to be regenerated
    ** the first time they're seen
    */
    for (i = 0; i<MAX_LIGHTSTYLES; i++)
    {
        lightstyles[i].rgb[0] = 1;
        lightstyles[i].rgb[1] = 1;
        lightstyles[i].rgb[2] = 1;
        lightstyles[i].white = 3;
    }
    r_newrefdef.lightstyles = lightstyles;

    if (!vk_state.lightmap_textures)
    {
        vk_state.lightmap_textures = TEXNUM_LIGHTMAPS;
        //		gl_state.lightmap_textures	= gl_state.texture_extension_number;
        //		gl_state.texture_extension_number = gl_state.lightmap_textures + MAX_LIGHTMAPS;
    }

    vk_lms.current_lightmap_texture = 1;

    vk_lms.internal_format = vk_tex_alpha_format;

    /*
    ** initialize the dynamic lightmap texture
    */
    // TODO: upload to Vulkan image
}

/*
=======================
VK_EndBuildingLightmaps
=======================
*/
void VK_EndBuildingLightmaps(void)
{
    // TODO:
}