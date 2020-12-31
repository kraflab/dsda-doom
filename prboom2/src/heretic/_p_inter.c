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

// P_inter.c

#include "doomdef.h"
#include "deh_str.h"
#include "i_system.h"
#include "i_timer.h"
#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

#define BONUSADD 6

//--------------------------------------------------------------------------
//
// PROC P_SetCenterMessage
//
// [crispy] Set centered message
//
//--------------------------------------------------------------------------
void P_SetCenterMessage(player_t * player, const char *message)
{
    if (!messageson)
    {
        return;
    }
    player->centerMessage = message;
    player->centerMessageTics = MESSAGETICS;
    BorderTopRefresh = true;
}

//---------------------------------------------------------------------------
//
// PROC P_KillMobj
//
//---------------------------------------------------------------------------

void P_KillMobj(mobj_t * source, mobj_t * target)
{
    target->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY | MF_NOGRAVITY);
    target->flags |= MF_CORPSE | MF_DROPOFF;
    target->flags2 &= ~MF2_PASSMOBJ;
    target->height >>= 2;
    if (source && source->player)
    {
        if (target->flags & MF_COUNTKILL)
        {                       // Count for intermission
            source->player->killcount++;
        }
        if (target->player)
        {                       // Frag stuff
            if (target == source)
            {                   // Self-frag
                target->player->frags[target->player - players]--;
            }
            else
            {
                source->player->frags[target->player - players]++;
                if (source->player == &players[consoleplayer])
                {
                    S_StartSound(NULL, sfx_gfrag);
                }
                if (source->player->chickenTics)
                {               // Make a super chicken
                    P_GivePower(source->player, pw_weaponlevel2);
                }
            }
        }
    }
    else if (!netgame && (target->flags & MF_COUNTKILL))
    {                           // Count all monster deaths
        players[0].killcount++;
    }
    if (target->player)
    {
        if (!source)
        {                       // Self-frag
            target->player->frags[target->player - players]--;
        }
        target->flags &= ~MF_SOLID;
        target->flags2 &= ~MF2_FLY;
        target->player->powers[pw_flight] = 0;
        target->player->powers[pw_weaponlevel2] = 0;
        target->player->playerstate = PST_DEAD;
        P_DropWeapon(target->player);
        if (target->flags2 & MF2_FIREDAMAGE)
        {                       // Player flame death
            P_SetMobjState(target, S_PLAY_FDTH1);
            //S_StartSound(target, sfx_hedat1); // Burn sound
            return;
        }
    }
    if (target->health < -(target->info->spawnhealth >> 1)
        && target->info->xdeathstate)
    {                           // Extreme death
        P_SetMobjState(target, target->info->xdeathstate);
    }
    else
    {                           // Normal death
        P_SetMobjState(target, target->info->deathstate);
    }
    target->tics -= P_Random() & 3;
//      I_StartSound(&actor->r, actor->info->deathsound);
}

//---------------------------------------------------------------------------
//
// FUNC P_MinotaurSlam
//
//---------------------------------------------------------------------------

void P_MinotaurSlam(mobj_t * source, mobj_t * target)
{
    angle_t angle;
    fixed_t thrust;

    angle = R_PointToAngle2(source->x, source->y, target->x, target->y);
    angle >>= ANGLETOFINESHIFT;
    thrust = 16 * FRACUNIT + (P_Random() << 10);
    target->momx += FixedMul(thrust, finecosine[angle]);
    target->momy += FixedMul(thrust, finesine[angle]);
    P_DamageMobj(target, NULL, NULL, HITDICE(6));
    if (target->player)
    {
        target->reactiontime = 14 + (P_Random() & 7);
    }
}

//---------------------------------------------------------------------------
//
// FUNC P_TouchWhirlwind
//
//---------------------------------------------------------------------------

void P_TouchWhirlwind(mobj_t * target)
{
    int randVal;

    target->angle += P_SubRandom() << 20;
    target->momx += P_SubRandom() << 10;
    target->momy += P_SubRandom() << 10;
    if (leveltime & 16 && !(target->flags2 & MF2_BOSS))
    {
        randVal = P_Random();
        if (randVal > 160)
        {
            randVal = 160;
        }
        target->momz += randVal << 10;
        if (target->momz > 12 * FRACUNIT)
        {
            target->momz = 12 * FRACUNIT;
        }
    }
    if (!(leveltime & 7))
    {
        P_DamageMobj(target, NULL, NULL, 3);
    }
}

//---------------------------------------------------------------------------
//
// FUNC P_ChickenMorphPlayer
//
// Returns true if the player gets turned into a chicken.
//
//---------------------------------------------------------------------------

boolean P_ChickenMorphPlayer(player_t * player)
{
    mobj_t *pmo;
    mobj_t *fog;
    mobj_t *chicken;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int oldFlags2;

    if (player->chickenTics)
    {
        if ((player->chickenTics < CHICKENTICS - TICRATE)
            && !player->powers[pw_weaponlevel2])
        {                       // Make a super chicken
            P_GivePower(player, pw_weaponlevel2);
        }
        return (false);
    }
    if (player->powers[pw_invulnerability])
    {                           // Immune when invulnerable
        return (false);
    }
    pmo = player->mo;
    x = pmo->x;
    y = pmo->y;
    z = pmo->z;
    angle = pmo->angle;
    oldFlags2 = pmo->flags2;
    P_SetMobjState(pmo, S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MT_TFOG);
    S_StartSound(fog, sfx_telept);
    chicken = P_SpawnMobj(x, y, z, MT_CHICPLAYER);
    chicken->special1.i = player->readyweapon;
    chicken->angle = angle;
    chicken->player = player;
    player->health = chicken->health = MAXCHICKENHEALTH;
    player->mo = chicken;
    player->armorpoints = player->armortype = 0;
    player->powers[pw_invisibility] = 0;
    player->powers[pw_weaponlevel2] = 0;
    if (oldFlags2 & MF2_FLY)
    {
        chicken->flags2 |= MF2_FLY;
    }
    player->chickenTics = CHICKENTICS;
    P_ActivateBeak(player);
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_ChickenMorph
//
//---------------------------------------------------------------------------

boolean P_ChickenMorph(mobj_t * actor)
{
    mobj_t *fog;
    mobj_t *chicken;
    mobj_t *target;
    mobjtype_t moType;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int ghost;

    if (actor->player)
    {
        return (false);
    }
    moType = actor->type;
    switch (moType)
    {
        case MT_POD:
        case MT_CHICKEN:
        case MT_HEAD:
        case MT_MINOTAUR:
        case MT_SORCERER1:
        case MT_SORCERER2:
            return (false);
        default:
            break;
    }
    x = actor->x;
    y = actor->y;
    z = actor->z;
    angle = actor->angle;
    ghost = actor->flags & MF_SHADOW;
    target = actor->target;
    P_SetMobjState(actor, S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, MT_TFOG);
    S_StartSound(fog, sfx_telept);
    chicken = P_SpawnMobj(x, y, z, MT_CHICKEN);
    chicken->special2.i = moType;
    chicken->special1.i = CHICKENTICS + P_Random();
    chicken->flags |= ghost;
    chicken->target = target;
    chicken->angle = angle;
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_AutoUseChaosDevice
//
//---------------------------------------------------------------------------

boolean P_AutoUseChaosDevice(player_t * player)
{
    int i;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti_teleport)
        {
            P_PlayerUseArtifact(player, arti_teleport);
            player->health = player->mo->health = (player->health + 1) / 2;
            return (true);
        }
    }
    return (false);
}

//---------------------------------------------------------------------------
//
// PROC P_AutoUseHealth
//
//---------------------------------------------------------------------------

void P_AutoUseHealth(player_t * player, int saveHealth)
{
    int i;
    int count;
    int normalCount;
    int normalSlot;
    int superCount;
    int superSlot;

    normalCount = 0;
    superCount = 0;
    normalSlot = 0;
    superSlot = 0;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti_health)
        {
            normalSlot = i;
            normalCount = player->inventory[i].count;
        }
        else if (player->inventory[i].type == arti_superhealth)
        {
            superSlot = i;
            superCount = player->inventory[i].count;
        }
    }
    if ((gameskill == sk_baby) && (normalCount * 25 >= saveHealth))
    {                           // Use quartz flasks
        count = (saveHealth + 24) / 25;
        for (i = 0; i < count; i++)
        {
            player->health += 25;
            P_PlayerRemoveArtifact(player, normalSlot);
        }
    }
    else if (superCount * 100 >= saveHealth)
    {                           // Use mystic urns
        count = (saveHealth + 99) / 100;
        for (i = 0; i < count; i++)
        {
            player->health += 100;
            P_PlayerRemoveArtifact(player, superSlot);
        }
    }
    else if ((gameskill == sk_baby)
             && (superCount * 100 + normalCount * 25 >= saveHealth))
    {                           // Use mystic urns and quartz flasks
        count = (saveHealth + 24) / 25;
        saveHealth -= count * 25;
        for (i = 0; i < count; i++)
        {
            player->health += 25;
            P_PlayerRemoveArtifact(player, normalSlot);
        }
        count = (saveHealth + 99) / 100;
        for (i = 0; i < count; i++)
        {
            player->health += 100;
            P_PlayerRemoveArtifact(player, normalSlot);
        }
    }
    player->mo->health = player->health;
}

/*
=================
=
= P_DamageMobj
=
= Damages both enemies and players
= inflictor is the thing that caused the damage
= 		creature or missile, can be NULL (slime, etc)
= source is the thing to target after taking damage
=		creature or NULL
= Source and inflictor are the same for melee attacks
= source can be null for barrel explosions and other environmental stuff
==================
*/

void P_DamageMobj
    (mobj_t * target, mobj_t * inflictor, mobj_t * source, int damage)
{
    unsigned ang;
    int saved;
    player_t *player;
    fixed_t thrust;
    int temp;

    if (!(target->flags & MF_SHOOTABLE))
    {
        // Shouldn't happen
        return;
    }
    if (target->health <= 0)
    {
        return;
    }
    if (target->flags & MF_SKULLFLY)
    {
        if (target->type == MT_MINOTAUR)
        {                       // Minotaur is invulnerable during charge attack
            return;
        }
        target->momx = target->momy = target->momz = 0;
    }
    player = target->player;
    if (player && gameskill == sk_baby)
    {
        // Take half damage in trainer mode
        damage >>= 1;
    }
    // Special damage types
    if (inflictor)
    {
        switch (inflictor->type)
        {
            case MT_EGGFX:
                if (player)
                {
                    P_ChickenMorphPlayer(player);
                }
                else
                {
                    P_ChickenMorph(target);
                }
                return;         // Always return
            case MT_WHIRLWIND:
                P_TouchWhirlwind(target);
                return;
            case MT_MINOTAUR:
                if (inflictor->flags & MF_SKULLFLY)
                {               // Slam only when in charge mode
                    P_MinotaurSlam(inflictor, target);
                    return;
                }
                break;
            case MT_MACEFX4:   // Death ball
                if ((target->flags2 & MF2_BOSS) || target->type == MT_HEAD)
                {               // Don't allow cheap boss kills
                    break;
                }
                else if (target->player)
                {               // Player specific checks
                    if (target->player->powers[pw_invulnerability])
                    {           // Can't hurt invulnerable players
                        break;
                    }
                    if (P_AutoUseChaosDevice(target->player))
                    {           // Player was saved using chaos device
                        return;
                    }
                }
                damage = 10000; // Something's gonna die
                break;
            case MT_PHOENIXFX2:        // Flame thrower
                if (target->player && P_Random() < 128)
                {               // Freeze player for a bit
                    target->reactiontime += 4;
                }
                break;
            case MT_RAINPLR1:  // Rain missiles
            case MT_RAINPLR2:
            case MT_RAINPLR3:
            case MT_RAINPLR4:
                if (target->flags2 & MF2_BOSS)
                {               // Decrease damage for bosses
                    damage = (P_Random() & 7) + 1;
                }
                break;
            case MT_HORNRODFX2:
            case MT_PHOENIXFX1:
                if (target->type == MT_SORCERER2 && P_Random() < 96)
                {               // D'Sparil teleports away
                    P_DSparilTeleport(target);
                    return;
                }
                break;
            case MT_BLASTERFX1:
            case MT_RIPPER:
                if (target->type == MT_HEAD)
                {               // Less damage to Ironlich bosses
                    damage = P_Random() & 1;
                    if (!damage)
                    {
                        return;
                    }
                }
                break;
            default:
                break;
        }
    }
    // Push the target unless source is using the gauntlets
    if (inflictor && (!source || !source->player
                      || source->player->readyweapon != wp_gauntlets)
        && !(inflictor->flags2 & MF2_NODMGTHRUST))
    {
        ang = R_PointToAngle2(inflictor->x, inflictor->y,
                              target->x, target->y);
        //thrust = damage*(FRACUNIT>>3)*100/target->info->mass;
        thrust = damage * (FRACUNIT >> 3) * 150 / target->info->mass;
        // make fall forwards sometimes
        if ((damage < 40) && (damage > target->health)
            && (target->z - inflictor->z > 64 * FRACUNIT) && (P_Random() & 1))
        {
            ang += ANG180;
            thrust *= 4;
        }
        ang >>= ANGLETOFINESHIFT;
        if (source && source->player && (source == inflictor)
            && source->player->powers[pw_weaponlevel2]
            && source->player->readyweapon == wp_staff)
        {
            // Staff power level 2
            target->momx += FixedMul(10 * FRACUNIT, finecosine[ang]);
            target->momy += FixedMul(10 * FRACUNIT, finesine[ang]);
            if (!(target->flags & MF_NOGRAVITY))
            {
                target->momz += 5 * FRACUNIT;
            }
        }
        else
        {
            target->momx += FixedMul(thrust, finecosine[ang]);
            target->momy += FixedMul(thrust, finesine[ang]);
        }
    }

    //
    // player specific
    //
    if (player)
    {
        // end of game hell hack
        //if(target->subsector->sector->special == 11
        //      && damage >= target->health)
        //{
        //      damage = target->health - 1;
        //}

        if (damage < 1000 && ((player->cheats & CF_GODMODE)
                              || player->powers[pw_invulnerability]))
        {
            return;
        }
        if (player->armortype)
        {
            if (player->armortype == 1)
            {
                saved = damage >> 1;
            }
            else
            {
                saved = (damage >> 1) + (damage >> 2);
            }
            if (player->armorpoints <= saved)
            {
                // armor is used up
                saved = player->armorpoints;
                player->armortype = 0;
            }
            player->armorpoints -= saved;
            damage -= saved;
        }
        if (damage >= player->health
            && ((gameskill == sk_baby) || deathmatch) && !player->chickenTics)
        {                       // Try to use some inventory health
            P_AutoUseHealth(player, damage - player->health + 1);
        }
        player->health -= damage;       // mirror mobj health here for Dave
        if (player->health < 0)
        {
            player->health = 0;
        }
        player->attacker = source;
        player->damagecount += damage;  // add damage after armor / invuln
        if (player->damagecount > 100)
        {
            player->damagecount = 100;  // teleport stomp does 10k points...
        }
        temp = damage < 100 ? damage : 100;
        if (player == &players[consoleplayer])
        {
            I_Tactile(40, 10, 40 + temp * 2);
            SB_PaletteFlash();
        }
    }

    //
    // do the damage
    //
    target->health -= damage;
    if (target->health <= 0)
    {                           // Death
        target->special1.i = damage;
        if (target->type == MT_POD && source && source->type != MT_POD)
        {                       // Make sure players get frags for chain-reaction kills
            target->target = source;
        }
        if (player && inflictor && !player->chickenTics)
        {                       // Check for flame death
            if ((inflictor->flags2 & MF2_FIREDAMAGE)
                || ((inflictor->type == MT_PHOENIXFX1)
                    && (target->health > -50) && (damage > 25)))
            {
                target->flags2 |= MF2_FIREDAMAGE;
            }
        }
        P_KillMobj(source, target);
        return;
    }
    if ((P_Random() < target->info->painchance)
        && !(target->flags & MF_SKULLFLY))
    {
        target->flags |= MF_JUSTHIT;    // fight back!
        P_SetMobjState(target, target->info->painstate);
    }
    target->reactiontime = 0;   // we're awake now...
    if (!target->threshold && source && !(source->flags2 & MF2_BOSS)
        && !(target->type == MT_SORCERER2 && source->type == MT_WIZARD))
    {
        // Target actor is not intent on another actor,
        // so make him chase after source
        target->target = source;
        target->threshold = BASETHRESHOLD;
        if (target->state == &states[target->info->spawnstate]
            && target->info->seestate != S_NULL)
        {
            P_SetMobjState(target, target->info->seestate);
        }
    }
}
