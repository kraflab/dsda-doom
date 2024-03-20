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

#include "v_video.h"
#include "gl_intern.h"
#include "m_random.h"
#include "lprintf.h"
#include "e6y.h"

#include "dsda/gl/render_scale.h"

static GLuint wipe_scr_start_tex = 0;
static GLuint wipe_scr_end_tex = 0;

GLuint CaptureScreenAsTexID(void)
{
  GLuint id;

  gld_EnableTexture2D(GL_TEXTURE0_ARB, true);

  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
    gld_GetTexDimension(gl_viewport_width), gld_GetTexDimension(gl_viewport_height),
    0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, gl_viewport_x, gl_viewport_y, gl_viewport_width, gl_viewport_height);

  return id;
}

int gld_wipe_doMelt(int ticks, int *y_lookup)
{
  int i, scaled_i, scaled_i2;
  int total_w, total_h;
  float fU1, fU2, fV1, fV2;
  int yoffs;
  float tx, tx2;
  int dx = MAX(1, (SCREENWIDTH > SCREENHEIGHT) ? SCREENHEIGHT / 200 : SCREENWIDTH / 200);

  total_w = gld_GetTexDimension(gl_viewport_width);
  total_h = gld_GetTexDimension(gl_viewport_height);

  fU1 = 0.0f;
  fV1 = (float)gl_viewport_height / (float)total_h;
  fU2 = (float)gl_viewport_width / (float)total_w;
  fV2 = 0.0f;


  glBindTexture(GL_TEXTURE_2D, wipe_scr_end_tex);
  glColor3f(1.0f, 1.0f, 1.0f);

  glBegin(GL_TRIANGLE_STRIP);
  {
    glTexCoord2f(fU1, fV1); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(fU1, fV2); glVertex2f(0.0f, (float)SCREENHEIGHT);
    glTexCoord2f(fU2, fV1); glVertex2f((float)SCREENWIDTH, 0.0f);
    glTexCoord2f(fU2, fV2); glVertex2f((float)SCREENWIDTH, (float)SCREENHEIGHT);
  }
  glEnd();

  glBindTexture(GL_TEXTURE_2D, wipe_scr_start_tex);
  glColor3f(1.0f, 1.0f, 1.0f);

  glBegin(GL_TRIANGLE_STRIP);
  for (i = 0; i < SCREENWIDTH; i += dx)
  {
    int i2 = (i + dx > SCREENWIDTH ? SCREENWIDTH : i + dx);
    yoffs = MAX(0, y_lookup[i]);

    // elim - melt texture is the pixel size of the GL viewport, not the game scene texture size
    scaled_i = MIN(gl_viewport_width, (int)((float)i * gl_scale_x));
    scaled_i2 = MIN(gl_viewport_width, (int)((float)i2 * gl_scale_x));

    // elim - texel coordinates don't necessarily match texture buffer dimensions, since textures
    //        have to be stored in dimensions that are power-of-2
    tx = (float) MIN(fU2, (float)scaled_i / (float)total_w);
    tx2 = (float) MIN(fU2, (float)scaled_i2 / (float)total_w);

    glTexCoord2f(tx, fV1); glVertex2f(i, yoffs);
    glTexCoord2f(tx, fV2); glVertex2f(i, yoffs + SCREENHEIGHT);
    glTexCoord2f(tx2, fV1); glVertex2f(i2, yoffs);
    glTexCoord2f(tx2, fV2); glVertex2f(i2, yoffs + SCREENHEIGHT);
  }
  glEnd();

  return 0;
}

int gld_wipe_exitMelt(int ticks)
{
  if (wipe_scr_start_tex != 0)
  {
    glDeleteTextures(1, &wipe_scr_start_tex);
    wipe_scr_start_tex = 0;
  }
  if (wipe_scr_end_tex != 0)
  {
    glDeleteTextures(1, &wipe_scr_end_tex);
    wipe_scr_end_tex = 0;
  }

  gld_ResetLastTexture();

  return 0;
}

int gld_wipe_StartScreen(void)
{
  wipe_scr_start_tex = CaptureScreenAsTexID();

  return 0;
}

int gld_wipe_EndScreen(void)
{
  glFlush();
  wipe_scr_end_tex = CaptureScreenAsTexID();

  return 0;
}
