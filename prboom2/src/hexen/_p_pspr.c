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
