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
// PROC P_PlayerNextArtifact
//
//----------------------------------------------------------------------------

void P_PlayerNextArtifact(player_t * player)
{
    if (player == &players[consoleplayer])
    {
        inv_ptr--;
        if (inv_ptr < 6)
        {
            curpos--;
            if (curpos < 0)
            {
                curpos = 0;
            }
        }
        if (inv_ptr < 0)
        {
            inv_ptr = player->inventorySlotNum - 1;
            if (inv_ptr < 6)
            {
                curpos = inv_ptr;
            }
            else
            {
                curpos = 6;
            }
        }
        player->readyArtifact = player->inventory[inv_ptr].type;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerRemoveArtifact
//
//----------------------------------------------------------------------------

void P_PlayerRemoveArtifact(player_t * player, int slot)
{
    int i;

    player->artifactCount--;
    if (!(--player->inventory[slot].count))
    {                           // Used last of a type - compact the artifact list
        player->readyArtifact = arti_none;
        player->inventory[slot].type = arti_none;
        for (i = slot + 1; i < player->inventorySlotNum; i++)
        {
            player->inventory[i - 1] = player->inventory[i];
        }
        player->inventorySlotNum--;
        if (player == &players[consoleplayer])
        {                       // Set position markers and get next readyArtifact
            inv_ptr--;
            if (inv_ptr < 6)
            {
                curpos--;
                if (curpos < 0)
                {
                    curpos = 0;
                }
            }
            if (inv_ptr >= player->inventorySlotNum)
            {
                inv_ptr = player->inventorySlotNum - 1;
            }
            if (inv_ptr < 0)
            {
                inv_ptr = 0;
            }
            player->readyArtifact = player->inventory[inv_ptr].type;
        }
    }
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerUseArtifact
//
//----------------------------------------------------------------------------

void P_PlayerUseArtifact(player_t * player, artitype_t arti)
{
    int i;

    for (i = 0; i < player->inventorySlotNum; i++)
    {
        if (player->inventory[i].type == arti)
        {                       // Found match - try to use
            if (P_UseArtifact(player, arti))
            {                   // Artifact was used - remove it from inventory
                P_PlayerRemoveArtifact(player, i);
                if (player == &players[consoleplayer])
                {
                    if (arti < arti_firstpuzzitem)
                    {
                        S_StartSound(NULL, hexen_sfx_artifact_use);
                    }
                    else
                    {
                        S_StartSound(NULL, hexen_sfx_puzzle_success);
                    }
                    ArtifactFlash = 4;
                }
            }
            else if (arti < arti_firstpuzzitem)
            {                   // Unable to use artifact, advance pointer
                P_PlayerNextArtifact(player);
            }
            break;
        }
    }
}

//==========================================================================
//
// P_UseArtifact
//
// Returns true if the artifact was used.
//
//==========================================================================

dboolean P_UseArtifact(player_t * player, artitype_t arti)
{
    mobj_t *mo;
    angle_t angle;
    int i;
    int count;

    switch (arti)
    {
        case arti_invulnerability:
            if (!P_GivePower(player, pw_invulnerability))
            {
                return (false);
            }
            break;
        case arti_health:
            if (!P_GiveBody(player, 25))
            {
                return (false);
            }
            break;
        case arti_superhealth:
            if (!P_GiveBody(player, 100))
            {
                return (false);
            }
            break;
        case arti_healingradius:
            if (!P_HealRadius(player))
            {
                return (false);
            }
            break;
        case arti_torch:
            if (!P_GivePower(player, pw_infrared))
            {
                return (false);
            }
            break;
        case arti_egg:
            mo = player->mo;
            P_SpawnPlayerMissile(mo, HEXEN_MT_EGGFX);
            P_SPMAngle(mo, HEXEN_MT_EGGFX, mo->angle - (ANG45 / 6));
            P_SPMAngle(mo, HEXEN_MT_EGGFX, mo->angle + (ANG45 / 6));
            P_SPMAngle(mo, HEXEN_MT_EGGFX, mo->angle - (ANG45 / 3));
            P_SPMAngle(mo, HEXEN_MT_EGGFX, mo->angle + (ANG45 / 3));
            break;
        case arti_fly:
            if (!P_GivePower(player, pw_flight))
            {
                return (false);
            }
            if (player->mo->momz <= -35 * FRACUNIT)
            {                   // stop falling scream
                S_StopSound(player->mo);
            }
            break;
        case arti_summon:
            mo = P_SpawnPlayerMissile(player->mo, HEXEN_MT_SUMMON_FX);
            if (mo)
            {
                mo->target = player->mo;
                mo->special1.m = (player->mo);
                mo->momz = 5 * FRACUNIT;
            }
            break;
        case arti_teleport:
            P_ArtiTele(player);
            break;
        case arti_teleportother:
            P_ArtiTeleportOther(player);
            break;
        case arti_poisonbag:
            angle = player->mo->angle >> ANGLETOFINESHIFT;
            if (player->pclass == PCLASS_CLERIC)
            {
                mo = P_SpawnMobj(player->mo->x + 16 * finecosine[angle],
                                 player->mo->y + 24 * finesine[angle],
                                 player->mo->z - player->mo->floorclip +
                                 8 * FRACUNIT, HEXEN_MT_POISONBAG);
                if (mo)
                {
                    mo->target = player->mo;
                }
            }
            else if (player->pclass == PCLASS_MAGE)
            {
                mo = P_SpawnMobj(player->mo->x + 16 * finecosine[angle],
                                 player->mo->y + 24 * finesine[angle],
                                 player->mo->z - player->mo->floorclip +
                                 8 * FRACUNIT, HEXEN_MT_FIREBOMB);
                if (mo)
                {
                    mo->target = player->mo;
                }
            }
            else                // PCLASS_FIGHTER, obviously (also pig, not so obviously)
            {
                mo = P_SpawnMobj(player->mo->x, player->mo->y,
                                 player->mo->z - player->mo->floorclip +
                                 35 * FRACUNIT, HEXEN_MT_THROWINGBOMB);
                if (mo)
                {
                    mo->angle =
                        player->mo->angle + (((P_Random(pr_hexen) & 7) - 4) << 24);
                    mo->momz =
                        4 * FRACUNIT + ((player->lookdir) << (FRACBITS - 4));
                    mo->z += player->lookdir << (FRACBITS - 4);
                    P_ThrustMobj(mo, mo->angle, mo->info->speed);
                    mo->momx += player->mo->momx >> 1;
                    mo->momy += player->mo->momy >> 1;
                    mo->target = player->mo;
                    mo->tics -= P_Random(pr_hexen) & 3;
                    P_CheckMissileSpawn(mo);
                }
            }
            break;
        case arti_speed:
            if (!P_GivePower(player, pw_speed))
            {
                return (false);
            }
            break;
        case arti_boostmana:
            if (!P_GiveMana(player, MANA_1, MAX_MANA))
            {
                if (!P_GiveMana(player, MANA_2, MAX_MANA))
                {
                    return false;
                }

            }
            else
            {
                P_GiveMana(player, MANA_2, MAX_MANA);
            }
            break;
        case arti_boostarmor:
            count = 0;

            for (i = 0; i < NUMARMOR; i++)
            {
                count += Hexen_P_GiveArmor(player, i, 1);     // 1 point per armor type
            }
            if (!count)
            {
                return false;
            }
            break;
        case arti_blastradius:
            P_BlastRadius(player);
            break;

        case arti_puzzskull:
        case arti_puzzgembig:
        case arti_puzzgemred:
        case arti_puzzgemgreen1:
        case arti_puzzgemgreen2:
        case arti_puzzgemblue1:
        case arti_puzzgemblue2:
        case arti_puzzbook1:
        case arti_puzzbook2:
        case arti_puzzskull2:
        case arti_puzzfweapon:
        case arti_puzzcweapon:
        case arti_puzzmweapon:
        case arti_puzzgear1:
        case arti_puzzgear2:
        case arti_puzzgear3:
        case arti_puzzgear4:
            if (P_UsePuzzleItem(player, arti - arti_firstpuzzitem))
            {
                return true;
            }
            else
            {
                P_SetYellowMessage(player, TXT_USEPUZZLEFAILED, false);
                return false;
            }
            break;
        default:
            return false;
    }
    return true;
}

//============================================================================
//
// A_SpeedFade
//
//============================================================================

void A_SpeedFade(mobj_t * actor)
{
    actor->flags |= MF_SHADOW;
    actor->flags &= ~MF_ALTSHADOW;
    actor->sprite = actor->target->sprite;
}
