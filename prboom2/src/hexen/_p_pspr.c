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


// HEADER FILES ------------------------------------------------------------

#include "h2def.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

// EXTERNAL FUNCTION PROTOTYPES --------------------------------------------

extern void P_ExplodeMissile(mobj_t * mo);
extern void A_UnHideThing(mobj_t * actor);

//
// WEAPON ATTACKS
//

//============================================================================
//
// A_FHammerThrow
//
//============================================================================

void A_FHammerThrow(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    if (!player->mo->special1.i)
    {
        return;
    }
    player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
    mo = P_SpawnPlayerMissile(player->mo, HEXEN_MT_HAMMER_MISSILE);
    if (mo)
    {
        mo->special1.i = 0;
    }
}

//============================================================================
//
// A_FSwordAttack
//
//============================================================================

void A_FSwordAttack(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;

    player->mana[MANA_1] -= WeaponManaUse[player->class][player->readyweapon];
    player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
    pmo = player->mo;
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z - 10 * FRACUNIT,
                  HEXEN_MT_FSWORD_MISSILE, pmo->angle + ANG45 / 4);
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z - 5 * FRACUNIT,
                  HEXEN_MT_FSWORD_MISSILE, pmo->angle + ANG45 / 8);
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z, HEXEN_MT_FSWORD_MISSILE, pmo->angle);
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z + 5 * FRACUNIT,
                  HEXEN_MT_FSWORD_MISSILE, pmo->angle - ANG45 / 8);
    P_SPMAngleXYZ(pmo, pmo->x, pmo->y, pmo->z + 10 * FRACUNIT,
                  HEXEN_MT_FSWORD_MISSILE, pmo->angle - ANG45 / 4);
    S_StartSound(pmo, SFX_FIGHTER_SWORD_FIRE);
}

//============================================================================
//
// A_FSwordAttack2
//
//============================================================================

void A_FSwordAttack2(mobj_t * actor)
{
    angle_t angle = actor->angle;

    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle + ANG45 / 4, 0);
    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle + ANG45 / 8, 0);
    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle, 0);
    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle - ANG45 / 8, 0);
    P_SpawnMissileAngle(actor, HEXEN_MT_FSWORD_MISSILE, angle - ANG45 / 4, 0);
    S_StartSound(actor, SFX_FIGHTER_SWORD_FIRE);
}

//============================================================================
//
// A_FSwordFlames
//
//============================================================================

void A_FSwordFlames(mobj_t * actor)
{
    int i;
    int r1,r2,r3;

    for (i = 1 + (P_Random(pr_hexen) & 3); i; i--)
    {
        r1 = P_Random(pr_hexen);
        r2 = P_Random(pr_hexen);
        r3 = P_Random(pr_hexen);
        P_SpawnMobj(actor->x + ((r3 - 128) << 12), actor->y
                    + ((r2 - 128) << 12),
                    actor->z + ((r1 - 128) << 11), HEXEN_MT_FSWORD_FLAME);
    }
}

//============================================================================
//
// A_MWandAttack
//
//============================================================================

void A_MWandAttack(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;

    mo = P_SpawnPlayerMissile(player->mo, HEXEN_MT_MWAND_MISSILE);
    if (mo)
    {
        mo->thinker.function = P_BlasterMobjThinker;
    }
    S_StartSound(player->mo, SFX_MAGE_WAND_FIRE);
}

// ===== Mage Lightning Weapon =====

//============================================================================
//
// A_LightningReady
//
//============================================================================

void A_LightningReady(player_t * player, pspdef_t * psp)
{
    A_WeaponReady(player, psp);
    if (P_Random(pr_hexen) < 160)
    {
        S_StartSound(player->mo, SFX_MAGE_LIGHTNING_READY);
    }
}

//============================================================================
//
// A_LightningClip
//
//============================================================================

#define ZAGSPEED	FRACUNIT

void A_LightningClip(mobj_t * actor)
{
    mobj_t *cMo;
    mobj_t *target = NULL;
    int zigZag;

    if (actor->type == HEXEN_MT_LIGHTNING_FLOOR)
    {
        actor->z = actor->floorz;
        target = actor->special2.m->special1.m;
    }
    else if (actor->type == HEXEN_MT_LIGHTNING_CEILING)
    {
        actor->z = actor->ceilingz - actor->height;
        target = actor->special1.m;
    }
    if (actor->type == HEXEN_MT_LIGHTNING_FLOOR)
    {                           // floor lightning zig-zags, and forces the ceiling lightning to mimic
        cMo = actor->special2.m;
        zigZag = P_Random(pr_hexen);
        if ((zigZag > 128 && actor->special1.i < 2) || actor->special1.i < -2)
        {
            P_ThrustMobj(actor, actor->angle + ANG90, ZAGSPEED);
            if (cMo)
            {
                P_ThrustMobj(cMo, actor->angle + ANG90, ZAGSPEED);
            }
            actor->special1.i++;
        }
        else
        {
            P_ThrustMobj(actor, actor->angle - ANG90, ZAGSPEED);
            if (cMo)
            {
                P_ThrustMobj(cMo, cMo->angle - ANG90, ZAGSPEED);
            }
            actor->special1.i--;
        }
    }
    if (target)
    {
        if (target->health <= 0)
        {
            P_ExplodeMissile(actor);
        }
        else
        {
            actor->angle = R_PointToAngle2(actor->x, actor->y, target->x,
                                           target->y);
            actor->momx = 0;
            actor->momy = 0;
            P_ThrustMobj(actor, actor->angle, actor->info->speed >> 1);
        }
    }
}

//============================================================================
//
// A_LightningZap
//
//============================================================================

void A_LightningZap(mobj_t * actor)
{
    mobj_t *mo;
    fixed_t deltaZ;
    int r1,r2;

    A_LightningClip(actor);

    actor->health -= 8;
    if (actor->health <= 0)
    {
        P_SetMobjState(actor, actor->info->deathstate);
        return;
    }
    if (actor->type == HEXEN_MT_LIGHTNING_FLOOR)
    {
        deltaZ = 10 * FRACUNIT;
    }
    else
    {
        deltaZ = -10 * FRACUNIT;
    }
    r1 = P_Random(pr_hexen);
    r2 = P_Random(pr_hexen);
    mo = P_SpawnMobj(actor->x + ((r2 - 128) * actor->radius / 256),
                     actor->y + ((r1 - 128) * actor->radius / 256),
                     actor->z + deltaZ, HEXEN_MT_LIGHTNING_ZAP);
    if (mo)
    {
        mo->special2.m = actor;
        mo->momx = actor->momx;
        mo->momy = actor->momy;
        mo->target = actor->target;
        if (actor->type == HEXEN_MT_LIGHTNING_FLOOR)
        {
            mo->momz = 20 * FRACUNIT;
        }
        else
        {
            mo->momz = -20 * FRACUNIT;
        }
    }
/*
	mo = P_SpawnMobj(actor->x+((P_Random(pr_hexen)-128)*actor->radius/256),
		actor->y+((P_Random(pr_hexen)-128)*actor->radius/256),
		actor->z+deltaZ, HEXEN_MT_LIGHTNING_ZAP);
	if(mo)
	{
		mo->special2.m = actor;
		mo->momx = actor->momx;
		mo->momy = actor->momy;
		mo->target = actor->target;
		if(actor->type == HEXEN_MT_LIGHTNING_FLOOR)
		{
			mo->momz = 16*FRACUNIT;
		}
		else
		{
			mo->momz = -16*FRACUNIT;
		}
	}
*/
    if (actor->type == HEXEN_MT_LIGHTNING_FLOOR && P_Random(pr_hexen) < 160)
    {
        S_StartSound(actor, SFX_MAGE_LIGHTNING_CONTINUOUS);
    }
}

//============================================================================
//
// A_MLightningAttack2
//
//============================================================================

void A_MLightningAttack2(mobj_t * actor)
{
    mobj_t *fmo, *cmo;

    fmo = P_SpawnPlayerMissile(actor, HEXEN_MT_LIGHTNING_FLOOR);
    cmo = P_SpawnPlayerMissile(actor, HEXEN_MT_LIGHTNING_CEILING);
    if (fmo)
    {
        fmo->special1.m = NULL;
        fmo->special2.m = cmo;
        A_LightningZap(fmo);
    }
    if (cmo)
    {
        cmo->special1.m = NULL;      // mobj that it will track
        cmo->special2.m = fmo;
        A_LightningZap(cmo);
    }
    S_StartSound(actor, SFX_MAGE_LIGHTNING_FIRE);
}

//============================================================================
//
// A_MLightningAttack
//
//============================================================================

void A_MLightningAttack(player_t * player, pspdef_t * psp)
{
    A_MLightningAttack2(player->mo);
    player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
}

//============================================================================
//
// A_ZapMimic
//
//============================================================================

void A_ZapMimic(mobj_t * actor)
{
    mobj_t *mo;

    mo = actor->special2.m;
    if (mo)
    {
        if (mo->state >= &states[mo->info->deathstate]
            || mo->state == &states[HEXEN_S_FREETARGMOBJ])
        {
            P_ExplodeMissile(actor);
        }
        else
        {
            actor->momx = mo->momx;
            actor->momy = mo->momy;
        }
    }
}

//============================================================================
//
// A_LastZap
//
//============================================================================

void A_LastZap(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HEXEN_MT_LIGHTNING_ZAP);
    if (mo)
    {
        P_SetMobjState(mo, HEXEN_S_LIGHTNING_ZAP_X1);
        mo->momz = 40 * FRACUNIT;
    }
}

//============================================================================
//
// A_LightningRemove
//
//============================================================================

void A_LightningRemove(mobj_t * actor)
{
    mobj_t *mo;

    mo = actor->special2.m;
    if (mo)
    {
        mo->special2.m = NULL;
        P_ExplodeMissile(mo);
    }
}


//============================================================================
//
// MStaffSpawn
//
//============================================================================
void MStaffSpawn(mobj_t * pmo, angle_t angle)
{
    mobj_t *mo;

    mo = P_SPMAngle(pmo, HEXEN_MT_MSTAFF_FX2, angle);
    if (mo)
    {
        mo->target = pmo;
        mo->special1.m = P_RoughMonsterSearch(mo, 10);
    }
}

//============================================================================
//
// A_MStaffAttack
//
//============================================================================

void A_MStaffAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    mobj_t *pmo;

    player->mana[MANA_1] -= WeaponManaUse[player->class][player->readyweapon];
    player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
    pmo = player->mo;
    angle = pmo->angle;

    MStaffSpawn(pmo, angle);
    MStaffSpawn(pmo, angle - ANG1 * 5);
    MStaffSpawn(pmo, angle + ANG1 * 5);
    S_StartSound(player->mo, SFX_MAGE_STAFF_FIRE);
    if (player == &players[consoleplayer])
    {
        player->damagecount = 0;
        player->bonuscount = 0;
        I_SetPalette((byte *) W_CacheLumpNum(W_GetNumForName("playpal"),
                                             PU_CACHE) +
                     STARTSCOURGEPAL * 768);
    }
}

//============================================================================
//
// A_MStaffPalette
//
//============================================================================

void A_MStaffPalette(player_t * player, pspdef_t * psp)
{
    int pal;

    if (player == &players[consoleplayer])
    {
        pal = STARTSCOURGEPAL + psp->state - (&states[HEXEN_S_MSTAFFATK_2]);
        if (pal == STARTSCOURGEPAL + 3)
        {                       // reset back to original playpal
            pal = 0;
        }
        I_SetPalette((byte *) W_CacheLumpNum(W_GetNumForName("playpal"),
                                             PU_CACHE) + pal * 768);
    }
}

//============================================================================
//
// A_MStaffWeave
//
//============================================================================

void A_MStaffWeave(mobj_t * actor)
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
    weaveXY = (weaveXY + 6) & 63;
    newX += FixedMul(finecosine[angle], FloatBobOffsets[weaveXY] << 2);
    newY += FixedMul(finesine[angle], FloatBobOffsets[weaveXY] << 2);
    P_TryMove(actor, newX, newY);
    actor->z -= FloatBobOffsets[weaveZ] << 1;
    weaveZ = (weaveZ + 3) & 63;
    actor->z += FloatBobOffsets[weaveZ] << 1;
    if (actor->z <= actor->floorz)
    {
        actor->z = actor->floorz + FRACUNIT;
    }
    actor->special2.i = weaveZ + (weaveXY << 16);
}


//============================================================================
//
// A_MStaffTrack
//
//============================================================================

void A_MStaffTrack(mobj_t * actor)
{
    if ((actor->special1.m == NULL) && (P_Random(pr_hexen) < 50))
    {
        actor->special1.m = P_RoughMonsterSearch(actor, 10);
    }
    P_SeekerMissile(actor, ANG1 * 2, ANG1 * 10);
}


//============================================================================
//
// MStaffSpawn2 - for use by mage class boss
//
//============================================================================

void MStaffSpawn2(mobj_t * actor, angle_t angle)
{
    mobj_t *mo;

    mo = P_SpawnMissileAngle(actor, HEXEN_MT_MSTAFF_FX2, angle, 0);
    if (mo)
    {
        mo->target = actor;
        mo->special1.m = P_RoughMonsterSearch(mo, 10);
    }
}

//============================================================================
//
// A_MStaffAttack2 - for use by mage class boss
//
//============================================================================

void A_MStaffAttack2(mobj_t * actor)
{
    angle_t angle;
    angle = actor->angle;
    MStaffSpawn2(actor, angle);
    MStaffSpawn2(actor, angle - ANG1 * 5);
    MStaffSpawn2(actor, angle + ANG1 * 5);
    S_StartSound(actor, SFX_MAGE_STAFF_FIRE);
}

//============================================================================
//
// A_FPunchAttack
//
//============================================================================

void A_FPunchAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;
    mobj_t *pmo = player->mo;
    fixed_t power;
    int i;

    damage = 40 + (P_Random(pr_hexen) & 15);
    power = 2 * FRACUNIT;
    PuffType = HEXEN_MT_PUNCHPUFF;
    for (i = 0; i < 16; i++)
    {
        angle = pmo->angle + i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, 2 * MELEERANGE);
        if (linetarget)
        {
            player->mo->special1.i++;
            if (pmo->special1.i == 3)
            {
                damage <<= 1;
                power = 6 * FRACUNIT;
                PuffType = HEXEN_MT_HAMMERPUFF;
            }
            P_LineAttack(pmo, angle, 2 * MELEERANGE, slope, damage);
            if (linetarget->flags & MF_COUNTKILL || linetarget->player)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            AdjustPlayerAngle(pmo);
            goto punchdone;
        }
        angle = pmo->angle - i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, 2 * MELEERANGE);
        if (linetarget)
        {
            pmo->special1.i++;
            if (pmo->special1.i == 3)
            {
                damage <<= 1;
                power = 6 * FRACUNIT;
                PuffType = HEXEN_MT_HAMMERPUFF;
            }
            P_LineAttack(pmo, angle, 2 * MELEERANGE, slope, damage);
            if (linetarget->flags & MF_COUNTKILL || linetarget->player)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            AdjustPlayerAngle(pmo);
            goto punchdone;
        }
    }
    // didn't find any creatures, so try to strike any walls
    pmo->special1.i = 0;

    angle = pmo->angle;
    slope = P_AimLineAttack(pmo, angle, MELEERANGE);
    P_LineAttack(pmo, angle, MELEERANGE, slope, damage);

  punchdone:
    if (pmo->special1.i == 3)
    {
        pmo->special1.i = 0;
        P_SetPsprite(player, ps_weapon, HEXEN_S_PUNCHATK2_1);
        S_StartSound(pmo, SFX_FIGHTER_GRUNT);
    }
    return;
}

//============================================================================
//
// A_FAxeAttack
//
//============================================================================

#define AXERANGE	2.25*MELEERANGE

void A_FAxeAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    mobj_t *pmo = player->mo;
    fixed_t power;
    int damage;
    int slope;
    int i;
    int useMana;
    int r;

    r = P_Random(pr_hexen);
    damage = 40 + (r & 15) + (P_Random(pr_hexen) & 7);
    power = 0;
    if (player->mana[MANA_1] > 0)
    {
        damage <<= 1;
        power = 6 * FRACUNIT;
        PuffType = HEXEN_MT_AXEPUFF_GLOW;
        useMana = 1;
    }
    else
    {
        PuffType = HEXEN_MT_AXEPUFF;
        useMana = 0;
    }
    for (i = 0; i < 16; i++)
    {
        angle = pmo->angle + i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, AXERANGE);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, AXERANGE, slope, damage);
            if (linetarget->flags & MF_COUNTKILL || linetarget->player)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            AdjustPlayerAngle(pmo);
            useMana++;
            goto axedone;
        }
        angle = pmo->angle - i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, AXERANGE);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, AXERANGE, slope, damage);
            if (linetarget->flags & MF_COUNTKILL)
            {
                P_ThrustMobj(linetarget, angle, power);
            }
            AdjustPlayerAngle(pmo);
            useMana++;
            goto axedone;
        }
    }
    // didn't find any creatures, so try to strike any walls
    pmo->special1.m = NULL;

    angle = pmo->angle;
    slope = P_AimLineAttack(pmo, angle, MELEERANGE);
    P_LineAttack(pmo, angle, MELEERANGE, slope, damage);

  axedone:
    if (useMana == 2)
    {
        player->mana[MANA_1] -=
            WeaponManaUse[player->class][player->readyweapon];
        if (player->mana[MANA_1] <= 0)
        {
            P_SetPsprite(player, ps_weapon, HEXEN_S_FAXEATK_5);
        }
    }
    return;
}

//===========================================================================
//
// A_CMaceAttack
//
//===========================================================================

void A_CMaceAttack(player_t * player, pspdef_t * psp)
{
    angle_t angle;
    int damage;
    int slope;
    int i;

    damage = 25 + (P_Random(pr_hexen) & 15);
    PuffType = HEXEN_MT_HAMMERPUFF;
    for (i = 0; i < 16; i++)
    {
        angle = player->mo->angle + i * (ANG45 / 16);
        slope = P_AimLineAttack(player->mo, angle, 2 * MELEERANGE);
        if (linetarget)
        {
            P_LineAttack(player->mo, angle, 2 * MELEERANGE, slope, damage);
            AdjustPlayerAngle(player->mo);
//                      player->mo->angle = R_PointToAngle2(player->mo->x,
//                              player->mo->y, linetarget->x, linetarget->y);
            goto macedone;
        }
        angle = player->mo->angle - i * (ANG45 / 16);
        slope = P_AimLineAttack(player->mo, angle, 2 * MELEERANGE);
        if (linetarget)
        {
            P_LineAttack(player->mo, angle, 2 * MELEERANGE, slope, damage);
            AdjustPlayerAngle(player->mo);
//                      player->mo->angle = R_PointToAngle2(player->mo->x,
//                              player->mo->y, linetarget->x, linetarget->y);
            goto macedone;
        }
    }
    // didn't find any creatures, so try to strike any walls
    player->mo->special1.m = NULL;

    angle = player->mo->angle;
    slope = P_AimLineAttack(player->mo, angle, MELEERANGE);
    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);
  macedone:
    return;
}

//============================================================================
//
// A_CStaffCheck
//
//============================================================================

void A_CStaffCheck(player_t * player, pspdef_t * psp)
{
    mobj_t *pmo;
    int damage;
    int newLife;
    angle_t angle;
    int slope;
    int i;

    pmo = player->mo;
    damage = 20 + (P_Random(pr_hexen) & 15);
    PuffType = HEXEN_MT_CSTAFFPUFF;
    for (i = 0; i < 3; i++)
    {
        angle = pmo->angle + i * (ANG45 / 16);
        slope = P_AimLineAttack(pmo, angle, 1.5 * MELEERANGE);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, 1.5 * MELEERANGE, slope, damage);
            pmo->angle = R_PointToAngle2(pmo->x, pmo->y,
                                         linetarget->x, linetarget->y);
            if ((linetarget->player || linetarget->flags & MF_COUNTKILL)
                && (!(linetarget->flags2 & (MF2_DORMANT + MF2_INVULNERABLE))))
            {
                newLife = player->health + (damage >> 3);
                newLife = newLife > 100 ? 100 : newLife;
                pmo->health = player->health = newLife;
                P_SetPsprite(player, ps_weapon, HEXEN_S_CSTAFFATK2_1);
            }
            player->mana[MANA_1] -=
                WeaponManaUse[player->class][player->readyweapon];
            break;
        }
        angle = pmo->angle - i * (ANG45 / 16);
        slope = P_AimLineAttack(player->mo, angle, 1.5 * MELEERANGE);
        if (linetarget)
        {
            P_LineAttack(pmo, angle, 1.5 * MELEERANGE, slope, damage);
            pmo->angle = R_PointToAngle2(pmo->x, pmo->y,
                                         linetarget->x, linetarget->y);
            if (linetarget->player || linetarget->flags & MF_COUNTKILL)
            {
                newLife = player->health + (damage >> 4);
                newLife = newLife > 100 ? 100 : newLife;
                pmo->health = player->health = newLife;
                P_SetPsprite(player, ps_weapon, HEXEN_S_CSTAFFATK2_1);
            }
            player->mana[MANA_1] -=
                WeaponManaUse[player->class][player->readyweapon];
            break;
        }
    }
}

//============================================================================
//
// A_CStaffAttack
//
//============================================================================

void A_CStaffAttack(player_t * player, pspdef_t * psp)
{
    mobj_t *mo;
    mobj_t *pmo;

    player->mana[MANA_1] -= WeaponManaUse[player->class][player->readyweapon];
    pmo = player->mo;
    mo = P_SPMAngle(pmo, HEXEN_MT_CSTAFF_MISSILE, pmo->angle - (ANG45 / 15));
    if (mo)
    {
        mo->special2.i = 32;
    }
    mo = P_SPMAngle(pmo, HEXEN_MT_CSTAFF_MISSILE, pmo->angle + (ANG45 / 15));
    if (mo)
    {
        mo->special2.i = 0;
    }
    S_StartSound(player->mo, SFX_CLERIC_CSTAFF_FIRE);
}

//============================================================================
//
// A_CStaffMissileSlither
//
//============================================================================

void A_CStaffMissileSlither(mobj_t * actor)
{
    fixed_t newX, newY;
    int weaveXY;
    int angle;

    weaveXY = actor->special2.i;
    angle = (actor->angle + ANG90) >> ANGLETOFINESHIFT;
    newX = actor->x - FixedMul(finecosine[angle], FloatBobOffsets[weaveXY]);
    newY = actor->y - FixedMul(finesine[angle], FloatBobOffsets[weaveXY]);
    weaveXY = (weaveXY + 3) & 63;
    newX += FixedMul(finecosine[angle], FloatBobOffsets[weaveXY]);
    newY += FixedMul(finesine[angle], FloatBobOffsets[weaveXY]);
    P_TryMove(actor, newX, newY);
    actor->special2.i = weaveXY;
}

//============================================================================
//
// A_CStaffInitBlink
//
//============================================================================

void A_CStaffInitBlink(player_t * player, pspdef_t * psp)
{
    player->mo->special1.i = (P_Random(pr_hexen) >> 1) + 20;
}

//============================================================================
//
// A_CStaffCheckBlink
//
//============================================================================

void A_CStaffCheckBlink(player_t * player, pspdef_t * psp)
{
    if (!--player->mo->special1.i)
    {
        P_SetPsprite(player, ps_weapon, HEXEN_S_CSTAFFBLINK1);
        player->mo->special1.i = (P_Random(pr_hexen) + 50) >> 2;
    }
}

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

    player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
    S_StartSound(player->mo, SFX_CLERIC_FLAME_FIRE);
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
    S_StartSound(actor, SFX_CLERIC_FLAME_EXPLODE);
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
    S_StartSound(actor, SFX_CLERIC_FLAME_EXPLODE);
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
    S_StartSound(actor, SFX_CHOLY_FIRE);
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
    player->mana[MANA_1] -= WeaponManaUse[player->class][player->readyweapon];
    player->mana[MANA_2] -= WeaponManaUse[player->class][player->readyweapon];
    P_SpawnPlayerMissile(player->mo, HEXEN_MT_HOLY_MISSILE);
    if (player == &players[consoleplayer])
    {
        player->damagecount = 0;
        player->bonuscount = 0;
        I_SetPalette((byte *) W_CacheLumpNum(W_GetNumForName("playpal"),
                                             PU_CACHE) + STARTHOLYPAL * 768);
    }
    S_StartSound(player->mo, SFX_CHOLY_FIRE);
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
        S_StartSound(actor, SFX_SPIRIT_ACTIVE);
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
    player->mana[MANA_1] -= WeaponManaUse[player->class][player->readyweapon];
    S_StartSound(pmo, SFX_MAGE_SHARDS_FIRE);

    damage = 90 + (P_Random(pr_hexen) & 15);
    for (i = 0; i < 16; i++)
    {
        angle = pmo->angle + i * (ANG45 / 16);
        P_AimLineAttack(pmo, angle, MELEERANGE);
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
