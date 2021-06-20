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
    fixed_t savedPercent;
    player_t *player;
    mobj_t *master;
    fixed_t thrust;
    int temp;
    int i;

    if (!(target->flags & MF_SHOOTABLE))
    {
        // Shouldn't happen
        return;
    }
    if (target->health <= 0)
    {
        if (inflictor && inflictor->flags2 & MF2_ICEDAMAGE)
        {
            return;
        }
        else if (target->flags & MF_ICECORPSE)  // frozen
        {
            target->tics = 1;
            target->momx = target->momy = 0;
        }
        return;
    }
    if ((target->flags2 & MF2_INVULNERABLE) && damage < 10000)
    {                           // mobj is invulnerable
        if (target->player)
            return;             // for player, no exceptions
        if (inflictor)
        {
            switch (inflictor->type)
            {
                    // These inflictors aren't foiled by invulnerability
                case HEXEN_MT_HOLY_FX:
                case HEXEN_MT_POISONCLOUD:
                case HEXEN_MT_FIREBOMB:
                    break;
                default:
                    return;
            }
        }
        else
        {
            return;
        }
    }
    if (target->player)
    {
        if (damage < 1000 && ((target->player->cheats & CF_GODMODE)
                              || target->player->powers[pw_invulnerability]))
        {
            return;
        }
    }
    if (target->flags & MF_SKULLFLY)
    {
        target->momx = target->momy = target->momz = 0;
    }
    if (target->flags2 & MF2_DORMANT)
    {
        // Invulnerable, and won't wake up
        return;
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
            case HEXEN_MT_EGGFX:
                if (player)
                {
                    P_MorphPlayer(player);
                }
                else
                {
                    P_MorphMonster(target);
                }
                return;         // Always return
            case HEXEN_MT_TELOTHER_FX1:
            case HEXEN_MT_TELOTHER_FX2:
            case HEXEN_MT_TELOTHER_FX3:
            case HEXEN_MT_TELOTHER_FX4:
            case HEXEN_MT_TELOTHER_FX5:
                if ((target->flags & MF_COUNTKILL) &&
                    (target->type != HEXEN_MT_SERPENT) &&
                    (target->type != HEXEN_MT_SERPENTLEADER) &&
                    (!(target->flags2 & MF2_BOSS)))
                {
                    P_TeleportOther(target);
                }
                return;
            case HEXEN_MT_MINOTAUR:
                if (inflictor->flags & MF_SKULLFLY)
                {               // Slam only when in charge mode
                    P_MinotaurSlam(inflictor, target);
                    return;
                }
                break;
            case HEXEN_MT_BISH_FX:
                // Bishops are just too nasty
                damage >>= 1;
                break;
            case HEXEN_MT_SHARDFX1:
                switch (inflictor->special2.i)
                {
                    case 3:
                        damage <<= 3;
                        break;
                    case 2:
                        damage <<= 2;
                        break;
                    case 1:
                        damage <<= 1;
                        break;
                    default:
                        break;
                }
                break;
            case HEXEN_MT_CSTAFF_MISSILE:
                // Cleric Serpent Staff does poison damage
                if (target->player)
                {
                    P_PoisonPlayer(target->player, source, 20);
                    damage >>= 1;
                }
                break;
            case HEXEN_MT_ICEGUY_FX2:
                damage >>= 1;
                break;
            case HEXEN_MT_POISONDART:
                if (target->player)
                {
                    P_PoisonPlayer(target->player, source, 20);
                    damage >>= 1;
                }
                break;
            case HEXEN_MT_POISONCLOUD:
                if (target->player)
                {
                    if (target->player->poisoncount < 4)
                    {
                        P_PoisonDamage(target->player, source, 15 + (P_Random(pr_hexen) & 15), false);  // Don't play painsound
                        P_PoisonPlayer(target->player, source, 50);
                        S_StartSound(target, hexen_sfx_player_poisoncough);
                    }
                    return;
                }
                else if (!(target->flags & MF_COUNTKILL))
                {               // only damage monsters/players with the poison cloud
                    return;
                }
                break;
            case HEXEN_MT_FSWORD_MISSILE:
                if (target->player)
                {
                    damage -= damage >> 2;
                }
                break;
            default:
                break;
        }
    }
    // Push the target unless source is using the gauntlets
    if (inflictor && (!source || !source->player)
        && !(inflictor->flags2 & MF2_NODMGTHRUST))
    {
        ang = R_PointToAngle2(inflictor->x, inflictor->y,
                              target->x, target->y);
        //thrust = damage*(FRACUNIT>>3)*100/target->info->mass;
        thrust = damage * (FRACUNIT >> 3) * 150 / target->info->mass;
        // make fall forwards sometimes
        if ((damage < 40) && (damage > target->health)
            && (target->z - inflictor->z > 64 * FRACUNIT) && (P_Random(pr_hexen) & 1))
        {
            ang += ANG180;
            thrust *= 4;
        }
        ang >>= ANGLETOFINESHIFT;
        target->momx += FixedMul(thrust, finecosine[ang]);
        target->momy += FixedMul(thrust, finesine[ang]);
    }

    //
    // player specific
    //
    if (player)
    {
        savedPercent = AutoArmorSave[player->pclass]
            + player->armorpoints[ARMOR_ARMOR] +
            player->armorpoints[ARMOR_SHIELD] +
            player->armorpoints[ARMOR_HELMET] +
            player->armorpoints[ARMOR_AMULET];
        if (savedPercent)
        {                       // armor absorbed some damage
            if (savedPercent > 100 * FRACUNIT)
            {
                savedPercent = 100 * FRACUNIT;
            }
            for (i = 0; i < NUMARMOR; i++)
            {
                if (player->armorpoints[i])
                {
                    player->armorpoints[i] -=
                        FixedDiv(FixedMul(damage << FRACBITS,
                                          ArmorIncrement[player->pclass][i]),
                                 300 * FRACUNIT);
                    if (player->armorpoints[i] < 2 * FRACUNIT)
                    {
                        player->armorpoints[i] = 0;
                    }
                }
            }
            saved = FixedDiv(FixedMul(damage << FRACBITS, savedPercent),
                             100 * FRACUNIT);
            if (saved > savedPercent * 2)
            {
                saved = savedPercent * 2;
            }
            damage -= saved >> FRACBITS;
        }
        if (damage >= player->health
            && ((gameskill == sk_baby) || deathmatch) && !player->morphTics)
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
            // HEXEN_TODO: SB_PaletteFlash
            // SB_PaletteFlash(false);
        }
    }

    //
    // do the damage
    //
    target->health -= damage;
    if (target->health <= 0)
    {                           // Death
        if (inflictor)
        {                       // check for special fire damage or ice damage deaths
            if (inflictor->flags2 & MF2_FIREDAMAGE)
            {
                if (player && !player->morphTics)
                {               // Check for flame death
                    if (target->health > -50 && damage > 25)
                    {
                        target->flags2 |= MF2_FIREDAMAGE;
                    }
                }
                else
                {
                    target->flags2 |= MF2_FIREDAMAGE;
                }
            }
            else if (inflictor->flags2 & MF2_ICEDAMAGE)
            {
                target->flags2 |= MF2_ICEDAMAGE;
            }
        }
        if (source && (source->type == HEXEN_MT_MINOTAUR))
        {                       // Minotaur's kills go to his master
            master = source->special1.m;
            // Make sure still alive and not a pointer to fighter head
            if (master->player && (master->player->mo == master))
            {
                source = master;
            }
        }
        if (source && (source->player) &&
            (source->player->readyweapon == wp_fourth))
        {
            // Always extreme death from fourth weapon
            target->health = -5000;
        }
        P_KillMobj(source, target);
        return;
    }
    if ((P_Random(pr_hexen) < target->info->painchance)
        && !(target->flags & MF_SKULLFLY))
    {
        if (inflictor && (inflictor->type >= HEXEN_MT_LIGHTNING_FLOOR
                          && inflictor->type <= HEXEN_MT_LIGHTNING_ZAP))
        {
            if (P_Random(pr_hexen) < 96)
            {
                target->flags |= MF_JUSTHIT;    // fight back!
                P_SetMobjState(target, target->info->painstate);
            }
            else
            {                   // "electrocute" the target
                target->frame |= FF_FULLBRIGHT;
                if (target->flags & MF_COUNTKILL && P_Random(pr_hexen) < 128
                    && !S_GetSoundPlayingInfo(target, hexen_sfx_puppybeat))
                {
                    if ((target->type == HEXEN_MT_CENTAUR) ||
                        (target->type == HEXEN_MT_CENTAURLEADER) ||
                        (target->type == HEXEN_MT_ETTIN))
                    {
                        S_StartSound(target, hexen_sfx_puppybeat);
                    }
                }
            }
        }
        else
        {
            target->flags |= MF_JUSTHIT;        // fight back!
            P_SetMobjState(target, target->info->painstate);
            if (inflictor && inflictor->type == HEXEN_MT_POISONCLOUD)
            {
                if (target->flags & MF_COUNTKILL && P_Random(pr_hexen) < 128
                    && !S_GetSoundPlayingInfo(target, hexen_sfx_puppybeat))
                {
                    if ((target->type == HEXEN_MT_CENTAUR) ||
                        (target->type == HEXEN_MT_CENTAURLEADER) ||
                        (target->type == HEXEN_MT_ETTIN))
                    {
                        S_StartSound(target, hexen_sfx_puppybeat);
                    }
                }
            }
        }
    }
    target->reactiontime = 0;   // we're awake now...
    if (!target->threshold && source && !(source->flags2 & MF2_BOSS)
        && !(target->type == HEXEN_MT_BISHOP) && !(target->type == HEXEN_MT_MINOTAUR))
    {
        // Target actor is not intent on another actor,
        // so make him chase after source
        if ((target->type == HEXEN_MT_CENTAUR && source->type == HEXEN_MT_CENTAURLEADER)
            || (target->type == HEXEN_MT_CENTAURLEADER
                && source->type == HEXEN_MT_CENTAUR))
        {
            return;
        }
        target->target = source;
        target->threshold = BASETHRESHOLD;
        if (target->state == &states[target->info->spawnstate]
            && target->info->seestate != HEXEN_S_NULL)
        {
            P_SetMobjState(target, target->info->seestate);
        }
    }
}

//==========================================================================
//
// P_PoisonPlayer - Sets up all data concerning poisoning
//
//==========================================================================

void P_PoisonPlayer(player_t * player, mobj_t * poisoner, int poison)
{
    if ((player->cheats & CF_GODMODE) || player->powers[pw_invulnerability])
    {
        return;
    }
    player->poisoncount += poison;
    player->poisoner = poisoner;
    if (player->poisoncount > 100)
    {
        player->poisoncount = 100;
    }
}
