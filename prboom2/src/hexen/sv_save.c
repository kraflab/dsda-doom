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

#include "doomstat.h"
#include "p_setup.h"
#include "p_spec.h"
#include "d_player.h"
#include "p_mobj.h"
#include "p_map.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "lprintf.h"

#include "hexen/p_acs.h"

#include "sv_save.h"

#define MAX_TARGET_PLAYERS 512
#define MOBJ_NULL -1
#define MOBJ_XX_PLAYER -2
#define MAX_MAPS 99
#define BASE_SLOT 6

typedef enum
{
    ASEG_GAME_HEADER = 101,
    ASEG_MAP_HEADER,
    ASEG_WORLD,
    ASEG_POLYOBJS,
    ASEG_MOBJS,
    ASEG_THINKERS,
    ASEG_SCRIPTS,
    ASEG_PLAYERS,
    ASEG_SOUNDS,
    ASEG_MISC,
    ASEG_END
} gameArchiveSegment_t;

typedef enum
{
    TC_NULL,
    TC_MOVE_CEILING,
    TC_VERTICAL_DOOR,
    TC_MOVE_FLOOR,
    TC_PLAT_RAISE,
    TC_INTERPRET_ACS,
    TC_FLOOR_WAGGLE,
    TC_LIGHT,
    TC_PHASE,
    TC_BUILD_PILLAR,
    TC_ROTATE_POLY,
    TC_MOVE_POLY,
    TC_POLY_DOOR
} thinkClass_t;

typedef struct
{
    thinkClass_t tClass;
    think_t thinkerFunc;
    void (*writeFunc)();
    void (*readFunc)();
    void (*restoreFunc) ();
    size_t size;
} thinkInfo_t;

typedef struct
{
    thinker_t thinker;
    sector_t *sector;
} ssthinker_t;

const char *SavePath = "hexndata/";

int vanilla_savegame_limit = 1;

static int MobjCount;
static mobj_t **MobjList;
static mobj_t ***TargetPlayerAddrs;
static int TargetPlayerCount;
static FILE *SavingFP;

extern int inv_ptr;
extern int curpos;

// static void ArchiveWorld(void);
// static void UnarchiveWorld(void);
// static void ArchivePolyobjs(void);
// static void UnarchivePolyobjs(void);
// static void ArchiveMobjs(void);
// static void UnarchiveMobjs(void);
// static void ArchiveThinkers(void);
// static void UnarchiveThinkers(void);
// static void ArchiveScripts(void);
// static void UnarchiveScripts(void);
// static void ArchivePlayers(void);
// static void UnarchivePlayers(void);
// static void ArchiveSounds(void);
// static void UnarchiveSounds(void);
// static void ArchiveMisc(void);
// static void UnarchiveMisc(void);
// static void SetMobjArchiveNums(void);
// static void RemoveAllThinkers(void);
// static int GetMobjNum(mobj_t * mobj);
// static void SetMobjPtr(mobj_t **ptr, unsigned int archiveNum);
// static void RestoreSSThinker(ssthinker_t * sst);
// static void RestorePlatRaise(plat_t * plat);
// static void RestoreMoveCeiling(ceiling_t * ceiling);
// static void AssertSegment(gameArchiveSegment_t segType);
// static void CopySaveSlot(int sourceSlot, int destSlot);
// static void CopyFile(char *sourceName, char *destName);
// static dboolean ExistingFile(char *name);
// static void SV_OpenRead(char *fileName);
// static void SV_OpenWrite(char *fileName);
// static void SV_Close(void);
// static void SV_Read(void *buffer, int size);
// static byte SV_ReadByte(void);
// static uint16_t SV_ReadWord(void);
// static uint32_t SV_ReadLong(void);
// static void *SV_ReadPtr(void);
// static void SV_Write(const void *buffer, int size);
// static void SV_WriteByte(byte val);
// static void SV_WriteWord(unsigned short val);
// static void SV_WriteLong(unsigned int val);
// static void SV_WritePtr(void *ptr);

static void StreamInMobjSpecials(mobj_t *mobj)
{
    mobj->special1.i = SV_ReadLong();
    SetMobjPtr(&mobj->special1.m, SV_ReadLong());
    mobj->special2.i = SV_ReadLong();
    SetMobjPtr(&mobj->special2.m, SV_ReadLong());
}

static void StreamIn_mobj_t(mobj_t *str)
{
    unsigned int i;

    // fixed_t x, y, z;
    str->x = SV_ReadLong();
    str->y = SV_ReadLong();
    str->z = SV_ReadLong();

    // struct mobj_s *snext, *sprev;
    // Pointer values are discarded:
    str->snext = SV_ReadPtr();
    str->snext = NULL;
    str->sprev = SV_ReadPtr();
    str->sprev = NULL;

    // angle_t angle;
    str->angle = SV_ReadLong();

    // spritenum_t sprite;
    str->sprite = SV_ReadLong();

    // int frame;
    str->frame = SV_ReadLong();

    // struct mobj_s *bnext, *bprev;
    // Values are read but discarded; this will be restored when the thing's
    // position is set.
    str->bnext = SV_ReadPtr();
    str->bnext = NULL;
    str->bprev = SV_ReadPtr();
    str->bprev = NULL;

    // struct subsector_s *subsector;
    // Read but discard: pointer will be restored when thing position is set.
    str->subsector = SV_ReadPtr();
    str->subsector = NULL;

    // fixed_t floorz, ceilingz;
    str->floorz = SV_ReadLong();
    str->ceilingz = SV_ReadLong();

    // fixed_t floorpic;
    str->floorpic = SV_ReadLong();

    // fixed_t radius, height;
    str->radius = SV_ReadLong();
    str->height = SV_ReadLong();

    // fixed_t momx, momy, momz;
    str->momx = SV_ReadLong();
    str->momy = SV_ReadLong();
    str->momz = SV_ReadLong();

    // int validcount;
    str->validcount = SV_ReadLong();

    // mobjtype_t type;
    str->type = SV_ReadLong();

    // mobjinfo_t *info;
    // Pointer value is read but discarded.
    str->info = SV_ReadPtr();
    str->info = NULL;

    // int tics;
    str->tics = SV_ReadLong();

    // state_t *state;
    // Restore as index into states table.
    i = SV_ReadLong();
    str->state = &states[i];

    // int damage;
    str->damage = SV_ReadLong();

    // int flags;
    str->flags = SV_ReadLong();

    // int flags2;
    str->flags2 = SV_ReadLong();

    // specialval_t special1;
    // specialval_t special2;
    // Read in special values: there are special cases to deal with with
    // mobj pointers.
    StreamInMobjSpecials(str);

    // int health;
    str->health = SV_ReadLong();

    // int movedir;
    str->movedir = SV_ReadLong();

    // int movecount;
    str->movecount = SV_ReadLong();

    // struct mobj_s *target;
    i = SV_ReadLong();
    SetMobjPtr(&str->target, i);

    // int reactiontime;
    str->reactiontime = SV_ReadLong();

    // int threshold;
    str->threshold = SV_ReadLong();

    // struct player_s *player;
    // Saved as player number.
    i = SV_ReadLong();
    if (i == 0)
    {
        str->player = NULL;
    }
    else
    {
        str->player = &players[i - 1];
        str->player->mo = str;
    }

    // int lastlook;
    str->lastlook = SV_ReadLong();

    // fixed_t floorclip;
    str->floorclip = SV_ReadLong();

    // int archiveNum;
    str->archiveNum = SV_ReadLong();

    // short tid;
    str->tid = SV_ReadWord();

    // byte special;
    str->special = SV_ReadByte();

    // byte args[5];
    for (i=0; i<5; ++i)
    {
        str->args[i] = SV_ReadByte();
    }
}

static void StreamOutMobjSpecials(mobj_t *mobj)
{
    dboolean corpse;

    corpse = (mobj->flags & MF_CORPSE) != 0;

    SV_WriteLong(mobj->type == HEXEN_MT_KORAX ? 0 : mobj->special1.i);
    SV_WriteLong(corpse ? MOBJ_NULL : GetMobjNum(mobj->special1.m));
    SV_WriteLong(mobj->special2.i);
    SV_WriteLong(corpse ? MOBJ_NULL : GetMobjNum(mobj->special2.m));
}

static void StreamOut_mobj_t(mobj_t *str)
{
    int i;

    // fixed_t x, y, z;
    SV_WriteLong(str->x);
    SV_WriteLong(str->y);
    SV_WriteLong(str->z);

    // struct mobj_s *snext, *sprev;
    SV_WritePtr(str->snext);
    SV_WritePtr(str->sprev);

    // angle_t angle;
    SV_WriteLong(str->angle);

    // spritenum_t sprite;
    SV_WriteLong(str->sprite);

    // int frame;
    SV_WriteLong(str->frame);

    // struct mobj_s *bnext, *bprev;
    SV_WritePtr(str->bnext);
    SV_WritePtr(str->bprev);

    // struct subsector_s *subsector;
    SV_WritePtr(str->subsector);

    // fixed_t floorz, ceilingz;
    SV_WriteLong(str->floorz);
    SV_WriteLong(str->ceilingz);

    // fixed_t floorpic;
    SV_WriteLong(str->floorpic);

    // fixed_t radius, height;
    SV_WriteLong(str->radius);
    SV_WriteLong(str->height);

    // fixed_t momx, momy, momz;
    SV_WriteLong(str->momx);
    SV_WriteLong(str->momy);
    SV_WriteLong(str->momz);

    // int validcount;
    SV_WriteLong(str->validcount);

    // mobjtype_t type;
    SV_WriteLong(str->type);

    // mobjinfo_t *info;
    SV_WritePtr(str->info);

    // int tics;
    SV_WriteLong(str->tics);

    // state_t *state;
    // Save as index into the states table.
    SV_WriteLong(str->state - states);

    // int damage;
    SV_WriteLong(str->damage);

    // int flags;
    SV_WriteLong(str->flags);

    // int flags2;
    SV_WriteLong(str->flags2);

    // specialval_t special1;
    // specialval_t special2;
    // There are lots of special cases for the special values:
    StreamOutMobjSpecials(str);

    // int health;
    SV_WriteLong(str->health);

    // int movedir;
    SV_WriteLong(str->movedir);

    // int movecount;
    SV_WriteLong(str->movecount);

    // struct mobj_s *target;
    if ((str->flags & MF_CORPSE) != 0)
    {
        SV_WriteLong(MOBJ_NULL);
    }
    else
    {
        SV_WriteLong(GetMobjNum(str->target));
    }

    // int reactiontime;
    SV_WriteLong(str->reactiontime);

    // int threshold;
    SV_WriteLong(str->threshold);

    // struct player_s *player;
    // Stored as index into players[] array, if there is a player pointer.
    if (str->player != NULL)
    {
        SV_WriteLong(str->player - players + 1);
    }
    else
    {
        SV_WriteLong(0);
    }

    // int lastlook;
    SV_WriteLong(str->lastlook);

    // fixed_t floorclip;
    SV_WriteLong(str->floorclip);

    // int archiveNum;
    SV_WriteLong(str->archiveNum);

    // short tid;
    SV_WriteWord(str->tid);

    // byte special;
    SV_WriteByte(str->special);

    // byte args[5];
    for (i=0; i<5; ++i)
    {
        SV_WriteByte(str->args[i]);
    }
}

static void StreamIn_floormove_t(floormove_t *str)
{
    int i;

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = sectors + i;

    // floor_e type;
    str->type = SV_ReadLong();

    // int crush;
    str->crush = SV_ReadLong();

    // int direction;
    str->direction = SV_ReadLong();

    // int newspecial;
    str->newspecial = SV_ReadLong();

    // short texture;
    str->texture = SV_ReadWord();

    // fixed_t floordestheight;
    str->floordestheight = SV_ReadLong();

    // fixed_t speed;
    str->speed = SV_ReadLong();

    // int delayCount;
    str->delayCount = SV_ReadLong();

    // int delayTotal;
    str->delayTotal = SV_ReadLong();

    // fixed_t stairsDelayHeight;
    str->stairsDelayHeight = SV_ReadLong();

    // fixed_t stairsDelayHeightDelta;
    str->stairsDelayHeightDelta = SV_ReadLong();

    // fixed_t resetHeight;
    str->resetHeight = SV_ReadLong();

    // short resetDelay;
    str->resetDelay = SV_ReadWord();

    // short resetDelayCount;
    str->resetDelayCount = SV_ReadWord();

    // byte textureChange;
    str->textureChange = SV_ReadByte();
}

static void StreamOut_floormove_t(floormove_t *str)
{
    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // floor_e type;
    SV_WriteLong(str->type);

    // int crush;
    SV_WriteLong(str->crush);

    // int direction;
    SV_WriteLong(str->direction);

    // int newspecial;
    SV_WriteLong(str->newspecial);

    // short texture;
    SV_WriteWord(str->texture);

    // fixed_t floordestheight;
    SV_WriteLong(str->floordestheight);

    // fixed_t speed;
    SV_WriteLong(str->speed);

    // int delayCount;
    SV_WriteLong(str->delayCount);

    // int delayTotal;
    SV_WriteLong(str->delayTotal);

    // fixed_t stairsDelayHeight;
    SV_WriteLong(str->stairsDelayHeight);

    // fixed_t stairsDelayHeightDelta;
    SV_WriteLong(str->stairsDelayHeightDelta);

    // fixed_t resetHeight;
    SV_WriteLong(str->resetHeight);

    // short resetDelay;
    SV_WriteWord(str->resetDelay);

    // short resetDelayCount;
    SV_WriteWord(str->resetDelayCount);

    // byte textureChange;
    SV_WriteByte(str->textureChange);
}

static void StreamIn_plat_t(plat_t *str)
{
    int i;

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = sectors + i;

    // fixed_t speed;
    str->speed = SV_ReadLong();

    // fixed_t low;
    str->low = SV_ReadLong();

    // fixed_t high;
    str->high = SV_ReadLong();

    // int wait;
    str->wait = SV_ReadLong();

    // int count;
    str->count = SV_ReadLong();

    // plat_e status;
    str->status = SV_ReadLong();

    // plat_e oldstatus;
    str->oldstatus = SV_ReadLong();

    // int crush;
    str->crush = SV_ReadLong();

    // int tag;
    str->tag = SV_ReadLong();

    // plattype_e type;
    str->type = SV_ReadLong();
}

static void StreamOut_plat_t(plat_t *str)
{
    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // fixed_t speed;
    SV_WriteLong(str->speed);

    // fixed_t low;
    SV_WriteLong(str->low);

    // fixed_t high;
    SV_WriteLong(str->high);

    // int wait;
    SV_WriteLong(str->wait);

    // int count;
    SV_WriteLong(str->count);

    // plat_e status;
    SV_WriteLong(str->status);

    // plat_e oldstatus;
    SV_WriteLong(str->oldstatus);

    // int crush;
    SV_WriteLong(str->crush);

    // int tag;
    SV_WriteLong(str->tag);

    // plattype_e type;
    SV_WriteLong(str->type);
}

static void StreamIn_ceiling_t(ceiling_t *str)
{
    int i;

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = sectors + i;

    // ceiling_e type;
    str->type = SV_ReadLong();

    // fixed_t bottomheight, topheight;
    str->bottomheight = SV_ReadLong();
    str->topheight = SV_ReadLong();

    // fixed_t speed;
    str->speed = SV_ReadLong();

    // int crush;
    str->crush = SV_ReadLong();

    // int direction;
    str->direction = SV_ReadLong();

    // int tag;
    str->tag = SV_ReadLong();

    // int olddirection;
    str->olddirection = SV_ReadLong();
}

static void StreamOut_ceiling_t(ceiling_t *str)
{
    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // ceiling_e type;
    SV_WriteLong(str->type);

    // fixed_t bottomheight, topheight;
    SV_WriteLong(str->bottomheight);
    SV_WriteLong(str->topheight);

    // fixed_t speed;
    SV_WriteLong(str->speed);

    // int crush;
    SV_WriteLong(str->crush);

    // int direction;
    SV_WriteLong(str->direction);

    // int tag;
    SV_WriteLong(str->tag);

    // int olddirection;
    SV_WriteLong(str->olddirection);
}

static void StreamIn_light_t(light_t *str)
{
    int i;

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = sectors + i;

    // lighttype_t type;
    str->type = SV_ReadLong();

    // int value1;
    str->value1 = SV_ReadLong();

    // int value2;
    str->value2 = SV_ReadLong();

    // int tics1;
    str->tics1 = SV_ReadLong();

    // int tics2;
    str->tics2 = SV_ReadLong();

    // int count;
    str->count = SV_ReadLong();
}

static void StreamOut_light_t(light_t *str)
{
    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // lighttype_t type;
    SV_WriteLong(str->type);

    // int value1;
    SV_WriteLong(str->value1);

    // int value2;
    SV_WriteLong(str->value2);

    // int tics1;
    SV_WriteLong(str->tics1);

    // int tics2;
    SV_WriteLong(str->tics2);

    // int count;
    SV_WriteLong(str->count);
}

static void StreamIn_vldoor_t(vldoor_t *str)
{
    int i;

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // vldoor_e type;
    str->type = SV_ReadLong();

    // fixed_t topheight;
    str->topheight = SV_ReadLong();

    // fixed_t speed;
    str->speed = SV_ReadLong();

    // int direction;
    str->direction = SV_ReadLong();

    // int topwait;
    str->topwait = SV_ReadLong();

    // int topcountdown;
    str->topcountdown = SV_ReadLong();
}

static void StreamOut_vldoor_t(vldoor_t *str)
{
    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // vldoor_e type;
    SV_WriteLong(str->type);

    // fixed_t topheight;
    SV_WriteLong(str->topheight);

    // fixed_t speed;
    SV_WriteLong(str->speed);

    // int direction;
    SV_WriteLong(str->direction);

    // int topwait;
    SV_WriteLong(str->topwait);

    // int topcountdown;
    SV_WriteLong(str->topcountdown);
}

static void StreamIn_phase_t(phase_t *str)
{
    int i;

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // int index;
    str->index = SV_ReadLong();

    // int base;
    str->base = SV_ReadLong();
}

static void StreamOut_phase_t(phase_t *str)
{
    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // int index;
    SV_WriteLong(str->index);

    // int base;
    SV_WriteLong(str->base);
}

static void StreamIn_acs_t(acs_t *str)
{
    int i;

    // mobj_t *activator;
    i = SV_ReadLong();
    SetMobjPtr(&str->activator, i);

    // line_t *line;
    i = SV_ReadLong();
    if (i != -1)
    {
        str->line = &lines[i];
    }
    else
    {
        str->line = NULL;
    }

    // int side;
    str->side = SV_ReadLong();

    // int number;
    str->number = SV_ReadLong();

    // int infoIndex;
    str->infoIndex = SV_ReadLong();

    // int delayCount;
    str->delayCount = SV_ReadLong();

    // int stack[ACS_STACK_DEPTH];
    for (i=0; i<ACS_STACK_DEPTH; ++i)
    {
        str->stack[i] = SV_ReadLong();
    }

    // int stackPtr;
    str->stackPtr = SV_ReadLong();

    // int vars[MAX_ACS_SCRIPT_VARS];
    for (i=0; i<MAX_ACS_SCRIPT_VARS; ++i)
    {
        str->vars[i] = SV_ReadLong();
    }

    // int *ip;
    str->ip = SV_ReadLong();
}

static void StreamOut_acs_t(acs_t *str)
{
    int i;

    // mobj_t *activator;
    SV_WriteLong(GetMobjNum(str->activator));

    // line_t *line;
    if (str->line != NULL)
    {
        SV_WriteLong(str->line - lines);
    }
    else
    {
        SV_WriteLong(-1);
    }

    // int side;
    SV_WriteLong(str->side);

    // int number;
    SV_WriteLong(str->number);

    // int infoIndex;
    SV_WriteLong(str->infoIndex);

    // int delayCount;
    SV_WriteLong(str->delayCount);

    // int stack[ACS_STACK_DEPTH];
    for (i=0; i<ACS_STACK_DEPTH; ++i)
    {
        SV_WriteLong(str->stack[i]);
    }

    // int stackPtr;
    SV_WriteLong(str->stackPtr);

    // int vars[MAX_ACS_SCRIPT_VARS];
    for (i=0; i<MAX_ACS_SCRIPT_VARS; ++i)
    {
        SV_WriteLong(str->vars[i]);
    }

    // int *ip;
    SV_WriteLong(str->ip);
}

static void StreamIn_polyevent_t(polyevent_t *str)
{
    // int polyobj;
    str->polyobj = SV_ReadLong();

    // int speed;
    str->speed = SV_ReadLong();

    // unsigned int dist;
    str->dist = SV_ReadLong();

    // int angle;
    str->angle = SV_ReadLong();

    // fixed_t xSpeed;
    str->xSpeed = SV_ReadLong();

    // fixed_t ySpeed;
    str->ySpeed = SV_ReadLong();
}

static void StreamOut_polyevent_t(polyevent_t *str)
{
    // int polyobj;
    SV_WriteLong(str->polyobj);

    // int speed;
    SV_WriteLong(str->speed);

    // unsigned int dist;
    SV_WriteLong(str->dist);

    // int angle;
    SV_WriteLong(str->angle);

    // fixed_t xSpeed;
    SV_WriteLong(str->xSpeed);

    // fixed_t ySpeed;
    SV_WriteLong(str->ySpeed);
}

static void StreamIn_pillar_t(pillar_t *str)
{
    int i;

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // int ceilingSpeed;
    str->ceilingSpeed = SV_ReadLong();

    // int floorSpeed;
    str->floorSpeed = SV_ReadLong();

    // int floordest;
    str->floordest = SV_ReadLong();

    // int ceilingdest;
    str->ceilingdest = SV_ReadLong();

    // int direction;
    str->direction = SV_ReadLong();

    // int crush;
    str->crush = SV_ReadLong();
}

static void StreamOut_pillar_t(pillar_t *str)
{
    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // int ceilingSpeed;
    SV_WriteLong(str->ceilingSpeed);

    // int floorSpeed;
    SV_WriteLong(str->floorSpeed);

    // int floordest;
    SV_WriteLong(str->floordest);

    // int ceilingdest;
    SV_WriteLong(str->ceilingdest);

    // int direction;
    SV_WriteLong(str->direction);

    // int crush;
    SV_WriteLong(str->crush);
}

static void StreamIn_polydoor_t(polydoor_t *str)
{
    // int polyobj;
    str->polyobj = SV_ReadLong();

    // int speed;
    str->speed = SV_ReadLong();

    // int dist;
    str->dist = SV_ReadLong();

    // int totalDist;
    str->totalDist = SV_ReadLong();

    // int direction;
    str->direction = SV_ReadLong();

    // fixed_t xSpeed, ySpeed;
    str->xSpeed = SV_ReadLong();
    str->ySpeed = SV_ReadLong();

    // int tics;
    str->tics = SV_ReadLong();

    // int waitTics;
    str->waitTics = SV_ReadLong();

    // podoortype_t type;
    str->type = SV_ReadLong();

    // dboolean close;
    str->close = SV_ReadLong();
}

static void StreamOut_polydoor_t(polydoor_t *str)
{
    // int polyobj;
    SV_WriteLong(str->polyobj);

    // int speed;
    SV_WriteLong(str->speed);

    // int dist;
    SV_WriteLong(str->dist);

    // int totalDist;
    SV_WriteLong(str->totalDist);

    // int direction;
    SV_WriteLong(str->direction);

    // fixed_t xSpeed, ySpeed;
    SV_WriteLong(str->xSpeed);
    SV_WriteLong(str->ySpeed);

    // int tics;
    SV_WriteLong(str->tics);

    // int waitTics;
    SV_WriteLong(str->waitTics);

    // podoortype_t type;
    SV_WriteLong(str->type);

    // dboolean close;
    SV_WriteLong(str->close);
}

static void StreamIn_floorWaggle_t(floorWaggle_t *str)
{
    int i;

    // sector_t *sector;
    i = SV_ReadLong();
    str->sector = &sectors[i];

    // fixed_t originalHeight;
    str->originalHeight = SV_ReadLong();

    // fixed_t accumulator;
    str->accumulator = SV_ReadLong();

    // fixed_t accDelta;
    str->accDelta = SV_ReadLong();

    // fixed_t targetScale;
    str->targetScale = SV_ReadLong();

    // fixed_t scale;
    str->scale = SV_ReadLong();

    // fixed_t scaleDelta;
    str->scaleDelta = SV_ReadLong();

    // int ticker;
    str->ticker = SV_ReadLong();

    // int state;
    str->state = SV_ReadLong();
}

static void StreamOut_floorWaggle_t(floorWaggle_t *str)
{
    // sector_t *sector;
    SV_WriteLong(str->sector - sectors);

    // fixed_t originalHeight;
    SV_WriteLong(str->originalHeight);

    // fixed_t accumulator;
    SV_WriteLong(str->accumulator);

    // fixed_t accDelta;
    SV_WriteLong(str->accDelta);

    // fixed_t targetScale;
    SV_WriteLong(str->targetScale);

    // fixed_t scale;
    SV_WriteLong(str->scale);

    // fixed_t scaleDelta;
    SV_WriteLong(str->scaleDelta);

    // int ticker;
    SV_WriteLong(str->ticker);

    // int state;
    SV_WriteLong(str->state);
}

static byte *map_archive[MAX_MAPS];

static dboolean MapArchiveExists(int map)
{
  return (map_archive[map] != NULL);
}

static void FreeMapArchive(void)
{
  int map;

  for (map = 0; map < MAX_MAPS; ++map)
    if (map_archive[map])
    {
      free(map_archive[map]);
      map_archive[map] = NULL;
    }
}

void SV_Init(void)
{
  FreeMapArchive();
}

void SV_SaveMap(void)
{
  return;
}

void SV_LoadMap(void)
{
  return;
}

void SV_MapTeleport(int map, int position)
{
    int i;
    int j;
    int key_i;
    player_t playerBackup[MAX_MAXPLAYERS];
    mobj_t *targetPlayerMobj;
    mobj_t *mobj;
    int inventoryPtr;
    int currentInvPos;
    dboolean rClass;
    dboolean playerWasReborn;
    dboolean oldWeaponowned[HEXEN_NUMWEAPONS];
    int oldKeys[NUMCARDS];
    int oldPieces = 0;
    int bestWeapon;

    if (!deathmatch)
    {
        if (P_GetMapCluster(gamemap) == P_GetMapCluster(map))
        {                       // Same cluster - save map without saving player mobjs
            SV_SaveMap();
        }
        else
        {                       // Entering new cluster - clear map archive
            SV_Init();
        }
    }

    // Store player structs for later
    rClass = randomclass;
    randomclass = false;
    for (i = 0; i < g_maxplayers; i++)
    {
        playerBackup[i] = players[i];
    }

    // Save some globals that get trashed during the load
    inventoryPtr = inv_ptr;
    currentInvPos = curpos;

    // Only SV_LoadMap() uses TargetPlayerAddrs, so it's NULLed here
    // for the following check (player mobj redirection)
    TargetPlayerAddrs = NULL;

    gamemap = map;
    if (!deathmatch && MapArchiveExists(gamemap))
    {                           // Unarchive map
        SV_LoadMap();
        P_MapStart();
    }
    else
    {                           // New map
        G_InitNew(gameskill, gameepisode, gamemap);

        P_MapStart();

        // Destroy all freshly spawned players
        for (i = 0; i < g_maxplayers; i++)
        {
            if (playeringame[i])
            {
                P_RemoveMobj(players[i].mo);
            }
        }
    }

    // Restore player structs
    targetPlayerMobj = NULL;
    for (i = 0; i < g_maxplayers; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }
        players[i] = playerBackup[i];
        ClearMessage();
        players[i].attacker = NULL;
        players[i].poisoner = NULL;

        if (netgame)
        {
            if (players[i].playerstate == PST_DEAD)
            {                   // In a network game, force all players to be alive
                players[i].playerstate = PST_REBORN;
            }
            if (!deathmatch)
            {                   // Cooperative net-play, retain keys and weapons
                for (key_i = 0; key_i < NUMCARDS; ++key_i)
                  oldKeys[key_i] = players[i].cards[key_i];
                oldPieces = players[i].pieces;
                for (j = 0; j < HEXEN_NUMWEAPONS; j++)
                {
                    oldWeaponowned[j] = players[i].weaponowned[j];
                }
            }
        }
        playerWasReborn = (players[i].playerstate == PST_REBORN);
        if (deathmatch)
        {
            memset(players[i].frags, 0, sizeof(players[i].frags));
            mobj = P_SpawnMobj(playerstarts[0][i].x << 16,
                               playerstarts[0][i].y << 16, 0,
                               HEXEN_MT_PLAYER_FIGHTER);
            players[i].mo = mobj;
            G_DeathMatchSpawnPlayer(i);
            P_RemoveMobj(mobj);
        }
        else
        {
            P_SpawnPlayer(i, &playerstarts[position][i]);
        }

        if (playerWasReborn && netgame && !deathmatch)
        {                       // Restore keys and weapons when reborn in co-op
            for (key_i = 0; key_i < NUMCARDS; ++key_i)
              players[i].cards[key_i] = oldKeys[key_i];
            players[i].pieces = oldPieces;
            for (bestWeapon = 0, j = 0; j < HEXEN_NUMWEAPONS; j++)
            {
                if (oldWeaponowned[j])
                {
                    bestWeapon = j;
                    players[i].weaponowned[j] = true;
                }
            }
            players[i].ammo[MANA_1] = 25;
            players[i].ammo[MANA_2] = 25;
            if (bestWeapon)
            {                   // Bring up the best weapon
                players[i].pendingweapon = bestWeapon;
            }
        }

        if (targetPlayerMobj == NULL)
        {                       // The poor sap
            targetPlayerMobj = players[i].mo;
        }
    }
    randomclass = rClass;

    // Redirect anything targeting a player mobj
    if (TargetPlayerAddrs)
    {
        for (i = 0; i < TargetPlayerCount; i++)
        {
            *TargetPlayerAddrs[i] = targetPlayerMobj;
        }
        Z_Free(TargetPlayerAddrs);
    }

    // Destroy all things touching players
    for (i = 0; i < g_maxplayers; i++)
    {
        if (playeringame[i])
        {
            P_TeleportMove(players[i].mo, players[i].mo->x, players[i].mo->y, false);
        }
    }

    // Restore trashed globals
    inv_ptr = inventoryPtr;
    curpos = currentInvPos;

    // Launch waiting scripts
    if (!deathmatch)
    {
        P_CheckACSStore();
    }

    P_MapEnd();
}
