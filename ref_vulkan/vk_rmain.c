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

viddef_t	vid;

refimport_t	ri;

vkcontext_t vk_context;
vkshaders_t vk_shaders;
vkconfig_t  vk_config;
vkstate_t   vk_state;
vkmatrices_t vk_transforms;

model_t		*r_worldmodel;

float		vkdepthmin, vkdepthmax;

vkconfig_t vk_config;
vkstate_t  vk_state;

image_t		*r_notexture;		// use for bad textures
image_t		*r_particletexture;	// little dot for particles

entity_t	*currententity;
model_t		*currentmodel;

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

XMMATRIX r_ortho;
XMMATRIX r_viewproj;

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
=================
R_CullBox

Returns true if the box is completely outside the frustom
=================
*/
qboolean R_CullBox(vec3_t mins, vec3_t maxs)
{
    int		i;

    if (r_nocull->value)
        return false;

    for (i = 0; i<4; i++)
        if (BOX_ON_PLANE_SIDE(mins, maxs, &frustum[i]) == 2)
            return true;
    return false;
}

void R_RotateForEntity(entity_t *e)
{
    float       angles[3];
    XMMATRIX    rx, ry, rz, tr;
    XMMATRIX    world, worldviewproj;
    uint32_t    dynamicoffset;
    
    angles[0] = XMConvertToRadians(e->angles[0]);
    angles[1] = XMConvertToRadians(e->angles[1]);
    angles[2] = XMConvertToRadians(e->angles[2]);

    rx = XMMatrixRotationX(-angles[2]);
    ry = XMMatrixRotationY(-angles[0]);
    rz = XMMatrixRotationZ(angles[1]);
    tr = XMMatrixTranslation(e->origin[0], e->origin[1], e->origin[2]);

    world = XMMatrixMultiply(&rx, &ry);
    world = XMMatrixMultiply(&world, &rz);
    world = XMMatrixMultiply(&world, &tr);
    worldviewproj = XMMatrixMultiply(&world, &r_viewproj);

    // write to array for now, buffer will be updated before command buffer execution
    vk_transforms.entities[vk_transforms.offset] = worldviewproj;

    dynamicoffset = vk_transforms.offset * sizeof(XMMATRIX);
    vkCmdBindDescriptorSets(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline_layout,
        0, 1, &vk_context.dset,
        1, &dynamicoffset); // fetch transform from specified offset during cmdbuffer execution

    ++vk_transforms.offset;
}

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
    int		i;

    if (!r_drawentities->value)
        return;

    R_BeginRenderAliasModels();

    // draw non-transparent first
    for (i = 0; i<r_newrefdef.num_entities; i++)
    {
        currententity = &r_newrefdef.entities[i];
        if (currententity->flags & RF_TRANSLUCENT)
            continue;	// solid

        if (currententity->flags & RF_BEAM)
        {
            R_DrawBeam(currententity);
        }
        else
        {
            currentmodel = currententity->model;
            if (!currentmodel)
            {
                R_DrawNullModel();
                continue;
            }
            switch (currentmodel->type)
            {
            case mod_alias:
                R_DrawAliasModel(currententity);
                break;
            case mod_brush:
                R_DrawBrushModel(currententity);
                break;
            case mod_sprite:
                //R_DrawSpriteModel(currententity);
                break;
            default:
                ri.Sys_Error(ERR_DROP, "Bad modeltype");
                break;
            }
        }
    }

    // draw transparent entities
    // we could sort these if it ever becomes a problem...
    //qglDepthMask(0);		// no z writes
    for (i = 0; i<r_newrefdef.num_entities; i++)
    {
        currententity = &r_newrefdef.entities[i];
        if (!(currententity->flags & RF_TRANSLUCENT))
            continue;	// solid

        if (currententity->flags & RF_BEAM)
        {
            R_DrawBeam(currententity);
        }
        else
        {
            currentmodel = currententity->model;

            if (!currentmodel)
            {
                R_DrawNullModel();
                continue;
            }
            switch (currentmodel->type)
            {
            case mod_alias:
                R_DrawAliasModel(currententity);
                break;
            case mod_brush:
                R_DrawBrushModel(currententity);
                break;
            case mod_sprite:
                //R_DrawSpriteModel(currententity);
                break;
            default:
                ri.Sys_Error(ERR_DROP, "Bad modeltype");
                break;
            }
        }
    }
    //qglDepthMask(1);		// back to writing

    R_EndRenderAliasModels();
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

//=======================================================================

int SignbitsForPlane(cplane_t *out)
{
    int	bits, j;

    // for fast box on planeside test

    bits = 0;
    for (j = 0; j<3; j++)
    {
        if (out->normal[j] < 0)
            bits |= 1 << j;
    }
    return bits;
}

/*
============
R_SetFrustum
============
*/
void R_SetFrustum(void)
{
    int		i;

#if 0
    /*
    ** this code is wrong, since it presume a 90 degree FOV both in the
    ** horizontal and vertical plane
    */
    // front side is visible
    VectorAdd(vpn, vright, frustum[0].normal);
    VectorSubtract(vpn, vright, frustum[1].normal);
    VectorAdd(vpn, vup, frustum[2].normal);
    VectorSubtract(vpn, vup, frustum[3].normal);

    // we theoretically don't need to normalize these vectors, but I do it
    // anyway so that debugging is a little easier
    VectorNormalize(frustum[0].normal);
    VectorNormalize(frustum[1].normal);
    VectorNormalize(frustum[2].normal);
    VectorNormalize(frustum[3].normal);
#else
    // rotate VPN right by FOV_X/2 degrees
    RotatePointAroundVector(frustum[0].normal, vup, vpn, -(90 - r_newrefdef.fov_x / 2));
    // rotate VPN left by FOV_X/2 degrees
    RotatePointAroundVector(frustum[1].normal, vup, vpn, 90 - r_newrefdef.fov_x / 2);
    // rotate VPN up by FOV_X/2 degrees
    RotatePointAroundVector(frustum[2].normal, vright, vpn, 90 - r_newrefdef.fov_y / 2);
    // rotate VPN down by FOV_X/2 degrees
    RotatePointAroundVector(frustum[3].normal, vright, vpn, -(90 - r_newrefdef.fov_y / 2));
#endif

    for (i = 0; i<4; i++)
    {
        frustum[i].type = PLANE_ANYZ;
        frustum[i].dist = DotProduct(r_origin, frustum[i].normal);
        frustum[i].signbits = SignbitsForPlane(&frustum[i]);
    }
}

//=======================================================================

/*
===============
R_SetupFrame
===============
*/
void R_SetupFrame(void)
{
    int i;
    mleaf_t	*leaf;

    r_framecount++;

    // build the transformation matrix for the given view angles
    VectorCopy(r_newrefdef.vieworg, r_origin);

    AngleVectors(r_newrefdef.viewangles, vpn, vright, vup);

    // current viewcluster
    if (!(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
    {
        r_oldviewcluster = r_viewcluster;
        r_oldviewcluster2 = r_viewcluster2;
        leaf = Mod_PointInLeaf(r_origin, r_worldmodel);
        r_viewcluster = r_viewcluster2 = leaf->cluster;

        // check above and below so crossing solid water doesn't draw wrong
        if (!leaf->contents)
        {	// look down a bit
            vec3_t	temp;

            VectorCopy(r_origin, temp);
            temp[2] -= 16;
            leaf = Mod_PointInLeaf(temp, r_worldmodel);
            if (!(leaf->contents & CONTENTS_SOLID) &&
                (leaf->cluster != r_viewcluster2))
                r_viewcluster2 = leaf->cluster;
        }
        else
        {	// look up a bit
            vec3_t	temp;

            VectorCopy(r_origin, temp);
            temp[2] += 16;
            leaf = Mod_PointInLeaf(temp, r_worldmodel);
            if (!(leaf->contents & CONTENTS_SOLID) &&
                (leaf->cluster != r_viewcluster2))
                r_viewcluster2 = leaf->cluster;
        }
    }

    for (i = 0; i<4; i++)
        v_blend[i] = r_newrefdef.blend[i];

    c_brush_polys = 0;
    c_alias_polys = 0;

    // clear out the portion of the screen that the NOWORLDMODEL defines
    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
    {
        // TODO:
    }

	// clear vertex offsets
	r_worldmodel->vertexbuffer->firstvertex = 0;
    for (i = 0; i<r_newrefdef.num_entities; i++)
	{
		entity_t *ent = &r_newrefdef.entities[i];
		model_t *mod = ent->model;
		if (mod)
		{
			vkbuffer_t *vb = ent->model->vertexbuffer;
			if (vb) vb->firstvertex = 0;
		}
	}
	vk_context.debugverts.firstvertex = 0;
}

/*
=============
R_Clear
=============
*/
void R_Clear(void)
{
    VkRenderPassBeginInfo render_pass_begin;
    VkClearValue clear_values[2];

	if (vk_clear->value)
	{
		// see GL_SetDefaultState for reference
		clear_values[0].color.float32[0] = 1.0f;
		clear_values[0].color.float32[1] = 0.0f;
		clear_values[0].color.float32[2] = 0.5f;
		clear_values[0].color.float32[3] = 0.5f;
	}
	else
	{
		// will be ignored, but don't want to pass junk
		memset(&clear_values[0], 0, sizeof(VkClearValue)); 
	}

	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0;

    render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin.pNext = NULL;
    render_pass_begin.renderPass = vk_context.renderpass; // TODO: recreate renderpass if vk_clear changed!
    render_pass_begin.framebuffer = vk_context.curr_framebuffer;
    render_pass_begin.renderArea.offset.x = 0;
    render_pass_begin.renderArea.offset.y = 0;
    render_pass_begin.renderArea.extent.width = vid.width;
    render_pass_begin.renderArea.extent.height = vid.height;
    render_pass_begin.clearValueCount = 2;
    render_pass_begin.pClearValues = clear_values;

    vkCmdBeginRenderPass(vk_context.cmdbuffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);

	vkdepthmin = 0.f;
	vkdepthmax = 1.f;
}

/*
=============
R_SetViewport
=============
*/
void R_SetViewport(void)
{
    int	x, x2, y2, y, w, h;
    
    x = floor(r_newrefdef.x * vid.width / vid.width);
    x2 = ceil((r_newrefdef.x + r_newrefdef.width) * vid.width / vid.width);
    y = floor(vid.height - r_newrefdef.y * vid.height / vid.height);
    y2 = ceil(vid.height - (r_newrefdef.y + r_newrefdef.height) * vid.height / vid.height);

    w = x2 - x;
    h = y - y2;

    vk_state.vp.x = x;
    vk_state.vp.y = y2;
    vk_state.vp.width = w;
    vk_state.vp.height = -h; // Inverse Y axis to match OpenGL
    vk_state.vp.minDepth = vkdepthmin;
    vk_state.vp.maxDepth = vkdepthmax;

    vk_state.scissor.offset.x = x;
    vk_state.scissor.offset.y = y2;
    vk_state.scissor.extent.width = w;
    vk_state.scissor.extent.height = h;

    vkCmdSetViewport(vk_context.cmdbuffer, 0, 1, &vk_state.vp);
    vkCmdSetScissor(vk_context.cmdbuffer, 0, 1, &vk_state.scissor);
}

/*
=============
R_SetModelViewProjection
=============
*/
void R_SetModelViewProjection()
{
    float    fov_y;
    float    screenaspect;
    float    znear, zfar;
    float    viewangles[3], rad90;
    XMMATRIX z_up[2], rx, ry, rz, tr;
    XMMATRIX proj, view;

    fov_y = XMConvertToRadians(r_newrefdef.fov_y);
    screenaspect = (float)r_newrefdef.width/r_newrefdef.height;
    znear = 4.f;
    zfar = 4096.f;

    proj = XMMatrixPerspectiveFovRH(fov_y, screenaspect, znear, zfar);

    viewangles[0] = XMConvertToRadians(r_newrefdef.viewangles[0]);
    viewangles[1] = XMConvertToRadians(r_newrefdef.viewangles[1]);
    viewangles[2] = XMConvertToRadians(r_newrefdef.viewangles[2]);

    rad90 = XMConvertToRadians(90.0f);
    z_up[0] = XMMatrixRotationX(-rad90);   // put Z going up
    z_up[1] = XMMatrixRotationZ(rad90);    // put Z going up
    rx = XMMatrixRotationX(-viewangles[2]);
    ry = XMMatrixRotationY(-viewangles[0]);
    rz = XMMatrixRotationZ(-viewangles[1]);
    tr = XMMatrixTranslation(-r_newrefdef.vieworg[0], -r_newrefdef.vieworg[1], -r_newrefdef.vieworg[2]);

    view = XMMatrixMultiply(&tr, &rz);
    view = XMMatrixMultiply(&view, &ry);
    view = XMMatrixMultiply(&view, &rx);
    view = XMMatrixMultiply(&view, &z_up[1]);
    view = XMMatrixMultiply(&view, &z_up[0]);

    r_viewproj = XMMatrixMultiply(&view, &proj);

    if (vkMapMemory(vk_context.device, vk_transforms.perframe.memory, 
		0, 
		sizeof(XMMATRIX), 
		0, &vk_transforms.perframe.memptr) == VK_SUCCESS)
    {
        memcpy(vk_transforms.perframe.memptr, &r_viewproj, sizeof(XMMATRIX));
        vkUnmapMemory(vk_context.device, vk_transforms.perframe.memory);
    }

    vk_transforms.offset = 0;
    vkCmdBindDescriptorSets(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline_layout, 
        0, 1, &vk_context.dset, 1, &vk_transforms.offset);

    // clear entity transforms
    memset(vk_transforms.entities, 0, sizeof(vk_transforms.entities));
}

/*
=============
R_Flash
=============
*/
void R_Flash(void)
{
    R_PolyBlend();
}

/*
=============
R_PostUpdate
=============
*/
void R_PostUpdate(void)
{
    VkDeviceSize size;

	if (vk_transforms.offset > 0)
	{
		size = vk_transforms.offset * sizeof(XMMATRIX);
		if (vkMapMemory(vk_context.device, vk_transforms.perobject.memory, 0, size, 0, &vk_transforms.perobject.memptr) == VK_SUCCESS)
		{
			memcpy(vk_transforms.perobject.memptr, vk_transforms.entities, size);
			vkUnmapMemory(vk_context.device, vk_transforms.perobject.memory);
		}
	}
}

/*
================
R_RenderView

r_newrefdef must be set before the first call
================
*/
void R_RenderView(refdef_t *fd)
{
    if (r_norefresh->value)
        return;

    r_newrefdef = *fd;

    if (!r_worldmodel && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
        ri.Sys_Error(ERR_DROP, "R_RenderView: NULL worldmodel");

    if (r_speeds->value)
    {
        c_brush_polys = 0;
        c_alias_polys = 0;
    }

    R_PushDlights();

    //if (vk_finish->value)
    //    sync();

    R_SetupFrame();

    R_SetFrustum();
    R_SetViewport();
    R_SetModelViewProjection();

    //R_SetupGL();

    R_MarkLeaves();	// done here so we know if we're in water

    R_DrawWorld();

    R_DrawEntitiesOnList();

    R_RenderDlights();

    R_DrawParticles();

    R_DrawAlphaSurfaces();

    R_Flash();

    R_PostUpdate();

    if (r_speeds->value)
    {
        ri.Con_Printf(PRINT_ALL, "%4i wpoly %4i epoly %i tex %i lmaps\n",
            c_brush_polys,
            c_alias_polys,
            c_visible_textures,
            c_visible_lightmaps);
    }
}

/*
====================
R_SetLightLevel

====================
*/
void R_SetLightLevel(void)
{
    vec3_t		shadelight;

    if (r_newrefdef.rdflags & RDF_NOWORLDMODEL)
        return;

    // save off light value for server to look at (BIG HACK!)

    R_LightPoint(r_newrefdef.vieworg, shadelight);

    // pick the greatest component, which should be the same
    // as the mono value returned by software
    if (shadelight[0] > shadelight[1])
    {
        if (shadelight[0] > shadelight[2])
            r_lightlevel->value = 150 * shadelight[0];
        else
            r_lightlevel->value = 150 * shadelight[2];
    }
    else
    {
        if (shadelight[1] > shadelight[2])
            r_lightlevel->value = 150 * shadelight[1];
        else
            r_lightlevel->value = 150 * shadelight[2];
    }

}

/*
@@@@@@@@@@@@@@@@@@@@@
R_RenderFrame

@@@@@@@@@@@@@@@@@@@@@
*/
void R_RenderFrame(refdef_t *fd)
{
    R_RenderView(fd);
    R_SetLightLevel();
    //R_SetGL2D();
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
    VkCommandPoolCreateInfo cmdpool_info;
    VkCommandBufferAllocateInfo cmdbuf_alloc_info;
	VkPushConstantRange pushconstant_range;
	VkPipelineLayoutCreateInfo layout_info;

	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphore_info.pNext = NULL;
    semaphore_info.flags = 0;

	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.pNext = NULL;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // create as signaled for first frame

    vkGetDeviceQueue(vk_context.device, 0, 0, &vk_context.queue);
    vkGetDeviceQueue(vk_context.device, 2, 0, &vk_context.transfer_queue);
    vkCreateSemaphore(vk_context.device, &semaphore_info, NULL, &vk_context.present);
    vkCreateSemaphore(vk_context.device, &semaphore_info, NULL, &vk_context.render);
    vkCreateFence(vk_context.device, &fence_info, NULL, &vk_context.fences[0]);
    vkCreateFence(vk_context.device, &fence_info, NULL, &vk_context.fences[1]);

    cmdpool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdpool_info.pNext = NULL;
    cmdpool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdpool_info.queueFamilyIndex = 0;

    vkCreateCommandPool(vk_context.device, &cmdpool_info, NULL, &vk_context.cmdpool);

    cmdbuf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdbuf_alloc_info.pNext = NULL;
    cmdbuf_alloc_info.commandPool = vk_context.cmdpool;
    cmdbuf_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdbuf_alloc_info.commandBufferCount = 1;

    vkAllocateCommandBuffers(vk_context.device, &cmdbuf_alloc_info, &vk_context.cmdbuffer);

    Vk_DSetSetupLayout();

	pushconstant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushconstant_range.offset = 0;
	pushconstant_range.size = sizeof(float) * 4; // color

    layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_info.pNext = NULL;
    layout_info.flags = 0;
    layout_info.setLayoutCount = 1;
    layout_info.pSetLayouts = &vk_context.dset_layout;
    layout_info.pushConstantRangeCount = 1;
    layout_info.pPushConstantRanges = &pushconstant_range;

    vkCreatePipelineLayout(vk_context.device, &layout_info, NULL, &vk_context.pipeline_layout);

	vk_context.p_world = Vk_CreatePipeline(vk_shaders.tnl_world_v, vk_shaders.tnl_world_f, VF_BRUSH, 
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, 
		true, VK_COMPARE_OP_LESS_OR_EQUAL, BLEND_NONE);

	vk_context.p_world_showtris = Vk_CreatePipeline(vk_shaders.mvp, vk_shaders.fill, VF_DEBUG,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_LINE, VK_CULL_MODE_FRONT_BIT, 
		false, VK_COMPARE_OP_ALWAYS, BLEND_NONE);

	vk_context.p_brush = Vk_CreatePipeline(vk_shaders.tnl_brush_v, vk_shaders.tnl_world_f, VF_BRUSH, 
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, 
		true, VK_COMPARE_OP_LESS_OR_EQUAL, BLEND_NONE);

	vk_context.p_alias_tristrip = Vk_CreatePipeline(vk_shaders.tnl_alias_v, vk_shaders.tnl_alias_f, VF_ALIAS, 
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, 
		true, VK_COMPARE_OP_LESS_OR_EQUAL,  BLEND_NONE);

	vk_context.p_alias_trifan = Vk_CreatePipeline(vk_shaders.tnl_alias_v, vk_shaders.tnl_alias_f, VF_ALIAS, 
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT, 
		true, VK_COMPARE_OP_LESS_OR_EQUAL, BLEND_NONE);

	vk_context.p_alias_tristrip_cull_back = Vk_CreatePipeline(vk_shaders.tnl_alias_v, vk_shaders.tnl_alias_f, VF_ALIAS, 
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 
		true, VK_COMPARE_OP_LESS_OR_EQUAL, BLEND_NONE);

	vk_context.p_alias_trifan_cull_back = Vk_CreatePipeline(vk_shaders.tnl_alias_v, vk_shaders.tnl_alias_f, VF_ALIAS, 
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, 
		true, VK_COMPARE_OP_LESS_OR_EQUAL, BLEND_NONE);
}

/*
===============
R_FreeContextObjects
===============
*/
static void R_FreeContextObjects()
{
	Vk_DestroyBuffer(&vk_context.debugverts);

	vkDestroyPipeline(vk_context.device, vk_context.p_world, NULL);
	vkDestroyPipeline(vk_context.device, vk_context.p_brush, NULL);
	vkDestroyPipeline(vk_context.device, vk_context.p_alias_tristrip, NULL);
	vkDestroyPipeline(vk_context.device, vk_context.p_alias_trifan, NULL);
	vkDestroyPipeline(vk_context.device, vk_context.p_alias_tristrip_cull_back, NULL);
	vkDestroyPipeline(vk_context.device, vk_context.p_alias_trifan_cull_back, NULL);
	vkDestroyPipelineLayout(vk_context.device, vk_context.pipeline_layout, NULL);

	Vk_DSetDestroyLayout();

	vkFreeCommandBuffers(vk_context.device, vk_context.cmdpool, 1, &vk_context.cmdbuffer);
	vkDestroyCommandPool(vk_context.device, vk_context.cmdpool, NULL);

	vkDestroySemaphore(vk_context.device, vk_context.present, NULL);
    vkDestroySemaphore(vk_context.device, vk_context.render, NULL);
	vkDestroyFence(vk_context.device, vk_context.fences[0], NULL);
	vkDestroyFence(vk_context.device, vk_context.fences[1], NULL);
}

/*
===============
R_LoadShaders
===============
*/
static void R_LoadShaders()
{
	Vk_LoadShader("shaders/mvp.o", "main", true, &vk_shaders.mvp);
    Vk_LoadShader("shaders/tnl_alias_v.o", "main", true, &vk_shaders.tnl_alias_v);
    Vk_LoadShader("shaders/tnl_alias_f.o", "main", false, &vk_shaders.tnl_alias_f);
	Vk_LoadShader("shaders/tnl_brush_v.o", "main", true, &vk_shaders.tnl_brush_v);
    Vk_LoadShader("shaders/tnl_world_v.o", "main", true, &vk_shaders.tnl_world_v);
    Vk_LoadShader("shaders/tnl_world_f.o", "main", false, &vk_shaders.tnl_world_f);
	Vk_LoadShader("shaders/draw2d_v.o", "main", true, &vk_shaders.draw2D_v);
    Vk_LoadShader("shaders/draw2d_f.o", "main", false, &vk_shaders.draw2D_f);
	Vk_LoadShader("shaders/fill.o", "main", false, &vk_shaders.fill);
}

/*
===============
R_FreeShaders
===============
*/
static void R_FreeShaders()
{
	Vk_DestroyShader(&vk_shaders.mvp);
    Vk_DestroyShader(&vk_shaders.tnl_alias_v);
    Vk_DestroyShader(&vk_shaders.tnl_alias_f);   
	Vk_DestroyShader(&vk_shaders.tnl_brush_v);
    Vk_DestroyShader(&vk_shaders.tnl_world_v);
    Vk_DestroyShader(&vk_shaders.tnl_world_f);
	Vk_DestroyShader(&vk_shaders.draw2D_v);
    Vk_DestroyShader(&vk_shaders.draw2D_f);
	Vk_DestroyShader(&vk_shaders.fill);
}

/*
===============
R_Init
===============
*/
qboolean R_Init(void *hinstance, void *hWnd)
{
#ifdef _DEBUG
    const char *layers[] = {
        "VK_LAYER_LUNARG_standard_validation"
    };
#else
    const char *layers = NULL;
#endif
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
        VK_AMD_NEGATIVE_VIEWPORT_HEIGHT_EXTENSION_NAME
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
    instance_info.enabledLayerCount = layers ? sizeof(layers) / sizeof(layers[0]) : 0;
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

    // initialize OS-specific parts of Vulkan
    if (!VKimp_Init(hinstance, hWnd))
        return false;

    // create the window and set up the context
    if (!R_SetMode())
    {
        ri.Con_Printf(PRINT_ALL, "ref_vulkan::R_Init() - could not R_SetMode()\n");
        return false;
    }

	// create framebuffer
	VK_CreateFramebuffer(vid.width, vid.height, vk_clear->value ? true : false);
	vk_clear->modified = false;

    // initialize common Vulkan objects
    R_LoadShaders();
    R_InitContextObjects();
	
    ri.Vid_MenuInit();

    Vk_InitImages();
    Mod_Init();
    R_InitParticleTexture();
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
	Draw_DestroyLocal();

	R_FreeShaders();
	R_FreeContextObjects();

    VK_DestroyFramebuffer();

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
	if ( vk_mode->modified || vid_fullscreen->modified )
	{	// FIXME: only restart if CDS is required
		cvar_t	*ref;

		ref = ri.Cvar_Get ("vid_ref", "vulkan", 0);
		ref->modified = true;
	}

	if ( vk_log->modified )
	{
		//VKimp_EnableLogging( vk_log->value );
		vk_log->modified = false;
	}

	if ( vk_log->value )
	{
		//VKimp_LogNewFrame();
	}

    /*
	** update 3Dfx gamma -- it is expected that a user will do a vid_restart
	** after tweaking this value
	*/
	if ( vid_gamma->modified )
	{
		vid_gamma->modified = false;

        // TODO:? 
	}

	if (vk_clear->modified)
	{
		Vk_ChangeRenderPass(vid.width, vid.height, vk_clear->value ? true : false);
		vk_clear->modified = false;
	}

	VKimp_BeginFrame(camera_separation);

	vk_state.vp.x = 0.f;
	vk_state.vp.y = 0.f;
	vk_state.vp.width = (float)vid.width;
	vk_state.vp.height = -(float)vid.height;
	vk_state.vp.minDepth = 0.f;
	vk_state.vp.maxDepth = 1.f;

	vk_state.scissor.offset.x = 0;
	vk_state.scissor.offset.y = 0;
	vk_state.scissor.extent.width = vid.width;
	vk_state.scissor.extent.height = vid.height;

	vkCmdSetViewport(vk_context.cmdbuffer, 0, 1, &vk_state.vp);
	vkCmdSetScissor(vk_context.cmdbuffer, 0, 1, &vk_state.scissor);

	/*
	** define projection matrix
	*/
	r_ortho = XMMatrixOrthographicOffCenterRH(0.f, (float)vid.width, (float)vid.height, 0.f, -99999.f, 99999.f);
	if (vkMapMemory(vk_context.device, vk_transforms.perframe.memory, 
		sizeof(XMMATRIX), 
		sizeof(XMMATRIX), 
		0, &vk_transforms.perframe.memptr) == VK_SUCCESS)
    {
        memcpy(vk_transforms.perframe.memptr, &r_ortho, sizeof(XMMATRIX));
        vkUnmapMemory(vk_context.device, vk_transforms.perframe.memory);
    }

	/*
	** texturemode stuff
	*/
	if ( vk_texturemode->modified )
	{
		vk_texturemode->modified = false;
	}

	if ( vk_texturealphamode->modified )
	{
		vk_texturealphamode->modified = false;
	}

	if ( vk_texturesolidmode->modified )
	{
		vk_texturesolidmode->modified = false;
	}

    //
	// clear screen if desired
	//
    R_Clear();

	Draw_Begin();
}

/*
@@@@@@@@@@@@@@@@@@@@@
R_EndFrame
@@@@@@@@@@@@@@@@@@@@@
*/
void R_EndFrame(void)
{
	Draw_End();

	VKimp_EndFrame();
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
    re.EndFrame = R_EndFrame;

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
