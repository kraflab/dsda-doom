/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *---------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>

#include "gl_opengl.h"

#include "z_zone.h"
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#include <stdio.h>
#include <string.h>
#include <SDL.h>
#include "doomtype.h"
#include "w_wad.h"
#include "d_event.h"
#include "v_video.h"
#include "doomstat.h"
#include "r_bsp.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_sky.h"
#include "r_plane.h"
#include "r_data.h"
#include "p_maputl.h"
#include "p_tick.h"
#include "m_bbox.h"
#include "lprintf.h"
#include "gl_intern.h"
#include "gl_struct.h"
#include "p_spec.h"
#include "e6y.h"

#include "dsda/mapinfo.h"

int imageformats[5] = {0, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_RGB, GL_RGBA};

/* TEXTURES */
static GLTexture **gld_GLTextures=NULL;
/* PATCHES FLATS SPRITES */
static GLTexture **gld_GLPatchTextures=NULL;
static GLTexture **gld_GLStaticPatchTextures = NULL;
/* [XA] Indexed textures, for indexed lightmode */
static GLTexture **gld_GLIndexedTextures = NULL;
static GLTexture **gld_GLIndexedPatchTextures = NULL;
static GLTexture **gld_GLIndexedStaticPatchTextures = NULL;
/* [XA] Colormap textures, for indexed lightmode */
static GLTexture **gld_GLColormapTextures = NULL;
static GLTexture **gld_GLFullbrightColormapTextures = NULL; // yeah, a "fullbright colormap" is really just a palette, but it matches the name elsewhere...
int gld_numGLColormaps = -1;
int gld_paletteIndex = 0;
/* [XA] Sky textures for indexed lightmode -- name is sort
of a misnomer since the textures themselves aren't indexed,
but rather have the GL colormaps pre-applied, but meh. */
static GLTexture **gld_GLIndexedSkyTextures = NULL;

GLuint* last_glTexID = NULL;

void gld_ResetLastTexture(void)
{
  last_glTexID = NULL;
}

int gld_GetTexDimension(int value)
{
  if (value > gl_max_texture_size)
    value = gl_max_texture_size;

  return value;
}

// e6y
// Creates TIntDynArray
void* NewIntDynArray(int dimCount, int *dims)
{
  int i, dim;
  int tableOffset;
  int bufferSize = 0;
  int tableSize = 1;
  void* buffer;

  for(dim = 0; dim < dimCount - 1; dim++)
  {
    tableSize *= dims[dim];
    bufferSize += sizeof(int*) * tableSize;
  }

  bufferSize += sizeof(int) * tableSize * dims[dimCount - 1];

  buffer = Z_Calloc(1, bufferSize);
  if(!buffer)
  {
    return 0;
  }

  tableOffset = 0;
  tableSize = 1;

  for(dim = 0; dim < dimCount - 1; dim++)
  {
    tableSize *= dims[dim];
    tableOffset += tableSize;

    for(i = 0; i < tableSize; i++)
    {
      if(dim < dimCount - 2)
      {
        *((int***)buffer + tableOffset - tableSize + i) =
          (((int**)buffer + tableOffset + i*dims[dim + 1]));
      }
      else
      {
        *((int**)buffer + tableOffset - tableSize + i) =
          ((int*)((int**)buffer + tableOffset) + i*dims[dim + 1]);
      }
    }
  }

  return buffer;
}

#define INVULN_PLAYER_CM 8

// e6y
// Get index of player->fixedcolormap for GLTexture().glTexExID array
// There are 9 known values for player->fixedcolormap:
// 0 (normal) -> 0; 1 (pw_infrared) -> 1; 2..7 -> 2..7 (heretic torch flicker);
// 32 (pw_invulnerability) -> 8
static void gld_GetTextureTexID(GLTexture *gltexture, int cm)
{
  static int data[NUMCOLORMAPS+1] = {
     0,  1,  2,  3,  4,  5,  6,  7,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    INVULN_PLAYER_CM
  };
  int player_cm;
  int bcm = boom_cm;

  gltexture->cm = cm;
  gltexture->player_cm = player_cm = data[frame_fixedcolormap];
  assert(gltexture->player_cm != -1);

  if (gltexture->flags & GLTEXTURE_INDEXED)
  {
    // Collapse indices that share the same texture contents in indexed mode
    if (cm == CR_LIMIT)
      cm = CR_DEFAULT;
    if (player_cm != INVULN_PLAYER_CM)
      player_cm = 0;
    bcm = 0;
  }
  gltexture->texflags_p = &gltexture->texflags[cm][gltexture->player_cm];
  gltexture->texid_p = &gltexture->glTexExID[cm][gltexture->player_cm][bcm];
  return;
}

// e6y
// The common function for adding textures and patches
// Used by gld_AddNewGLTexture and gld_AddNewGLPatchTexture
static GLTexture *gld_AddNewGLTexItem(int num, int count, GLTexture ***items)
{
  if (num<0)
    return NULL;
  if (num>=count)
    return NULL;
  if (!(*items))
  {
    (*items)=Z_Calloc(count, sizeof(GLTexture *));
  }
  if (!(*items)[num])
  {
    (*items)[num]=Z_Calloc(1, sizeof(GLTexture));
    (*items)[num]->textype=GLDT_UNREGISTERED;

    {
      GLTexture *texture = (*items)[num];
      int dims[3] = {(CR_LIMIT+MAX_MAXPLAYERS), (PLAYERCOLORMAP_COUNT), numcolormaps};
      texture->glTexExID = NewIntDynArray(3, dims);
    }
  }
  return (*items)[num];
}

static GLTexture *gld_AddNewGLTexture(int texture_num, dboolean indexed)
{
  return gld_AddNewGLTexItem(texture_num, numtextures, indexed ? &gld_GLIndexedTextures : &gld_GLTextures);
}

static GLTexture *gld_AddNewGLPatchTexture(int lump, dboolean indexed)
{
  if (lumpinfo[lump].flags & LUMP_STATIC)
    return gld_AddNewGLTexItem(lump, numlumps, indexed ? &gld_GLIndexedStaticPatchTextures : &gld_GLStaticPatchTextures);
  else
    return gld_AddNewGLTexItem(lump, numlumps, indexed ? &gld_GLIndexedPatchTextures : &gld_GLPatchTextures);
}

static GLTexture *gld_AddNewGLColormapTexture(int palette_index, int gamma_level, dboolean fullbright)
{
  return gld_AddNewGLTexItem(palette_index + (gamma_level * V_GetPlaypalCount()), gld_numGLColormaps, fullbright ? &gld_GLFullbrightColormapTextures : &gld_GLColormapTextures);
}

// [XA] adds a memory-contiguous batch of sky textures,
// one for each GL colormap, and returns the first one
static GLTexture *gld_AddNewGLIndexedSkyTextures(int texture_num)
{
  int numIndexedSkyTextures = numtextures * gld_numGLColormaps;
  int baseTextureNum = texture_num * gld_numGLColormaps;
  int i;

  for (i = 0; i < gld_numGLColormaps; i++) {
    gld_AddNewGLTexItem(baseTextureNum + i, numIndexedSkyTextures, &gld_GLIndexedSkyTextures);
  }

  return gld_GLIndexedSkyTextures[baseTextureNum];
}

static GLTexture *gld_AddNewGLSkyTexture(int texture_num)
{
  int numIndexedSkyTextures = numtextures * gld_numGLColormaps;
  int baseTextureNum = texture_num * gld_numGLColormaps;

  gld_AddNewGLTexItem(baseTextureNum, numIndexedSkyTextures, &gld_GLIndexedSkyTextures);

  return gld_GLIndexedSkyTextures[baseTextureNum];
}

// [XA] returns a specific sky texture from a set.
static GLTexture *gld_GetGLIndexedSkyTexture(int texture_num, int index)
{
  if (texture_num < 0 || texture_num >= numtextures)
    return NULL;

  return gld_GLIndexedSkyTextures[texture_num * gld_numGLColormaps + index];
}

// Translate from base texture reference to sky texture
static GLTexture *gld_GetGLSkyTexture(GLTexture *basetexture)
{
  int index = 0;

  if (!basetexture)
    return NULL;

  // [XA] returns the indexed sky texture for the current palette & gamma level.
  index = gld_paletteIndex * NUM_GAMMA_LEVELS + usegamma;

  return gld_GetGLIndexedSkyTexture(basetexture->index, index);
}

void gld_SetIndexedPalette(int palette_index)
{
  gld_paletteIndex = palette_index;
}

static void gld_AddPatchToTexture_UnTranslated(GLTexture *gltexture, unsigned char *buffer, const rpatch_t *patch, int originx, int originy)
{
  int x,y,j;
  int xs,xe;
  int js,je;
  const rcolumn_t *column;
  const byte *source;
  int i, pos;
  int bpp;
  const unsigned char *playpal;
  const lighttable_t *colormap = (fixedcolormap ? fixedcolormap : fullcolormap);

  if (!gltexture)
    return;
  if (!patch)
    return;
  playpal = V_GetPlaypal();
  xs=0;
  xe=patch->width;
  if ((xs+originx)>=gltexture->realtexwidth)
    return;
  if ((xe+originx)<=0)
    return;
  if ((xs+originx)<0)
    xs=-originx;
  if ((xe+originx)>gltexture->realtexwidth)
    xe+=(gltexture->realtexwidth-(xe+originx));

  //e6y
  if (patch->flags&PATCH_HASHOLES)
    gltexture->flags |= GLTEXTURE_HASHOLES;

  bpp = gltexture->flags & GLTEXTURE_INDEXED ? 2 : 4;

  for (x=xs;x<xe;x++)
  {
#ifdef RANGECHECK
    if (x>=patch->width)
    {
      lprintf(LO_ERROR,"gld_AddPatchToTexture_UnTranslated x>=patch->width (%i >= %i)\n",x,patch->width);
      return;
    }
#endif
    column = &patch->columns[x];
    for (i=0; i<column->numPosts; i++) {
      const rpost_t *post = &column->posts[i];
      y=(post->topdelta+originy);
      js=0;
      je=post->length;
      if ((js+y)>=gltexture->realtexheight)
        continue;
      if ((je+y)<=0)
        continue;
      if ((js+y)<0)
        js=-y;
      if ((je+y)>gltexture->realtexheight)
        je+=(gltexture->realtexheight-(je+y));
      source = column->pixels + post->topdelta;
      pos=bpp*(((js+y)*gltexture->buffer_width)+x+originx);
      for (j=js;j<je;j++,pos+=(bpp*gltexture->buffer_width))
      {
#ifdef RANGECHECK
        if ((pos+bpp)>gltexture->buffer_size)
        {
          lprintf(LO_ERROR,"gld_AddPatchToTexture_UnTranslated pos+bpp>size (%i > %i)\n",pos+bpp,gltexture->buffer_size);
          return;
        }
#endif
        if (gltexture->flags & GLTEXTURE_INDEXED)
        {
          // [XA] new indexed color mode: store the palette index in
          // the R channel and get the colormap index in the shader.
          buffer[pos] = gltexture->player_cm == INVULN_PLAYER_CM ? colormap[source[j]] : source[j];
          buffer[pos+1] = 255;
        }
        else if (use_boom_cm && !(comp[comp_skymap] && (gltexture->flags&GLTEXTURE_SKY)))
        {
          //e6y: Boom's color maps
          buffer[pos+0]=playpal[colormap[source[j]]*3+0];
          buffer[pos+1]=playpal[colormap[source[j]]*3+1];
          buffer[pos+2]=playpal[colormap[source[j]]*3+2];
          buffer[pos+3]=255;
        }
        else
        {
          buffer[pos+0]=playpal[source[j]*3+0];
          buffer[pos+1]=playpal[source[j]*3+1];
          buffer[pos+2]=playpal[source[j]*3+2];
          buffer[pos+3]=255;
        }
      }
    }
  }
}

void gld_AddPatchToTexture(GLTexture *gltexture, unsigned char *buffer, const rpatch_t *patch, int originx, int originy, int cm)
{
  int x,y,j;
  int xs,xe;
  int js,je;
  const rcolumn_t *column;
  const byte *source;
  int i, pos;
  const unsigned char *playpal;
  const unsigned char *outr;
  const lighttable_t *colormap = (fixedcolormap ? fixedcolormap : fullcolormap);
  int bpp;

  if (!gltexture)
    return;
  if (!patch)
    return;
  if ((cm==CR_DEFAULT) || (cm==CR_LIMIT))
  {
    gld_AddPatchToTexture_UnTranslated(gltexture,buffer,patch,originx,originy);
    return;
  }
  if (cm<CR_LIMIT)
    outr=colrngs[cm];
  else
    outr=translationtables + 256*((cm-CR_LIMIT)-1);
  playpal = V_GetPlaypal();
  xs=0;
  xe=patch->width;
  if ((xs+originx)>=gltexture->realtexwidth)
    return;
  if ((xe+originx)<=0)
    return;
  if ((xs+originx)<0)
    xs=-originx;
  if ((xe+originx)>gltexture->realtexwidth)
    xe+=(gltexture->realtexwidth-(xe+originx));

  //e6y
  if (patch->flags&PATCH_HASHOLES)
    gltexture->flags |= GLTEXTURE_HASHOLES;

  bpp = gltexture->flags & GLTEXTURE_INDEXED ? 2 : 4;

  for (x=xs;x<xe;x++)
  {
#ifdef RANGECHECK
    if (x>=patch->width)
    {
      lprintf(LO_ERROR,"gld_AddPatchToTexture x>=patch->width (%i >= %i)\n",x,patch->width);
      return;
    }
#endif
    column = &patch->columns[x];
    for (i=0; i<column->numPosts; i++) {
      const rpost_t *post = &column->posts[i];
      y=(post->topdelta+originy);
      js=0;
      je=post->length;
      if ((js+y)>=gltexture->realtexheight)
        continue;
      if ((je+y)<=0)
        continue;
      if ((js+y)<0)
        js=-y;
      if ((je+y)>gltexture->realtexheight)
        je+=(gltexture->realtexheight-(je+y));
      source = column->pixels + post->topdelta;
      pos=bpp*(((js+y)*gltexture->buffer_width)+x+originx);
      for (j=js;j<je;j++,pos+=(bpp*gltexture->buffer_width))
      {
#ifdef RANGECHECK
        if ((pos+bpp)>gltexture->buffer_size)
        {
          lprintf(LO_ERROR,"gld_AddPatchToTexture pos+bpp>size (%i > %i)\n",pos+bpp,gltexture->buffer_size);
          return;
        }
#endif
        //e6y: Boom's color maps
        if (gltexture->flags & GLTEXTURE_INDEXED)
        {
          // [XA] new indexed color mode
          buffer[pos]=gltexture->player_cm == INVULN_PLAYER_CM ? colormap[outr[source[j]]] : outr[source[j]];
          buffer[pos+1]=255;
        }
        else if (use_boom_cm)
        {
          //e6y: Boom's color maps
          buffer[pos+0]=playpal[colormap[outr[source[j]]]*3+0];
          buffer[pos+1]=playpal[colormap[outr[source[j]]]*3+1];
          buffer[pos+2]=playpal[colormap[outr[source[j]]]*3+2];
          buffer[pos+3]=255;
        }
        else
        {
          buffer[pos+0]=playpal[outr[source[j]]*3+0];
          buffer[pos+1]=playpal[outr[source[j]]*3+1];
          buffer[pos+2]=playpal[outr[source[j]]*3+2];
          buffer[pos+3]=255;
        }
      }
    }
  }
}

static void gld_AddRawToTexture(GLTexture *gltexture, unsigned char *buffer, const unsigned char *raw)
{
  int x,y,w,pos,bpp;
  const unsigned char *playpal;
  const lighttable_t *colormap = (fixedcolormap ? fixedcolormap : fullcolormap);

  if (!gltexture)
    return;
  if (!raw)
    return;
  w = gltexture->realtexwidth;
  playpal = V_GetPlaypal();
  bpp = gltexture->flags & GLTEXTURE_INDEXED ? 2 : 4;
  for (y=0;y<gltexture->realtexheight;y++)
  {
    pos=bpp*(y*gltexture->buffer_width);
    for (x=0;x<gltexture->realtexwidth;x++,pos+=bpp)
    {
#ifdef RANGECHECK
      if ((pos+bpp)>=gltexture->buffer_size)
      {
        lprintf(LO_ERROR,"gld_AddRawToTexture pos+bpp>size (%i > %i)\n",pos+bpp,gltexture->buffer_size);
        return;
      }
#endif
      if (gltexture->flags & GLTEXTURE_INDEXED)
      {
        // [XA] new indexed color mode
        buffer[pos]=gltexture->player_cm == INVULN_PLAYER_CM ? colormap[raw[y*w+x]] : raw[y*w+x];
        buffer[pos+1] = 255;
      }
      else if (use_boom_cm)
      {
        //e6y: Boom's color maps
        buffer[pos+0]=playpal[colormap[raw[y*w+x]]*3+0];
        buffer[pos+1]=playpal[colormap[raw[y*w+x]]*3+1];
        buffer[pos+2]=playpal[colormap[raw[y*w+x]]*3+2];
        buffer[pos+3]=255;
      }
      else
      {
        buffer[pos+0]=playpal[raw[y*w+x]*3+0];
        buffer[pos+1]=playpal[raw[y*w+x]*3+1];
        buffer[pos+2]=playpal[raw[y*w+x]*3+2];
        buffer[pos+3]=255;
      }
    }
  }
}

static void gld_AddColormapToTexture(GLTexture *gltexture, unsigned char *buffer, int palette_index, int gamma_level, dboolean fullbright)
{
  int x,y,pos;
  const unsigned char *playpal;
  const lighttable_t *colormap;
  const byte * gtable;
  int gtlump;

  if (!gltexture)
    return;
  if (gltexture->realtexwidth<256)
    return;

  // figure out which palette variant to use
  // (e.g. normal, pain flash, item flash, etc).
  playpal = V_GetPlaypal() + (palette_index*PALETTE_SIZE);
  colormap = fullcolormap;

  // fallback in case the current colormap hasn't been set
  // yet; this occurs when rendering the main menu for the
  // first time if the game is launched in indexed lightmode
  if (colormap == NULL)
    colormap = colormaps[0];

  // also yoink the gamma table and apply
  // software gamma emulation to the texture.
  gtlump = W_CheckNumForName2("GAMMATBL", ns_prboom);
  gtable = (const byte*) W_LumpByNum(gtlump) + 256 * gamma_level;

  // construct a colormap texture using the selected
  // palette variant & gamma, for shader lookup.
  for (y=0;y<gltexture->realtexheight;y++)
  {
    pos=4*(y*gltexture->buffer_width);
    for (x=0;x<gltexture->realtexwidth;x++,pos+=4)
    {
#ifdef RANGECHECK
      if ((pos+3)>=gltexture->buffer_size)
      {
        lprintf(LO_ERROR,"gld_AddColormapToTexture pos+3>=size (%i >= %i)\n",pos+3,gltexture->buffer_size);
        return;
      }
#endif
      // [XA] also build a set of "fullbright" colormaps for UI.
      // this is needed because the first row of the colormap
      // isn't necessarily identical to the palette itself (e.g.
      // in Heretic it's slightly off for a few colors), and UI
      // needs to be drawn with exact palette colors, no lighting.
      if (fullbright)
      {
        buffer[pos+0]=gtable[playpal[x*3+0]];
        buffer[pos+1]=gtable[playpal[x*3+1]];
        buffer[pos+2]=gtable[playpal[x*3+2]];
        buffer[pos+3]=255;
      }
      else
      {
        buffer[pos+0]=gtable[playpal[colormap[y*256+x]*3+0]];
        buffer[pos+1]=gtable[playpal[colormap[y*256+x]*3+1]];
        buffer[pos+2]=gtable[playpal[colormap[y*256+x]*3+2]];
        buffer[pos+3]=255;
      }
    }
  }
}

// [XA] indexed lightmode sky support -- this basically creates
// a "pre-baked" version of the sky texture with the given
// palette (i.e. normal, pain flash, etc.) and gamma applied.
// the game selects the correct texture to use on the fly.
static void gld_AddIndexedSkyToTexture(GLTexture *gltexture, unsigned char *buffer, const rpatch_t *patch, int palette_index, int gamma_level)
{
  int x,y,j;
  int xs,xe;
  int js,je;
  const rcolumn_t *column;
  const byte *source;
  int i, pos;
  const unsigned char *playpal;
  const byte * gtable;
  int gtlump;

  if (!gltexture || !patch)
    return;

  // get palette & gamma table for the given args
  playpal = V_GetPlaypal() + (palette_index*PALETTE_SIZE);
  gtlump = W_CheckNumForName2("GAMMATBL", ns_prboom);
  gtable = (const byte*)W_LumpByNum(gtlump) + 256 * gamma_level;

  xs=0;
  xe=patch->width;
  if (xs>=gltexture->realtexwidth)
    return;
  if (xe<=0)
    return;
  if (xs<0)
    xs=0;
  if (xe>gltexture->realtexwidth)
    xe+=(gltexture->realtexwidth-xe);

  //e6y
  if (patch->flags&PATCH_HASHOLES)
    gltexture->flags |= GLTEXTURE_HASHOLES;

  for (x=xs;x<xe;x++)
  {
#ifdef RANGECHECK
    if (x>=patch->width)
    {
      lprintf(LO_ERROR,"gld_AddIndexedSkyToTexture x>=patch->width (%i >= %i)\n",x,patch->width);
      return;
    }
#endif
    column = &patch->columns[x];
    for (i=0; i<column->numPosts; i++) {
      const rpost_t *post = &column->posts[i];
      y=(post->topdelta);
      js=0;
      je=post->length;
      if ((js+y)>=gltexture->realtexheight)
        continue;
      if ((je+y)<=0)
        continue;
      if ((js+y)<0)
        js=-y;
      if ((je+y)>gltexture->realtexheight)
        je+=(gltexture->realtexheight-(je+y));
      source = column->pixels + post->topdelta;
      pos=4*(((js+y)*gltexture->buffer_width)+x);
      for (j=js;j<je;j++,pos+=(4*gltexture->buffer_width))
      {
#ifdef RANGECHECK
        if ((pos+3)>=gltexture->buffer_size)
        {
          lprintf(LO_ERROR,"gld_AddIndexedSkyToTexture pos+3>=size (%i >= %i)\n",pos+3,gltexture->buffer_size);
          return;
        }
#endif
        //e6y: Boom's color maps
        if (use_boom_cm && !comp[comp_skymap])
        {
          const lighttable_t *colormap = (fixedcolormap ? fixedcolormap : fullcolormap);
          buffer[pos+0]=gtable[playpal[colormap[source[j]]*3+0]];
          buffer[pos+1]=gtable[playpal[colormap[source[j]]*3+1]];
          buffer[pos+2]=gtable[playpal[colormap[source[j]]*3+2]];
        }
        else
        {
          buffer[pos+0]=gtable[playpal[source[j]*3+0]];
          buffer[pos+1]=gtable[playpal[source[j]*3+1]];
          buffer[pos+2]=gtable[playpal[source[j]*3+2]];
        }
        buffer[pos+3]=255;
      }
    }
  }
}

static GLTexture *gld_InitUnregisteredTexture(int texture_num, GLTexture *gltexture, dboolean indexed, dboolean sky)
{
  texture_t *texture = NULL;

  if (texture_num >= 0 || texture_num < numtextures)
    texture = textures[texture_num];

  if (!texture)
    return NULL;

  gltexture->textype = GLDT_BROKEN;
  gltexture->index = texture_num;

  //e6y
  gltexture->flags = 0;

  if (indexed && !sky)
    gltexture->flags |= GLTEXTURE_INDEXED;

  gltexture->realtexwidth = texture->width;
  gltexture->realtexheight = texture->height;

  if (sky)
  {
    const rpatch_t *patch;

    patch = R_HackedSkyPatch(texture);

    if (patch)
    {
      gltexture->patch_index = texture->patches[0].patch;
      gltexture->flags |= GLTEXTURE_SKYHACK;
      gltexture->realtexheight=patch->height;
    }
  }

  gltexture->leftoffset = 0;
  gltexture->topoffset = 0;
  gltexture->tex_width = gld_GetTexDimension(gltexture->realtexwidth);
  gltexture->tex_height = gld_GetTexDimension(gltexture->realtexheight);
  gltexture->width = MIN(gltexture->realtexwidth, gltexture->tex_width);
  gltexture->height = MIN(gltexture->realtexheight, gltexture->tex_height);
  gltexture->buffer_width = gltexture->tex_width;
  gltexture->buffer_height = gltexture->tex_height;
  gltexture->width = gltexture->tex_width;
  gltexture->height = gltexture->tex_height;
  gltexture->buffer_width = gltexture->realtexwidth;
  gltexture->buffer_height = gltexture->realtexheight;

  //e6y: right/bottom UV coordinates for texture drawing
  gltexture->scalexfac = (float) gltexture->width / (float) gltexture->tex_width;
  gltexture->scaleyfac = (float) gltexture->height / (float) gltexture->tex_height;

  gltexture->buffer_size = gltexture->buffer_width * gltexture->buffer_height * ((indexed && !sky) ? 2 : 4);

  if (gltexture->realtexwidth > gltexture->buffer_width)
    return gltexture;

  if (gltexture->realtexheight > gltexture->buffer_height)
    return gltexture;

  gltexture->textype = GLDT_TEXTURE;

  return gltexture;
}

//e6y: "force" flag for loading texture with zero index
// [XA]  "indexed" flag for new indexed lightmode
GLTexture *gld_RegisterTexture(int texture_num, dboolean mipmap, dboolean force, dboolean indexed, dboolean sky)
{
  GLTexture *gltexture;

  //e6y: textures with zero index should be loaded sometimes
  if (texture_num == NO_TEXTURE && !force)
    return NULL;

  if (sky)
    gltexture = gld_AddNewGLSkyTexture(texture_num);
  else
    gltexture = gld_AddNewGLTexture(texture_num, indexed);

  if (!gltexture)
    return NULL;

  if (gltexture->textype == GLDT_UNREGISTERED)
    gltexture = gld_InitUnregisteredTexture(texture_num, gltexture, indexed, sky);

  return gltexture;
}

unsigned char* gld_GetTextureBuffer(GLuint texid, int miplevel, int *width, int *height)
{
  int w, h;
  static unsigned char *buf = NULL;
  static int buf_size = 512 * 256 * 4;

  if (!buf)
  {
    buf = Z_Malloc(buf_size);
  }

  if (texid)
  {
    glBindTexture(GL_TEXTURE_2D, texid);
  }

  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
  if (w * h * 4 > buf_size)
  {
    Z_Free(buf);
    buf_size = w * h * 4;
    buf = Z_Malloc(buf_size);
  }
  glGetTexImage(GL_TEXTURE_2D, miplevel, GL_RGBA, GL_UNSIGNED_BYTE, buf);

  if (width)
    *width = w;
  if (height)
    *height = h;

  return buf;
}

void gld_SetTexFilters(GLTexture *gltexture)
{
  float aniso_filter = 0.0f;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  if (aniso_filter > 0.0f)
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso_filter);
}

void gld_SetTexClamp(GLTexture *gltexture, unsigned int flags)
{
  //if ((gltexture->flags & GLTEXTURE_CLAMPXY) != (flags & GLTEXTURE_CLAMPXY))
  /* sp1n0za 05/2010: simplify */
  if ((*gltexture->texflags_p ^ flags) & GLTEXTURE_CLAMPXY)
  {
    int need_clamp_x = (flags & GLTEXTURE_CLAMPX);
    int need_clamp_y = (flags & GLTEXTURE_CLAMPY);
    int has_clamp_x = (*gltexture->texflags_p & GLTEXTURE_CLAMPX);
    int has_clamp_y = (*gltexture->texflags_p & GLTEXTURE_CLAMPY);

    if (need_clamp_x)
    {
      if (!has_clamp_x)
      {
        *gltexture->texflags_p |= GLTEXTURE_CLAMPX;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      }
    }
    else
    {
      if (has_clamp_x)
      {
        *gltexture->texflags_p &= ~GLTEXTURE_CLAMPX;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      }
    }

    if (need_clamp_y)
    {
      if (!has_clamp_y)
      {
        *gltexture->texflags_p |= GLTEXTURE_CLAMPY;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      }
    }
    else
    {
      if (has_clamp_y)
      {
        *gltexture->texflags_p &= ~GLTEXTURE_CLAMPY;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      }
    }
  }
}

int gld_BuildTexture(GLTexture *gltexture, void *data, dboolean readonly, int width, int height)
{
  int tex_width, tex_height, tex_format;

  tex_format = gltexture->flags & GLTEXTURE_INDEXED ? GL_RG : GL_RGBA;
  tex_width  = gld_GetTexDimension(width);
  tex_height = gld_GetTexDimension(height);

  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

  glTexImage2D( GL_TEXTURE_2D, 0, tex_format,
    tex_width, tex_height,
    0, tex_format, GL_UNSIGNED_BYTE, data);

  gld_SetTexFilters(gltexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  if (!readonly)
  {
    Z_Free(data);
    data = NULL;
  }

  return true;
}

void gld_BindTexture(GLTexture *gltexture, unsigned int flags, dboolean sky)
{
  const rpatch_t *patch;
  unsigned char *buffer;

  // resolve the actual texture for the related sky
  if (sky)
  {
    gltexture = gld_GetGLSkyTexture(gltexture);
  }

  if (!gltexture || gltexture->textype != GLDT_TEXTURE)
  {
    glBindTexture(GL_TEXTURE_2D, 0);
    last_glTexID = NULL;
    return;
  }

  gld_GetTextureTexID(gltexture, CR_DEFAULT);

  if (last_glTexID == gltexture->texid_p)
  {
    gld_SetTexClamp(gltexture, flags);
    return;
  }

  last_glTexID = gltexture->texid_p;

  if (*gltexture->texid_p != 0)
  {
    glBindTexture(GL_TEXTURE_2D, *gltexture->texid_p);
    gld_SetTexClamp(gltexture, flags);
    glsl_SetTextureDims(0, gltexture->realtexwidth, gltexture->realtexheight);
    return;
  }

  buffer=(unsigned char*)Z_Malloc(gltexture->buffer_size);
  memset(buffer,0,gltexture->buffer_size);

  if (gltexture->flags & GLTEXTURE_SKYHACK)
  {
    patch=R_PatchByNum(gltexture->patch_index);
  }
  else
  {
    patch=R_TextureCompositePatchByNum(gltexture->index);
  }

  if (sky)
  {
    gld_AddIndexedSkyToTexture(gltexture, buffer, patch, gld_paletteIndex, usegamma);
  }
  else
  {
    gld_AddPatchToTexture(gltexture, buffer, patch, 0, 0, CR_DEFAULT);
  }

  if (*gltexture->texid_p == 0)
    glGenTextures(1, gltexture->texid_p);
  glBindTexture(GL_TEXTURE_2D, *gltexture->texid_p);

  gld_BuildTexture(gltexture, buffer, false, gltexture->buffer_width, gltexture->buffer_height);

  gld_SetTexClamp(gltexture, flags);
  glsl_SetTextureDims(0, gltexture->realtexwidth, gltexture->realtexheight);
}

GLTexture *gld_RegisterPatch(int lump, int cm, dboolean is_sprite, dboolean indexed)
{
  const rpatch_t *patch;
  GLTexture *gltexture;

  gltexture=gld_AddNewGLPatchTexture(lump, indexed);
  if (!gltexture)
    return NULL;
  if (gltexture->textype==GLDT_UNREGISTERED)
  {
    patch=R_PatchByNum(lump);
    if (!patch)
      return NULL;
    gltexture->textype=GLDT_BROKEN;
    gltexture->index=lump;

    //e6y
    gltexture->flags = 0;
    if (is_sprite)
    {
      gltexture->flags |= GLTEXTURE_SPRITE;
    }

    if (indexed)
      gltexture->flags |= GLTEXTURE_INDEXED;

    gltexture->realtexwidth=patch->width;
    gltexture->realtexheight=patch->height;
    gltexture->leftoffset=patch->leftoffset;
    gltexture->topoffset=patch->topoffset;
    gltexture->tex_width=gld_GetTexDimension(gltexture->realtexwidth);
    gltexture->tex_height=gld_GetTexDimension(gltexture->realtexheight);
    gltexture->width=MIN(gltexture->realtexwidth, gltexture->tex_width);
    gltexture->height=MIN(gltexture->realtexheight, gltexture->tex_height);
    gltexture->buffer_width=gltexture->tex_width;
    gltexture->buffer_height=gltexture->tex_height;
    gltexture->width=MIN(gltexture->realtexwidth, gltexture->tex_width);
    gltexture->height=MIN(gltexture->realtexheight, gltexture->tex_height);
    gltexture->buffer_width=MAX(gltexture->realtexwidth, gltexture->tex_width);
    gltexture->buffer_height=MAX(gltexture->realtexheight, gltexture->tex_height);

    //e6y: right/bottom UV coordinates for patch drawing
    gltexture->scalexfac=(float)gltexture->width/(float)gltexture->tex_width;
    gltexture->scaleyfac=(float)gltexture->height/(float)gltexture->tex_height;

    gltexture->buffer_size=gltexture->buffer_width*gltexture->buffer_height*(indexed ? 2 : 4);
    if (gltexture->realtexwidth>gltexture->buffer_width)
      return gltexture;
    if (gltexture->realtexheight>gltexture->buffer_height)
      return gltexture;
    gltexture->textype=GLDT_PATCH;
  }
  return gltexture;
}

void gld_BindPatch(GLTexture *gltexture, int cm)
{
  const rpatch_t *patch;
  unsigned char *buffer;

  if (!gltexture || gltexture->textype != GLDT_PATCH)
  {
    glBindTexture(GL_TEXTURE_2D, 0);
    last_glTexID = NULL;
    return;
  }

  gld_GetTextureTexID(gltexture, cm);

  if (last_glTexID == gltexture->texid_p)
  {
    gld_SetTexClamp(gltexture, GLTEXTURE_CLAMPXY);
    return;
  }

  last_glTexID = gltexture->texid_p;

  if (*gltexture->texid_p != 0)
  {
    glBindTexture(GL_TEXTURE_2D, *gltexture->texid_p);
    gld_SetTexClamp(gltexture, GLTEXTURE_CLAMPXY);
    glsl_SetTextureDims(0, gltexture->realtexwidth, gltexture->realtexheight);
    return;
  }

  patch=R_PatchByNum(gltexture->index);
  buffer=(unsigned char*)Z_Malloc(gltexture->buffer_size);
  memset(buffer,0,gltexture->buffer_size);
  gld_AddPatchToTexture(gltexture, buffer, patch, 0, 0, cm);

  if (*gltexture->texid_p == 0)
    glGenTextures(1, gltexture->texid_p);
  glBindTexture(GL_TEXTURE_2D, *gltexture->texid_p);

  gld_BuildTexture(gltexture, buffer, false, gltexture->buffer_width, gltexture->buffer_height);

  gld_SetTexClamp(gltexture, GLTEXTURE_CLAMPXY);
  glsl_SetTextureDims(0, gltexture->realtexwidth, gltexture->realtexheight);
}

GLTexture *gld_RegisterRaw(int lump, int width, int height, dboolean mipmap, dboolean indexed)
{
  GLTexture *gltexture;

  gltexture=gld_AddNewGLPatchTexture(lump, indexed);
  if (!gltexture)
    return NULL;
  if (gltexture->textype==GLDT_UNREGISTERED)
  {
    gltexture->textype=GLDT_BROKEN;
    gltexture->index=lump;

    //e6y
    gltexture->flags = 0;

    if (indexed)
      gltexture->flags |= GLTEXTURE_INDEXED;

    gltexture->realtexwidth= width;
    gltexture->realtexheight= height;
    gltexture->leftoffset=0;
    gltexture->topoffset=0;
    gltexture->tex_width=gld_GetTexDimension(gltexture->realtexwidth);
    gltexture->tex_height=gld_GetTexDimension(gltexture->realtexheight);
    gltexture->width=MIN(gltexture->realtexwidth, gltexture->tex_width);
    gltexture->height=MIN(gltexture->realtexheight, gltexture->tex_height);
    gltexture->buffer_width=gltexture->tex_width;
    gltexture->buffer_height=gltexture->tex_height;
    gltexture->width=gltexture->tex_width;
    gltexture->height=gltexture->tex_height;
    gltexture->buffer_width=gltexture->realtexwidth;
    gltexture->buffer_height=gltexture->realtexheight;

    //e6y: right/bottom UV coordinates for flat drawing
    gltexture->scalexfac=(float)gltexture->width/(float)gltexture->tex_width;
    gltexture->scaleyfac=(float)gltexture->height/(float)gltexture->tex_height;

    gltexture->buffer_size=gltexture->buffer_width*gltexture->buffer_height*(indexed ? 2 : 4);
    if (gltexture->realtexwidth>gltexture->buffer_width)
      return gltexture;
    if (gltexture->realtexheight>gltexture->buffer_height)
      return gltexture;

    gltexture->textype=GLDT_FLAT;
  }
  return gltexture;
}

void gld_BindRaw(GLTexture *gltexture, unsigned int flags)
{
  const unsigned char *raw;
  unsigned char *buffer;

  if (!gltexture || gltexture->textype != GLDT_FLAT)
  {
    glBindTexture(GL_TEXTURE_2D, 0);
    last_glTexID = NULL;
    return;
  }

  gld_GetTextureTexID(gltexture, CR_DEFAULT);

  if (last_glTexID == gltexture->texid_p)
  {
    gld_SetTexClamp(gltexture, flags);
    return;
  }

  last_glTexID = gltexture->texid_p;

  if (*gltexture->texid_p != 0)
  {
    glBindTexture(GL_TEXTURE_2D, *gltexture->texid_p);
    gld_SetTexClamp(gltexture, flags);
    glsl_SetTextureDims(0, gltexture->realtexwidth, gltexture->realtexheight);
    return;
  }

  raw=W_LumpByNum(gltexture->index);
  buffer=(unsigned char*)Z_Malloc(gltexture->buffer_size);
  memset(buffer,0,gltexture->buffer_size);
  gld_AddRawToTexture(gltexture, buffer, raw);
  if (*gltexture->texid_p == 0)
    glGenTextures(1, gltexture->texid_p);
  glBindTexture(GL_TEXTURE_2D, *gltexture->texid_p);

  gld_BuildTexture(gltexture, buffer, false, gltexture->buffer_width, gltexture->buffer_height);

  gld_SetTexClamp(gltexture, flags);
  glsl_SetTextureDims(0, gltexture->realtexwidth, gltexture->realtexheight);
}


// [XA] new functions for registering & binding sky textures.
// for truecolor lightmodes, these just call into the regular
// RegisterTexture function; for indexed lightmode, there's
// a special set of sky textures with palettes pre-applied
// that gets selected based on current palette, and we need
// to generate & bind those here.
// NOTE: since these only differer in behavior in indexed
// lightmode, and indexed lightmode enforces gld_DrawStripsSky,
// only that sky function bothers to call these, because lazy. ;)
GLTexture *gld_RegisterSkyTexture(int texture_num, dboolean force)
{
  GLTexture *basetexture;
  GLTexture *gltexture;
  int i;

  if (texture_num == NO_TEXTURE && !force)
    return NULL;

  basetexture = gld_AddNewGLIndexedSkyTextures(texture_num);
  if (!basetexture)
    return NULL;

  // [XA] initialize all textures in the set.
  for(i = 0; i < gld_numGLColormaps; i++)
  {
    gltexture = gld_GetGLIndexedSkyTexture(texture_num, i);
    if (gltexture->textype == GLDT_UNREGISTERED)
      gld_InitUnregisteredTexture(texture_num, gltexture, true, true);
  }

  return basetexture;
}

void gld_BindSkyTexture(GLTexture *gltexture)
{
  gld_BindTexture(gltexture, 0, true);
}

GLTexture *gld_RegisterColormapTexture(int palette_index, int gamma_level, dboolean fullbright)
{
  GLTexture *gltexture;

  gltexture=gld_AddNewGLColormapTexture(palette_index, gamma_level, fullbright);
  if (!gltexture)
    return NULL;
  if (gltexture->textype==GLDT_UNREGISTERED)
  {
    gltexture->textype=GLDT_BROKEN;
    gltexture->index=palette_index;

    gltexture->realtexwidth=256;
    gltexture->realtexheight=(fullbright ? 1 : NUMCOLORMAPS+1); // it's +1 'cause of the invuln palette
    gltexture->leftoffset=0;
    gltexture->topoffset=0;
    gltexture->tex_width=gld_GetTexDimension(gltexture->realtexwidth);
    gltexture->tex_height=gld_GetTexDimension(gltexture->realtexheight);
    gltexture->width=MIN(gltexture->realtexwidth, gltexture->tex_width);
    gltexture->height=MIN(gltexture->realtexheight, gltexture->tex_height);
    gltexture->buffer_width=gltexture->tex_width;
    gltexture->buffer_height=gltexture->tex_height;

    //e6y: right/bottom UV coordinates for patch drawing
    gltexture->scalexfac=(float)gltexture->width/(float)gltexture->tex_width;
    gltexture->scaleyfac=(float)gltexture->height/(float)gltexture->tex_height;

    gltexture->buffer_size=gltexture->buffer_width*gltexture->buffer_height*4;

    if (gltexture->realtexwidth>gltexture->buffer_width)
      return gltexture;
    if (gltexture->realtexheight>gltexture->buffer_height)
      return gltexture;
    gltexture->textype=GLDT_COLORMAP;
  }
  return gltexture;
}

void gld_BindColormapTexture(GLTexture *gltexture, int palette_index, int gamma_level, dboolean fullbright)
{
  unsigned char *buffer;

  // abort if we're trying to bind the wrong texture type
  if (!gltexture || gltexture->textype != GLDT_COLORMAP)
  {
    glBindTexture(GL_TEXTURE_2D, 0);
    last_glTexID = NULL;
    return;
  }

  // 'push' the active texture -- use texture2 for
  // colormaps since texture1 is already in use.
  GLEXT_glActiveTextureARB(GL_TEXTURE2_ARB);

  gld_GetTextureTexID(gltexture, CR_DEFAULT);

  if (last_glTexID == gltexture->texid_p)
  {
    GLEXT_glActiveTextureARB(GL_TEXTURE0_ARB);
    return;
  }
  last_glTexID = gltexture->texid_p;

  // texure data already initialized; use it
  if (*gltexture->texid_p != 0)
  {
    glBindTexture(GL_TEXTURE_2D, *gltexture->texid_p);
    GLEXT_glActiveTextureARB(GL_TEXTURE0_ARB);
    glsl_SetTextureDims(2, gltexture->realtexwidth, gltexture->realtexheight);
    return;
  }

  // collect the actual texture data
  buffer = (unsigned char*) Z_Malloc(gltexture->buffer_size);
  memset(buffer, 0, gltexture->buffer_size);

  gld_AddColormapToTexture(gltexture, buffer, palette_index, gamma_level, fullbright);

  // bind it, finally :P
  if (*gltexture->texid_p == 0)
    glGenTextures(1, gltexture->texid_p);
  glBindTexture(GL_TEXTURE_2D, *gltexture->texid_p);

  gld_BuildTexture(gltexture, buffer, false, gltexture->buffer_width, gltexture->buffer_height);

  // 'pop' the active texture back to the default.
  GLEXT_glActiveTextureARB(GL_TEXTURE0_ARB);
  glsl_SetTextureDims(2, gltexture->realtexwidth, gltexture->realtexheight);
}

void gld_InitColormapTextures(dboolean fullbright)
{
  GLTexture *gltexture;

  // figure out how many palette variants are in the
  // PLAYPAL lump and create a colormap texture for
  // each palette at each gamma level. word.
  gld_numGLColormaps = V_GetPlaypalCount() * NUM_GAMMA_LEVELS;

  // abort if we're trying to show an out-of-bounds palette.
  if (gld_paletteIndex < 0 || gld_paletteIndex >= gld_numGLColormaps)
    return;

  // lazy-init and bind the colormap texture for
  // the current palette index. since colormaps
  // won't change during the frame, we can go
  // ahead and bind them now and be done with it.
  gltexture = gld_RegisterColormapTexture(gld_paletteIndex, usegamma, fullbright);
  if (gltexture)
    gld_BindColormapTexture(gltexture, gld_paletteIndex, usegamma, fullbright);
}

// Darkness values for 1 to 5 iterations of fuzz darkening, derived from
// dcolors.c by lovely847
#define FUZZ1 0.188235294117647058824
#define FUZZ2 0.341176470588235294118
#define FUZZ3 0.462745098039215686275
#define FUZZ4 0.564705882352941176471
#define FUZZ5 0.647058823529411764706

// Darkness values for each fuzz pixel in a column, derived from software fuzz
// offset table by counting consecutive negative offsets
static const float fuzz[50] =
{
  FUZZ1, FUZZ2, FUZZ1, FUZZ2, FUZZ1, FUZZ1, FUZZ2, FUZZ1, FUZZ1, FUZZ2,
  FUZZ1, FUZZ1, FUZZ1, FUZZ2, FUZZ1, FUZZ1, FUZZ1, FUZZ2, FUZZ3, FUZZ4,
  FUZZ5, FUZZ1, FUZZ2, FUZZ3, FUZZ1, FUZZ1, FUZZ1, FUZZ1, FUZZ2, FUZZ1,
  FUZZ2, FUZZ1, FUZZ1, FUZZ2, FUZZ3, FUZZ1, FUZZ1, FUZZ2, FUZZ3, FUZZ4,
  FUZZ5, FUZZ1, FUZZ1, FUZZ1, FUZZ1, FUZZ2, FUZZ1, FUZZ1, FUZZ2, FUZZ1
};

static GLuint fuzz_texid = 0;

void gld_InitFuzzTexture(void)
{
  if (fuzz_texid == 0)
  {
    GLEXT_glActiveTextureARB(GL_TEXTURE1_ARB);

    glGenTextures(1, &fuzz_texid);
    glBindTexture(GL_TEXTURE_2D, fuzz_texid);

    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                 sizeof(fuzz) / sizeof(*fuzz), 1, 0, GL_RED,
                 GL_FLOAT, fuzz);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLEXT_glActiveTextureARB(GL_TEXTURE0_ARB);

    glsl_SetTextureDims(1, sizeof(fuzz) / sizeof(*fuzz), 1);
  }
}

// e6y
// The common function for cleaning textures and patches
// gld_CleanTextures and gld_CleanPatchTextures are replaced with that
static void gld_CleanTexItems(int count, GLTexture ***items)
{
  int i, j;

  if (!(*items))
    return;

  for (i=0; i<count; i++)
  {
    if ((*items)[i])
    {
      int cm, n;
      for (j=0; j<(CR_LIMIT+MAX_MAXPLAYERS); j++)
      {
        for (n=0; n<PLAYERCOLORMAP_COUNT; n++)
        {
          for (cm=0; cm<numcolormaps; cm++)
          {
            if ((*items) && (*items)[i]->glTexExID[j][n][cm])
            {
              glDeleteTextures(1,&((*items)[i]->glTexExID[j][n][cm]));
            }
          }
        }
      }

      Z_Free((*items)[i]->glTexExID);
      (*items)[i]->glTexExID = NULL;

      Z_Free((*items)[i]);
    }
  }
  memset((*items),0,count*sizeof(GLTexture *));
}

void gld_FlushTextures(void)
{
  gld_CleanTexItems(numtextures, &gld_GLTextures);
  gld_CleanTexItems(numlumps, &gld_GLPatchTextures);
  gld_CleanTexItems(numlumps, &gld_GLStaticPatchTextures);
  gld_CleanTexItems(numtextures, &gld_GLIndexedTextures);
  gld_CleanTexItems(numlumps, &gld_GLIndexedPatchTextures);
  gld_CleanTexItems(numlumps, &gld_GLIndexedStaticPatchTextures);
  gld_CleanTexItems(gld_numGLColormaps, &gld_GLColormapTextures);
  gld_CleanTexItems(gld_numGLColormaps, &gld_GLFullbrightColormapTextures);
  gld_CleanTexItems(numtextures * gld_numGLColormaps, &gld_GLIndexedSkyTextures);

  if (fuzz_texid)
  {
    glDeleteTextures(1, &fuzz_texid);
    fuzz_texid = 0;
  }

  gld_ResetLastTexture();

  gld_InitSky();
  gld_InitColormapTextures(V_IsUILightmodeIndexed() || V_IsAutomapLightmodeIndexed());

  // do not draw anything in current frame after flushing
  gld_ResetDrawInfo();
}

static void CalcHitsCount(const byte *hitlist, int size, int *hit, int*hitcount)
{
  int i;

  if (!hitlist || !hit || !hitcount)
    return;

  *hit = 0;
  *hitcount = 0;
  for (i = 0; i < size; i++)
  {
    if (hitlist[i])
      (*hitcount)++;
  }
}

void gld_Precache(void)
{
  int i;
  byte *hitlist;
  int hit, hitcount = 0;
  GLTexture *gltexture;

  unsigned int tics = SDL_GetTicks();

  if (nodrawers)
    return;

  if (timingdemo)
    return;

  gld_ProgressStart();

  {
    size_t size = numflats > num_sprites  ? numflats : num_sprites;
    hitlist = Z_Malloc((size_t)numtextures > size ? (size_t)numtextures : size);
  }

  // Precache flats.

  memset(hitlist, 0, numflats);

  for (i = numsectors; --i >= 0; )
  {
    int j,k;

    int floorpic = sectors[i].floorpic;
    int ceilingpic = sectors[i].ceilingpic;

    anim_t *flatanims[2] = {
      anim_flats[floorpic].anim,
      anim_flats[ceilingpic].anim
    };

    hitlist[floorpic] = hitlist[ceilingpic] = 1;

    //e6y: animated flats
    for (k = 0; k < 2; k++)
    {
      if (flatanims[k] && !flatanims[k]->istexture)
      {
        for (j = flatanims[k]->basepic; j < flatanims[k]->basepic + flatanims[k]->numpics; j++)
          hitlist[j] = 1;
      }
    }
  }

  CalcHitsCount(hitlist, numflats, &hit, &hitcount);

  for (i = numflats; --i >= 0; )
    if (hitlist[i])
    {
      gld_ProgressUpdate("Loading Flats...", ++hit, hitcount);
      gltexture = gld_RegisterFlat(i, true, true);
      if (gltexture)
      {
        gld_BindFlat(gltexture, 0);
      }
    }

  // Precache textures.

  memset(hitlist, 0, numtextures);

  for (i = numsides; --i >= 0;)
  {
    int j, k;
    int bottomtexture, toptexture, midtexture;
    anim_t *textureanims[3];

    bottomtexture = sides[i].bottomtexture;
    toptexture = sides[i].toptexture;
    midtexture = sides[i].midtexture;

    textureanims[0] = anim_textures[bottomtexture].anim;
    textureanims[1] = anim_textures[toptexture].anim;
    textureanims[2] = anim_textures[midtexture].anim;

    hitlist[bottomtexture] =
      hitlist[toptexture] =
      hitlist[midtexture] = 1;

    //e6y: animated textures
    for (k = 0; k < 3; k++)
    {
      if (textureanims[k] && textureanims[k]->istexture)
      {
        for (j = textureanims[k]->basepic; j < textureanims[k]->basepic + textureanims[k]->numpics; j++)
          hitlist[j] = 1;
      }
    }

    //e6y: swithes
    {
      int GetPairForSwitchTexture(side_t *side);
      int pair = GetPairForSwitchTexture(&sides[i]);
      if (pair != -1)
        hitlist[pair] = 1;
    }
  }

  // Sky texture is always present.
  // Note that F_SKY1 is the name used to
  //  indicate a sky floor/ceiling as a flat,
  //  while the sky texture is stored like
  //  a wall texture, with an episode dependend
  //  name.

  if (hitlist)
    hitlist[skytexture] = 0;

  CalcHitsCount(hitlist, numtextures, &hit, &hitcount);

  for (i = numtextures; --i >= 0; )
    if (hitlist[i])
    {
      gld_ProgressUpdate("Loading Textures...", ++hit, hitcount);
      gltexture = gld_RegisterTexture(i, i != skytexture, false, true, false);
      if (gltexture)
      {
        gld_BindTexture(gltexture, 0, false);
      }
    }

  // Precache sprites.
  memset(hitlist, 0, num_sprites);

  if (thinkercap.next)
  {
    thinker_t *th;
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
      if (th->function == P_MobjThinker)
        hitlist[((mobj_t *)th)->sprite] = 1;
    }
  }

  hit = 0;
  hitcount = 0;
  for (i = 0; i < num_sprites; i++)
  {
    if (hitlist[i])
      hitcount += 7 * sprites[i].numframes;
  }

  for (i=num_sprites; --i >= 0;)
    if (hitlist[i])
      {
        int j = sprites[i].numframes;
        while (--j >= 0)
          {
            short *sflump = sprites[i].spriteframes[j].lump;
            int k = 7;
            do
            {
              gld_ProgressUpdate("Loading Sprites...", ++hit, hitcount);
              gltexture = gld_RegisterPatch(firstspritelump + sflump[k], CR_LIMIT, true, true);
              if (gltexture)
              {
                gld_BindPatch(gltexture, CR_LIMIT);
              }
            }
            while (--k >= 0);
          }
      }
  Z_Free(hitlist);

  gld_ProgressEnd();

  gld_InitFBO();

  // e6y: some statistics
  {
    char map[9];

    snprintf(map, sizeof(map), "%s", dsda_MapLumpName(gameepisode, gamemap));

    lprintf(LO_DEBUG, "gld_Precache: %s done in %d ms\n", map, SDL_GetTicks() - tics);
  }
}

void gld_CleanMemory(void)
{
  gld_CleanVertexData();
  gld_CleanTexItems(numtextures, &gld_GLTextures);
  gld_CleanTexItems(numlumps, &gld_GLPatchTextures);
  gld_CleanTexItems(numtextures, &gld_GLIndexedTextures);
  gld_CleanTexItems(numlumps, &gld_GLIndexedPatchTextures);
  gld_CleanTexItems(numtextures * gld_numGLColormaps, &gld_GLIndexedSkyTextures);
  gl_preprocessed = false;
}

void gld_CleanStaticMemory(void)
{
  gld_CleanTexItems(numlumps, &gld_GLStaticPatchTextures);
  gld_CleanTexItems(numlumps, &gld_GLIndexedStaticPatchTextures);
  gld_CleanTexItems(gld_numGLColormaps, &gld_GLColormapTextures);
  gld_CleanTexItems(gld_numGLColormaps, &gld_GLFullbrightColormapTextures);
}
