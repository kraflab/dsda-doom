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

// P_mobj.c

#include "doomdef.h"
#include "i_system.h"
#include "m_random.h"
#include "p_local.h"
#include "sounds.h"
#include "s_sound.h"

/*
============
=
= P_SpawnPlayer
=
= Called when a player is spawned on the level 
= Most of the player structure stays unchanged between levels
============
*/

void P_SpawnPlayer(mapthing_t * mthing)
{
    player_t *p;
    fixed_t x, y, z;
    mobj_t *mobj;
    int i;
    extern int playerkeys;

    if (!playeringame[mthing->type - 1])
        return;                 // not playing

    p = &players[mthing->type - 1];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn(mthing->type - 1);

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    z = ONFLOORZ;
    mobj = P_SpawnMobj(x, y, z, MT_PLAYER);
    if (mthing->type > 1)       // set color translations for player sprites
        mobj->flags |= (mthing->type - 1) << MF_TRANSSHIFT;

    mobj->angle = ANG45 * (mthing->angle / 45);
    mobj->player = p;
    mobj->health = p->health;
    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    p->message = NULL;
    // [JN] Reset ultimatemsg, so other messages may appear.
    // See: https://github.com/chocolate-doom/chocolate-doom/issues/781
    ultimatemsg = false;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->chickenTics = 0;
    p->rain1 = NULL;
    p->rain2 = NULL;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = VIEWHEIGHT;
    P_SetupPsprites(p);         // setup gun psprite        
    if (deathmatch)
    {                           // Give all keys in death match mode
        for (i = 0; i < NUMKEYS; i++)
        {
            p->keys[i] = true;
            if (p == &players[consoleplayer])
            {
                playerkeys = 7;
                UpdateState |= I_STATBAR;
            }
        }
    }
    else if (p == &players[consoleplayer])
    {
        playerkeys = 0;
        UpdateState |= I_STATBAR;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_SpawnMapThing
//
// The fields of the mapthing should already be in host byte order.
//
//----------------------------------------------------------------------------

void P_SpawnMapThing(mapthing_t * mthing)
{
    int i;
    int bit;
    mobj_t *mobj;
    fixed_t x, y, z;

// count deathmatch start positions
    if (mthing->type == 11)
    {
        if (deathmatch_p < &deathmatchstarts[10])
        {
            memcpy(deathmatch_p, mthing, sizeof(*mthing));
            deathmatch_p++;
        }
        return;
    }

// check for players specially
    if (mthing->type <= 4)
    {
        // save spots for respawning in network games
        playerstarts[mthing->type - 1] = *mthing;
        playerstartsingame[mthing->type - 1] = true;
        if (!deathmatch)
        {
            P_SpawnPlayer(mthing);
        }
        return;
    }

    // Ambient sound sequences
    if (mthing->type >= 1200 && mthing->type < 1300)
    {
        P_AddAmbientSfx(mthing->type - 1200);
        return;
    }

    // Check for boss spots
    if (mthing->type == 56)     // Monster_BossSpot
    {
        P_AddBossSpot(mthing->x << FRACBITS, mthing->y << FRACBITS,
                      ANG45 * (mthing->angle / 45));
        return;
    }

// check for apropriate skill level
    if (!netgame && (mthing->options & 16))
        return;

    if (gameskill == sk_baby)
        bit = 1;
    else if (gameskill == sk_nightmare)
        bit = 4;
    else
        bit = 1 << (gameskill - 1);
    if (!(mthing->options & bit))
        return;

// find which type to spawn
    for (i = 0; i < NUMMOBJTYPES; i++)
        if (mthing->type == mobjinfo[i].doomednum)
            break;

    if (i == NUMMOBJTYPES)
        I_Error("P_SpawnMapThing: Unknown type %i at (%i, %i)", mthing->type,
                mthing->x, mthing->y);

// don't spawn keys and players in deathmatch
    if (deathmatch && mobjinfo[i].flags & MF_NOTDMATCH)
        return;

// don't spawn any monsters if -nomonsters
    if (nomonsters && (mobjinfo[i].flags & MF_COUNTKILL))
        return;

// spawn it
    switch (i)
    {                           // Special stuff
        case MT_WSKULLROD:
        case MT_WPHOENIXROD:
        case MT_AMSKRDWIMPY:
        case MT_AMSKRDHEFTY:
        case MT_AMPHRDWIMPY:
        case MT_AMPHRDHEFTY:
        case MT_AMMACEWIMPY:
        case MT_AMMACEHEFTY:
        case MT_ARTISUPERHEAL:
        case MT_ARTITELEPORT:
        case MT_ITEMSHIELD2:
            if (gamemode == shareware)
            {                   // Don't place on map in shareware version
                return;
            }
            break;
        case MT_WMACE:
            if (gamemode != shareware)
            {                   // Put in the mace spot list
                P_AddMaceSpot(mthing);
                return;
            }
            return;
        default:
            break;
    }
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
    {
        z = ONCEILINGZ;
    }
    else if (mobjinfo[i].flags2 & MF2_SPAWNFLOAT)
    {
        z = FLOATRANDZ;
    }
    else
    {
        z = ONFLOORZ;
    }
    mobj = P_SpawnMobj(x, y, z, i);
    if (mobj->flags2 & MF2_FLOATBOB)
    {                           // Seed random starting index for bobbing motion
        mobj->health = P_Random();
    }
    if (mobj->tics > 0)
    {
        mobj->tics = 1 + (P_Random() % mobj->tics);
    }
    if (mobj->flags & MF_COUNTKILL)
    {
        totalkills++;
        mobj->spawnpoint = *mthing;
    }
    if (mobj->flags & MF_COUNTITEM)
    {
        totalitems++;
    }
    mobj->angle = ANG45 * (mthing->angle / 45);
    if (mthing->options & MTF_AMBUSH)
    {
        mobj->flags |= MF_AMBUSH;
    }
}
