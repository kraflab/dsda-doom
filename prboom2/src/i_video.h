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
 *      System specific interface stuff.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __I_VIDEO__
#define __I_VIDEO__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <SDL_opengl.h>

#include "doomtype.h"
#include "v_video.h"
#include "SDL.h"

extern SDL_Window *sdl_window;
extern SDL_Renderer *sdl_renderer;

extern const char *screen_resolutions_list[];

extern const char *sdl_video_window_pos;

void I_PreInitGraphics(void); /* CPhipps - do stuff immediately on start */
void I_InitScreenResolution(void); /* init resolution */
void I_SetWindowCaption(void); /* Set the window caption */
void I_SetWindowIcon(void); /* Set the application icon */
void I_InitGraphics (void);
void I_UpdateVideoMode(void);
void I_ShutdownGraphics(void);

/* Takes full 8 bit values. */
void I_SetPalette(int pal); /* CPhipps - pass down palette number */

void I_QueueFrameCapture(void);
void I_QueueScreenshot(void);
void I_HandleCapture(void);

void I_FinishUpdate (void);

int I_ScreenShot (const char *fname);
// NSM expose lower level screen data grab for vidcap
unsigned char *I_GrabScreen (void);

/* I_StartTic
 * Called by D_DoomLoop,
 * called before processing each tic in a frame.
 * Quick syncronous operations are performed here.
 * Can call D_PostEvent.
 */
void I_StartTic (void);

/* I_StartFrame
 * Called by D_DoomLoop,
 * called before processing any tics in a frame
 * (just after displaying a frame).
 * Time consuming syncronous operations
 * are performed here (joystick reading).
 * Can call D_PostEvent.
 */

void I_StartFrame (void);

extern int desired_fullscreen; //e6y
extern int exclusive_fullscreen;

void I_UpdateRenderSize(void);	// Handle potential
extern int renderW;		// resolution scaling
extern int renderH;		// - DTIED

extern dboolean window_focused;
dboolean I_WindowFocused(void);
void UpdateGrab(void);

#endif
