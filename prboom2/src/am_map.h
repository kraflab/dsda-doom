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
 *  AutoMap module.
 *
 *-----------------------------------------------------------------------------*/

#ifndef __AMMAP_H__
#define __AMMAP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "d_event.h"
#include "m_fixed.h"
#include "m_misc.h"

typedef struct
{
  int back;
  int grid;
  int wall;
  int fchg;
  int cchg;
  int clsd;
  int rkey;
  int bkey;
  int ykey;
  int rdor;
  int bdor;
  int ydor;
  int tele;
  int secr;
  int revsecr;
  int exit;
  int unsn;
  int flat;
  int sprt;
  int item;
  int frnd;
  int enemy;
  int hair;
  int sngl;
  int me;
  int plyr[8];
  int trail_1;
  int trail_2;
} mapcolor_t;

typedef struct map_point_s
{
  float x, y;
  unsigned char r, g, b, a;
} PACKEDATTR map_point_t;

typedef struct map_line_s
{
  map_point_t point[2];
} PACKEDATTR map_line_t;

extern array_t map_lines;

#define MAPBITS 12
#define FRACTOMAPBITS (FRACBITS-MAPBITS)

// Called by main loop.
dboolean AM_Responder (event_t* ev);

// Called by main loop.
void AM_Ticker (void);

// Called by main loop,
// called instead of view drawer if automap active.
void AM_Drawer (dboolean minimap);

// Called to force the automap to quit
// if the level is completed while it is up.
void AM_Stop (dboolean minimap);

// killough 2/22/98: for saving automap information in savegame:

void AM_Start(dboolean full_automap);

//jff 4/16/98 make externally available

void AM_clearMarks(void);

void AM_setMarkParams(int num);

void AM_SetResolution(void);

typedef struct
{
 fixed_t x,y;
 float fx,fy;
} mpoint_t;

typedef struct
{
 fixed_t x, y;
 fixed_t w, h;

 char label[16];
 int widths[16];
} markpoint_t;

extern markpoint_t *markpoints;
extern int markpointnum, markpointnum_max;

// end changes -- killough 2/22/98

extern mapcolor_t mapcolor;

void M_ChangeMapTextured(void);
void M_ChangeMapMultisamling(void);
void AM_ResetIDDTcheat(void);
void AM_SetMapCenter(fixed_t x, fixed_t y);

typedef struct am_frame_s
{
  fixed_t centerx, centery;
  fixed_t sin, cos;

  float centerx_f, centery_f;
  float sin_f, cos_f;

  fixed_t bbox[4];

  int precise;
} am_frame_t;
extern am_frame_t am_frame;

typedef enum
{
  map_things_appearance_classic,
  map_things_appearance_scaled,
#if defined(HAVE_LIBSDL2_IMAGE)
  map_things_appearance_icon,
#endif

  map_things_appearance_max
} map_things_appearance_t;

typedef enum
{
  map_trail_mode_off,
  map_trail_mode_ignore_collisions,
  map_trail_mode_include_collisions,
  map_trail_mode_max
} map_trail_mode_t;

extern map_trail_mode_t map_trail_mode;

void AM_updatePlayerTrail(fixed_t x, fixed_t y);

#endif
