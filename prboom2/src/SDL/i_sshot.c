/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2006 by
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
 *  Screenshot functions, moved out of i_video.c
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "SDL.h"

#ifdef HAVE_LIBSDL2_IMAGE
#include <SDL_image.h>
#endif

#include "doomstat.h"
#include "doomdef.h"
#include "doomtype.h"
#include "v_video.h"
#include "i_video.h"
#include "z_zone.h"
#include "lprintf.h"

#include "dsda/gl/render_scale.h"

int renderW;
int renderH;
int screenshotW;
int screenshotH;
enum software_sshot_resolution_type_e software_sshot_type;

void I_UpdateRenderSize(void)
{
  if (V_IsOpenGLMode())
  {
    renderW = gl_window_width;
    renderH = gl_window_height;
  }
  else
  {
    SDL_GetRendererOutputSize(sdl_renderer, &renderW, &renderH);
  }
}

//
// I_ScreenShot // Modified to work with SDL2 resizeable window and fullscreen desktop - DTIED
//

int I_ScreenShot(const char *fname)
{
  int result = -1;
  unsigned char *pixels = I_GrabScreen();
  SDL_Surface *screenshot = NULL;

  if (pixels)
  {
    screenshot = SDL_CreateRGBSurfaceFrom(pixels, screenshotW, screenshotH, 24,
      screenshotW * 3, 0x000000ff, 0x0000ff00, 0x00ff0000, 0);
  }

  if (screenshot)
  {
#ifdef HAVE_LIBSDL2_IMAGE
    result = IMG_SavePNG(screenshot, fname);
#else
    result = SDL_SaveBMP(screenshot, fname);
#endif
    SDL_FreeSurface(screenshot);
  }
  return result;
}

// NSM
// returns current screen contents as RGB24 (raw)
// returned pointer should be freed when done
//
// Modified to work with SDL2 resizeable window and fullscreen desktop - DTIED
//

unsigned char *I_GrabScreen(void)
{
  static unsigned char *pixels = NULL;
  static int pixels_size = 0;
  int size;
  SDL_Surface *screenshot_surface;

  I_UpdateRenderSize();

  #ifdef GL_DOOM
  if (V_IsOpenGLMode())
  {
    return gld_ReadScreen();
  }
  #endif

  if (software_sshot_type == SSHOT_RES_WINDOW)
  {
    screenshotW = renderW;
    screenshotH = renderH;
  }
  else // software_sshot_type == SSHOT_RES_GAME
  {
    screenshotW = SCREENWIDTH;
    screenshotH = SCREENHEIGHT;
  }

  size = screenshotW * screenshotH * 3;
  if (!pixels || size > pixels_size)
  {
    pixels_size = size;
    pixels = (unsigned char*)realloc(pixels, size);
  }

  if (pixels && size)
  {
    if (software_sshot_type == SSHOT_RES_WINDOW)
    {
      SDL_Rect screen = { 0, 0, screenshotW, screenshotH };
      SDL_RenderReadPixels(sdl_renderer, &screen, SDL_PIXELFORMAT_RGB24, pixels, screenshotW * 3);
    }
    else // software_sshot_typee == SSHOT_RES_GAME
    {
      SDL_PixelFormat *screenshot_format = SDL_AllocFormat(SDL_PIXELFORMAT_RGB24);
      SDL_LockSurface(sdl_buffer);
      screenshot_surface = SDL_ConvertSurface(sdl_buffer, screenshot_format, 0);
      SDL_UnlockSurface(sdl_buffer);

      SDL_LockSurface(screenshot_surface);
      memcpy(pixels, screenshot_surface->pixels, pixels_size);
      SDL_UnlockSurface(screenshot_surface);

      SDL_FreeSurface(screenshot_surface);
      SDL_FreeFormat(screenshot_format);
    }

  }

  return pixels;
}
