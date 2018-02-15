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
=============================================================

BRUSH MODELS

=============================================================
*/

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
image_t *R_TextureAnimation(mtexinfo_t *tex)
{
    int		c;

    if (!tex->next)
        return tex->image;

    c = currententity->frame % tex->numframes;
    while (c)
    {
        tex = tex->next;
        c--;
    }

    return tex->image;
}

/*
** R_DrawTriangleOutlines
*/
void R_DrawTriangleOutlines(void)
{
}

/*
** R_BlendLightMaps
**
** This routine takes all the given light mapped surfaces in the world and
** blends them into the framebuffer.
*/
void R_BlendLightmaps(void)
{
    int			i;
    msurface_t	*surf, *newdrawsurf = 0;

    // don't bother if we're set to fullbright
    if (r_fullbright->value)
        return;
    if (!r_worldmodel->lightdata)
        return;

    if (currentmodel == r_worldmodel)
        c_visible_lightmaps = 0;

    /*
    ** render static lightmaps first
    */
    for (i = 1; i < MAX_LIGHTMAPS; i++)
    {
        if (vk_lms.lightmap_surfaces[i])
        {
            if (currentmodel == r_worldmodel)
                c_visible_lightmaps++;
            //GL_Bind(vk_state.lightmap_textures + i);

            for (surf = vk_lms.lightmap_surfaces[i]; surf != 0; surf = surf->lightmapchain)
            {
                //if (surf->polys)
                //    DrawGLPolyChain(surf->polys, 0, 0);
            }
        }
    }

    /*
    ** render dynamic lightmaps
    */
    if (vk_dynamic->value)
    {
        LM_InitBlock();

        //GL_Bind(vk_state.lightmap_textures + 0);

        if (currentmodel == r_worldmodel)
            c_visible_lightmaps++;

        newdrawsurf = vk_lms.lightmap_surfaces[0];

        for (surf = vk_lms.lightmap_surfaces[0]; surf != 0; surf = surf->lightmapchain)
        {
            int		smax, tmax;
            byte	*base;

            smax = (surf->extents[0] >> 4) + 1;
            tmax = (surf->extents[1] >> 4) + 1;

            if (LM_AllocBlock(smax, tmax, &surf->dlight_s, &surf->dlight_t))
            {
                base = vk_lms.lightmap_buffer;
                base += (surf->dlight_t * BLOCK_WIDTH + surf->dlight_s) * LIGHTMAP_BYTES;

                R_BuildLightMap(surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
            }
            else
            {
                msurface_t *drawsurf;

                // upload what we have so far
                LM_UploadBlock(true);

                // draw all surfaces that use this lightmap
                for (drawsurf = newdrawsurf; drawsurf != surf; drawsurf = drawsurf->lightmapchain)
                {
                    if (drawsurf->polys)
                    {
                        //DrawGLPolyChain(drawsurf->polys,
                        //(drawsurf->light_s - drawsurf->dlight_s) * (1.0 / 128.0),
                        //    (drawsurf->light_t - drawsurf->dlight_t) * (1.0 / 128.0));
                    }
                }

                newdrawsurf = drawsurf;

                // clear the block
                LM_InitBlock();

                // try uploading the block now
                if (!LM_AllocBlock(smax, tmax, &surf->dlight_s, &surf->dlight_t))
                {
                    ri.Sys_Error(ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed (dynamic)\n", smax, tmax);
                }

                base = vk_lms.lightmap_buffer;
                base += (surf->dlight_t * BLOCK_WIDTH + surf->dlight_s) * LIGHTMAP_BYTES;

                R_BuildLightMap(surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
            }
        }

        /*
        ** draw remainder of dynamic lightmaps that haven't been uploaded yet
        */
        if (newdrawsurf)
            LM_UploadBlock(true);

        for (surf = newdrawsurf; surf != 0; surf = surf->lightmapchain)
        {
            //if (surf->polys)
            //    DrawGLPolyChain(surf->polys, (surf->light_s - surf->dlight_s) * (1.0 / 128.0), (surf->light_t - surf->dlight_t) * (1.0 / 128.0));
        }
    }
}

/*
================
R_RenderBrushPoly
================
*/
void R_RenderBrushPoly(msurface_t *fa)
{
    int			maps;
    image_t		*image;
    qboolean is_dynamic = false;

    c_brush_polys++;

    image = R_TextureAnimation(fa->texinfo);

    if (fa->flags & SURF_DRAWTURB)
    {
        return;
    }
    else
    {
        //GL_Bind(image->texnum);
    }

    //======
    //PGM
    //if (fa->texinfo->flags & SURF_FLOWING)
    //    DrawGLFlowingPoly(fa);
    //else
    //    DrawGLPoly(fa->polys);
    //PGM
    //======

    /*
    ** check for lightmap modification
    */
    for (maps = 0; maps < MAXLIGHTMAPS && fa->styles[maps] != 255; maps++)
    {
        if (r_newrefdef.lightstyles[fa->styles[maps]].white != fa->cached_light[maps])
            goto dynamic;
    }

    // dynamic this frame or dynamic previously
    if ((fa->dlightframe == r_framecount))
    {
    dynamic:
        if (vk_dynamic->value)
        {
            if (!(fa->texinfo->flags & (SURF_SKY | SURF_TRANS33 | SURF_TRANS66 | SURF_WARP)))
            {
                is_dynamic = true;
            }
        }
    }

    if (is_dynamic)
    {
        if ((fa->styles[maps] >= 32 || fa->styles[maps] == 0) && (fa->dlightframe != r_framecount))
        {
            unsigned	temp[34 * 34];
            int			smax, tmax;

            smax = (fa->extents[0] >> 4) + 1;
            tmax = (fa->extents[1] >> 4) + 1;

            R_BuildLightMap(fa, (void *)temp, smax * 4);
            R_SetCacheState(fa);

            fa->lightmapchain = vk_lms.lightmap_surfaces[fa->lightmaptexturenum];
            vk_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
        }
        else
        {
            fa->lightmapchain = vk_lms.lightmap_surfaces[0];
            vk_lms.lightmap_surfaces[0] = fa;
        }
    }
    else
    {
        fa->lightmapchain = vk_lms.lightmap_surfaces[fa->lightmaptexturenum];
        vk_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
    }
}


/*
================
R_DrawAlphaSurfaces

Draw water surfaces and windows.
The BSP tree is waled front to back, so unwinding the chain
of alpha_surfaces will draw back to front, giving proper ordering.
================
*/
void R_DrawAlphaSurfaces(void)
{
    msurface_t	*s;
    float		intens;

    // the textures are prescaled up for a better lighting range,
    // so scale it back down
    intens = vk_state.inverse_intensity;

    for (s = r_alpha_surfaces; s; s = s->texturechain)
    {
        //GL_Bind(s->texinfo->image->texnum);
        c_brush_polys++;

        //if (s->flags & SURF_DRAWTURB)
        //    EmitWaterPolys(s);
        //else
        //    DrawGLPoly(s->polys);
    }

    r_alpha_surfaces = NULL;
}

/*
================
DrawTextureChains
================
*/
void DrawTextureChains(void)
{
    int		i;
    msurface_t	*s;
    image_t		*image;

    c_visible_textures = 0;

    if (false)
    {
        for (i = 0, image = vktextures; i<numvktextures; i++, image++)
        {
            if (!image->registration_sequence)
                continue;
            s = image->texturechain;
            if (!s)
                continue;
            c_visible_textures++;

            for (; s; s = s->texturechain)
                R_RenderBrushPoly(s);

            image->texturechain = NULL;
        }
    }
    else
    {
        for (i = 0, image = vktextures; i<numvktextures; i++, image++)
        {
            if (!image->registration_sequence)
                continue;
            if (!image->texturechain)
                continue;
            c_visible_textures++;

            for (s = image->texturechain; s; s = s->texturechain)
            {
                if (!(s->flags & SURF_DRAWTURB))
                    R_RenderBrushPoly(s);
            }
        }

        for (i = 0, image = vktextures; i<numvktextures; i++, image++)
        {
            if (!image->registration_sequence)
                continue;
            s = image->texturechain;
            if (!s)
                continue;

            for (; s; s = s->texturechain)
            {
                if (s->flags & SURF_DRAWTURB)
                    R_RenderBrushPoly(s);
            }

            image->texturechain = NULL;
        }
    }
}

/*
=================
R_DrawInlineBModel
=================
*/
void R_DrawInlineBModel(void)
{
    int			i, k;
    cplane_t	*pplane;
    float		dot;
    msurface_t	*psurf;
    dlight_t	*lt;

    // calculate dynamic lighting for bmodel
    if (!vk_flashblend->value)
    {
        lt = r_newrefdef.dlights;
        for (k = 0; k<r_newrefdef.num_dlights; k++, lt++)
        {
            R_MarkLights(lt, 1 << k, currentmodel->nodes + currentmodel->firstnode);
        }
    }

    psurf = &currentmodel->surfaces[currentmodel->firstmodelsurface];

    if (currententity->flags & RF_TRANSLUCENT)
    {
        // TODO:
    }

    //
    // draw texture
    //
    for (i = 0; i<currentmodel->nummodelsurfaces; i++, psurf++)
    {
        // find which side of the node we are on
        pplane = psurf->plane;

        dot = DotProduct(modelorg, pplane->normal) - pplane->dist;

        // draw the polygon
        if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
            (!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
        {
            if (psurf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66))
            {	// add to the translucent chain
                psurf->texturechain = r_alpha_surfaces;
                r_alpha_surfaces = psurf;
            }
            else
            {
                R_RenderBrushPoly(psurf);
            }
        }
    }

    if (!(currententity->flags & RF_TRANSLUCENT))
    {
        R_BlendLightmaps();
    }
    else
    {
        // TODO:
    }
}

/*
=================
R_DrawBrushModel
=================
*/
void R_DrawBrushModel(entity_t *e)
{
    vec3_t		mins, maxs;
    int			i;
    qboolean	rotated;

    if (currentmodel->nummodelsurfaces == 0)
        return;

    currententity = e;
    vk_state.currenttextures[0] = vk_state.currenttextures[1] = -1;

    if (e->angles[0] || e->angles[1] || e->angles[2])
    {
        rotated = true;
        for (i = 0; i<3; i++)
        {
            mins[i] = e->origin[i] - currentmodel->radius;
            maxs[i] = e->origin[i] + currentmodel->radius;
        }
    }
    else
    {
        rotated = false;
        VectorAdd(e->origin, currentmodel->mins, mins);
        VectorAdd(e->origin, currentmodel->maxs, maxs);
    }

    if (R_CullBox(mins, maxs))
        return;

    //qglColor3f(1, 1, 1);
    memset(vk_lms.lightmap_surfaces, 0, sizeof(vk_lms.lightmap_surfaces));

    VectorSubtract(r_newrefdef.vieworg, e->origin, modelorg);
    if (rotated)
    {
        vec3_t	temp;
        vec3_t	forward, right, up;

        VectorCopy(modelorg, temp);
        AngleVectors(e->angles, forward, right, up);
        modelorg[0] = DotProduct(temp, forward);
        modelorg[1] = -DotProduct(temp, right);
        modelorg[2] = DotProduct(temp, up);
    }

    e->angles[0] = -e->angles[0];	// stupid quake bug
    e->angles[2] = -e->angles[2];	// stupid quake bug
    R_RotateForEntity(e);
    e->angles[0] = -e->angles[0];	// stupid quake bug
    e->angles[2] = -e->angles[2];	// stupid quake bug

    R_DrawInlineBModel();
}

/*
=============================================================

WORLD MODEL

=============================================================
*/

/*
================
R_RecursiveWorldNode
================
*/
void R_RecursiveWorldNode(mnode_t *node)
{
    int			c, side, sidebit;
    cplane_t	*plane;
    msurface_t	*surf, **mark;
    mleaf_t		*pleaf;
    float		dot;
    image_t		*image;

    if (node->contents == CONTENTS_SOLID)
        return;		// solid

    if (node->visframe != r_visframecount)
        return;
    if (R_CullBox(node->minmaxs, node->minmaxs + 3))
        return;

    // if a leaf node, draw stuff
    if (node->contents != -1)
    {
        pleaf = (mleaf_t *)node;

        // check for door connected areas
        if (r_newrefdef.areabits)
        {
            if (!(r_newrefdef.areabits[pleaf->area >> 3] & (1 << (pleaf->area & 7))))
                return;		// not visible
        }

        mark = pleaf->firstmarksurface;
        c = pleaf->nummarksurfaces;

        if (c)
        {
            do
            {
                (*mark)->visframe = r_framecount;
                mark++;
            } while (--c);
        }

        return;
    }

    // node is just a decision point, so go down the apropriate sides

    // find which side of the node we are on
    plane = node->plane;

    switch (plane->type)
    {
    case PLANE_X:
        dot = modelorg[0] - plane->dist;
        break;
    case PLANE_Y:
        dot = modelorg[1] - plane->dist;
        break;
    case PLANE_Z:
        dot = modelorg[2] - plane->dist;
        break;
    default:
        dot = DotProduct(modelorg, plane->normal) - plane->dist;
        break;
    }

    if (dot >= 0)
    {
        side = 0;
        sidebit = 0;
    }
    else
    {
        side = 1;
        sidebit = SURF_PLANEBACK;
    }

    // recurse down the children, front side first
    R_RecursiveWorldNode(node->children[side]);

    // draw stuff
    for (c = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; c; c--, surf++)
    {
        if (surf->visframe != r_framecount)
            continue;

        if ((surf->flags & SURF_PLANEBACK) != sidebit)
            continue;		// wrong side

        if (surf->texinfo->flags & SURF_SKY)
        {	// just adds to visible sky bounds
            R_AddSkySurface(surf);
        }
        else if (surf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66))
        {	// add to the translucent chain
            surf->texturechain = r_alpha_surfaces;
            r_alpha_surfaces = surf;
        }
        else
        {
            //if (qglMTexCoord2fSGIS && !(surf->flags & SURF_DRAWTURB))
            //{
            //    GL_RenderLightmappedPoly(surf);
            //}
            //else
            {
                // the polygon is visible, so add it to the texture
                // sorted chain
                // FIXME: this is a hack for animation
                image = R_TextureAnimation(surf->texinfo);
                surf->texturechain = image->texturechain;
                image->texturechain = surf;
            }
        }
    }

    // recurse down the back side
    R_RecursiveWorldNode(node->children[!side]);
    /*
    for ( ; c ; c--, surf++)
    {
    if (surf->visframe != r_framecount)
    continue;

    if ( (surf->flags & SURF_PLANEBACK) != sidebit )
    continue;		// wrong side

    if (surf->texinfo->flags & SURF_SKY)
    {	// just adds to visible sky bounds
    R_AddSkySurface (surf);
    }
    else if (surf->texinfo->flags & (SURF_TRANS33|SURF_TRANS66))
    {	// add to the translucent chain
    //			surf->texturechain = alpha_surfaces;
    //			alpha_surfaces = surf;
    }
    else
    {
    if ( qglMTexCoord2fSGIS && !( surf->flags & SURF_DRAWTURB ) )
    {
    GL_RenderLightmappedPoly( surf );
    }
    else
    {
    // the polygon is visible, so add it to the texture
    // sorted chain
    // FIXME: this is a hack for animation
    image = R_TextureAnimation (surf->texinfo);
    surf->texturechain = image->texturechain;
    image->texturechain = surf;
    }
    }
    }
    */
}


/*
=============
R_DrawWorld
=============
*/
void R_DrawWorld(void)
{
    entity_t	ent;

    if (!r_drawworld->value)
        return;

    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
        return;

    currentmodel = r_worldmodel;

    VectorCopy(r_newrefdef.vieworg, modelorg);

    // auto cycle the world frame for texture animation
    memset(&ent, 0, sizeof(ent));
    ent.frame = (int)(r_newrefdef.time * 2);
    currententity = &ent;

    vk_state.currenttextures[0] = vk_state.currenttextures[1] = -1;

    //qglColor3f(1, 1, 1);
    memset(vk_lms.lightmap_surfaces, 0, sizeof(vk_lms.lightmap_surfaces));
    R_ClearSkyBox();

    R_RecursiveWorldNode(r_worldmodel->nodes);

    /*
    ** theoretically nothing should happen in the next two functions
    ** if multitexture is enabled
    */
    DrawTextureChains();
    R_BlendLightmaps();

    R_DrawSkyBox();

    R_DrawTriangleOutlines();
}


/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
void R_MarkLeaves(void)
{
    byte	*vis;
    byte	fatvis[MAX_MAP_LEAFS / 8];
    mnode_t	*node;
    int		i, c;
    mleaf_t	*leaf;
    int		cluster;

    if (r_oldviewcluster == r_viewcluster && r_oldviewcluster2 == r_viewcluster2 && !r_novis->value && r_viewcluster != -1)
        return;

    // development aid to let you run around and see exactly where
    // the pvs ends
    if (vk_lockpvs->value)
        return;

    r_visframecount++;
    r_oldviewcluster = r_viewcluster;
    r_oldviewcluster2 = r_viewcluster2;

    if (r_novis->value || r_viewcluster == -1 || !r_worldmodel->vis)
    {
        // mark everything
        for (i = 0; i<r_worldmodel->numleafs; i++)
            r_worldmodel->leafs[i].visframe = r_visframecount;
        for (i = 0; i<r_worldmodel->numnodes; i++)
            r_worldmodel->nodes[i].visframe = r_visframecount;
        return;
    }

    vis = Mod_ClusterPVS(r_viewcluster, r_worldmodel);
    // may have to combine two clusters because of solid water boundaries
    if (r_viewcluster2 != r_viewcluster)
    {
        memcpy(fatvis, vis, (r_worldmodel->numleafs + 7) / 8);
        vis = Mod_ClusterPVS(r_viewcluster2, r_worldmodel);
        c = (r_worldmodel->numleafs + 31) / 32;
        for (i = 0; i<c; i++)
            ((int *)fatvis)[i] |= ((int *)vis)[i];
        vis = fatvis;
    }

    for (i = 0, leaf = r_worldmodel->leafs; i<r_worldmodel->numleafs; i++, leaf++)
    {
        cluster = leaf->cluster;
        if (cluster == -1)
            continue;
        if (vis[cluster >> 3] & (1 << (cluster & 7)))
        {
            node = (mnode_t *)leaf;
            do
            {
                if (node->visframe == r_visframecount)
                    break;
                node->visframe = r_visframecount;
                node = node->parent;
            } while (node);
        }
    }

#if 0
    for (i = 0; i<r_worldmodel->vis->numclusters; i++)
    {
        if (vis[i >> 3] & (1 << (i & 7)))
        {
            node = (mnode_t *)&r_worldmodel->leafs[i];	// FIXME: cluster
            do
            {
                if (node->visframe == r_visframecount)
                    break;
                node->visframe = r_visframecount;
                node = node->parent;
            } while (node);
        }
    }
#endif
}

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