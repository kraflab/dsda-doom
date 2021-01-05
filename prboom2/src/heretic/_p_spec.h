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

// P_spec.h

/*
===============================================================================

							P_SPEC

===============================================================================
*/

//
//      Animating textures and planes
//
typedef struct
{
    boolean istexture;
    int picnum;
    int basepic;
    int numpics;
    int speed;
} anim_t;

//
//      source animation definition
//
typedef struct
{
    int istexture;          // if false, it's a flat
    char endname[9];
    char startname[9];
    int speed;
} animdef_t;

#define	MAXANIMS		32

extern anim_t anims[MAXANIMS], *lastanim;

//
//      Animating line specials
//
#define	MAXLINEANIMS		64*256
extern short numlinespecials;
extern line_t *linespeciallist[MAXLINEANIMS];

//      Define values for map objects
#define	MO_TELEPORTMAN		14

// at game start
void P_InitPicAnims(void);
void P_InitTerrainTypes(void);
void P_InitLava(void);

// when needed
boolean P_UseSpecialLine(mobj_t * thing, line_t * line);
void P_ShootSpecialLine(mobj_t * thing, line_t * line);
void P_CrossSpecialLine(int linenum, int side, mobj_t * thing);

void P_PlayerInSpecialSector(player_t * player);

int twoSided(int sector, int line);
sector_t *getSector(int currentSector, int line, int side);
side_t *getSide(int currentSector, int line, int side);
fixed_t P_FindLowestFloorSurrounding(sector_t * sec);
fixed_t P_FindHighestFloorSurrounding(sector_t * sec);
fixed_t P_FindNextHighestFloor(sector_t * sec, int currentheight);
fixed_t P_FindLowestCeilingSurrounding(sector_t * sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t * sec);
int P_FindSectorFromLineTag(line_t * line, int start);
int P_FindMinSurroundingLight(sector_t * sector, int max);
sector_t *getNextSector(line_t * line, sector_t * sec);
