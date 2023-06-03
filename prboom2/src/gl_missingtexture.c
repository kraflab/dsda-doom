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

#include <SDL.h>
#ifdef HAVE_LIBSDL2_IMAGE
#include <SDL_image.h>
#endif
#include "doomstat.h"
#include "v_video.h"
#include "gl_intern.h"
#include "i_system.h"
#include "lprintf.h"
#include "i_video.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "r_main.h"
#include "e6y.h"

//==========================================================================
//
// Flood gaps with the back side's ceiling/floor texture
// This requires a stencil because the projected plane interferes with
// the depth buffer
//
//==========================================================================

void gld_SetupFloodStencil(GLWall *wall)
{
  int recursion = 0;

  // Create stencil
  glStencilFunc(GL_EQUAL, recursion, ~0); // create stencil
  glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); // increment stencil of valid pixels
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // don't write to the graphics buffer
  gld_EnableTexture2D(GL_TEXTURE0_ARB, false);
  glColor3f(1, 1, 1);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(true);

  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(wall->glseg->x1, wall->ytop, wall->glseg->z1);
  glVertex3f(wall->glseg->x1, wall->ybottom, wall->glseg->z1);
  glVertex3f(wall->glseg->x2, wall->ybottom, wall->glseg->z2);
  glVertex3f(wall->glseg->x2, wall->ytop, wall->glseg->z2);
  glEnd();

  glStencilFunc(GL_EQUAL, recursion+1, ~0); // draw sky into stencil
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);   // this stage doesn't modify the stencil

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // don't write to the graphics buffer
  gld_EnableTexture2D(GL_TEXTURE0_ARB, true);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(false);
}

void gld_ClearFloodStencil(GLWall *wall)
{
  int recursion = 0;

  glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
  gld_EnableTexture2D(GL_TEXTURE0_ARB, false);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // don't write to the graphics buffer
  glColor3f(1, 1, 1);

  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(wall->glseg->x1, wall->ytop, wall->glseg->z1);
  glVertex3f(wall->glseg->x1, wall->ybottom, wall->glseg->z1);
  glVertex3f(wall->glseg->x2, wall->ybottom, wall->glseg->z2);
  glVertex3f(wall->glseg->x2, wall->ytop, wall->glseg->z2);
  glEnd();

  // restore old stencil op.
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
  glStencilFunc(GL_EQUAL, recursion, ~0);
  gld_EnableTexture2D(GL_TEXTURE0_ARB, true);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(true);
}

//
// Calculation of the coordinates of the gap
//
void gld_SetupFloodedPlaneCoords(GLWall *wall, gl_strip_coords_t *c)
{
  float prj_fac1, prj_fac2;
  float k = 0.5f;
  float ytop, ybottom, planez;

  if (wall->flag == GLDWF_TOPFLUD)
  {
    ytop = wall->ybottom;
    ybottom = wall->ytop;
    planez = wall->ybottom;
  }
  else
  {
    ytop = wall->ytop;
    ybottom = wall->ybottom;
    planez = wall->ytop;
  }

  prj_fac1 = (ytop - zCamera) / (ytop - zCamera);
  prj_fac2 = (ytop - zCamera) / (ybottom - zCamera);

  c->v[0][0] = xCamera + prj_fac1 * (wall->glseg->x1 - xCamera);
  c->v[0][1] = planez;
  c->v[0][2] = yCamera + prj_fac1 * (wall->glseg->z1 - yCamera);

  c->v[1][0] = xCamera + prj_fac2 * (wall->glseg->x1 - xCamera);
  c->v[1][1] = planez;
  c->v[1][2] = yCamera + prj_fac2 * (wall->glseg->z1 - yCamera);

  c->v[2][0] = xCamera + prj_fac1 * (wall->glseg->x2 - xCamera);
  c->v[2][1] = planez;
  c->v[2][2] = yCamera + prj_fac1 * (wall->glseg->z2 - yCamera);

  c->v[3][0] = xCamera + prj_fac2 * (wall->glseg->x2 - xCamera);
  c->v[3][1] = planez;
  c->v[3][2] = yCamera + prj_fac2 * (wall->glseg->z2 - yCamera);

  c->t[0][0] = -c->v[0][0] / k;
  c->t[0][1] = -c->v[0][2] / k;

  c->t[1][0] = -c->v[1][0] / k;
  c->t[1][1] = -c->v[1][2] / k;

  c->t[2][0] = -c->v[2][0] / k;
  c->t[2][1] = -c->v[2][2] / k;

  c->t[3][0] = -c->v[3][0] / k;
  c->t[3][1] = -c->v[3][2] / k;
}

void gld_SetupFloodedPlaneLight(GLWall *wall)
{
  gld_StaticLightAlpha(wall->light, wall->alpha);
}
