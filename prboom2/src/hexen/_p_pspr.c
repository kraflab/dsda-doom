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

//============================================================================
//
// A_CFlameAttack
//
//============================================================================

#define FLAMESPEED	(0.45*FRACUNIT)
#define CFLAMERANGE	(12*64*FRACUNIT)

void A_CFlameAttack(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    mo = P_SpawnPlayerMissile(player->mo, HEXEN_MT_CFLAME_MISSILE);
    if (mo)
    {
        mo->thinker.function = P_BlasterMobjThinker;
        mo->special1.i = 2;
    }

    player->ammo[MANA_2] -= WeaponManaUse[player->pclass][player->readyweapon];
    S_StartSound(player->mo, hexen_sfx_cleric_flame_fire);
}

//============================================================================
//
// A_CFlamePuff
//
//============================================================================

void A_CFlamePuff(mobj_t * actor)
{
    A_UnHideThing(actor);
    actor->momx = 0;
    actor->momy = 0;
    actor->momz = 0;
    S_StartSound(actor, hexen_sfx_cleric_flame_explode);
}

//============================================================================
//
// A_CFlameMissile
//
//============================================================================

void A_CFlameMissile(mobj_t * actor)
{
    int i;
    int an;
    fixed_t dist;
    mobj_t *mo;

    A_UnHideThing(actor);
    S_StartSound(actor, hexen_sfx_cleric_flame_explode);
    if (BlockingMobj && BlockingMobj->flags & MF_SHOOTABLE)
    {                           // Hit something, so spawn the flame circle around the thing
        dist = BlockingMobj->radius + 18 * FRACUNIT;
        for (i = 0; i < 4; i++)
        {
            an = (i * ANG45) >> ANGLETOFINESHIFT;
            mo = P_SpawnMobj(BlockingMobj->x + FixedMul(dist, finecosine[an]),
                             BlockingMobj->y + FixedMul(dist, finesine[an]),
                             BlockingMobj->z + 5 * FRACUNIT, HEXEN_MT_CIRCLEFLAME);
            if (mo)
            {
                mo->angle = an << ANGLETOFINESHIFT;
                mo->target = actor->target;
                mo->momx = mo->special1.i =
                    FixedMul(FLAMESPEED, finecosine[an]);
                mo->momy = mo->special2.i = FixedMul(FLAMESPEED, finesine[an]);
                mo->tics -= P_Random(pr_hexen) & 3;
            }
            mo = P_SpawnMobj(BlockingMobj->x - FixedMul(dist, finecosine[an]),
                             BlockingMobj->y - FixedMul(dist, finesine[an]),
                             BlockingMobj->z + 5 * FRACUNIT, HEXEN_MT_CIRCLEFLAME);
            if (mo)
            {
                mo->angle = ANG180 + (an << ANGLETOFINESHIFT);
                mo->target = actor->target;
                mo->momx = mo->special1.i = FixedMul(-FLAMESPEED,
                                                     finecosine[an]);
                mo->momy = mo->special2.i = FixedMul(-FLAMESPEED, finesine[an]);
                mo->tics -= P_Random(pr_hexen) & 3;
            }
        }
        P_SetMobjState(actor, HEXEN_S_FLAMEPUFF2_1);
    }
}

//============================================================================
//
// A_CFlameRotate
//
//============================================================================

#define FLAMEROTSPEED	2*FRACUNIT

void A_CFlameRotate(mobj_t * actor)
{
    int an;

    an = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
    actor->momx = actor->special1.i + FixedMul(FLAMEROTSPEED, finecosine[an]);
    actor->momy = actor->special2.i + FixedMul(FLAMEROTSPEED, finesine[an]);
    actor->angle += ANG90 / 15;
}


//============================================================================
//
// A_CHolyAttack3
//
//      Spawns the spirits
//============================================================================

void A_CHolyAttack3(mobj_t * actor)
{
    P_SpawnMissile(actor, actor->target, HEXEN_MT_HOLY_MISSILE);
    S_StartSound(actor, hexen_sfx_choly_fire);
}


//============================================================================
//
// A_CHolyAttack2
//
//      Spawns the spirits
//============================================================================

void A_CHolyAttack2(mobj_t * actor)
{
    int j;
    int i;
    int r;
    mobj_t *mo;
    mobj_t *tail, *next;

    for (j = 0; j < 4; j++)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_HOLY_FX);
        if (!mo)
        {
            continue;
        }
        switch (j)
        {                       // float bob index
            case 0:
                mo->special2.i = P_Random(pr_hexen) & 7;  // upper-left
                break;
            case 1:
                mo->special2.i = 32 + (P_Random(pr_hexen) & 7);   // upper-right
                break;
            case 2:
                mo->special2.i = (32 + (P_Random(pr_hexen) & 7)) << 16;   // lower-left
                break;
            case 3:
                r = P_Random(pr_hexen);
                mo->special2.i =
                    ((32 + (r & 7)) << 16) + 32 + (P_Random(pr_hexen) & 7);
                break;
        }
        mo->z = actor->z;
        mo->angle = actor->angle + (ANG45 + ANG45 / 2) - ANG45 * j;
        P_ThrustMobj(mo, mo->angle, mo->info->speed);
        mo->target = actor->target;
        mo->args[0] = 10;       // initial turn value
        mo->args[1] = 0;        // initial look angle
        if (deathmatch)
        {                       // Ghosts last slightly less longer in DeathMatch
            mo->health = 85;
        }
        if (linetarget)
        {
            mo->special1.m = linetarget;
            mo->flags |= MF_NOCLIP | MF_SKULLFLY;
            mo->flags &= ~MF_MISSILE;
        }
        tail = P_SpawnMobj(mo->x, mo->y, mo->z, HEXEN_MT_HOLY_TAIL);
        tail->special2.m = mo;      // parent
        for (i = 1; i < 3; i++)
        {
            next = P_SpawnMobj(mo->x, mo->y, mo->z, HEXEN_MT_HOLY_TAIL);
            P_SetMobjState(next, next->info->spawnstate + 1);
            tail->special1.m = next;
            tail = next;
        }
        tail->special1.m = NULL;     // last tail bit
    }
}

//============================================================================
//
// A_CHolyAttack
//
//============================================================================

void A_CHolyAttack(player_t * player, pspdef_t * psp)
{
    player->ammo[MANA_1] -= WeaponManaUse[player->pclass][player->readyweapon];
    player->ammo[MANA_2] -= WeaponManaUse[player->pclass][player->readyweapon];
    P_SpawnPlayerMissile(player->mo, HEXEN_MT_HOLY_MISSILE);
    if (player == &players[consoleplayer])
    {
        player->damagecount = 0;
        player->bonuscount = 0;
        I_SetPalette((byte *) W_CacheLumpNum(W_GetNumForName("playpal"),
                                             PU_CACHE) + STARTHOLYPAL * 768);
    }
    S_StartSound(player->mo, hexen_sfx_choly_fire);
}

//============================================================================
//
// A_CHolyPalette
//
//============================================================================

void A_CHolyPalette(player_t * player, pspdef_t * psp)
{
    int pal;

    if (player == &players[consoleplayer])
    {
        pal = STARTHOLYPAL + psp->state - (&states[HEXEN_S_CHOLYATK_6]);
        if (pal == STARTHOLYPAL + 3)
        {                       // reset back to original playpal
            pal = 0;
        }
        I_SetPalette((byte *) W_CacheLumpNum(W_GetNumForName("playpal"),
                                             PU_CACHE) + pal * 768);
    }
}

//============================================================================
//
// CHolyFindTarget
//
//============================================================================

static void CHolyFindTarget(mobj_t * actor)
{
    mobj_t *target;

    target = P_RoughMonsterSearch(actor, 6);
    if (target != NULL)
    {
        actor->special1.m = target;
        actor->flags |= MF_NOCLIP | MF_SKULLFLY;
        actor->flags &= ~MF_MISSILE;
    }
}

//============================================================================
//
// CHolySeekerMissile
//
//       Similar to P_SeekerMissile, but seeks to a random Z on the target
//============================================================================

static void CHolySeekerMissile(mobj_t * actor, angle_t thresh,
                               angle_t turnMax)
{
    int dir;
    int dist;
    angle_t delta;
    angle_t angle;
    mobj_t *target;
    fixed_t newZ;
    fixed_t deltaZ;

    target = actor->special1.m;
    if (target == NULL)
    {
        return;
    }
    if (!(target->flags & MF_SHOOTABLE)
        || (!(target->flags & MF_COUNTKILL) && !target->player))
    {                           // Target died/target isn't a player or creature
        actor->special1.m = NULL;
        actor->flags &= ~(MF_NOCLIP | MF_SKULLFLY);
        actor->flags |= MF_MISSILE;
        CHolyFindTarget(actor);
        return;
    }
    dir = P_FaceMobj(actor, target, &delta);
    if (delta > thresh)
    {
        delta >>= 1;
        if (delta > turnMax)
        {
            delta = turnMax;
        }
    }
    if (dir)
    {                           // Turn clockwise
        actor->angle += delta;
    }
    else
    {                           // Turn counter clockwise
        actor->angle -= delta;
    }
    angle = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(actor->info->speed, finecosine[angle]);
    actor->momy = FixedMul(actor->info->speed, finesine[angle]);
    if (!(leveltime & 15)
        || actor->z > target->z + (target->height)
        || actor->z + actor->height < target->z)
    {
        newZ = target->z + ((P_Random(pr_hexen) * target->height) >> 8);
        deltaZ = newZ - actor->z;
        if (abs(deltaZ) > 15 * FRACUNIT)
        {
            if (deltaZ > 0)
            {
                deltaZ = 15 * FRACUNIT;
            }
            else
            {
                deltaZ = -15 * FRACUNIT;
            }
        }
        dist = P_AproxDistance(target->x - actor->x, target->y - actor->y);
        dist = dist / actor->info->speed;
        if (dist < 1)
        {
            dist = 1;
        }
        actor->momz = deltaZ / dist;
    }
    return;
}

//============================================================================
//
// A_CHolyWeave
//
//============================================================================

static void CHolyWeave(mobj_t * actor)
{
    fixed_t newX, newY;
    int weaveXY, weaveZ;
    int angle;

    weaveXY = actor->special2.i >> 16;
    weaveZ = actor->special2.i & 0xFFFF;
    angle = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
    newX = actor->x - FixedMul(finecosine[angle],
                               FloatBobOffsets[weaveXY] << 2);
    newY = actor->y - FixedMul(finesine[angle],
                               FloatBobOffsets[weaveXY] << 2);
    weaveXY = (weaveXY + (P_Random(pr_hexen) % 5)) & 63;
    newX += FixedMul(finecosine[angle], FloatBobOffsets[weaveXY] << 2);
    newY += FixedMul(finesine[angle], FloatBobOffsets[weaveXY] << 2);
    P_TryMove(actor, newX, newY);
    actor->z -= FloatBobOffsets[weaveZ] << 1;
    weaveZ = (weaveZ + (P_Random(pr_hexen) % 5)) & 63;
    actor->z += FloatBobOffsets[weaveZ] << 1;
    actor->special2.i = weaveZ + (weaveXY << 16);
}

//============================================================================
//
// A_CHolySeek
//
//============================================================================

void A_CHolySeek(mobj_t * actor)
{
    actor->health--;
    if (actor->health <= 0)
    {
        actor->momx >>= 2;
        actor->momy >>= 2;
        actor->momz = 0;
        P_SetMobjState(actor, actor->info->deathstate);
        actor->tics -= P_Random(pr_hexen) & 3;
        return;
    }
    if (actor->special1.m)
    {
        CHolySeekerMissile(actor, actor->args[0] * ANG1,
                           actor->args[0] * ANG1 * 2);
        if (!((leveltime + 7) & 15))
        {
            actor->args[0] = 5 + (P_Random(pr_hexen) / 20);
        }
    }
    CHolyWeave(actor);
}

//============================================================================
//
// CHolyTailFollow
//
//============================================================================

static void CHolyTailFollow(mobj_t * actor, fixed_t dist)
{
    mobj_t *child;
    int an;
    fixed_t oldDistance, newDistance;

    child = actor->special1.m;
    if (child)
    {
        an = R_PointToAngle2(actor->x, actor->y, child->x,
                             child->y) >> ANGLETOFINESHIFT;
        oldDistance =
            P_AproxDistance(child->x - actor->x, child->y - actor->y);
        if (P_TryMove
            (child, actor->x + FixedMul(dist, finecosine[an]),
             actor->y + FixedMul(dist, finesine[an])))
        {
            newDistance = P_AproxDistance(child->x - actor->x,
                                          child->y - actor->y) - FRACUNIT;
            if (oldDistance < FRACUNIT)
            {
                if (child->z < actor->z)
                {
                    child->z = actor->z - dist;
                }
                else
                {
                    child->z = actor->z + dist;
                }
            }
            else
            {
                child->z = actor->z + FixedMul(FixedDiv(newDistance,
                                                        oldDistance),
                                               child->z - actor->z);
            }
        }
        CHolyTailFollow(child, dist - FRACUNIT);
    }
}

//============================================================================
//
// CHolyTailRemove
//
//============================================================================

static void CHolyTailRemove(mobj_t * actor)
{
    mobj_t *child;

    child = actor->special1.m;
    if (child)
    {
        CHolyTailRemove(child);
    }
    P_RemoveMobj(actor);
}

//============================================================================
//
// A_CHolyTail
//
//============================================================================

void A_CHolyTail(mobj_t * actor)
{
    mobj_t *parent;

    parent = actor->special2.m;

    if (parent)
    {
        if (parent->state >= &states[parent->info->deathstate])
        {                       // Ghost removed, so remove all tail parts
            CHolyTailRemove(actor);
            return;
        }
        else if (P_TryMove(actor, parent->x - FixedMul(14 * FRACUNIT,
                                                       finecosine[parent->
                                                                  angle >>
                                                                  ANGLETOFINESHIFT]),
                           parent->y - FixedMul(14 * FRACUNIT,
                                                finesine[parent->
                                                         angle >>
                                                         ANGLETOFINESHIFT])))
        {
            actor->z = parent->z - 5 * FRACUNIT;
        }
        CHolyTailFollow(actor, 10 * FRACUNIT);
    }
}

//============================================================================
//
// A_CHolyCheckScream
//
//============================================================================

void A_CHolyCheckScream(mobj_t * actor)
{
    A_CHolySeek(actor);
    if (P_Random(pr_hexen) < 20)
    {
        S_StartSound(actor, hexen_sfx_spirit_active);
    }
    if (!actor->special1.m)
    {
        CHolyFindTarget(actor);
    }
}

//============================================================================
//
// A_CHolySpawnPuff
//
//============================================================================

void A_CHolySpawnPuff(mobj_t * actor)
{
    P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_HOLY_MISSILE_PUFF);
}

//----------------------------------------------------------------------------
//
// PROC A_FireConePL1
//
//----------------------------------------------------------------------------

#define SHARDSPAWN_LEFT		1
#define SHARDSPAWN_RIGHT	2
#define SHARDSPAWN_UP		4
#define SHARDSPAWN_DOWN		8

void A_FireConePL1(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int i;
    mobj_t *pmo, *mo;
    int conedone = false;

    pmo = player->mo;
    player->ammo[MANA_1] -= WeaponManaUse[player->pclass][player->readyweapon];
    S_StartSound(pmo, hexen_sfx_mage_shards_fire);

    damage = 90 + (P_Random(pr_hexen) & 15);
    for (i = 0; i < 16; i++)
    {
        angle = pmo->angle + i * (ANG45 / 16);
        P_AimLineAttack(pmo, angle, MELEERANGE, 0);
        if (linetarget)
        {
            pmo->flags2 |= MF2_ICEDAMAGE;
            P_DamageMobj(linetarget, pmo, pmo, damage);
            pmo->flags2 &= ~MF2_ICEDAMAGE;
            conedone = true;
            break;
        }
    }

    // didn't find any creatures, so fire projectiles
    if (!conedone)
    {
        mo = P_SpawnPlayerMissile(pmo, HEXEN_MT_SHARDFX1);
        if (mo)
        {
            mo->special1.i = SHARDSPAWN_LEFT | SHARDSPAWN_DOWN | SHARDSPAWN_UP
                | SHARDSPAWN_RIGHT;
            mo->special2.i = 3;   // Set sperm count (levels of reproductivity)
            mo->target = pmo;
            mo->args[0] = 3;    // Mark Initial shard as super damage
        }
    }
}

void A_ShedShard(mobj_t * actor)
{
    mobj_t *mo;
    int spawndir = actor->special1.i;
    int spermcount = actor->special2.i;

    if (spermcount <= 0)
        return;                 // No sperm left
    actor->special2.i = 0;
    spermcount--;

    // every so many calls, spawn a new missile in it's set directions
    if (spawndir & SHARDSPAWN_LEFT)
    {
        mo = P_SpawnMissileAngleSpeed(actor, HEXEN_MT_SHARDFX1,
                                      actor->angle + (ANG45 / 9), 0,
                                      (20 + 2 * spermcount) << FRACBITS);
        if (mo)
        {
            mo->special1.i = SHARDSPAWN_LEFT;
            mo->special2.i = spermcount;
            mo->momz = actor->momz;
            mo->target = actor->target;
            mo->args[0] = (spermcount == 3) ? 2 : 0;
        }
    }
    if (spawndir & SHARDSPAWN_RIGHT)
    {
        mo = P_SpawnMissileAngleSpeed(actor, HEXEN_MT_SHARDFX1,
                                      actor->angle - (ANG45 / 9), 0,
                                      (20 + 2 * spermcount) << FRACBITS);
        if (mo)
        {
            mo->special1.i = SHARDSPAWN_RIGHT;
            mo->special2.i = spermcount;
            mo->momz = actor->momz;
            mo->target = actor->target;
            mo->args[0] = (spermcount == 3) ? 2 : 0;
        }
    }
    if (spawndir & SHARDSPAWN_UP)
    {
        mo = P_SpawnMissileAngleSpeed(actor, HEXEN_MT_SHARDFX1, actor->angle,
                                      0, (15 + 2 * spermcount) << FRACBITS);
        if (mo)
        {
            mo->momz = actor->momz;
            mo->z += 8 * FRACUNIT;
            if (spermcount & 1) // Every other reproduction
                mo->special1.i =
                    SHARDSPAWN_UP | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
            else
                mo->special1.i = SHARDSPAWN_UP;
            mo->special2.i = spermcount;
            mo->target = actor->target;
            mo->args[0] = (spermcount == 3) ? 2 : 0;
        }
    }
    if (spawndir & SHARDSPAWN_DOWN)
    {
        mo = P_SpawnMissileAngleSpeed(actor, HEXEN_MT_SHARDFX1, actor->angle,
                                      0, (15 + 2 * spermcount) << FRACBITS);
        if (mo)
        {
            mo->momz = actor->momz;
            mo->z -= 4 * FRACUNIT;
            if (spermcount & 1) // Every other reproduction
                mo->special1.i =
                    SHARDSPAWN_DOWN | SHARDSPAWN_LEFT | SHARDSPAWN_RIGHT;
            else
                mo->special1.i = SHARDSPAWN_DOWN;
            mo->special2.i = spermcount;
            mo->target = actor->target;
            mo->args[0] = (spermcount == 3) ? 2 : 0;
        }
    }
}
