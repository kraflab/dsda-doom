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

#include "gl_opengl.h"

#include "z_zone.h"
#include <SDL.h>

#ifdef HAVE_LIBSDL2_IMAGE
#include <SDL_image.h>
#endif

#include <math.h>

#include "v_video.h"
#include "r_main.h"
#include "gl_intern.h"
#include "w_wad.h"
#include "lprintf.h"
#include "p_spec.h"
#include "m_misc.h"
#include "sc_man.h"
#include "e6y.h"
#include "i_system.h"

typedef enum
{
  TAG_DETAIL_WALL,
  TAG_DETAIL_FLAT,
  TAG_DETAIL_MAX,
} tag_detail_e;

float xCamera,yCamera,zCamera;
TAnimItemParam *anim_flats = NULL;
TAnimItemParam *anim_textures = NULL;

void gld_InitDetail(void)
{
  gld_EnableDetail(true);
  gld_EnableDetail(false);
  gld_FlushTextures();
}

void gld_PreprocessDetail(void)
{
  if (gl_arb_multitexture)
  {
    GLEXT_glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glTexCoordPointer(2, GL_FLOAT, sizeof(flats_vbo[0]), flats_vbo_u);

    GLEXT_glClientActiveTextureARB(GL_TEXTURE1_ARB);
    glTexCoordPointer(2, GL_FLOAT, sizeof(flats_vbo[0]), flats_vbo_u);
    GLEXT_glClientActiveTextureARB(GL_TEXTURE0_ARB);

    GLEXT_glActiveTextureARB(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2);
    GLEXT_glActiveTextureARB(GL_TEXTURE0_ARB);
  }
}


void gld_EnableDetail(int enable)
{
  if (!gl_arb_multitexture)
    return;

  gld_EnableTexture2D(GL_TEXTURE1_ARB, enable);
  gld_EnableClientCoordArray(GL_TEXTURE1_ARB, enable);
}
