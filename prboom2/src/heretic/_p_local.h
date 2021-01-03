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

#define STARTREDPALS	1
#define STARTBONUSPALS	9
#define NUMREDPALS		8
#define NUMBONUSPALS	4

#define TOCENTER -8
#define	FLOATSPEED (FRACUNIT*4)

#define	MAXHEALTH 100
#define	VIEWHEIGHT (41*FRACUNIT)

// player radius for movement checking
#define PLAYERRADIUS 16*FRACUNIT

#define	USERANGE (64*FRACUNIT)
#define	MELEERANGE (64*FRACUNIT)
#define	MISSILERANGE (32*64*FRACUNIT)

typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
} dirtype_t;

#define BASETHRESHOLD 100       // follow a player exlusively for 3 seconds

// ***** P_MAPUTL *****

typedef struct
{
    fixed_t x, y, dx, dy;
} divline_t;

typedef struct
{
    fixed_t frac;               // along trace line
    boolean isaline;
    union
    {
        mobj_t *thing;
        line_t *line;
    } d;
} intercept_t;

#define	MAXINTERCEPTS	128
extern intercept_t *intercepts, *intercept_p; // [crispy] remove INTERCEPTS limit
extern void check_intercept(void); // [crispy] remove INTERCEPTS limit
typedef boolean(*traverser_t) (intercept_t * in);


fixed_t P_AproxDistance(fixed_t dx, fixed_t dy);
int P_PointOnLineSide(fixed_t x, fixed_t y, line_t * line);
int P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t * line);
void P_MakeDivline(line_t * li, divline_t * dl);
fixed_t P_InterceptVector(divline_t * v2, divline_t * v1);
int P_BoxOnLineSide(fixed_t * tmbox, line_t * ld);

extern fixed_t opentop, openbottom, openrange;
extern fixed_t lowfloor;
void P_LineOpening(line_t * linedef);

boolean P_BlockLinesIterator(int x, int y, boolean(*func) (line_t *));
boolean P_BlockThingsIterator(int x, int y, boolean(*func) (mobj_t *));

#define PT_ADDLINES		1
#define	PT_ADDTHINGS	2
#define	PT_EARLYOUT		4

extern divline_t trace;
boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
                       int flags, boolean(*trav) (intercept_t *));

void P_UnsetThingPosition(mobj_t * thing);
void P_SetThingPosition(mobj_t * thing);

// ***** P_MAP *****

extern boolean floatok;         // if true, move would be ok if
extern fixed_t tmfloorz, tmceilingz;    // within tmfloorz - tmceilingz

extern line_t *ceilingline;
boolean P_CheckPosition(mobj_t * thing, fixed_t x, fixed_t y);
boolean P_TryMove(mobj_t * thing, fixed_t x, fixed_t y);
boolean P_TeleportMove(mobj_t * thing, fixed_t x, fixed_t y);
void P_SlideMove(mobj_t * mo);
boolean P_CheckSight(mobj_t * t1, mobj_t * t2);
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
