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

							P_DOORS

===============================================================================
*/
typedef enum
{
    DREV_NORMAL,
    DREV_CLOSE30THENOPEN,
    DREV_CLOSE,
    DREV_OPEN,
    DREV_RAISEIN5MINS,
} vldoor_e;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    vldoor_e type;
    fixed_t topheight;
    fixed_t speed;
    int direction;              // 1 = up, 0 = waiting at top, -1 = down
    int topwait;                // tics to wait at the top (keep in case a door going down is reset)
    int topcountdown;           // when it reaches 0, start going down
} vldoor_t;

#define VDOORSPEED FRACUNIT*2
#define VDOORWAIT 150

dboolean EV_VerticalDoor(line_t * line, mobj_t * thing);
int EV_DoDoor(line_t * line, byte * args, vldoor_e type);
void T_VerticalDoor(vldoor_t * door);

/*
===============================================================================

							P_FLOOR

===============================================================================
*/
typedef enum
{
    FLEV_LOWERFLOOR,            // lower floor to highest surrounding floor
    FLEV_LOWERFLOORTOLOWEST,    // lower floor to lowest surrounding floor
    FLEV_LOWERFLOORBYVALUE,
    FLEV_RAISEFLOOR,            // raise floor to lowest surrounding CEILING
    FLEV_RAISEFLOORTONEAREST,   // raise floor to next highest surrounding floor
    FLEV_RAISEFLOORBYVALUE,
    FLEV_RAISEFLOORCRUSH,
    FLEV_RAISEBUILDSTEP,        // One step of a staircase
    FLEV_RAISEBYVALUETIMES8,
    FLEV_LOWERBYVALUETIMES8,
    FLEV_LOWERTIMES8INSTANT,
    FLEV_RAISETIMES8INSTANT,
    FLEV_MOVETOVALUETIMES8
} floor_e;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    floor_e type;
    int crush;
    int direction;
    int newspecial;
    short texture;
    fixed_t floordestheight;
    fixed_t speed;
    int delayCount;
    int delayTotal;
    fixed_t stairsDelayHeight;
    fixed_t stairsDelayHeightDelta;
    fixed_t resetHeight;
    short resetDelay;
    short resetDelayCount;
    byte textureChange;
} floormove_t;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    int ceilingSpeed;
    int floorSpeed;
    int floordest;
    int ceilingdest;
    int direction;
    int crush;
} pillar_t;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    fixed_t originalHeight;
    fixed_t accumulator;
    fixed_t accDelta;
    fixed_t targetScale;
    fixed_t scale;
    fixed_t scaleDelta;
    int ticker;
    int state;
} floorWaggle_t;

#define FLOORSPEED FRACUNIT

typedef enum
{
    STAIRS_NORMAL,
    STAIRS_SYNC,
    STAIRS_PHASED
} stairs_e;

result_e T_MovePlane(sector_t * sector, fixed_t speed,
                     fixed_t dest, int crush, int floorOrCeiling,
                     int direction);

int EV_BuildStairs(line_t * line, byte * args, int direction, stairs_e type);
int EV_DoFloor(line_t * line, byte * args, floor_e floortype);
void T_MoveFloor(floormove_t * floor);
void T_BuildPillar(pillar_t * pillar);
void T_FloorWaggle(floorWaggle_t * waggle);
int EV_BuildPillar(line_t * line, byte * args, dboolean crush);
int EV_OpenPillar(line_t * line, byte * args);
int EV_DoFloorAndCeiling(line_t * line, byte * args, dboolean raise);
int EV_FloorCrushStop(line_t * line, byte * args);
dboolean EV_StartFloorWaggle(int tag, int height, int speed, int offset,
                            int timer);

//--------------------------------------------------------------------------
//
// p_telept
//
//--------------------------------------------------------------------------

dboolean P_Teleport(mobj_t * thing, fixed_t x, fixed_t y, angle_t angle,
                   dboolean useFog);
dboolean EV_Teleport(int tid, mobj_t * thing, dboolean fog);

//--------------------------------------------------------------------------
//
// p_things
//
//--------------------------------------------------------------------------

extern mobjtype_t TranslateThingType[];

dboolean EV_ThingProjectile(byte * args, dboolean gravity);
dboolean EV_ThingSpawn(byte * args, dboolean fog);
dboolean EV_ThingActivate(int tid);
dboolean EV_ThingDeactivate(int tid);
dboolean EV_ThingRemove(int tid);
dboolean EV_ThingDestroy(int tid);
