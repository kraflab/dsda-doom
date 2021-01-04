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

//
//      SPECIAL
//
int EV_DoDonut(line_t * line);

/*
===============================================================================

							P_LIGHTS

===============================================================================
*/
typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    int count;
    int maxlight;
    int minlight;
    int maxtime;
    int mintime;
} lightflash_t;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    int count;
    int minlight;
    int maxlight;
    int darktime;
    int brighttime;
} strobe_t;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    int minlight;
    int maxlight;
    int direction;
} glow_t;

#define GLOWSPEED		8
#define	STROBEBRIGHT	5
#define	FASTDARK		15
#define	SLOWDARK		35

void T_LightFlash(lightflash_t * flash);
void P_SpawnLightFlash(sector_t * sector);
void T_StrobeFlash(strobe_t * flash);
void P_SpawnStrobeFlash(sector_t * sector, int fastOrSlow, int inSync);
void EV_StartLightStrobing(line_t * line);
void EV_TurnTagLightsOff(line_t * line);
void EV_LightTurnOn(line_t * line, int bright);
void T_Glow(glow_t * g);
void P_SpawnGlowingLight(sector_t * sector);

/*
===============================================================================

							P_PLATS

===============================================================================
*/
typedef enum
{
    up,
    down,
    waiting,
    in_stasis
} plat_e;

typedef enum
{
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange
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
    boolean crush;
    int tag;
    plattype_e type;
} plat_t;

#define	PLATWAIT	3
#define	PLATSPEED	FRACUNIT
#define	MAXPLATS	30*256

extern plat_t *activeplats[MAXPLATS];

void T_PlatRaise(plat_t * plat);
int EV_DoPlat(line_t * line, plattype_e type, int amount);
void P_AddActivePlat(plat_t * plat);
void P_RemoveActivePlat(plat_t * plat);
void EV_StopPlat(line_t * line);
void P_ActivateInStasis(int tag);
