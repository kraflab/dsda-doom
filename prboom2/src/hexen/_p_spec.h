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


extern int *TerrainTypes;

//
//      scrolling line specials
//

#define MAXLINEANIMS 64*256
extern short numlinespecials;
extern line_t *linespeciallist[MAXLINEANIMS];

//      Define values for map objects
#define MO_TELEPORTMAN 14

// at game start
void P_InitTerrainTypes(void);
void P_InitLava(void);

// at map load
void P_SpawnSpecials(void);

// every tic
void P_UpdateSpecials(void);

// when needed
dboolean P_ExecuteLineSpecial(int special, byte * args, line_t * line,
                             int side, mobj_t * mo);
dboolean P_ActivateLine(line_t * ld, mobj_t * mo, int side,
                       int activationType);
//dboolean P_UseSpecialLine ( mobj_t *thing, line_t *line);
//void    P_ShootSpecialLine ( mobj_t *thing, line_t *line);
//void    P_CrossSpecialLine (int linenum, int side, mobj_t *thing);

void P_PlayerInSpecialSector(player_t * player);
void P_PlayerOnSpecialFlat(player_t * player, int floorType);

//int twoSided(int sector,int line);
//sector_t *getSector(int currentSector,int line,int side);
//side_t  *getSide(int currentSector,int line, int side);
fixed_t P_FindLowestFloorSurrounding(sector_t * sec);
fixed_t P_FindHighestFloorSurrounding(sector_t * sec);
fixed_t P_FindNextHighestFloor(sector_t * sec, int currentheight);
fixed_t P_FindLowestCeilingSurrounding(sector_t * sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t * sec);
//int P_FindSectorFromLineTag(line_t  *line,int start);
int P_FindSectorFromTag(int tag, int start);
//int P_FindMinSurroundingLight(sector_t *sector,int max);
sector_t *getNextSector(line_t * line, sector_t * sec);
line_t *P_FindLine(int lineTag, int *searchPosition);

//
//      SPECIAL
//
//int EV_DoDonut(line_t *line);

//-------------------------------
// P_anim.c
//-------------------------------

void P_AnimateSurfaces(void);
void P_InitFTAnims(void);
void P_InitLightning(void);
void P_ForceLightning(void);

/*
===============================================================================

							P_PLATS

===============================================================================
*/

typedef enum
{
    PLAT_UP,
    PLAT_DOWN,
    PLAT_WAITING,
//      PLAT_IN_STASIS
} plat_e;

typedef enum
{
    PLAT_PERPETUALRAISE,
    PLAT_DOWNWAITUPSTAY,
    PLAT_DOWNBYVALUEWAITUPSTAY,
    PLAT_UPWAITDOWNSTAY,
    PLAT_UPBYVALUEWAITDOWNSTAY,
    //PLAT_RAISEANDCHANGE,
    //PLAT_RAISETONEARESTANDCHANGE
} plattype_e;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    fixed_t speed;
    fixed_t low;
    fixed_t high;
    int wait;
    int count;
    plat_e status;
    plat_e oldstatus;
    int crush;
    int tag;
    plattype_e type;
} plat_t;

#define PLATWAIT 3
#define PLATSPEED FRACUNIT
#define MAXPLATS 30*256

extern plat_t *activeplats[MAXPLATS];

void T_PlatRaise(plat_t * plat);
int EV_DoPlat(line_t * line, byte * args, plattype_e type, int amount);
void P_AddActivePlat(plat_t * plat);
void P_RemoveActivePlat(plat_t * plat);
void EV_StopPlat(line_t * line, byte * args);

/*
===============================================================================

							P_FLOOR

===============================================================================
*/

void T_FloorWaggle(floorWaggle_t * waggle);
int EV_FloorCrushStop(line_t * line, byte * args);
dboolean EV_StartFloorWaggle(int tag, int height, int speed, int offset,
                            int timer);
