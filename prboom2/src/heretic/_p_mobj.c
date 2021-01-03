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

void G_PlayerReborn(int player);
void P_SpawnMapThing(mapthing_t * mthing);

static fixed_t FloatBobOffsets[64] = {
    0, 51389, 102283, 152192,
    200636, 247147, 291278, 332604,
    370727, 405280, 435929, 462380,
    484378, 501712, 514213, 521763,
    524287, 521763, 514213, 501712,
    484378, 462380, 435929, 405280,
    370727, 332604, 291278, 247147,
    200636, 152192, 102283, 51389,
    -1, -51390, -102284, -152193,
    -200637, -247148, -291279, -332605,
    -370728, -405281, -435930, -462381,
    -484380, -501713, -514215, -521764,
    -524288, -521764, -514214, -501713,
    -484379, -462381, -435930, -405280,
    -370728, -332605, -291279, -247148,
    -200637, -152193, -102284, -51389
};

//----------------------------------------------------------------------------
//
// PROC P_MobjThinker
//
//----------------------------------------------------------------------------

void P_MobjThinker(mobj_t * mobj)
{
    mobj_t *onmo;

    // [crispy] suppress interpolation of player missiles for the first tic
    if (mobj->interp == -1)
    {
        mobj->interp = false;
    }
    else
    // [AM] Handle interpolation unless we're an active player.
    if (!(mobj->player != NULL && mobj == mobj->player->mo))
    {
        // Assume we can interpolate at the beginning
        // of the tic.
        mobj->interp = true;

        // Store starting position for mobj interpolation.
        mobj->oldx = mobj->x;
        mobj->oldy = mobj->y;
        mobj->oldz = mobj->z;
        mobj->oldangle = mobj->angle;
    }

    // Handle X and Y momentums
    if (mobj->momx || mobj->momy || (mobj->flags & MF_SKULLFLY))
    {
        P_XYMovement(mobj);
        if (mobj->thinker.function == (think_t) - 1)
        {                       // mobj was removed
            return;
        }
    }
    if (mobj->flags2 & MF2_FLOATBOB)
    {                           // Floating item bobbing motion
        mobj->z = mobj->floorz + FloatBobOffsets[(mobj->health++) & 63];
    }
    else if ((mobj->z != mobj->floorz) || mobj->momz)
    {                           // Handle Z momentum and gravity
        if (mobj->flags2 & MF2_PASSMOBJ)
        {
            if (!(onmo = P_CheckOnmobj(mobj)))
            {
                P_ZMovement(mobj);
            }
            else
            {
                if (mobj->player && mobj->momz < 0)
                {
                    mobj->flags2 |= MF2_ONMOBJ;
                    mobj->momz = 0;
                }
                if (mobj->player && (onmo->player || onmo->type == MT_POD))
                {
                    mobj->momx = onmo->momx;
                    mobj->momy = onmo->momy;
                    if (onmo->z < onmo->floorz)
                    {
                        mobj->z += onmo->floorz - onmo->z;
                        if (onmo->player)
                        {
                            onmo->player->viewheight -=
                                onmo->floorz - onmo->z;
                            onmo->player->deltaviewheight =
                                (VIEWHEIGHT - onmo->player->viewheight) >> 3;
                        }
                        onmo->z = onmo->floorz;
                    }
                }
            }
        }
        else
        {
            P_ZMovement(mobj);
        }
        if (mobj->thinker.function == (think_t) - 1)
        {                       // mobj was removed
            return;
        }
    }

//
// cycle through states, calling action functions at transitions
//
    if (mobj->tics != -1)
    {
        mobj->tics--;
        // you can cycle through multiple states in a tic
        while (!mobj->tics)
        {
            if (!P_SetMobjState(mobj, mobj->state->nextstate))
            {                   // mobj was removed
                return;
            }
        }
    }
    else
    {                           // Check for monster respawn
        if (!(mobj->flags & MF_COUNTKILL))
        {
            return;
        }
        if (!respawnmonsters)
        {
            return;
        }
        mobj->movecount++;
        if (mobj->movecount < 12 * 35)
        {
            return;
        }
        if (leveltime & 31)
        {
            return;
        }
        if (P_Random() > 4)
        {
            return;
        }
        P_NightmareRespawn(mobj);
    }
}

/*
===============
=
= P_SpawnMobj
=
===============
*/

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{
    mobj_t *mobj;
    state_t *st;
    mobjinfo_t *info;
    fixed_t space;

    mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
    memset(mobj, 0, sizeof(*mobj));
    info = &mobjinfo[type];
    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;
    mobj->flags2 = info->flags2;
    mobj->damage = info->damage;
    mobj->health = info->spawnhealth;
    if (gameskill != sk_nightmare)
    {
        mobj->reactiontime = info->reactiontime;
    }
    mobj->lastlook = P_Random() % MAXPLAYERS;

    // Set the state, but do not use P_SetMobjState, because action
    // routines can't be called yet.  If the spawnstate has an action
    // routine, it will not be called.
    st = &states[info->spawnstate];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;

    // Set subsector and/or block links.
    P_SetThingPosition(mobj);
    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;
    if (z == ONFLOORZ)
    {
        mobj->z = mobj->floorz;
    }
    else if (z == ONCEILINGZ)
    {
        mobj->z = mobj->ceilingz - mobj->info->height;
    }
    else if (z == FLOATRANDZ)
    {
        space = ((mobj->ceilingz) - (mobj->info->height)) - mobj->floorz;
        if (space > 48 * FRACUNIT)
        {
            space -= 40 * FRACUNIT;
            mobj->z =
                ((space * P_Random()) >> 8) + mobj->floorz + 40 * FRACUNIT;
        }
        else
        {
            mobj->z = mobj->floorz;
        }
    }
    else
    {
        mobj->z = z;
    }
    if (mobj->flags2 & MF2_FOOTCLIP
        && P_GetThingFloorType(mobj) != FLOOR_SOLID
        && mobj->floorz == mobj->subsector->sector->floorheight)
    {
        mobj->flags2 |= MF2_FEETARECLIPPED;
    }
    else
    {
        mobj->flags2 &= ~MF2_FEETARECLIPPED;
    }

    // [AM] Do not interpolate on spawn.
    mobj->interp = false;

    // [AM] Just in case interpolation is attempted...
    mobj->oldx = mobj->x;
    mobj->oldy = mobj->y;
    mobj->oldz = mobj->z;
    mobj->oldangle = mobj->angle;

    mobj->thinker.function = P_MobjThinker;
    P_AddThinker(&mobj->thinker);
    return (mobj);
}

//=============================================================================


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

/*
===============================================================================

						GAME SPAWN FUNCTIONS

===============================================================================
*/

//---------------------------------------------------------------------------
//
// PROC P_SpawnPuff
//
//---------------------------------------------------------------------------

extern fixed_t attackrange;

void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z)
{
    mobj_t *puff;

    z += (P_SubRandom() << 10);
    puff = P_SpawnMobj(x, y, z, PuffType);
    if (puff->info->attacksound)
    {
        S_StartSound(puff, puff->info->attacksound);
    }
    switch (PuffType)
    {
        case MT_BEAKPUFF:
        case MT_STAFFPUFF:
            puff->momz = FRACUNIT;
            break;
        case MT_GAUNTLETPUFF1:
        case MT_GAUNTLETPUFF2:
            puff->momz = (fixed_t)(.8 * FRACUNIT);
        default:
            break;
    }
    // [crispy] suppress interpolation for the first tic
    puff->interp = -1;
}

//---------------------------------------------------------------------------
//
// PROC P_BloodSplatter
//
//---------------------------------------------------------------------------

void P_BloodSplatter(fixed_t x, fixed_t y, fixed_t z, mobj_t * originator)
{
    mobj_t *mo;

    mo = P_SpawnMobj(x, y, z, MT_BLOODSPLATTER);
    mo->target = originator;
    mo->momx = P_SubRandom() << 9;
    mo->momy = P_SubRandom() << 9;
    mo->momz = FRACUNIT * 2;
}

//---------------------------------------------------------------------------
//
// PROC P_RipperBlood
//
//---------------------------------------------------------------------------

void P_RipperBlood(mobj_t * mo)
{
    mobj_t *th;
    fixed_t x, y, z;

    x = mo->x + (P_SubRandom() << 12);
    y = mo->y + (P_SubRandom() << 12);
    z = mo->z + (P_SubRandom() << 12);
    th = P_SpawnMobj(x, y, z, MT_BLOOD);
    th->flags |= MF_NOGRAVITY;
    th->momx = mo->momx >> 1;
    th->momy = mo->momy >> 1;
    th->tics += P_Random() & 3;
}

//---------------------------------------------------------------------------
//
// FUNC P_SpawnMissile
//
// Returns NULL if the missile exploded immediately, otherwise returns
// a mobj_t pointer to the missile.
//
//---------------------------------------------------------------------------

mobj_t *P_SpawnMissile(mobj_t * source, mobj_t * dest, mobjtype_t type)
{
    fixed_t z;
    mobj_t *th;
    angle_t an;
    int dist;

    switch (type)
    {
        case MT_MNTRFX1:       // Minotaur swing attack missile
            z = source->z + 40 * FRACUNIT;
            break;
        case MT_MNTRFX2:       // Minotaur floor fire missile
            z = ONFLOORZ;
            break;
        case MT_SRCRFX1:       // Sorcerer Demon fireball
            z = source->z + 48 * FRACUNIT;
            break;
        case MT_KNIGHTAXE:     // Knight normal axe
        case MT_REDAXE:        // Knight red power axe
            z = source->z + 36 * FRACUNIT;
            break;
        default:
            z = source->z + 32 * FRACUNIT;
            break;
    }
    if (source->flags2 & MF2_FEETARECLIPPED)
    {
        z -= FOOTCLIPSIZE;
    }
    th = P_SpawnMobj(source->x, source->y, z, type);
    if (th->info->seesound)
    {
        S_StartSound(th, th->info->seesound);
    }
    th->target = source;        // Originator
    an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);
    if (dest->flags & MF_SHADOW)
    {                           // Invisible target
        an += P_SubRandom() << 21;
    }
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);
    dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;
    if (dist < 1)
    {
        dist = 1;
    }
    th->momz = (dest->z - source->z) / dist;
    return (P_CheckMissileSpawn(th) ? th : NULL);
}
