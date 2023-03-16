/* Emacs style mode select   -*- C++ -*-
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

#ifdef _MSC_VER
//#include <ddraw.h> /* needed for DirectX's DDSURFACEDESC2 structure definition */
#include <io.h>
#else
#include <unistd.h>
#endif
#include <SDL.h>
#ifdef HAVE_LIBSDL2_IMAGE
#include <SDL_image.h>
#endif
#include "doomstat.h"
#include "v_video.h"
#include "gl_intern.h"
#include "i_system.h"
#include "w_wad.h"
#include "lprintf.h"
#include "i_video.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "r_main.h"
#include "r_sky.h"
#include "m_misc.h"
#include "e6y.h"

#include "dsda/utility.h"

unsigned int gl_has_hires = 0;

static GLuint progress_texid = 0;
static unsigned int lastupdate = 0;

int gld_ProgressStart(void)
{
  if (!progress_texid)
  {
    progress_texid = CaptureScreenAsTexID();
    lastupdate = SDL_GetTicks() - 100;
    return true;
  }

  return false;
}

int gld_ProgressRestoreScreen(void)
{
  int total_w, total_h;
  float fU1, fU2, fV1, fV2;

  if (progress_texid)
  {
    total_w = gld_GetTexDimension(SCREENWIDTH);
    total_h = gld_GetTexDimension(SCREENHEIGHT);

    fU1 = 0.0f;
    fV1 = (float)SCREENHEIGHT / (float)total_h;
    fU2 = (float)SCREENWIDTH / (float)total_w;
    fV2 = 0.0f;

    gld_EnableTexture2D(GL_TEXTURE0_ARB, true);

    glBindTexture(GL_TEXTURE_2D, progress_texid);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLE_STRIP);
    {
      glTexCoord2f(fU1, fV1); glVertex2f(0.0f, 0.0f);
      glTexCoord2f(fU1, fV2); glVertex2f(0.0f, (float)SCREENHEIGHT);
      glTexCoord2f(fU2, fV1); glVertex2f((float)SCREENWIDTH, 0.0f);
      glTexCoord2f(fU2, fV2); glVertex2f((float)SCREENWIDTH, (float)SCREENHEIGHT);
    }
    glEnd();

    return true;
  }

  return false;
}

int gld_ProgressEnd(void)
{
  if (progress_texid != 0)
  {
    gld_ProgressRestoreScreen();
    I_FinishUpdate();
    gld_ProgressRestoreScreen();
    glDeleteTextures(1, &progress_texid);
    progress_texid = 0;
    return true;
  }

  return false;
}

extern patchnum_t hu_font[HU_FONTSIZE];
static hu_textline_t w_precache;

static void gld_InitProgressUpdate(void)
{
  HUlib_initTextLine
  (
    &w_precache,
    16,
    186,
    hu_font,
    HU_FONTSTART,
    CR_RED,
    VPT_ALIGN_LEFT_BOTTOM
  );
}

void gld_ProgressUpdate(const char * text, int progress, int total)
{
  int len;
  static char last_text[32] = {0};
  unsigned int tic;

  if (!progress_texid)
    return;

  // do not do it often
  tic = SDL_GetTicks();
  if (tic - lastupdate < 100)
    return;
  lastupdate = tic;

  DO_ONCE
    gld_InitProgressUpdate();
  END_ONCE

  if (text && *text && strcmp(last_text, text))
  {
    const char *s;
    strcpy(last_text, text);

    HUlib_clearTextLine(&w_precache);
    s = text;
    while (*s)
      HUlib_addCharToTextLine(&w_precache, *(s++));
    HUlib_setTextXCenter(&w_precache);
  }

  gld_ProgressRestoreScreen();
  HUlib_drawTextLine(&w_precache, false);

  len = MIN(SCREENWIDTH, (int)((int64_t)SCREENWIDTH * progress / total));
  V_FillRect(0, 0, SCREENHEIGHT - 4, len - 0, 4, 4);
  if (len > 4)
  {
    V_FillRect(0, 2, SCREENHEIGHT - 3, len - 4, 2, 31);
  }

  I_FinishUpdate();
}

#ifdef HAVE_LIBSDL2_IMAGE

static const char* gld_HiRes_GetInternalName(GLTexture *gltexture);
static void gld_HiRes_Bind(GLTexture *gltexture, GLuint *glTexID);

static byte* RGB2PAL = NULL;

static const char* gld_HiRes_GetInternalName(GLTexture *gltexture)
{
  static char texname[9];
  char *texname_p = NULL;

  switch (gltexture->textype)
  {
  case GLDT_TEXTURE:
    texname_p = textures[gltexture->index]->name;
    break;
  case GLDT_FLAT:
  case GLDT_PATCH:
    texname_p = lumpinfo[gltexture->index].name;
    break;
  }

  if (!texname_p)
    return NULL;

  strncpy(texname, texname_p, 8);
  texname[8] = 0;
  M_Strlwr(texname);

  return texname;
}

static void gld_HiRes_Bind(GLTexture *gltexture, GLuint *glTexID)
{
  switch (gltexture->textype)
  {
  case GLDT_TEXTURE:
    gl_has_hires |= 1;
    break;
  case GLDT_FLAT:
    gl_has_hires |= 2;
    break;
  case GLDT_PATCH:
    gl_has_hires |= 4;
    break;
  }

  if ((gltexture->textype == GLDT_TEXTURE) || (gltexture->textype == GLDT_FLAT))
    gltexture->flags |= GLTEXTURE_MIPMAP;
  else
    gltexture->flags &= ~GLTEXTURE_MIPMAP;

  gltexture->flags |= GLTEXTURE_HIRES;

  if (gltexture->textype == GLDT_PATCH)
  {
    gltexture->scalexfac = 1.0f;
    gltexture->scaleyfac = 1.0f;
  }

  if (*glTexID == 0)
    glGenTextures(1, glTexID);

  glBindTexture(GL_TEXTURE_2D, *glTexID);
}

void gld_HiRes_ProcessColormap(unsigned char *buffer, int bufSize)
{
  int pos;
  const lighttable_t *colormap;
  const unsigned char *playpal;

  if (!RGB2PAL)
    return;

  playpal = V_GetPlaypal();
  colormap = (fixedcolormap ? fixedcolormap : fullcolormap);

  for (pos = 0; pos < bufSize; pos += 4)
  {
    byte color;

    color = RGB2PAL[((buffer[pos+0]>>3)<<10) + ((buffer[pos+1]>>3)<<5) + (buffer[pos+2]>>3)];

    buffer[pos+0] = playpal[colormap[color]*3+0];
    buffer[pos+1] = playpal[colormap[color]*3+1];
    buffer[pos+2] = playpal[colormap[color]*3+2];
  }
}

int gld_HiRes_BuildTables(void)
{
  const int chanel_bits = 5;
  const int numcolors_per_chanel = (1 << chanel_bits);
  const int RGB2PAL_size = numcolors_per_chanel * numcolors_per_chanel * numcolors_per_chanel;
  unsigned char* RGB2PAL_fname;
  int lump, size;

  if (!gl_boom_colormaps)
    return false;

  if (RGB2PAL)
    return true;

  {
    int ok = true;
    FILE *RGB2PAL_fp = NULL;
    char fname[PATH_MAX+1];

    if (ok)
    {
      void* NewIntDynArray(int dimCount, int *dims);
      const byte* palette;
      int r, g, b, k, color;
      int **x, **y, **z;
      int dims[2] = {numcolors_per_chanel, 256};

      x = NewIntDynArray(2, dims);
      y = NewIntDynArray(2, dims);
      z = NewIntDynArray(2, dims);

      RGB2PAL = Z_Malloc(RGB2PAL_size);
      palette = V_GetPlaypal();

      // create the RGB24to8 lookup table
      gld_ProgressStart();
      gld_ProgressUpdate(NULL, 0, numcolors_per_chanel);
      for (k = 0; k < numcolors_per_chanel; k++)
      {
        int color_p = 0;
        int kk = ((k<<3)|(k>>2));
        for (color = 0; color < 256; color++)
        {
          x[k][color] = (kk - palette[color_p++]);
          x[k][color] *= x[k][color];
          y[k][color] = (kk - palette[color_p++]);
          y[k][color] *= y[k][color];
          z[k][color] = (kk - palette[color_p++]);
          z[k][color] *= z[k][color];
        }
      }

      k = 0;
      for (r = 0; r < numcolors_per_chanel; r++)
      {
        gld_ProgressUpdate(NULL, r, numcolors_per_chanel);
        for (g = 0; g < numcolors_per_chanel; g++)
        {
          int xy[256];
          for (color = 0; color < 256; color++)
          {
            xy[color] = x[r][color] + y[g][color];
          }
          for (b = 0; b < numcolors_per_chanel; b++)
          {
            int dist;
            int bestcolor = 0;
            int bestdist = xy[0] + z[b][0];
            #define CHECK_BEST dist = xy[color] + z[b][color];\
              if (dist < bestdist) {bestdist = dist; bestcolor = color;} color++;
            for (color = 0; color < 256;)
            {
              CHECK_BEST;
              CHECK_BEST;
              CHECK_BEST;
              CHECK_BEST;
              CHECK_BEST;
              CHECK_BEST;
              CHECK_BEST;
              CHECK_BEST;
            }
            RGB2PAL[k++] = bestcolor;
          }
        }
      }
      gld_ProgressEnd();

      Z_Free(z);
      Z_Free(y);
      Z_Free(x);

      return true;
    }
  }

  gl_boom_colormaps_default = false;
  M_ChangeAllowBoomColormaps();
  return false;
}

void gld_InitHiRes(void)
{
  gld_HiRes_BuildTables();

  gl_has_hires = 0;
}

int gld_LoadHiresTex(GLTexture *gltexture, int cm)
{
  int result = false;
  GLuint *texid;

  // [XA] hires textures aren't supported in indexed
  // lightmode, since they're typically truecolor and
  // would look like super-poop if downsampled. do
  // abort early though so the function doesn't flag
  // the texture as "no high-res texture found", else
  // it'll bork up if switching the lightmode later on.
  if (V_IsWorldLightmodeIndexed())
    return false;

  // do we need it?
  if (!(gltexture->flags & GLTEXTURE_HASNOHIRES))
  {
    // default buffer
    texid = &gltexture->glTexExID[CR_DEFAULT][0][0];

    // do not try to load hires twice
    if ((*texid == 0) || (gltexture->flags & GLTEXTURE_HIRES))
    {
      // try to load in-wad texture
      if (*texid == 0)
      {
        const char *lumpname = gld_HiRes_GetInternalName(gltexture);

        if (lumpname)
        {
          int lump = W_CheckNumForName2(lumpname, ns_hires);
          if (lump != LUMP_NOT_FOUND)
          {
            SDL_RWops *rw_data = SDL_RWFromConstMem(W_LumpByNum(lump), W_LumpLength(lump));
            SDL_Surface *surf_tmp = IMG_Load_RW(rw_data, false);

            // SDL can't load some TGA with common method
            if (!surf_tmp)
            {
              surf_tmp = IMG_LoadTyped_RW(rw_data, false, "TGA");
            }

            SDL_FreeRW(rw_data);

            if (!surf_tmp)
            {
              lprintf(LO_WARN, "gld_LoadHiresTex: %s\n", SDL_GetError());
            }
            else
            {
              SDL_Surface *surf = SDL_ConvertSurface(surf_tmp, &RGBAFormat, 0);
              SDL_FreeSurface(surf_tmp);

              if (surf)
              {
                if (SDL_LockSurface(surf) >= 0)
                {
                  if (SmoothEdges(surf->pixels, surf->pitch / 4, surf->h))
                    gltexture->flags |= GLTEXTURE_HASHOLES;
                  else
                    gltexture->flags &= ~GLTEXTURE_HASHOLES;
                  SDL_UnlockSurface(surf);
                }
                gld_HiRes_Bind(gltexture, texid);
                gld_BuildTexture(gltexture, surf->pixels, true, surf->w, surf->h);

                SDL_FreeSurface(surf);
              }
            }
          }
        }
      }

      if (*texid)
      {
        gld_GetTextureTexID(gltexture, cm);

        if (last_glTexID == gltexture->texid_p)
        {
          result = true;
        }
        else
        {
          if (texid == gltexture->texid_p)
          {
            glBindTexture(GL_TEXTURE_2D, *gltexture->texid_p);
            result = true;
          }
          else
          {
            if (boom_cm && use_boom_cm && gl_boom_colormaps)
            {
              int w, h;
              unsigned char *buf;

              if (*gltexture->texid_p == 0)
              {
                buf = gld_GetTextureBuffer(*texid, 0, &w, &h);
                gld_HiRes_Bind(gltexture, gltexture->texid_p);
                gld_HiRes_ProcessColormap(buf, w * h * 4);
                if (gld_BuildTexture(gltexture, buf, true, w, h))
                {
                  result = true;
                }
              }
              else
              {
                gld_HiRes_Bind(gltexture, gltexture->texid_p);
                result = true;
              }
            }
            else
            {
              *gltexture->texid_p = *texid;
              gld_HiRes_Bind(gltexture, gltexture->texid_p);
              result = true;
            }
          }
        }
      }
    }
  }

  if (result)
  {
    last_glTexID = gltexture->texid_p;
  }
  else
  {
    // there is no corresponding hires
    gltexture->flags |= GLTEXTURE_HASNOHIRES;
  }

  return result;
}

#endif // HAVE_LIBSDL2_IMAGE
