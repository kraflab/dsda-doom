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
