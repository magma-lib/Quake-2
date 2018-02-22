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
#include "vk_local.h"

#define MAX_DRAW_PICS	128
#define MAX_DRAW_CHARS	16384
#define MAX_DRAW_FILL	16

typedef struct
{
	float x, y;
	float s, t;
} vkdrawvert_t;

typedef struct
{
	VkPipeline	p_texture;
	VkPipeline	p_fill;
	VkPipeline	p_blend;

	vkbuffer_t	pics;
	vkbuffer_t	chars;
	vkbuffer_t	quads;

	int			numchars;
} vkdraw_t;

vkdraw_t	vk_draw;

static uint32_t s_dynamicoffset = 0;
static VkDeviceSize s_zero = 0;

image_t		*draw_chars;

extern	qboolean	scrap_dirty;
void Scrap_Upload(void);


/*
===============
Draw_InitLocal
===============
*/
void Draw_InitLocal(void)
{
	vk_draw.p_texture = Vk_CreatePipeline(vk_shaders.draw2D_v, vk_shaders.draw2D_f, VF_DRAW, 
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, 
		VK_COMPARE_OP_ALWAYS, BLEND_NONE);

	vk_draw.p_fill = Vk_CreatePipeline(vk_shaders.draw2D_v, vk_shaders.fill2D_f, VF_FILL,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
		VK_COMPARE_OP_ALWAYS, BLEND_NONE);

	vk_draw.p_blend = Vk_CreatePipeline(vk_shaders.draw2D_v, vk_shaders.fill2D_f, VF_FILL,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE,
		VK_COMPARE_OP_ALWAYS, BLEND_NORMAL);

	Vk_CreateVertexBuffer(MAX_DRAW_CHARS * sizeof(vkdrawvert_t) * 6, &vk_draw.chars);
	Vk_CreateVertexBuffer(MAX_DRAW_PICS * sizeof(vkdrawvert_t) * 6, &vk_draw.pics);
	Vk_CreateVertexBuffer(MAX_DRAW_FILL * sizeof(float) * 2 * 6, &vk_draw.quads);

	vkMapMemory(vk_context.device, vk_draw.chars.memory, 
		0, VK_WHOLE_SIZE, 
		0, &vk_draw.chars.memptr);
}


/*
===============
Draw_DestroyLocal
===============
*/
void Draw_DestroyLocal(void)
{
	Vk_DestroyBuffer(&vk_draw.quads);
	Vk_DestroyBuffer(&vk_draw.chars);

	vkDestroyPipeline(vk_context.device, vk_draw.p_texture, NULL);
	vkDestroyPipeline(vk_context.device, vk_draw.p_fill, NULL);
	vkDestroyPipeline(vk_context.device, vk_draw.p_blend, NULL);
}


/*
===============
Draw_Begin
===============
*/
void Draw_Begin(void)
{
	if (!vk_draw.chars.memptr)
	{
		// optimize chars drawing by mapping buffer only once per frame
		vkMapMemory(vk_context.device, vk_draw.chars.memory, 
			0, VK_WHOLE_SIZE, 
			0, &vk_draw.chars.memptr);
	}

	vk_draw.pics.firstvertex = 0;
	vk_draw.chars.firstvertex = 0;
	vk_draw.quads.firstvertex = 0;
	vk_draw.numchars = 0;
}


/*
===============
Draw_Flush
===============
*/
void Draw_End(void)
{
	if (0 == vk_draw.numchars)
		return;

	vkUnmapMemory(vk_context.device, vk_draw.chars.memory);
	vk_draw.chars.memptr = NULL;

	vkCmdBindPipeline(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_draw.p_texture);
	vkCmdBindDescriptorSets(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline_layout, 
		0, 1, &vk_context.dset, 1, &s_dynamicoffset);
	vkCmdBindVertexBuffers(vk_context.cmdbuffer, 0, 1, &vk_draw.chars.buffer, &s_zero);
	// TODO: bind char texture
	vkCmdDraw(vk_context.cmdbuffer, vk_draw.numchars * 6, 1, 0, 0);
}


/*
================
Draw_Char

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Char(int x, int y, int num)
{
	int				row, col;
	float			frow, fcol, size;
	vkdrawvert_t	v[4];
	
	num &= 255;

	if ( (num&127) == 32 )
		return;		// space

	if (y <= -8)
		return;			// totally off screen

	row = num>>4;
	col = num&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	v[0].x = x;		v[0].y = y;
	v[0].s = 0.f;	v[0].t = 0.f;
	v[1].x = x + 8;	v[1].y = y;
	v[1].s = 1.f;	v[1].t = 0.f;
	v[2].x = x;		v[2].y = y + 8;
	v[2].s = 0.f;	v[2].t = 1.f;
	v[3].x = x + 8;	v[3].y = y + 8;
	v[3].s = 1.f;	v[3].t = 1.f;

	if (vk_draw.numchars < MAX_DRAW_CHARS)
	{
		vkdrawvert_t *pv = vk_draw.chars.memptr;
		if (pv)
		{
			*pv++ = v[0]; *pv++ = v[1]; *pv++ = v[2];
			*pv++ = v[2]; *pv++ = v[1]; *pv++ = v[3];
			vk_draw.chars.memptr = pv;
			++vk_draw.numchars;
		}
	}
	else
	{
		ri.Con_Printf(PRINT_DEVELOPER, "Draw_Char: number of chars (%d) exceeds buffer size\n", 
			vk_draw.numchars);
	}
}


/*
=============
Draw_FindPic
=============
*/
image_t	*Draw_FindPic (char *name)
{
	image_t *img;
	char	fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf (fullname, sizeof(fullname), "pics/%s.pcx", name);
		img = Vk_FindImage (fullname, it_pic);
	}
	else
		img = Vk_FindImage (name+1, it_pic);

	return img;
}


/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize(int *w, int *h, char *pic)
{
	image_t *img;

	img = Draw_FindPic (pic);
	if (!img)
	{
		*w = *h = -1;
		return;
	}
	*w = img->width;
	*h = img->height;
}


/*
=============
Draw_StretchPic
=============
*/
void Draw_StretchPic(int x, int y, int w, int h, char *pic)
{
	image_t			*img;
	vkbuffer_t		*vb;
	vkdrawvert_t	v[4], *pv;
	
	img = Draw_FindPic (pic);
	if (!img)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	if (scrap_dirty)
		Scrap_Upload ();

	if (!img->has_alpha)
		;//qglDisable (GL_ALPHA_TEST);

	v[0].x = x;		v[0].y = y;
	v[0].s = 0.f;	v[0].t = 0.f;
	v[1].x = x + w;	v[1].y = y;
	v[1].s = 1.f;	v[1].t = 0.f;
	v[2].x = x;		v[2].y = y + h;
	v[2].s = 0.f;	v[2].t = 1.f;
	v[3].x = x + w;	v[3].y = y + h;
	v[3].s = 1.f;	v[3].t = 1.f;

	vb = &vk_draw.pics;
	if (vkMapMemory(vk_context.device, vb->memory, 
		sizeof(vkdrawvert_t) * vb->firstvertex, // offset
		sizeof(vkdrawvert_t) * 6, // chunk size
		0, (void **)&pv) == VK_SUCCESS)
	{
		*pv++ = v[0]; *pv++ = v[1]; *pv++ = v[2];
		*pv++ = v[2]; *pv++ = v[1]; *pv++ = v[3];
		vkUnmapMemory(vk_context.device, vb->memory);
	}

	vkCmdBindPipeline(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_draw.p_texture);
	vkCmdBindDescriptorSets(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline_layout, 
		0, 1, &vk_context.dset, 1, &s_dynamicoffset);
	vkCmdBindVertexBuffers(vk_context.cmdbuffer, 0, 1, &vb->buffer, &s_zero);
	vkCmdDraw(vk_context.cmdbuffer, 6, 1, vb->firstvertex, 0);

	if (!img->has_alpha)
		;//qglEnable (GL_ALPHA_TEST);

	vb->firstvertex += 6;
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic(int x, int y, char *pic)
{
	image_t		*img;
	int			w, h;
	vkbuffer_t	*vb;
	vkdrawvert_t v[4], *pv;
	
	img = Draw_FindPic (pic);
	if (!img)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}
	if (scrap_dirty)
		Scrap_Upload ();

	if (!img->has_alpha)
		;//qglDisable (GL_ALPHA_TEST);

	w = img->width;
	h = img->height;

	v[0].x = x;		v[0].y = y;
	v[0].s = 0.f;	v[0].t = 0.f;
	v[1].x = x + w;	v[1].y = y;
	v[1].s = 1.f;	v[1].t = 0.f;
	v[2].x = x;		v[2].y = y + h;
	v[2].s = 0.f;	v[2].t = 1.f;
	v[3].x = x + w;	v[3].y = y + h;
	v[3].s = 1.f;	v[3].t = 1.f;

	vb = &vk_draw.pics;
	if (vkMapMemory(vk_context.device, vb->memory, 
		sizeof(vkdrawvert_t) * vb->firstvertex, // offset
		sizeof(vkdrawvert_t) * 6, // chunk size
		0, (void *)&pv) == VK_SUCCESS)
	{
		*pv++ = v[0]; *pv++ = v[1]; *pv++ = v[2];
		*pv++ = v[2]; *pv++ = v[1]; *pv++ = v[3];
		vkUnmapMemory(vk_context.device, vb->memory);
	}

	vkCmdBindPipeline(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_draw.p_texture);
	vkCmdBindDescriptorSets(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline_layout, 
		0, 1, &vk_context.dset, 1, &s_dynamicoffset);
	vkCmdBindVertexBuffers(vk_context.cmdbuffer, 0, 1, &vb->buffer, &s_zero);
	vkCmdDraw(vk_context.cmdbuffer, 6, 1, vb->firstvertex, 0);

	if (!img->has_alpha)
		;//qglEnable (GL_ALPHA_TEST);

	vb->firstvertex += 6;
}


/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear(int x, int y, int w, int h, char *pic)
{
	image_t		*img;
	vkbuffer_t	*vb;
	vkdrawvert_t v[4], *pv;

	img = Draw_FindPic (pic);
	if (!img)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	//if (!img->has_alpha)
	//	qglDisable (GL_ALPHA_TEST);

	v[0].x = x;				v[0].y = y;
	v[0].s = x/64.f;		v[0].t = y/64.f;
	v[1].x = x + w;			v[1].y = y;
	v[1].s = (x + w)/64.f;	v[1].t = y/64.f;
	v[2].x = x;				v[2].y = y + h;
	v[2].s = x/64.f;		v[2].t = (y + h)/64.f;
	v[3].x = x + w;			v[3].y = y + h;
	v[3].s = (x + w)/64.f;	v[3].t = (y + h)/64.f;

	vb = &vk_draw.pics;
	if (vkMapMemory(vk_context.device, vb->memory, 
		sizeof(vkdrawvert_t) * vb->firstvertex, // offset
		sizeof(vkdrawvert_t) * 6, // chunk size
		0, (void **)&pv) == VK_SUCCESS)
	{
		*pv++ = v[0]; *pv++ = v[1]; *pv++ = v[2];
		*pv++ = v[2]; *pv++ = v[1]; *pv++ = v[3];
		vkUnmapMemory(vk_context.device, vb->memory);
	}

	vkCmdBindPipeline(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_draw.p_texture);
	vkCmdBindDescriptorSets(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline_layout, 
		0, 1, &vk_context.dset, 1, &s_dynamicoffset);
	vkCmdBindVertexBuffers(vk_context.cmdbuffer, 0, 1, &vb->buffer, &s_zero);
	vkCmdDraw(vk_context.cmdbuffer, 6, 1, vb->firstvertex, 0);

	if (!img->has_alpha)
		;//qglEnable (GL_ALPHA_TEST);

	vk_draw.pics.firstvertex += 6;
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill(int x, int y, int w, int h, int c)
{
	union
	{
		unsigned	c;
		byte		v[4];
	} color;

	float		fcolor[4];
	vkbuffer_t	*vb;
	vkdrawvert_t v[4];
	float		*pos;

	if ( (unsigned)c > 255)
		ri.Sys_Error (ERR_FATAL, "Draw_Fill: bad color");

	color.c = d_8to24table[c];

	fcolor[0] = color.v[0]/255.f;
	fcolor[1] = color.v[1]/255.f;
	fcolor[2] = color.v[2]/255.f;
	fcolor[3] = 1.f;

	vkCmdBindPipeline(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_draw.p_fill);
	vkCmdPushConstants(vk_context.cmdbuffer, vk_context.pipeline_layout, 
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
		0, sizeof(fcolor), fcolor);

	v[0].x = x;		v[0].y = y;
	v[1].x = x + w;	v[1].y = y;
	v[2].x = x;		v[2].y = y + h;
	v[3].x = x + w;	v[3].y = y + h;

	vb = &vk_draw.quads;
	if (vkMapMemory(vk_context.device, vb->memory, 
		sizeof(float) * 2 * vb->firstvertex, // offset
		sizeof(float) * 2 * 6, // chunk size
		0, (void **)&pos) == VK_SUCCESS)
	{
		*pos++ = v[0].x; *pos++ = v[0].y;
		*pos++ = v[1].x; *pos++ = v[1].y;
		*pos++ = v[2].x; *pos++ = v[2].y;
		*pos++ = v[2].x; *pos++ = v[2].y;
		*pos++ = v[1].x; *pos++ = v[1].y;
		*pos++ = v[3].x; *pos++ = v[3].y;
		vkUnmapMemory(vk_context.device, vb->memory);
	}

	vkCmdBindDescriptorSets(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline_layout, 
		0, 1, &vk_context.dset, 1, &s_dynamicoffset);
	vkCmdBindVertexBuffers(vk_context.cmdbuffer, 0, 1, &vb->buffer, &s_zero);
	vkCmdDraw(vk_context.cmdbuffer, 6, 1, vb->firstvertex, 0);

	vb->firstvertex += 6;
}


/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen(void)
{
	float		color[4];
	vkbuffer_t	*vb;
	vkdrawvert_t v[4];
	float		*pos;
	
	color[0] = 0.f;
	color[1] = 0.f;
	color[2] = 0.f;
	color[3] = 0.8f;
	
	vkCmdBindPipeline(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_draw.p_blend);
	vkCmdPushConstants(vk_context.cmdbuffer, vk_context.pipeline_layout, 
		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
		0, sizeof(color), color);

	v[0].x = 0.f;		v[0].y = 0.f;
	v[1].x = vid.width;	v[1].y = 0.f;
	v[2].x = 0.f;		v[2].y = vid.height;
	v[3].x = vid.width;	v[3].y = vid.height;

	vb = &vk_draw.quads;
	if (vkMapMemory(vk_context.device, vb->memory, 
		sizeof(float) * 2 * vb->firstvertex, // offset
		sizeof(float) * 2 * 6, // chunk size
		0, (void **)&pos) == VK_SUCCESS)
	{
		*pos++ = v[0].x; *pos++ = v[0].y;
		*pos++ = v[1].x; *pos++ = v[1].y;
		*pos++ = v[2].x; *pos++ = v[2].y;
		*pos++ = v[2].x; *pos++ = v[2].y;
		*pos++ = v[1].x; *pos++ = v[1].y;
		*pos++ = v[3].x; *pos++ = v[3].y;
		vkUnmapMemory(vk_context.device, vb->memory);
	}

	vkCmdBindDescriptorSets(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline_layout, 
		0, 1, &vk_context.dset, 1, &s_dynamicoffset);
	vkCmdBindVertexBuffers(vk_context.cmdbuffer, 0, 1, &vb->buffer, &s_zero);
	vkCmdDraw(vk_context.cmdbuffer, 6, 1, vb->firstvertex, 0);

	vb->firstvertex += 6;
}


/*
=============
Draw_StretchRaw

=============
*/
void Draw_StretchRaw(int x, int y, int w, int h, int cols, int rows, byte *data)
{
	vkbuffer_t	*vb;
	vkdrawvert_t v[4], *pv;
	
	v[0].x = x;		v[0].y = y;
	v[0].s = 0.f;	v[0].t = 0.f;
	v[1].x = x + w;	v[1].y = y;
	v[1].s = 1.f;	v[1].t = 0.f;
	v[2].x = x;		v[2].y = y + h;
	v[2].s = 0.f;	v[2].t = 1.f;
	v[3].x = x + w;	v[3].y = y + h;
	v[3].s = 1.f;	v[3].t = 1.f;

	vb = &vk_draw.pics;
	if (vkMapMemory(vk_context.device, vb->memory, 
		sizeof(vkdrawvert_t) * vb->firstvertex, // offset
		sizeof(vkdrawvert_t) * 6, // chunk size
		0, (void **)&pv) == VK_SUCCESS)
	{
		*pv++ = v[0]; *pv++ = v[1]; *pv++ = v[2];
		*pv++ = v[2]; *pv++ = v[1]; *pv++ = v[3];
		vkUnmapMemory(vk_context.device, vb->memory);
	}

	vkCmdBindPipeline(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_draw.p_texture);
	vkCmdBindDescriptorSets(vk_context.cmdbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_context.pipeline_layout, 
		0, 1, &vk_context.dset, 1, &s_dynamicoffset);
	vkCmdBindVertexBuffers(vk_context.cmdbuffer, 0, 1, &vb->buffer, &s_zero);
	vkCmdDraw(vk_context.cmdbuffer, 6, 1, vb->firstvertex, 0);

	vb->firstvertex += 6;
}
