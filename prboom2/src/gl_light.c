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

#include <SDL.h>

#include <math.h>

#include "doomstat.h"
#include "lprintf.h"
#include "v_video.h"
#include "r_main.h"
#include "gl_intern.h"
#include "gl_opengl.h"
#include "e6y.h"

#include "dsda/configuration.h"

gl_lightmode_t gl_lightmode;
const char *gl_lightmodes[] = {"glboom", "shaders", "indexed", NULL};
dboolean gl_ui_lightmode_indexed = false;
dboolean gl_automap_lightmode_indexed = false;

int gl_fog;
int gl_use_fog;

int gl_fogenabled;
int gl_distfog = 70;
float gl_CurrentFogDensity = -1.0f;
float distfogtable[3][256];

typedef void (*gld_InitLightTable_f)(void);

static float lighttable_glboom[5][256];

static void gld_InitLightTable_glboom(void);

int gl_hardware_gamma = false;

void M_ChangeLightMode(void)
{
  gl_lightmode_t gl_lightmode_default = dsda_IntConfig(dsda_config_gl_lightmode);

  if (!glsl_Init())
  {
    I_Error("glsl_Init failed!");
  }

  gl_lightmode = gl_lightmode_default;

  gl_hardware_gamma = (gl_lightmode == gl_lightmode_shaders);

  if (gl_hardware_gamma)
  {
    gld_SetGammaRamp(gl_usegamma);
  }
  else
  {
    gld_SetGammaRamp(-1);
    gld_FlushTextures();
  }

  // [XA] recalculate skymode since it depends
  // on whether or not indexed mode is set
  M_ChangeSkyMode();
}

/*
 * lookuptable for lightvalues
 * calculated as follow:
 * floatlight=(1.0-exp((light^3)*gamma)) / (1.0-exp(1.0*gamma));
 * gamma=-0,2;-2,0;-4,0;-6,0;-8,0
 * light=0,0 .. 1,0
 */
void gld_InitLightTable(void)
{
  int i, g;
  float gamma[5] = {-0.2f, -2.0f, -4.0f, -6.0f, -8.0f};

  for (g = 0; g < 5; g++)
  {
    for (i = 0; i < 256; i++)
    {
      lighttable_glboom[g][i] = (float)((1.0f - exp(pow(i / 255.0f, 3) * gamma[g])) / (1.0f - exp(1.0f * gamma[g])));
    }
  }
}

float gld_Calc2DLightLevel(int lightlevel)
{
  return lighttable_glboom[usegamma][BETWEEN(0, 255, lightlevel)];
}

float gld_CalcLightLevel(int lightlevel)
{
  int light;

  light = BETWEEN(0, 255, lightlevel);

  return (float)light/255.0f;
}

void gld_StaticLightAlpha(float light, float alpha)
{
  player_t *player = &players[displayplayer];

  if (!player->fixedcolormap)
  {
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
  }
  else
  {
    if (invul_method != INVUL_BW)
    {
      glColor4f(1.0f, 1.0f, 1.0f, alpha);
    }
    else
    {
      if (SceneInTexture)
      {
        glColor4f(0.5f, 0.5f, 0.5f, alpha);
      }
      else
      {
        glColor4f(bw_red, bw_green, bw_blue, alpha);
      }
    }
  }

  glsl_SetLightLevel((player->fixedcolormap ? 1.0f : light));
}

// [XA] return amount of light to add from the player's gun flash.
// for non-indexed modes this is twice as large for some reason.
int gld_GetGunFlashLight(void)
{
  if (V_IsWorldLightmodeIndexed())
    return (extralight << 4);
  else
    return (extralight << 5);
}

void M_ChangeAllowFog(void)
{
  int i;
  GLfloat FogColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

  glFogi (GL_FOG_MODE, GL_EXP);
  glFogfv(GL_FOG_COLOR, FogColor);
  glHint (GL_FOG_HINT, GL_NICEST);

  gl_CurrentFogDensity = -1;

  gl_EnableFog(true);
  gl_EnableFog(false);

  for (i = 0; i < 256; i++)
  {
    if (i < 164)
    {
      distfogtable[0][i] = (float)((gl_distfog >> 1) + (gl_distfog) * (164 - i) / 164);
    }
    else if (i < 230)
    {
      distfogtable[0][i] = (float)((gl_distfog >> 1) - (gl_distfog >> 1) * (i - 164) / (230 - 164));
    }
    else
    {
      distfogtable[0][i] = 0.0f;
    }

    if (i < 128)
    {
      distfogtable[1][i] = 6.0f + (gl_distfog >> 1) + (gl_distfog) * (128 - i) / 48;
    }
    else if (i < 216)
    {
      distfogtable[1][i] = (216.0f - i) / ((216.0f - 128.0f)) * gl_distfog / 10;
    }
    else
    {
      distfogtable[1][i] = 0.0f;
    }

    if (i <= 128)
    {
      distfogtable[2][i] = (float)(1<<16) / (float)pow(1.46f, ((float)i / 8.0f));
      if (distfogtable[2][i] > 2048)
        distfogtable[2][i] = 2048;
    }
    else if (i < 192)
    {
      distfogtable[2][i] = (float)(1<<13) / (float)pow(1.30f, ((float)i / 8.0f));
    }
    else if (i < 216)
    {
      distfogtable[2][i] = (216.0f - i) / ((216.0f - 128.0f)) * gl_distfog / 10;
    }
    else
    {
      distfogtable[2][i] = 0.0f;
    }
  }
}

float gld_CalcFogDensity(sector_t *sector, int lightlevel, GLDrawItemType type)
{
  return 0;
}

void gld_SetFog(float fogdensity)
{
  if (fogdensity)
  {
    gl_EnableFog(true);
    if (fogdensity != gl_CurrentFogDensity)
    {
      glFogf(GL_FOG_DENSITY, fogdensity / 512.0f);
      gl_CurrentFogDensity = fogdensity;
    }
  }
  else
  {
    gl_EnableFog(false);
    gl_CurrentFogDensity = -1.0f;
  }
}

void gl_EnableFog(int on)
{
  if (on)
  {
    if (!gl_fogenabled)
    glEnable(GL_FOG);
  }
  else
  {
    if (gl_fogenabled)
      glDisable(GL_FOG);
  }
  gl_fogenabled=on;
}
