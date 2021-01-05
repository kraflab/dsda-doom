//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

// P_local.h

#ifndef __P_LOCAL__
#define __P_LOCAL__

#ifndef __R_LOCAL__
#include "r_local.h"
#endif

// ***** P_MAP *****

extern boolean floatok;         // if true, move would be ok if
extern fixed_t tmfloorz, tmceilingz;    // within tmfloorz - tmceilingz

extern line_t *ceilingline;
boolean P_CheckPosition(mobj_t * thing, fixed_t x, fixed_t y);
boolean P_TryMove(mobj_t * thing, fixed_t x, fixed_t y);
boolean P_TeleportMove(mobj_t * thing, fixed_t x, fixed_t y);
void P_SlideMove(mobj_t * mo);
void P_UseLines(player_t * player);

boolean P_ChangeSector(sector_t * sector, boolean crunch);

extern mobj_t *linetarget;      // who got hit (or NULL)
fixed_t P_AimLineAttack(mobj_t * t1, angle_t angle, fixed_t distance);

void P_LineAttack(mobj_t * t1, angle_t angle, fixed_t distance, fixed_t slope,
                  int damage);

void P_RadiusAttack(mobj_t * spot, mobj_t * source, int damage);

// ***** P_SETUP *****

extern byte *rejectmatrix;      // for fast sight rejection
extern int32_t *blockmaplump;   // offsets in blockmap are from here // [crispy] BLOCKMAP limit
extern int32_t *blockmap;       // [crispy] BLOCKMAP limit
extern int bmapwidth, bmapheight;       // in mapblocks
extern fixed_t bmaporgx, bmaporgy;      // origin of block map
extern mobj_t **blocklinks;     // for thing chains

// ***** AM_MAP *****

boolean AM_Responder(event_t * ev);
void AM_Ticker(void);
void AM_Drawer(void);

// ***** SB_BAR *****

extern int SB_state;
void SB_PaletteFlash(void);

#include "p_spec.h"

#endif // __P_LOCAL__
