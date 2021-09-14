/* Emacs style mode select   -*- C++ -*-
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
 * DESCRIPTION:  definitions, declarations and prototypes for specials
 *
 *-----------------------------------------------------------------------------*/

#ifndef __P_SPEC__
#define __P_SPEC__

#include "r_defs.h"
#include "d_player.h"

//      Define values for map objects
#define MO_TELEPORTMAN  14

// p_floor

#define ELEVATORSPEED (FRACUNIT*4)
#define FLOORSPEED     FRACUNIT

// p_ceilng

#define CEILSPEED   FRACUNIT
#define CEILWAIT    150

// p_doors

#define VDOORSPEED  (FRACUNIT*2)
#define VDOORWAIT   150

// p_plats

#define PLATWAIT    3
#define PLATSPEED   FRACUNIT

// p_switch

// 4 players, 4 buttons each at once, max.
#define MAXBUTTONS  16

// 1 second, in ticks.
#define BUTTONTIME  TICRATE

// p_lights

#define GLOWSPEED       8
#define STROBEBRIGHT    5
#define FASTDARK        15
#define SLOWDARK        35

//jff 3/14/98 add bits and shifts for generalized sector types

#define DAMAGE_MASK     0x60
#define DAMAGE_SHIFT    5
#define SECRET_MASK     0x80
#define SECRET_SHIFT    7
#define FRICTION_MASK   0x100
#define FRICTION_SHIFT  8
#define PUSH_MASK       0x200
#define PUSH_SHIFT      9

// reserved by boom spec - not implemented?
// bit 10: suppress all sounds within the sector
// bit 11: disable any sounds due to floor or ceiling motion by the sector

// mbf21
#define DEATH_MASK 0x1000 // bit 12
#define KILL_MONSTERS_MASK 0x2000 // bit 13

//jff 02/04/98 Define masks, shifts, for fields in
// generalized linedef types

#define GenEnd                0x8000
#define GenFloorBase          0x6000
#define GenCeilingBase        0x4000
#define GenDoorBase           0x3c00
#define GenLockedBase         0x3800
#define GenLiftBase           0x3400
#define GenStairsBase         0x3000
#define GenCrusherBase        0x2F80

#define TriggerType           0x0007
#define TriggerTypeShift      0

// define masks and shifts for the floor type fields

#define FloorCrush            0x1000
#define FloorChange           0x0c00
#define FloorTarget           0x0380
#define FloorDirection        0x0040
#define FloorModel            0x0020
#define FloorSpeed            0x0018

#define FloorCrushShift           12
#define FloorChangeShift          10
#define FloorTargetShift           7
#define FloorDirectionShift        6
#define FloorModelShift            5
#define FloorSpeedShift            3

// define masks and shifts for the ceiling type fields

#define CeilingCrush          0x1000
#define CeilingChange         0x0c00
#define CeilingTarget         0x0380
#define CeilingDirection      0x0040
#define CeilingModel          0x0020
#define CeilingSpeed          0x0018

#define CeilingCrushShift         12
#define CeilingChangeShift        10
#define CeilingTargetShift         7
#define CeilingDirectionShift      6
#define CeilingModelShift          5
#define CeilingSpeedShift          3

// define masks and shifts for the lift type fields

#define LiftTarget            0x0300
#define LiftDelay             0x00c0
#define LiftMonster           0x0020
#define LiftSpeed             0x0018

#define LiftTargetShift            8
#define LiftDelayShift             6
#define LiftMonsterShift           5
#define LiftSpeedShift             3

// define masks and shifts for the stairs type fields

#define StairIgnore           0x0200
#define StairDirection        0x0100
#define StairStep             0x00c0
#define StairMonster          0x0020
#define StairSpeed            0x0018

#define StairIgnoreShift           9
#define StairDirectionShift        8
#define StairStepShift             6
#define StairMonsterShift          5
#define StairSpeedShift            3

// define masks and shifts for the crusher type fields

#define CrusherSilent         0x0040
#define CrusherMonster        0x0020
#define CrusherSpeed          0x0018

#define CrusherSilentShift         6
#define CrusherMonsterShift        5
#define CrusherSpeedShift          3

// define masks and shifts for the door type fields

#define DoorDelay             0x0300
#define DoorMonster           0x0080
#define DoorKind              0x0060
#define DoorSpeed             0x0018

#define DoorDelayShift             8
#define DoorMonsterShift           7
#define DoorKindShift              5
#define DoorSpeedShift             3

// define masks and shifts for the locked door type fields

#define LockedNKeys           0x0200
#define LockedKey             0x01c0
#define LockedKind            0x0020
#define LockedSpeed           0x0018

#define LockedNKeysShift           9
#define LockedKeyShift             6
#define LockedKindShift            5
#define LockedSpeedShift           3

//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
    dboolean     istexture;
    int         picnum;
    int         basepic;
    int         numpics;
    int         speed;

} anim_t;

//e6y
typedef struct
{
  int index;
  anim_t *anim;
} TAnimItemParam;
extern TAnimItemParam *anim_flats;
extern TAnimItemParam *anim_textures;

// define names for the TriggerType field of the general linedefs

typedef enum
{
  WalkOnce,
  WalkMany,
  SwitchOnce,
  SwitchMany,
  GunOnce,
  GunMany,
  PushOnce,
  PushMany,
} triggertype_e;

// define names for the Speed field of the general linedefs

typedef enum
{
  SpeedSlow,
  SpeedNormal,
  SpeedFast,
  SpeedTurbo,
} motionspeed_e;

// define names for the Target field of the general floor

typedef enum
{
  FtoHnF,
  FtoLnF,
  FtoNnF,
  FtoLnC,
  FtoC,
  FbyST,
  Fby24,
  Fby32,
} floortarget_e;

// define names for the Changer Type field of the general floor

typedef enum
{
  FNoChg,
  FChgZero,
  FChgTxt,
  FChgTyp,
} floorchange_e;

// define names for the Change Model field of the general floor

typedef enum
{
  FTriggerModel,
  FNumericModel,
} floormodel_t;

// define names for the Target field of the general ceiling

typedef enum
{
  CtoHnC,
  CtoLnC,
  CtoNnC,
  CtoHnF,
  CtoF,
  CbyST,
  Cby24,
  Cby32,
} ceilingtarget_e;

// define names for the Changer Type field of the general ceiling

typedef enum
{
  CNoChg,
  CChgZero,
  CChgTxt,
  CChgTyp,
} ceilingchange_e;

// define names for the Change Model field of the general ceiling

typedef enum
{
  CTriggerModel,
  CNumericModel,
} ceilingmodel_t;

// define names for the Target field of the general lift

typedef enum
{
  F2LnF,
  F2NnF,
  F2LnC,
  LnF2HnF,
} lifttarget_e;

// define names for the door Kind field of the general ceiling

typedef enum
{
  OdCDoor,
  ODoor,
  CdODoor,
  CDoor,
} doorkind_e;

// define names for the locked door Kind field of the general ceiling

typedef enum
{
  AnyKey,
  RCard,
  BCard,
  YCard,
  RSkull,
  BSkull,
  YSkull,
  AllKeys,
} keykind_e;

//////////////////////////////////////////////////////////////////
//
// enums for classes of linedef triggers
//
//////////////////////////////////////////////////////////////////

//jff 2/23/98 identify the special classes that can share sectors

typedef enum
{
  floor_special,
  ceiling_special,
  lighting_special,
} special_e;

//jff 3/15/98 pure texture/type change for better generalized support
typedef enum
{
  trigChangeOnly,
  numChangeOnly,
} change_e;

// p_plats

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
  raiseToNearestAndChange,
  blazeDWUS,
  genLift,      //jff added to support generalized Plat types
  genPerpetual,
  toggleUpDn,   //jff 3/14/98 added to support instant toggle type

  // hexen - can probably be merged
  PLAT_PERPETUALRAISE,
  PLAT_DOWNWAITUPSTAY,
  PLAT_DOWNBYVALUEWAITUPSTAY,
  PLAT_UPWAITDOWNSTAY,
  PLAT_UPBYVALUEWAITDOWNSTAY,
} plattype_e;

// p_doors

typedef enum
{
  normal,
  close30ThenOpen,
  closeDoor,
  openDoor,
  raiseIn5Mins,
  blazeRaise,
  blazeOpen,
  blazeClose,

  //jff 02/05/98 add generalize door types
  genRaise,
  genBlazeRaise,
  genOpen,
  genBlazeOpen,
  genClose,
  genBlazeClose,
  genCdO,
  genBlazeCdO,

  // heretic
  vld_normal,
  vld_normal_turbo,
  vld_close30ThenOpen,
  vld_close,
  vld_open,
  vld_raiseIn5Mins,

  // hexen - can probably be merged
  DREV_NORMAL,
  DREV_CLOSE30THENOPEN,
  DREV_CLOSE,
  DREV_OPEN,
  DREV_RAISEIN5MINS,
} vldoor_e;

// p_ceilng

typedef enum
{
  lowerToFloor,
  raiseToHighest,
  lowerToLowest,
  lowerToMaxFloor,
  lowerAndCrush,
  crushAndRaise,
  fastCrushAndRaise,
  silentCrushAndRaise,

  //jff 02/04/98 add types for generalized ceiling mover
  genCeiling,
  genCeilingChg,
  genCeilingChg0,
  genCeilingChgT,

  //jff 02/05/98 add types for generalized ceiling mover
  genCrusher,
  genSilentCrusher,

  // hexen - can probably be merged
  CLEV_LOWERTOFLOOR,
  CLEV_RAISETOHIGHEST,
  CLEV_LOWERANDCRUSH,
  CLEV_CRUSHANDRAISE,
  CLEV_LOWERBYVALUE,
  CLEV_RAISEBYVALUE,
  CLEV_CRUSHRAISEANDSTAY,
  CLEV_MOVETOVALUETIMES8,
} ceiling_e;

// p_floor

typedef enum
{
  // lower floor to highest surrounding floor
  lowerFloor,

  // lower floor to lowest surrounding floor
  lowerFloorToLowest,

  // lower floor to highest surrounding floor VERY FAST
  turboLower,

  // raise floor to lowest surrounding CEILING
  raiseFloor,

  // raise floor to next highest surrounding floor
  raiseFloorToNearest,

  //jff 02/03/98 lower floor to next lowest neighbor
  lowerFloorToNearest,

  //jff 02/03/98 lower floor 24 absolute
  lowerFloor24,

  //jff 02/03/98 lower floor 32 absolute
  lowerFloor32Turbo,

  // raise floor to shortest height texture around it
  raiseToTexture,

  // lower floor to lowest surrounding floor
  //  and change floorpic
  lowerAndChange,

  raiseFloor24,

  //jff 02/03/98 raise floor 32 absolute
  raiseFloor32Turbo,

  raiseFloor24AndChange,
  raiseFloorCrush,

  // raise to next highest floor, turbo-speed
  raiseFloorTurbo,
  donutRaise,
  raiseFloor512,

  //jff 02/04/98  add types for generalized floor mover
  genFloor,
  genFloorChg,
  genFloorChg0,
  genFloorChgT,

  //new types for stair builders
  buildStair,
  genBuildStair,

  // hexen - can probably be merged
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
  FLEV_MOVETOVALUETIMES8,
} floor_e;

typedef enum
{
  build8, // slowly build by 8
  turbo16, // quickly build by 16

  // heretic
  heretic_build8,
  heretic_turbo16
} stair_e;

typedef enum
{
  elevateUp,
  elevateDown,
  elevateCurrent,
} elevator_e;

//////////////////////////////////////////////////////////////////
//
// general enums
//
//////////////////////////////////////////////////////////////////

// texture type enum
typedef enum
{
    top,
    middle,
    bottom

} bwhere_e;

// crush check returns
typedef enum
{
  ok,
  crushed,
  pastdest
} result_e;

//////////////////////////////////////////////////////////////////
//
// linedef and sector special data types
//
//////////////////////////////////////////////////////////////////

// p_switch

// switch animation structure type

#if defined(__MWERKS__)
#pragma options align=packed
#endif

typedef struct
{
  char name1[9];
  char name2[9];
  short episode;
} PACKEDATTR switchlist_t; //jff 3/23/98 pack to read from memory

#if defined(__MWERKS__)
#pragma options align=reset
#endif

typedef struct
{
  line_t* line;
  bwhere_e where;
  int   btexture;
  int   btimer;
  mobj_t* soundorg;

} button_t;

// p_lights

typedef struct
{
  thinker_t thinker;
  sector_t* sector;
  int count;
  int maxlight;
  int minlight;

} fireflicker_t;

typedef struct
{
  thinker_t thinker;
  sector_t* sector;
  int count;
  int maxlight;
  int minlight;
  int maxtime;
  int mintime;

} lightflash_t;

typedef struct
{
  thinker_t thinker;
  sector_t* sector;
  int count;
  int minlight;
  int maxlight;
  int darktime;
  int brighttime;

} strobe_t;

typedef struct
{
  thinker_t thinker;
  sector_t* sector;
  int minlight;
  int maxlight;
  int direction;

} glow_t;

// p_plats

typedef struct
{
  thinker_t thinker;
  sector_t* sector;
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

  struct platlist *list;   // killough
} plat_t;

// New limit-free plat structure -- killough

typedef struct platlist {
  plat_t *plat;
  struct platlist *next,**prev;
} platlist_t;

// p_ceilng

typedef struct
{
  thinker_t thinker;
  vldoor_e type;
  sector_t* sector;
  fixed_t topheight;
  fixed_t speed;

  // 1 = up, 0 = waiting at top, -1 = down
  int direction;

  // tics to wait at the top
  int topwait;
  // (keep in case a door going down is reset)
  // when it reaches 0, start going down
  int topcountdown;

  //jff 1/31/98 keep track of line door is triggered by
  line_t *line;

  /* killough 10/98: sector tag for gradual lighting effects */
  int lighttag;
} vldoor_t;

// p_doors

typedef struct
{
  thinker_t thinker;
  ceiling_e type;
  sector_t* sector;
  fixed_t bottomheight;
  fixed_t topheight;
  fixed_t speed;
  fixed_t oldspeed;
  int crush;

  //jff 02/04/98 add these to support ceiling changers
  int newspecial;
  unsigned int flags;
  short texture;

  // 1 = up, 0 = waiting, -1 = down
  int direction;

  // ID
  int tag;
  int olddirection;
  struct ceilinglist *list;   // jff 2/22/98 copied from killough's plats
} ceiling_t;

typedef struct ceilinglist {
  ceiling_t *ceiling;
  struct ceilinglist *next,**prev;
} ceilinglist_t;

// p_floor

typedef struct
{
  thinker_t thinker;
  floor_e type;
  int crush;
  sector_t* sector;
  int direction;
  int newspecial;
  unsigned int flags;
  short texture;
  fixed_t floordestheight;
  fixed_t speed;

  // hexen
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
  elevator_e type;
  sector_t* sector;
  int direction;
  fixed_t floordestheight;
  fixed_t ceilingdestheight;
  fixed_t speed;
} elevator_t;

// p_spec

// killough 3/7/98: Add generalized scroll effects

typedef struct {
  thinker_t thinker;   // Thinker structure for scrolling
  fixed_t dx, dy;      // (dx,dy) scroll speeds
  int affectee;        // Number of affected sidedef, sector, tag, or whatever
  int control;         // Control sector (-1 if none) used to control scrolling
  fixed_t last_height; // Last known height of control sector
  fixed_t vdx, vdy;    // Accumulated velocity if accelerative
  int accel;           // Whether it's accelerative
  enum
  {
    sc_side,
    sc_floor,
    sc_ceiling,
    sc_carry,
    sc_carry_ceiling,  // killough 4/11/98: carry objects hanging on ceilings
  } type;              // Type of scroll effect
} scroll_t;

// phares 3/12/98: added new model of friction for ice/sludge effects

typedef struct {
  thinker_t thinker;   // Thinker structure for friction
  int friction;        // friction value (E800 = normal)
  int movefactor;      // inertia factor when adding to momentum
  int affectee;        // Number of affected sector
} friction_t;

// phares 3/20/98: added new model of Pushers for push/pull effects

typedef struct {
  thinker_t thinker;   // Thinker structure for Pusher
  enum
  {
    p_push,
    p_pull,
    p_wind,
    p_current,
  } type;
  mobj_t* source;      // Point source if point pusher
  int x_mag;           // X Strength
  int y_mag;           // Y Strength
  int magnitude;       // Vector strength for point pusher
  int radius;          // Effective radius for point pusher
  int x;               // X of point source if point pusher
  int y;               // Y of point source if point pusher
  int affectee;        // Number of affected sector
} pusher_t;

//////////////////////////////////////////////////////////////////
//
// external data declarations
//
//////////////////////////////////////////////////////////////////

// list of retriggerable buttons active
extern button_t buttonlist[MAXBUTTONS];

extern platlist_t *activeplats;        // killough 2/14/98

extern ceilinglist_t *activeceilings;  // jff 2/22/98

////////////////////////////////////////////////////////////////
//
// Linedef and sector special utility function prototypes
//
////////////////////////////////////////////////////////////////

int twoSided
( int sector,
  int line );

sector_t* getSector
( int currentSector,
  int line,
  int side );

side_t* getSide
( int   currentSector,
  int   line,
  int   side );

fixed_t P_FindLowestFloorSurrounding
( sector_t* sec );

fixed_t P_FindHighestFloorSurrounding
( sector_t* sec );

fixed_t P_FindNextHighestFloor
( sector_t* sec,
  int currentheight );

fixed_t P_FindNextLowestFloor
( sector_t* sec,
  int currentheight );

fixed_t P_FindLowestCeilingSurrounding
( sector_t* sec ); // jff 2/04/98

fixed_t P_FindHighestCeilingSurrounding
( sector_t* sec ); // jff 2/04/98

fixed_t P_FindNextLowestCeiling
( sector_t *sec,
  int currentheight ); // jff 2/04/98

fixed_t P_FindNextHighestCeiling
( sector_t *sec,
  int currentheight ); // jff 2/04/98

fixed_t P_FindShortestTextureAround
( int secnum ); // jff 2/04/98

fixed_t P_FindShortestUpperAround
( int secnum ); // jff 2/04/98

sector_t* P_FindModelFloorSector
( fixed_t floordestheight,
  int secnum ); //jff 02/04/98

sector_t* P_FindModelCeilingSector
( fixed_t ceildestheight,
  int secnum ); //jff 02/04/98

int P_FindSectorFromLineTag
( const line_t *line,
  int start ); // killough 4/17/98

int P_FindLineFromLineTag
( const line_t *line,
  int start );   // killough 4/17/98

int P_FindMinSurroundingLight
( sector_t* sector,
  int max );

sector_t* getNextSector
( line_t* line,
  sector_t* sec );

int P_CheckTag
(line_t *line); // jff 2/27/98

dboolean P_CanUnlockGenDoor
( line_t* line,
  player_t* player);

dboolean PUREFUNC P_SectorActive
( special_e t,
  const sector_t* s );

dboolean PUREFUNC P_IsSecret
( const sector_t *sec );

dboolean PUREFUNC P_WasSecret
( const sector_t *sec );

void P_ChangeSwitchTexture
( line_t* line,
  int useAgain );

////////////////////////////////////////////////////////////////
//
// Linedef and sector special action function prototypes
//
////////////////////////////////////////////////////////////////

// p_lights

void T_LightFlash
( lightflash_t* flash );

void T_StrobeFlash
( strobe_t* flash );

// jff 8/8/98 add missing thinker for flicker
void T_FireFlicker
( fireflicker_t* flick );

void T_Glow
( glow_t* g );

// p_plats

void T_PlatRaise
( plat_t* plat );

// p_doors

void T_VerticalDoor
( vldoor_t* door );

// p_ceilng

void T_MoveCeiling
( ceiling_t* ceiling );

// p_floor

result_e T_MovePlane
( sector_t* sector,
  fixed_t speed,
  fixed_t dest,
  dboolean crush,
  int floorOrCeiling,
  int direction );

void T_MoveFloor
( floormove_t* floor );

void T_MoveElevator
( elevator_t* elevator );

// p_spec

void T_Scroll
( scroll_t * );      // killough 3/7/98: scroll effect thinker

void T_Friction
( friction_t * );    // phares 3/12/98: friction thinker

void T_Pusher
( pusher_t * );      // phares 3/20/98: Push thinker

////////////////////////////////////////////////////////////////
//
// Linedef and sector special handler prototypes
//
////////////////////////////////////////////////////////////////

// p_telept

int EV_Teleport
( line_t* line,
  int side,
  mobj_t* thing );

// killough 2/14/98: Add silent teleporter
int EV_SilentTeleport
( line_t* line,
  int side,
  mobj_t* thing );

// killough 1/31/98: Add silent line teleporter
int EV_SilentLineTeleport
( line_t* line,
  int side,
  mobj_t* thing,
  dboolean reverse);

// p_floor

int
EV_DoElevator
( line_t* line,
  elevator_e type );

int EV_BuildStairs
( line_t* line,
  stair_e type );

int EV_DoFloor
( line_t* line,
  floor_e floortype );

// p_ceilng

int EV_DoCeiling
( line_t* line,
  ceiling_e type );

int EV_CeilingCrushStop
( line_t* line );

// p_doors

int EV_VerticalDoor
( line_t* line,
  mobj_t* thing );

int EV_DoDoor
( line_t* line,
  vldoor_e type );

int EV_DoLockedDoor
( line_t* line,
  vldoor_e type,
  mobj_t* thing );

// p_lights

int EV_StartLightStrobing
( line_t* line );

int EV_TurnTagLightsOff
( line_t* line );

int EV_LightTurnOn
( line_t* line,
  int   bright );

int EV_LightTurnOnPartway(line_t* line, fixed_t level); // killough 10/10/98

// p_floor

int EV_DoChange
( line_t* line,
  change_e changetype );

int EV_DoDonut
( line_t* line );

// p_plats

int EV_DoPlat
( line_t* line,
  plattype_e  type,
  int amount );

int EV_StopPlat
( line_t* line );

// p_genlin

int EV_DoGenFloor
( line_t* line );

int EV_DoGenCeiling
( line_t* line );

int EV_DoGenLift
( line_t* line );

int EV_DoGenStairs
( line_t* line );

int EV_DoGenCrusher
( line_t* line );

int EV_DoGenDoor
( line_t* line );

int EV_DoGenLockedDoor
( line_t* line );

////////////////////////////////////////////////////////////////
//
// Linedef and sector special thinker spawning
//
////////////////////////////////////////////////////////////////

// at game start
void P_InitPicAnims
( void );

void P_InitSwitchList
( void );

// at map load
void P_SpawnSpecials
( void );

// every tic
void P_UpdateSpecials
( void );

// when needed
dboolean P_UseSpecialLine
( mobj_t* thing,
  line_t* line,
  int   side,
	dboolean noplayercheck);

void P_ShootSpecialLine
( mobj_t* thing,
  line_t* line );

void P_CrossSpecialLine(line_t *line, int side, mobj_t *thing, dboolean noplayercheck);

void P_PlayerInSpecialSector
( player_t* player );

// p_lights

void P_SpawnFireFlicker
( sector_t* sector );

void P_SpawnLightFlash
( sector_t* sector );

void P_SpawnStrobeFlash
( sector_t* sector,
  int fastOrSlow,
  int inSync );

void P_SpawnGlowingLight
( sector_t* sector );

// p_plats

void P_AddActivePlat
( plat_t* plat );

void P_RemoveActivePlat
( plat_t* plat );

void P_RemoveAllActivePlats
( void );    // killough

void P_ActivateInStasis
( int tag );

// p_doors

void P_SpawnDoorCloseIn30
( sector_t* sec );

void P_SpawnDoorRaiseIn5Mins
( sector_t* sec,
  int secnum );

// p_ceilng

void P_RemoveActiveCeiling
( ceiling_t* ceiling );  //jff 2/22/98

void P_RemoveAllActiveCeilings
( void );                //jff 2/22/98

void P_AddActiveCeiling
( ceiling_t* c );

int P_ActivateInStasisCeiling
( line_t* line );

mobj_t* P_GetPushThing(int);                                // phares 3/23/98

// heretic

void P_InitTerrainTypes(void);
void P_InitLava(void);
void Heretic_P_CrossSpecialLine(line_t * line, int side, mobj_t * thing);
void P_SpawnLineSpecials(void);

extern int *TerrainTypes;

void P_InitAmbientSound(void);
void P_AmbientSound(void);
void P_AddAmbientSfx(int sequence);
dboolean P_Teleport(mobj_t * thing, fixed_t x, fixed_t y, angle_t angle, dboolean useFog);
dboolean Heretic_EV_Teleport(line_t * line, int side, mobj_t * thing);
dboolean Heretic_P_UseSpecialLine(mobj_t * thing, line_t * line, int side, dboolean bossaction);
void Heretic_EV_VerticalDoor(line_t * line, mobj_t * thing);

// hexen

// p_lights

typedef enum
{
    LITE_RAISEBYVALUE,
    LITE_LOWERBYVALUE,
    LITE_CHANGETOVALUE,
    LITE_FADE,
    LITE_GLOW,
    LITE_FLICKER,
    LITE_STROBE
} lighttype_t;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    lighttype_t type;
    int value1;
    int value2;
    int tics1;
    int tics2;
    int count;
} light_t;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
    int index;
    int base;
} phase_t;

#define LIGHT_SEQUENCE_START    2
#define LIGHT_SEQUENCE          3
#define LIGHT_SEQUENCE_ALT      4

void T_Phase(phase_t * phase);
void T_Light(light_t * light);
void P_SpawnPhasedLight(sector_t * sector, int base, int index);
void P_SpawnLightSequence(sector_t * sector, int indexStep);
dboolean EV_SpawnLight(line_t * line, byte * arg, lighttype_t type);

// p_ceilng

int Hexen_EV_CeilingCrushStop(line_t * line, byte * args);
int Hexen_EV_DoCeiling(line_t * line, byte * arg, ceiling_e type);

// p_telept

dboolean Hexen_EV_Teleport(int tid, mobj_t * thing, dboolean fog);

// p_doors

int Hexen_EV_DoDoor(line_t * line, byte * args, vldoor_e type);
dboolean Hexen_EV_VerticalDoor(line_t * line, mobj_t * thing);

// p_floor

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

typedef enum
{
  STAIRS_NORMAL,
  STAIRS_SYNC,
  STAIRS_PHASED
} stairs_e;

int Hexen_EV_DoFloor(line_t * line, byte * args, floor_e floortype);
int EV_DoFloorAndCeiling(line_t * line, byte * args, dboolean raise);
int Hexen_EV_BuildStairs(line_t * line, byte * args, int direction, stairs_e stairsType);
void T_BuildPillar(pillar_t * pillar);
int EV_BuildPillar(line_t * line, byte * args, dboolean crush);
int EV_OpenPillar(line_t * line, byte * args);
int EV_FloorCrushStop(line_t * line, byte * args);
void T_FloorWaggle(floorWaggle_t * waggle);
dboolean EV_StartFloorWaggle(int tag, int height, int speed, int offset, int timer);

// p_plats

int Hexen_EV_DoPlat(line_t * line, byte * args, plattype_e type, int amount);
void Hexen_EV_StopPlat(line_t * line, byte * args);

//

dboolean P_ActivateLine(line_t * line, mobj_t * mo, int side, int activationType);
dboolean P_ExecuteLineSpecial(int special, byte * args, line_t * line, int side, mobj_t * mo);
void P_PlayerOnSpecialFlat(player_t * player, int floorType);
line_t *P_FindLine(int lineTag, int *searchPosition);
int P_FindSectorFromTag(int tag, int start);

void P_TransferSectorFlags(unsigned int *dest, unsigned int source);
void P_ResetSectorTransferFlags(unsigned int *flags);
void P_ClearNonGeneralizedSectorSpecial(sector_t *sector);

typedef enum {
  zs_light_phased            = 1,
  zs_light_sequence_start    = 2,
  zs_light_sequence_special1 = 3,
  zs_light_sequence_special2 = 4,

  zs_stairs_special1 = 26,
  zs_stairs_special2 = 27,

  zs_wind_east_weak    = 40,
  zs_wind_east_medium  = 41,
  zs_wind_east_strong  = 42,
  zs_wind_north_weak   = 43,
  zs_wind_north_medium = 44,
  zs_wind_north_strong = 45,
  zs_wind_south_weak   = 46,
  zs_wind_south_medium = 47,
  zs_wind_south_strong = 48,
  zs_wind_west_weak    = 49,
  zs_wind_west_medium  = 50,
  zs_wind_west_strong  = 51,

  zs_d_light_flicker               = 65,
  zs_d_light_strobe_fast           = 66,
  zs_d_light_strobe_slow           = 67,
  zs_d_light_strobe_hurt           = 68,
  zs_d_damage_hellslime            = 69,

  zs_d_damage_nukage               = 71,
  zs_d_light_glow                  = 72,

  zs_d_sector_door_close_in_30     = 74,
  zs_d_damage_end                  = 75,
  zs_d_light_strobe_slow_sync      = 76,
  zs_d_light_strobe_fast_sync      = 77,
  zs_d_sector_door_raise_in_5_mins = 78,
  zs_d_friction_low                = 79,
  zs_d_damage_super_hellslime      = 80,
  zs_d_light_fire_flicker          = 81,
  zs_d_damage_lava_wimpy           = 82,
  zs_d_damage_lava_hefty           = 83,
  zs_d_scroll_east_lava_damage     = 84,
  zs_h_damage_sludge               = 85,

  zs_sector_outside                = 87,

  zs_s_light_strobe_hurt      = 104,
  zs_s_damage_hellslime       = 105,
  zs_damage_instant_death     = 115,
  zs_s_damage_super_hellslime = 116,
  zs_scroll_strife_current    = 118,

  zs_sector_hidden = 195,
  zs_sector_heal   = 196,

  zs_light_outdoor_lightning = 197,
  zs_light_indoor_lightning1 = 198,
  zs_light_indoor_lightning2 = 199,

  zs_sky2 = 200,

  // hexen-type scrollers
  zs_scroll_north_slow       = 201,
  zs_scroll_north_medium     = 202,
  zs_scroll_north_fast       = 203,
  zs_scroll_east_slow        = 204,
  zs_scroll_east_medium      = 205,
  zs_scroll_east_fast        = 206,
  zs_scroll_south_slow       = 207,
  zs_scroll_south_medium     = 208,
  zs_scroll_south_fast       = 209,
  zs_scroll_west_slow        = 210,
  zs_scroll_west_medium      = 211,
  zs_scroll_west_fast        = 212,
  zs_scroll_northwest_slow   = 213,
  zs_scroll_northwest_medium = 214,
  zs_scroll_northwest_fast   = 215,
  zs_scroll_northeast_slow   = 216,
  zs_scroll_northeast_medium = 217,
  zs_scroll_northeast_fast   = 218,
  zs_scroll_southeast_slow   = 219,
  zs_scroll_southeast_medium = 220,
  zs_scroll_southeast_fast   = 221,
  zs_scroll_southwest_slow   = 222,
  zs_scroll_southwest_medium = 223,
  zs_scroll_southwest_fast   = 224,

  // heretic-type scrollers
  zs_carry_east5   = 225,
  zs_carry_east10  = 226,
  zs_carry_east25  = 227,
  zs_carry_east30  = 228,
  zs_carry_east35  = 229,
  zs_carry_north5  = 230,
  zs_carry_north10 = 231,
  zs_carry_north25 = 232,
  zs_carry_north30 = 233,
  zs_carry_north35 = 234,
  zs_carry_south5  = 235,
  zs_carry_south10 = 236,
  zs_carry_south25 = 237,
  zs_carry_south30 = 238,
  zs_carry_south35 = 239,
  zs_carry_west5   = 240,
  zs_carry_west10  = 241,
  zs_carry_west25  = 242,
  zs_carry_west30  = 243,
  zs_carry_west35  = 244
} zdoom_sectorspecial_t;

typedef enum {
  zl_polyobj_start_line                  = 1,
  zl_polyobj_rotate_left                 = 2,
  zl_polyobj_rotate_right                = 3,
  zl_polyobj_move                        = 4,
  zl_polyobj_explicit_line               = 5,
  zl_polyobj_move_times_8                = 6,
  zl_polyobj_door_swing                  = 7,
  zl_polyobj_door_slide                  = 8,
  zl_line_horizon                        = 9,
  zl_door_close                          = 10,
  zl_door_open                           = 11,
  zl_door_raise                          = 12,
  zl_door_locked_raise                   = 13,
  zl_door_animated                       = 14,
  zl_autosave                            = 15,
  zl_transfer_wall_light                 = 16,
  zl_thing_raise                         = 17,
  zl_start_conversation                  = 18,
  zl_thing_stop                          = 19,
  zl_floor_lower_by_value                = 20,
  zl_floor_lower_to_lowest               = 21,
  zl_floor_lower_to_nearest              = 22,
  zl_floor_raise_by_value                = 23,
  zl_floor_raise_to_highest              = 24,
  zl_floor_raise_to_nearest              = 25,
  zl_stairs_build_down                   = 26,
  zl_stairs_build_up                     = 27,
  zl_floor_raise_and_crush               = 28,
  zl_pillar_build                        = 29,
  zl_pillar_open                         = 30,
  zl_stairs_build_down_sync              = 31,
  zl_stairs_build_up_sync                = 32,
  zl_force_field                         = 33,
  zl_clear_force_field                   = 34,
  zl_floor_raise_by_value_times_8        = 35,
  zl_floor_lower_by_value_times_8        = 36,
  zl_floor_move_to_value                 = 37,
  zl_ceiling_waggle                      = 38,
  zl_teleport_zombie_changer             = 39,
  zl_ceiling_lower_by_value              = 40,
  zl_ceiling_raise_by_value              = 41,
  zl_ceiling_crush_and_raise             = 42,
  zl_ceiling_lower_and_crush             = 43,
  zl_ceiling_crush_stop                  = 44,
  zl_ceiling_crush_raise_and_stay        = 45,
  zl_floor_crush_stop                    = 46,
  zl_ceiling_move_to_value               = 47,
  zl_sector_attach_3d_midtex             = 48,
  zl_glass_break                         = 49,
  zl_extra_floor_light_only              = 50,
  zl_sector_set_link                     = 51,
  zl_scroll_wall                         = 52,
  zl_line_set_texture_offset             = 53,
  zl_sector_change_flags                 = 54,
  zl_line_set_blocking                   = 55,
  zl_line_set_texturescale               = 56,
  zl_sector_set_portal                   = 57,
  zl_sector_copy_scroller                = 58,
  zl_polyobj_or_move_to_spot             = 59,
  zl_plat_perpetual_raise                = 60,
  zl_plat_stop                           = 61,
  zl_plat_down_wait_up_stay              = 62,
  zl_plat_down_by_value                  = 63,
  zl_plat_up_wait_down_stay              = 64,
  zl_plat_up_by_value                    = 65,
  zl_floor_lower_instant                 = 66,
  zl_floor_raise_instant                 = 67,
  zl_floor_move_to_value_times_8         = 68,
  zl_ceiling_move_to_value_times_8       = 69,
  zl_teleport                            = 70,
  zl_teleport_no_fog                     = 71,
  zl_thrust_thing                        = 72,
  zl_damage_thing                        = 73,
  zl_teleport_new_map                    = 74,
  zl_teleport_end_game                   = 75,
  zl_teleport_other                      = 76,
  zl_teleport_group                      = 77,
  zl_teleport_in_sector                  = 78,
  zl_thing_set_conversation              = 79,
  zl_acs_execute                         = 80,
  zl_acs_suspend                         = 81,
  zl_acs_terminate                       = 82,
  zl_acs_locked_execute                  = 83,
  zl_acs_execute_with_result             = 84,
  zl_acs_locked_execute_door             = 85,
  zl_polyobj_move_to_spot                = 86,
  zl_polyobj_stop                        = 87,
  zl_polyobj_move_to                     = 88,
  zl_polyobj_or_move_to                  = 89,
  zl_polyobj_or_rotate_left              = 90,
  zl_polyobj_or_rotate_right             = 91,
  zl_polyobj_or_move                     = 92,
  zl_polyobj_or_move_times_8             = 93,
  zl_pillar_build_and_crush              = 94,
  zl_floor_and_ceiling_lower_by_value    = 95,
  zl_floor_and_ceiling_raise_by_value    = 96,
  zl_ceiling_lower_and_crush_dist        = 97,
  zl_sector_set_translucent              = 98,
  zl_floor_raise_and_crushdoom           = 99,
  zl_scroll_texture_left                 = 100,
  zl_scroll_texture_right                = 101,
  zl_scroll_texture_up                   = 102,
  zl_scroll_texture_down                 = 103,
  zl_ceiling_crush_and_raise_silent_dist = 104,
  zl_door_wait_raise                     = 105,
  zl_door_wait_close                     = 106,
  zl_line_set_portal_target              = 107,

  zl_light_force_lightning   = 109,
  zl_light_raise_by_value    = 110,
  zl_light_lower_by_value    = 111,
  zl_light_change_to_value   = 112,
  zl_light_fade              = 113,
  zl_light_glow              = 114,
  zl_light_flicker           = 115,
  zl_light_strobe            = 116,
  zl_light_stop              = 117,
  zl_plane_copy              = 118,
  zl_thing_damage            = 119,
  zl_radius_quake            = 120,
  zl_line_set_identification = 121,

  zl_thing_move = 125,

  zl_thing_set_special        = 127,
  zl_thrust_thing_z           = 128,
  zl_use_puzzle_item          = 129,
  zl_thing_activate           = 130,
  zl_thing_deactivate         = 131,
  zl_thing_remove             = 132,
  zl_thing_destroy            = 133,
  zl_thing_projectile         = 134,
  zl_thing_spawn              = 135,
  zl_thing_projectile_gravity = 136,
  zl_thing_spawn_no_fog       = 137,
  zl_floor_waggle             = 138,
  zl_thing_spawn_facing       = 139,
  zl_sector_change_sound      = 140,

  zl_player_set_team = 145,

  zl_team_score       = 152,
  zl_team_give_points = 153,
  zl_teleport_no_stop = 154,

  zl_set_global_fog_parameter    = 157,
  zl_fs_execute                  = 158,
  zl_sector_set_plane_reflection = 159,
  zl_sector_set_3d_floor         = 160,
  zl_sector_set_contents         = 161,

  zl_ceiling_crush_and_raise_dist       = 168,
  zl_generic_crusher2                   = 169,
  zl_sector_set_ceiling_scale2          = 170,
  zl_sector_set_floor_scale2            = 171,
  zl_plat_up_nearest_wait_down_stay     = 172,
  zl_noise_alert                        = 173,
  zl_send_to_communicator               = 174,
  zl_thing_projectile_intercept         = 175,
  zl_thing_change_tid                   = 176,
  zl_thing_hate                         = 177,
  zl_thing_projectile_aimed             = 178,
  zl_change_skill                       = 179,
  zl_thing_set_translation              = 180,
  zl_plane_align                        = 181,
  zl_line_mirror                        = 182,
  zl_line_align_ceiling                 = 183,
  zl_line_align_floor                   = 184,
  zl_sector_set_rotation                = 185,
  zl_sector_set_ceiling_panning         = 186,
  zl_sector_set_floor_panning           = 187,
  zl_sector_set_ceiling_scale           = 188,
  zl_sector_set_floor_scale             = 189,
  zl_static_init                        = 190,
  zl_set_player_property                = 191,
  zl_ceiling_lower_to_highest_floor     = 192,
  zl_ceiling_lower_instant              = 193,
  zl_ceiling_raise_instant              = 194,
  zl_ceiling_crush_raise_and_stay_a     = 195,
  zl_ceiling_crush_and_raise_a          = 196,
  zl_ceiling_crush_and_raise_silent_a   = 197,
  zl_ceiling_raise_by_value_times_8     = 198,
  zl_ceiling_lower_by_value_times_8     = 199,
  zl_generic_floor                      = 200,
  zl_generic_ceiling                    = 201,
  zl_generic_door                       = 202,
  zl_generic_lift                       = 203,
  zl_generic_stairs                     = 204,
  zl_generic_crusher                    = 205,
  zl_plat_down_wait_up_stay_lip         = 206,
  zl_plat_perpetual_raise_lip           = 207,
  zl_translucent_line                   = 208,
  zl_transfer_heights                   = 209,
  zl_transfer_floor_light               = 210,
  zl_transfer_ceiling_light             = 211,
  zl_sector_set_color                   = 212,
  zl_sector_set_fade                    = 213,
  zl_sector_set_damage                  = 214,
  zl_teleport_line                      = 215,
  zl_sector_set_gravity                 = 216,
  zl_stairs_build_up_doom               = 217,
  zl_sector_set_wind                    = 218,
  zl_sector_set_friction                = 219,
  zl_sector_set_current                 = 220,
  zl_scroll_texture_both                = 221,
  zl_scroll_texture_model               = 222,
  zl_scroll_floor                       = 223,
  zl_scroll_ceiling                     = 224,
  zl_scroll_texture_offsets             = 225,
  zl_acs_execute_always                 = 226,
  zl_point_push_set_force               = 227,
  zl_plat_raise_and_stay_tx0            = 228,
  zl_thing_set_goal                     = 229,
  zl_plat_up_by_value_stay_tx           = 230,
  zl_plat_toggle_ceiling                = 231,
  zl_light_strobe_doom                  = 232,
  zl_light_min_neighbor                 = 233,
  zl_light_max_neighbor                 = 234,
  zl_floor_transfer_trigger             = 235,
  zl_floor_transfer_numeric             = 236,
  zl_change_camera                      = 237,
  zl_floor_raise_to_lowest_ceiling      = 238,
  zl_floor_raise_by_value_tx_ty         = 239,
  zl_floor_raise_by_texture             = 240,
  zl_floor_lower_to_lowest_tx_ty        = 241,
  zl_floor_lower_to_highest             = 242,
  zl_exit_normal                        = 243,
  zl_exit_secret                        = 244,
  zl_elevator_raise_to_nearest          = 245,
  zl_elevator_move_to_floor             = 246,
  zl_elevator_lower_to_nearest          = 247,
  zl_heal_thing                         = 248,
  zl_door_close_wait_open               = 249,
  zl_floor_donut                        = 250,
  zl_floorandceiling_lowerraise         = 251,
  zl_ceiling_raise_to_nearest           = 252,
  zl_ceiling_lower_to_lowest            = 253,
  zl_ceiling_lower_to_floor             = 254,
  zl_ceiling_crush_raise_and_stay_sil_a = 255,

  zl_floor_lower_to_highest_ee      = 256,
  zl_floor_raise_to_lowest          = 257,
  zl_floor_lower_to_lowest_ceiling  = 258,
  zl_floor_raise_to_ceiling         = 259,
  zl_floor_to_ceiling_instant       = 260,
  zl_floor_lower_by_texture         = 261,
  zl_ceiling_raise_to_highest       = 262,
  zl_ceiling_to_highest_instant     = 263,
  zl_ceiling_lower_to_nearest       = 264,
  zl_ceiling_raise_to_lowest        = 265,
  zl_ceiling_raise_to_highest_floor = 266,
  zl_ceiling_to_floor_instant       = 267,
  zl_ceiling_raise_by_texture       = 268,
  zl_ceiling_lower_by_texture       = 269,
  zl_stairs_build_down_doom         = 270,
  zl_stairs_build_up_doom_sync      = 271,
  zl_stairs_build_down_doom_sync    = 272
} zl_linespecial_t;

#define ZDOOM_DAMAGE_MASK   0x0300
#define ZDOOM_DAMAGE_05P    0x0100
#define ZDOOM_DAMAGE_10P    0x0200
#define ZDOOM_DAMAGE_20P    0x0300
#define ZDOOM_SECRET_MASK   0x0400
#define ZDOOM_FRICTION_MASK 0x0800
#define ZDOOM_PUSH_MASK     0x1000

#endif
