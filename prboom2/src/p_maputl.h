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
 *      Map utility functions
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_MAPUTL__
#define __P_MAPUTL__

#include "r_defs.h"

/* mapblocks are used to check movement against lines and things */
#define MAPBLOCKUNITS   128
#define MAPBLOCKSIZE    (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT   (FRACBITS+7)
#define MAPBMASK        (MAPBLOCKSIZE-1)
#define MAPBTOFRAC      (MAPBLOCKSHIFT-FRACBITS)

#define PT_ADDLINES     1
#define PT_ADDTHINGS    2
#define PT_EARLYOUT     4

typedef struct {
  fixed_t top;
  fixed_t bottom;
  fixed_t range;
  fixed_t lowfloor;
  sector_t *frontsector;
  sector_t *backsector;

  dboolean touchmidtex;
  dboolean abovemidtex;
} line_opening_t;

typedef struct {
  fixed_t     x;
  fixed_t     y;
  fixed_t     dx;
  fixed_t     dy;
} divline_t;

typedef struct {
  fixed_t     frac;           /* along trace line */
  dboolean     isaline;
  union {
    mobj_t* thing;
    line_t* line;
  } d;
} intercept_t;

typedef dboolean (*traverser_t)(intercept_t *in);

fixed_t CONSTFUNC P_AproxDistance (fixed_t dx, fixed_t dy);

int PUREFUNC P_CompatiblePointOnLineSide(fixed_t x, fixed_t y, const line_t *line);
int PUREFUNC P_ZDoomPointOnLineSide(fixed_t x, fixed_t y, const line_t *line);
extern int (*P_PointOnLineSide)(fixed_t x, fixed_t y, const line_t *line);

int     PUREFUNC  P_BoxOnLineSide (const fixed_t *tmbox, const line_t *ld);
fixed_t PUREFUNC  P_InterceptVector (const divline_t *v2, const divline_t *v1);
/* cph - old compatibility version below */
fixed_t PUREFUNC  P_InterceptVector2(const divline_t *v2, const divline_t *v1);

extern intercept_t *intercepts, *intercept_p;
void P_MakeDivline(const line_t *li, divline_t *dl);

int PUREFUNC P_CompatiblePointOnDivlineSide(fixed_t x, fixed_t y, const divline_t *line);
int PUREFUNC P_ZDoomPointOnDivlineSide(fixed_t x, fixed_t y, const divline_t *line);
extern int (*P_PointOnDivlineSide)(fixed_t x, fixed_t y, const divline_t *line);

void check_intercept(void);

void    P_LineOpening (const line_t *linedef, const mobj_t *actor);
void    P_UnsetThingPosition(mobj_t *thing);
void    P_SetThingPosition(mobj_t *thing);
dboolean P_BlockLinesIterator (int x, int y, dboolean func(line_t *));
dboolean P_BlockLinesIterator2(int x, int y, dboolean func(line_t *));
dboolean P_BlockThingsIterator(int x, int y, dboolean func(mobj_t *));
dboolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
                       int flags, dboolean trav(intercept_t *));

angle_t P_PointToAngle(fixed_t xo, fixed_t yo, fixed_t x, fixed_t y);
mobj_t *P_RoughTargetSearch(mobj_t *mo, angle_t fov, int distance);

// MAES: support 512x512 blockmaps.
int P_GetSafeBlockX(int coord);
int P_GetSafeBlockY(int coord);

extern line_opening_t line_opening;
extern divline_t trace;

dboolean P_GetMidTexturePosition(const line_t *line, int sideno, fixed_t *top, fixed_t *bottom);

// bes 01/20/24: foul hacks to draw these lines on automap
//  bes 02/28/24: moved this to maputl.h and renamed pathtrace to amlinetrace
typedef struct
{
	fixed_t x1, y1, x2, y2;
	int when;
} amlinetrace_t;

#define NUMAMLINETRACES 64
extern amlinetrace_t amlinetraces[NUMAMLINETRACES];
extern unsigned int cur_amlinetrace;

// bes 02/28/24: automap rectangle traces for blocks (itc ovf blockmap tiles)
typedef struct
{
	fixed_t x1, y1, x2, y2;
	int when;
} amrecttrace_t;

#define NUMAMRECTTRACES 64
extern amrecttrace_t amrecttraces[NUMAMRECTTRACES];
extern unsigned int cur_amrecttrace;

#endif  /* __P_MAPUTL__ */
