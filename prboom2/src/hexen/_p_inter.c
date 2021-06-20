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
    thrust = 16 * FRACUNIT + (P_Random(pr_hexen) << 10);
    target->momx += FixedMul(thrust, finecosine[angle]);
    target->momy += FixedMul(thrust, finesine[angle]);
    P_DamageMobj(target, NULL, source, HITDICE(4));
    if (target->player)
    {
        target->reactiontime = 14 + (P_Random(pr_hexen) & 7);
    }
    source->args[0] = 0;        // Stop charging
}


//---------------------------------------------------------------------------
//
// FUNC P_MorphPlayer
//
// Returns true if the player gets turned into a pig
//
//---------------------------------------------------------------------------

dboolean P_MorphPlayer(player_t * player)
{
    mobj_t *pmo;
    mobj_t *fog;
    mobj_t *beastMo;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    angle_t angle;
    int oldFlags2;

    if (player->powers[pw_invulnerability])
    {                           // Immune when invulnerable
        return (false);
    }
    if (player->morphTics)
    {                           // Player is already a beast
        return false;
    }
    pmo = player->mo;
    x = pmo->x;
    y = pmo->y;
    z = pmo->z;
    angle = pmo->angle;
    oldFlags2 = pmo->flags2;
    P_SetMobjState(pmo, HEXEN_S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HEXEN_MT_TFOG);
    S_StartSound(fog, hexen_sfx_teleport);
    beastMo = P_SpawnMobj(x, y, z, HEXEN_MT_PIGPLAYER);
    beastMo->special1.i = player->readyweapon;
    beastMo->angle = angle;
    beastMo->player = player;
    player->health = beastMo->health = MAXMORPHHEALTH;
    player->mo = beastMo;
    memset(&player->armorpoints[0], 0, NUMARMOR * sizeof(int));
    player->pclass = PCLASS_PIG;
    if (oldFlags2 & MF2_FLY)
    {
        beastMo->flags2 |= MF2_FLY;
    }
    player->morphTics = MORPHTICS;
    P_ActivateMorphWeapon(player);
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_MorphMonster
//
//---------------------------------------------------------------------------

dboolean P_MorphMonster(mobj_t * actor)
{
    mobj_t *master, *monster, *fog;
    mobjtype_t moType;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    mobj_t oldMonster;

    if (actor->player)
        return (false);
    if (!(actor->flags & MF_COUNTKILL))
        return false;
    if (actor->flags2 & MF2_BOSS)
        return false;
    moType = actor->type;
    switch (moType)
    {
        case HEXEN_MT_PIG:
            return (false);
        case HEXEN_MT_FIGHTER_BOSS:
        case HEXEN_MT_CLERIC_BOSS:
        case HEXEN_MT_MAGE_BOSS:
            return (false);
        default:
            break;
    }

    oldMonster = *actor;
    x = oldMonster.x;
    y = oldMonster.y;
    z = oldMonster.z;
    P_RemoveMobjFromTIDList(actor);
    P_SetMobjState(actor, HEXEN_S_FREETARGMOBJ);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HEXEN_MT_TFOG);
    S_StartSound(fog, hexen_sfx_teleport);
    monster = P_SpawnMobj(x, y, z, HEXEN_MT_PIG);
    monster->special2.i = moType;
    monster->special1.i = MORPHTICS + P_Random(pr_hexen);
    monster->flags |= (oldMonster.flags & MF_SHADOW);
    monster->target = oldMonster.target;
    monster->angle = oldMonster.angle;
    monster->tid = oldMonster.tid;
    monster->special = oldMonster.special;
    P_InsertMobjIntoTIDList(monster, oldMonster.tid);
    memcpy(monster->args, oldMonster.args, 5);

    // check for turning off minotaur power for active icon
    if (moType == HEXEN_MT_MINOTAUR)
    {
        master = oldMonster.special1.m;
        if (master->health > 0)
        {
            if (!ActiveMinotaur(master->player))
            {
                master->player->powers[pw_minotaur] = 0;
            }
        }
    }
    return (true);
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
    int normalSlot = 0;
    int superCount;
    int superSlot = 0;

    normalCount = superCount = 0;
    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == hexen_arti_health)
        {
            normalSlot = i;
            normalCount = player->inventory[i].count;
        }
        else if (player->inventory[i].type == hexen_arti_superhealth)
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
