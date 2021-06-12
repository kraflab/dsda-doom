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


#include "h2def.h"
#include "m_misc.h"
#include "m_random.h"
#include "i_system.h"
#include "p_local.h"
#include "s_sound.h"

#define BONUSADD 6

int ArmorIncrement[NUMCLASSES][NUMARMOR] = {
    {25 * FRACUNIT, 20 * FRACUNIT, 15 * FRACUNIT, 5 * FRACUNIT},
    {10 * FRACUNIT, 25 * FRACUNIT, 5 * FRACUNIT, 20 * FRACUNIT},
    {5 * FRACUNIT, 15 * FRACUNIT, 10 * FRACUNIT, 25 * FRACUNIT},
    {0, 0, 0, 0}
};

int AutoArmorSave[NUMCLASSES] =
    { 15 * FRACUNIT, 10 * FRACUNIT, 5 * FRACUNIT, 0 };

const char *TextKeyMessages[] = {
    TXT_KEY_STEEL,
    TXT_KEY_CAVE,
    TXT_KEY_AXE,
    TXT_KEY_FIRE,
    TXT_KEY_EMERALD,
    TXT_KEY_DUNGEON,
    TXT_KEY_SILVER,
    TXT_KEY_RUSTED,
    TXT_KEY_HORN,
    TXT_KEY_SWAMP,
    TXT_KEY_CASTLE
};

static void SetDormantArtifact(mobj_t * arti);
static void TryPickupArtifact(player_t * player, artitype_t artifactType,
                              mobj_t * artifact);
static void TryPickupWeapon(player_t * player, pclass_t weaponClass,
                            weapontype_t weaponType, mobj_t * weapon,
                            const char *message);
static void TryPickupWeaponPiece(player_t * player, pclass_t matchClass,
                                 int pieceValue, mobj_t * pieceMobj);

//--------------------------------------------------------------------------
//
// PROC P_SetMessage
//
//--------------------------------------------------------------------------

void P_SetMessage(player_t * player, const char *message, dboolean ultmsg)
{
    if ((player->ultimateMessage || !messageson) && !ultmsg)
    {
        return;
    }

    M_StringCopy(player->message, message, sizeof(player->message));
//    strupr(player->message);
    player->messageTics = MESSAGETICS;
    player->yellowMessage = false;
    if (ultmsg)
    {
        player->ultimateMessage = true;
    }
    if (player == &players[consoleplayer])
    {
        BorderTopRefresh = true;
    }
}

//==========================================================================
//
// P_SetYellowMessage
//
//==========================================================================

void P_SetYellowMessage(player_t * player, const char *message, dboolean ultmsg)
{
    if ((player->ultimateMessage || !messageson) && !ultmsg)
    {
        return;
    }
    M_StringCopy(player->message, message, sizeof(player->message));
    player->messageTics = 5 * MESSAGETICS;      // Bold messages last longer
    player->yellowMessage = true;
    if (ultmsg)
    {
        player->ultimateMessage = true;
    }
    if (player == &players[consoleplayer])
    {
        BorderTopRefresh = true;
    }
}

//==========================================================================
//
// P_ClearMessage
//
//==========================================================================

void P_ClearMessage(player_t * player)
{
    player->messageTics = 0;
    if (player == &players[consoleplayer])
    {
        BorderTopRefresh = true;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_HideSpecialThing
//
//----------------------------------------------------------------------------

void P_HideSpecialThing(mobj_t * thing)
{
    thing->flags &= ~MF_SPECIAL;
    thing->flags2 |= MF2_DONTDRAW;
    P_SetMobjState(thing, HEXEN_S_HIDESPECIAL1);
}

//--------------------------------------------------------------------------
//
// FUNC P_GiveMana
//
// Returns true if the player accepted the mana, false if it was
// refused (player has MAX_MANA).
//
//--------------------------------------------------------------------------

dboolean P_GiveMana(player_t * player, manatype_t mana, int count)
{
    int prevMana;
    //weapontype_t changeWeapon;

    if (mana == MANA_NONE || mana == MANA_BOTH)
    {
        return (false);
    }
    if ((unsigned int) mana > NUMMANA)
    {
        I_Error("P_GiveMana: bad type %i", mana);
    }
    if (player->mana[mana] == MAX_MANA)
    {
        return (false);
    }
    if (gameskill == sk_baby || gameskill == sk_nightmare)
    {                           // extra mana in baby mode and nightmare mode
        count += count >> 1;
    }
    prevMana = player->mana[mana];

    player->mana[mana] += count;
    if (player->mana[mana] > MAX_MANA)
    {
        player->mana[mana] = MAX_MANA;
    }
    if (player->class == PCLASS_FIGHTER && player->readyweapon == WP_SECOND
        && mana == MANA_1 && prevMana <= 0)
    {
        P_SetPsprite(player, ps_weapon, HEXEN_S_FAXEREADY_G);
    }
    return (true);
}

//==========================================================================
//
// TryPickupWeapon
//
//==========================================================================

static void TryPickupWeapon(player_t * player, pclass_t weaponClass,
                            weapontype_t weaponType, mobj_t * weapon,
                            const char *message)
{
    dboolean remove;
    dboolean gaveMana;
    dboolean gaveWeapon;

    remove = true;
    if (player->class != weaponClass)
    {                           // Wrong class, but try to pick up for mana
        if (netgame && !deathmatch)
        {                       // Can't pick up weapons for other classes in coop netplay
            return;
        }
        if (weaponType == WP_SECOND)
        {
            if (!P_GiveMana(player, MANA_1, 25))
            {
                return;
            }
        }
        else
        {
            if (!P_GiveMana(player, MANA_2, 25))
            {
                return;
            }
        }
    }
    else if (netgame && !deathmatch)
    {                           // Cooperative net-game
        if (player->weaponowned[weaponType])
        {
            return;
        }
        player->weaponowned[weaponType] = true;
        if (weaponType == WP_SECOND)
        {
            P_GiveMana(player, MANA_1, 25);
        }
        else
        {
            P_GiveMana(player, MANA_2, 25);
        }
        player->pendingweapon = weaponType;
        remove = false;
    }
    else
    {                           // Deathmatch or single player game
        if (weaponType == WP_SECOND)
        {
            gaveMana = P_GiveMana(player, MANA_1, 25);
        }
        else
        {
            gaveMana = P_GiveMana(player, MANA_2, 25);
        }
        if (player->weaponowned[weaponType])
        {
            gaveWeapon = false;
        }
        else
        {
            gaveWeapon = true;
            player->weaponowned[weaponType] = true;
            if (weaponType > player->readyweapon)
            {                   // Only switch to more powerful weapons
                player->pendingweapon = weaponType;
            }
        }
        if (!(gaveWeapon || gaveMana))
        {                       // Player didn't need the weapon or any mana
            return;
        }
    }

    P_SetMessage(player, message, false);
    if (weapon->special)
    {
        P_ExecuteLineSpecial(weapon->special, weapon->args,
                             NULL, 0, player->mo);
        weapon->special = 0;
    }

    if (remove)
    {
        if (deathmatch && !(weapon->flags & MF_DROPPED))
        {
            P_HideSpecialThing(weapon);
        }
        else
        {
            P_RemoveMobj(weapon);
        }
    }

    player->bonuscount += BONUSADD;
    if (player == &players[consoleplayer])
    {
        S_StartSound(NULL, hexen_sfx_pickup_weapon);
        SB_PaletteFlash(false);
    }
}

//--------------------------------------------------------------------------
//
// FUNC P_GiveWeapon
//
// Returns true if the weapon or its mana was accepted.
//
//--------------------------------------------------------------------------

/*
dboolean P_GiveWeapon(player_t *player, pclass_t class, weapontype_t weapon)
{
	dboolean gaveMana;
	dboolean gaveWeapon;

	if(player->class != class)
	{ // player cannot use this weapon, take it anyway, and get mana
		if(netgame && !deathmatch)
		{ // Can't pick up weapons for other classes in coop netplay
			return false;
		}
		if(weapon == WP_SECOND)
		{
			return P_GiveMana(player, MANA_1, 25);
		}
		else
		{
			return P_GiveMana(player, MANA_2, 25);
		}
	}
	if(netgame && !deathmatch)
	{ // Cooperative net-game
		if(player->weaponowned[weapon])
		{
			return(false);
		}
		player->bonuscount += BONUSADD;
		player->weaponowned[weapon] = true;
		if(weapon == WP_SECOND)
		{
			P_GiveMana(player, MANA_1, 25);
		}
		else
		{
			P_GiveMana(player, MANA_2, 25);
		}
		player->pendingweapon = weapon;
		if(player == &players[consoleplayer])
		{
			S_StartSound(NULL, hexen_sfx_pickup_weapon);
		}
		return(false);
	}
	if(weapon == WP_SECOND)
	{
		gaveMana = P_GiveMana(player, MANA_1, 25);
	}
	else
	{
		gaveMana = P_GiveMana(player, MANA_2, 25);
	}
	if(player->weaponowned[weapon])
	{
		gaveWeapon = false;
	}
	else
	{
		gaveWeapon = true;
		player->weaponowned[weapon] = true;
		if(weapon > player->readyweapon)
		{ // Only switch to more powerful weapons
			player->pendingweapon = weapon;
		}
	}
	return(gaveWeapon || gaveMana);
}
*/

//===========================================================================
//
// P_GiveWeaponPiece
//
//===========================================================================

/*
dboolean P_GiveWeaponPiece(player_t *player, pclass_t class, int piece)
{
	P_GiveMana(player, MANA_1, 20);
	P_GiveMana(player, MANA_2, 20);
	if(player->class != class)
	{
		return true;
	}
	else if(player->pieces&piece)
	{ // player already has that weapon piece
		return true;
	}
	player->pieces |= piece;
	if(player->pieces == 7)
	{ // player has built the fourth weapon!
		P_GiveWeapon(player, class, WP_FOURTH);
		S_StartSound(player->mo, hexen_sfx_weapon_build);
	}
	return true;
}
*/

//==========================================================================
//
// TryPickupWeaponPiece
//
//==========================================================================

static void TryPickupWeaponPiece(player_t * player, pclass_t matchClass,
                                 int pieceValue, mobj_t * pieceMobj)
{
    dboolean remove;
    dboolean checkAssembled;
    dboolean gaveWeapon;
    int gaveMana;
    static const char *fourthWeaponText[] = {
        TXT_WEAPON_F4,
        TXT_WEAPON_C4,
        TXT_WEAPON_M4
    };
    static const char *weaponPieceText[] = {
        TXT_QUIETUS_PIECE,
        TXT_WRAITHVERGE_PIECE,
        TXT_BLOODSCOURGE_PIECE
    };
    static int pieceValueTrans[] = {
        0,                      // 0: never
        WPIECE1 | WPIECE2 | WPIECE3,    // WPIECE1 (1)
        WPIECE2 | WPIECE3,      // WPIECE2 (2)
        0,                      // 3: never
        WPIECE3                 // WPIECE3 (4)
    };

    remove = true;
    checkAssembled = true;
    gaveWeapon = false;
    if (player->class != matchClass)
    {                           // Wrong class, but try to pick up for mana
        if (netgame && !deathmatch)
        {                       // Can't pick up wrong-class weapons in coop netplay
            return;
        }
        checkAssembled = false;
        gaveMana = P_GiveMana(player, MANA_1, 20) +
            P_GiveMana(player, MANA_2, 20);
        if (!gaveMana)
        {                       // Didn't need the mana, so don't pick it up
            return;
        }
    }
    else if (netgame && !deathmatch)
    {                           // Cooperative net-game
        if (player->pieces & pieceValue)
        {                       // Already has the piece
            return;
        }
        pieceValue = pieceValueTrans[pieceValue];
        P_GiveMana(player, MANA_1, 20);
        P_GiveMana(player, MANA_2, 20);
        remove = false;
    }
    else
    {                           // Deathmatch or single player game
        gaveMana = P_GiveMana(player, MANA_1, 20) +
            P_GiveMana(player, MANA_2, 20);
        if (player->pieces & pieceValue)
        {                       // Already has the piece, check if mana needed
            if (!gaveMana)
            {                   // Didn't need the mana, so don't pick it up
                return;
            }
            checkAssembled = false;
        }
    }

    // Pick up the weapon piece
    if (pieceMobj->special)
    {
        P_ExecuteLineSpecial(pieceMobj->special, pieceMobj->args,
                             NULL, 0, player->mo);
        pieceMobj->special = 0;
    }
    if (remove)
    {
        if (deathmatch && !(pieceMobj->flags & MF_DROPPED))
        {
            P_HideSpecialThing(pieceMobj);
        }
        else
        {
            P_RemoveMobj(pieceMobj);
        }
    }
    player->bonuscount += BONUSADD;
    if (player == &players[consoleplayer])
    {
        SB_PaletteFlash(false);
    }

    // Check if fourth weapon assembled
    if (checkAssembled)
    {
        player->pieces |= pieceValue;
        if (player->pieces == (WPIECE1 | WPIECE2 | WPIECE3))
        {
            gaveWeapon = true;
            player->weaponowned[WP_FOURTH] = true;
            player->pendingweapon = WP_FOURTH;
        }
    }

    if (gaveWeapon)
    {
        P_SetMessage(player, fourthWeaponText[matchClass], false);
        // Play the build-sound full volume for all players
        S_StartSound(NULL, hexen_sfx_weapon_build);
    }
    else
    {
        P_SetMessage(player, weaponPieceText[matchClass], false);
        if (player == &players[consoleplayer])
        {
            S_StartSound(NULL, hexen_sfx_pickup_weapon);
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC P_GiveBody
//
// Returns false if the body isn't needed at all.
//
//---------------------------------------------------------------------------

dboolean P_GiveBody(player_t * player, int num)
{
    int max;

    max = MAXHEALTH;
    if (player->morphTics)
    {
        max = MAXMORPHHEALTH;
    }
    if (player->health >= max)
    {
        return (false);
    }
    player->health += num;
    if (player->health > max)
    {
        player->health = max;
    }
    player->mo->health = player->health;
    return (true);
}

//---------------------------------------------------------------------------
//
// FUNC P_GiveArmor
//
// Returns false if the armor is worse than the current armor.
//
//---------------------------------------------------------------------------

dboolean P_GiveArmor(player_t * player, armortype_t armortype, int amount)
{
    int hits;
    int totalArmor;

    extern int ArmorMax[NUMCLASSES];

    if (amount == -1)
    {
        hits = ArmorIncrement[player->class][armortype];
        if (player->armorpoints[armortype] >= hits)
        {
            return false;
        }
        else
        {
            player->armorpoints[armortype] = hits;
        }
    }
    else
    {
        hits = amount * 5 * FRACUNIT;
        totalArmor = player->armorpoints[ARMOR_ARMOR]
            + player->armorpoints[ARMOR_SHIELD]
            + player->armorpoints[ARMOR_HELMET]
            + player->armorpoints[ARMOR_AMULET]
            + AutoArmorSave[player->class];
        if (totalArmor < ArmorMax[player->class] * 5 * FRACUNIT)
        {
            player->armorpoints[armortype] += hits;
        }
        else
        {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------
//
// PROC P_GiveKey
//
//---------------------------------------------------------------------------

int P_GiveKey(player_t * player, keytype_t key)
{
    if (player->cards[key])
    {
        return false;
    }
    player->bonuscount += BONUSADD;
    player->cards[key] = true;
    return true;
}

//---------------------------------------------------------------------------
//
// FUNC P_GivePower
//
// Returns true if power accepted.
//
//---------------------------------------------------------------------------

dboolean P_GivePower(player_t * player, powertype_t power)
{
    if (power == pw_invulnerability)
    {
        if (player->powers[power] > BLINKTHRESHOLD)
        {                       // Already have it
            return (false);
        }
        player->powers[power] = INVULNTICS;
        player->mo->flags2 |= MF2_INVULNERABLE;
        if (player->class == PCLASS_MAGE)
        {
            player->mo->flags2 |= MF2_REFLECTIVE;
        }
        return (true);
    }
    if (power == pw_flight)
    {
        if (player->powers[power] > BLINKTHRESHOLD)
        {                       // Already have it
            return (false);
        }
        player->powers[power] = FLIGHTTICS;
        player->mo->flags2 |= MF2_FLY;
        player->mo->flags |= MF_NOGRAVITY;
        if (player->mo->z <= player->mo->floorz)
        {
            player->flyheight = 10;     // thrust the player in the air a bit
        }
        return (true);
    }
    if (power == pw_infrared)
    {
        if (player->powers[power] > BLINKTHRESHOLD)
        {                       // Already have it
            return (false);
        }
        player->powers[power] = INFRATICS;
        return (true);
    }
    if (power == pw_speed)
    {
        if (player->powers[power] > BLINKTHRESHOLD)
        {                       // Already have it
            return (false);
        }
        player->powers[power] = SPEEDTICS;
        return (true);
    }
    if (power == pw_minotaur)
    {
        // Doesn't matter if already have power, renew ticker
        player->powers[power] = MAULATORTICS;
        return (true);
    }
/*
	if(power == pw_ironfeet)
	{
		player->powers[power] = IRONTICS;
		return(true);
	}
	if(power == pw_strength)
	{
		P_GiveBody(player, 100);
		player->powers[power] = 1;
		return(true);
	}
*/
    if (player->powers[power])
    {
        return (false);         // already got it
    }
    player->powers[power] = 1;
    return (true);
}

//==========================================================================
//
// TryPickupArtifact
//
//==========================================================================

static void TryPickupArtifact(player_t * player, artitype_t artifactType,
                              mobj_t * artifact)
{
    static const char *artifactMessages[NUMARTIFACTS] = {
        NULL,
        TXT_ARTIINVULNERABILITY,
        TXT_ARTIHEALTH,
        TXT_ARTISUPERHEALTH,
        TXT_ARTIHEALINGRADIUS,
        TXT_ARTISUMMON,
        TXT_ARTITORCH,
        TXT_ARTIEGG,
        TXT_ARTIFLY,
        TXT_ARTIBLASTRADIUS,
        TXT_ARTIPOISONBAG,
        TXT_ARTITELEPORTOTHER,
        TXT_ARTISPEED,
        TXT_ARTIBOOSTMANA,
        TXT_ARTIBOOSTARMOR,
        TXT_ARTITELEPORT,
        TXT_ARTIPUZZSKULL,
        TXT_ARTIPUZZGEMBIG,
        TXT_ARTIPUZZGEMRED,
        TXT_ARTIPUZZGEMGREEN1,
        TXT_ARTIPUZZGEMGREEN2,
        TXT_ARTIPUZZGEMBLUE1,
        TXT_ARTIPUZZGEMBLUE2,
        TXT_ARTIPUZZBOOK1,
        TXT_ARTIPUZZBOOK2,
        TXT_ARTIPUZZSKULL2,
        TXT_ARTIPUZZFWEAPON,
        TXT_ARTIPUZZCWEAPON,
        TXT_ARTIPUZZMWEAPON,
        TXT_ARTIPUZZGEAR,       // All gear pickups use the same text
        TXT_ARTIPUZZGEAR,
        TXT_ARTIPUZZGEAR,
        TXT_ARTIPUZZGEAR
    };

    if (gamemode == shareware)
    {
        artifactMessages[arti_blastradius] = TXT_ARTITELEPORT;
        artifactMessages[arti_teleport] = TXT_ARTIBLASTRADIUS;
    }

    if (P_GiveArtifact(player, artifactType, artifact))
    {
        if (artifact->special)
        {
            P_ExecuteLineSpecial(artifact->special, artifact->args,
                                 NULL, 0, NULL);
            artifact->special = 0;
        }
        player->bonuscount += BONUSADD;
        if (artifactType < arti_firstpuzzitem)
        {
            SetDormantArtifact(artifact);
            S_StartSound(artifact, hexen_sfx_pickup_artifact);
            P_SetMessage(player, artifactMessages[artifactType], false);
        }
        else
        {                       // Puzzle item
            S_StartSound(NULL, hexen_sfx_pickup_item);
            P_SetMessage(player, artifactMessages[artifactType], true);
            if (!netgame || deathmatch)
            {                   // Remove puzzle items if not cooperative netplay
                P_RemoveMobj(artifact);
            }
        }
    }
}

//---------------------------------------------------------------------------
//
// FUNC P_GiveArtifact
//
// Returns true if artifact accepted.
//
//---------------------------------------------------------------------------

dboolean P_GiveArtifact(player_t * player, artitype_t arti, mobj_t * mo)
{
    int i;
    int j;
    dboolean slidePointer;

    slidePointer = false;
    i = 0;
    while (player->inventory[i].type != arti && i < player->inventorySlotNum)
    {
        i++;
    }
    if (i == player->inventorySlotNum)
    {
        if (arti < arti_firstpuzzitem)
        {
            i = 0;
            while (player->inventory[i].type < arti_firstpuzzitem
                   && i < player->inventorySlotNum)
            {
                i++;
            }
            if (i != player->inventorySlotNum)
            {
                for (j = player->inventorySlotNum; j > i; j--)
                {
                    player->inventory[j].count =
                        player->inventory[j - 1].count;
                    player->inventory[j].type = player->inventory[j - 1].type;
                    slidePointer = true;
                }
            }
        }
        player->inventory[i].count = 1;
        player->inventory[i].type = arti;
        player->inventorySlotNum++;
    }
    else
    {
        if (arti >= arti_firstpuzzitem && netgame && !deathmatch)
        {                       // Can't carry more than 1 puzzle item in coop netplay
            return false;
        }
        if (player->inventory[i].count >= 25)
        {                       // Player already has 25 of this item
            return false;
        }
        player->inventory[i].count++;
    }
    if (!player->artifactCount)
    {
        player->readyArtifact = arti;
    }
    else if (player == &players[consoleplayer] && slidePointer
             && i <= inv_ptr)
    {
        inv_ptr++;
        curpos++;
        if (curpos > 6)
        {
            curpos = 6;
        }
    }
    player->artifactCount++;
    return (true);
}

//==========================================================================
//
// SetDormantArtifact
//
// Removes the MF_SPECIAL flag and initiates the artifact pickup
// animation.
//
//==========================================================================

static void SetDormantArtifact(mobj_t * arti)
{
    arti->flags &= ~MF_SPECIAL;
    if (deathmatch && !(arti->flags & MF_DROPPED))
    {
        if (arti->type == HEXEN_MT_ARTIINVULNERABILITY)
        {
            P_SetMobjState(arti, HEXEN_S_DORMANTARTI3_1);
        }
        else if (arti->type == HEXEN_MT_SUMMONMAULATOR || arti->type == HEXEN_MT_ARTIFLY)
        {
            P_SetMobjState(arti, HEXEN_S_DORMANTARTI2_1);
        }
        else
        {
            P_SetMobjState(arti, HEXEN_S_DORMANTARTI1_1);
        }
    }
    else
    {                           // Don't respawn
        P_SetMobjState(arti, HEXEN_S_DEADARTI1);
    }
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreArtifact
//
//---------------------------------------------------------------------------

void A_RestoreArtifact(mobj_t * arti)
{
    arti->flags |= MF_SPECIAL;
    P_SetMobjState(arti, arti->info->spawnstate);
    S_StartSound(arti, hexen_sfx_respawn);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing1
//
// Make a special thing visible again.
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing1(mobj_t * thing)
{
    thing->flags2 &= ~MF2_DONTDRAW;
    S_StartSound(thing, hexen_sfx_respawn);
}

//---------------------------------------------------------------------------
//
// PROC A_RestoreSpecialThing2
//
//---------------------------------------------------------------------------

void A_RestoreSpecialThing2(mobj_t * thing)
{
    thing->flags |= MF_SPECIAL;
    P_SetMobjState(thing, thing->info->spawnstate);
}

//---------------------------------------------------------------------------
//
// PROC P_TouchSpecialThing
//
//---------------------------------------------------------------------------

void P_TouchSpecialThing(mobj_t * special, mobj_t * toucher)
{
    player_t *player;
    fixed_t delta;
    int sound;
    dboolean respawn;

    delta = special->z - toucher->z;
    if (delta > toucher->height || delta < -32 * FRACUNIT)
    {                           // Out of reach
        return;
    }
    if (toucher->health <= 0)
    {                           // Toucher is dead
        return;
    }
    sound = hexen_sfx_pickup_item;
    player = toucher->player;
    respawn = true;
    switch (special->sprite)
    {
            // Items
        case HEXEN_SPR_PTN1:         // Item_HealingPotion
            if (!P_GiveBody(player, 10))
            {
                return;
            }
            P_SetMessage(player, TXT_ITEMHEALTH, false);
            break;
        case HEXEN_SPR_ARM1:
            if (!P_GiveArmor(player, ARMOR_ARMOR, -1))
            {
                return;
            }
            P_SetMessage(player, TXT_ARMOR1, false);
            break;
        case HEXEN_SPR_ARM2:
            if (!P_GiveArmor(player, ARMOR_SHIELD, -1))
            {
                return;
            }
            P_SetMessage(player, TXT_ARMOR2, false);
            break;
        case HEXEN_SPR_ARM3:
            if (!P_GiveArmor(player, ARMOR_HELMET, -1))
            {
                return;
            }
            P_SetMessage(player, TXT_ARMOR3, false);
            break;
        case HEXEN_SPR_ARM4:
            if (!P_GiveArmor(player, ARMOR_AMULET, -1))
            {
                return;
            }
            P_SetMessage(player, TXT_ARMOR4, false);
            break;

            // Keys
        case HEXEN_SPR_KEY1:
        case HEXEN_SPR_KEY2:
        case HEXEN_SPR_KEY3:
        case HEXEN_SPR_KEY4:
        case HEXEN_SPR_KEY5:
        case HEXEN_SPR_KEY6:
        case HEXEN_SPR_KEY7:
        case HEXEN_SPR_KEY8:
        case HEXEN_SPR_KEY9:
        case HEXEN_SPR_KEYA:
        case HEXEN_SPR_KEYB:
            if (!P_GiveKey(player, special->sprite - HEXEN_SPR_KEY1))
            {
                return;
            }
            P_SetMessage(player, TextKeyMessages[special->sprite - HEXEN_SPR_KEY1],
                         true);
            sound = hexen_sfx_pickup_key;

            // Check and process the special now in case the key doesn't
            // get removed for coop netplay
            if (special->special)
            {
                P_ExecuteLineSpecial(special->special, special->args,
                                     NULL, 0, toucher);
                special->special = 0;
            }

            if (!netgame)
            {                   // Only remove keys in single player game
                break;
            }
            player->bonuscount += BONUSADD;
            if (player == &players[consoleplayer])
            {
                S_StartSound(NULL, sound);
                SB_PaletteFlash(false);
            }
            return;

            // Artifacts
        case HEXEN_SPR_PTN2:
            TryPickupArtifact(player, arti_health, special);
            return;
        case HEXEN_SPR_SOAR:
            TryPickupArtifact(player, arti_fly, special);
            return;
        case HEXEN_SPR_INVU:
            TryPickupArtifact(player, arti_invulnerability, special);
            return;
        case HEXEN_SPR_SUMN:
            TryPickupArtifact(player, arti_summon, special);
            return;
        case HEXEN_SPR_PORK:
            TryPickupArtifact(player, arti_egg, special);
            return;
        case HEXEN_SPR_SPHL:
            TryPickupArtifact(player, arti_superhealth, special);
            return;
        case HEXEN_SPR_HRAD:
            TryPickupArtifact(player, arti_healingradius, special);
            return;
        case HEXEN_SPR_TRCH:
            TryPickupArtifact(player, arti_torch, special);
            return;
        case HEXEN_SPR_ATLP:
            TryPickupArtifact(player, arti_teleport, special);
            return;
        case HEXEN_SPR_TELO:
            TryPickupArtifact(player, arti_teleportother, special);
            return;
        case HEXEN_SPR_PSBG:
            TryPickupArtifact(player, arti_poisonbag, special);
            return;
        case HEXEN_SPR_SPED:
            TryPickupArtifact(player, arti_speed, special);
            return;
        case HEXEN_SPR_BMAN:
            TryPickupArtifact(player, arti_boostmana, special);
            return;
        case HEXEN_SPR_BRAC:
            TryPickupArtifact(player, arti_boostarmor, special);
            return;
        case HEXEN_SPR_BLST:
            TryPickupArtifact(player, arti_blastradius, special);
            return;

            // Puzzle artifacts
        case HEXEN_SPR_ASKU:
            TryPickupArtifact(player, arti_puzzskull, special);
            return;
        case HEXEN_SPR_ABGM:
            TryPickupArtifact(player, arti_puzzgembig, special);
            return;
        case HEXEN_SPR_AGMR:
            TryPickupArtifact(player, arti_puzzgemred, special);
            return;
        case HEXEN_SPR_AGMG:
            TryPickupArtifact(player, arti_puzzgemgreen1, special);
            return;
        case HEXEN_SPR_AGG2:
            TryPickupArtifact(player, arti_puzzgemgreen2, special);
            return;
        case HEXEN_SPR_AGMB:
            TryPickupArtifact(player, arti_puzzgemblue1, special);
            return;
        case HEXEN_SPR_AGB2:
            TryPickupArtifact(player, arti_puzzgemblue2, special);
            return;
        case HEXEN_SPR_ABK1:
            TryPickupArtifact(player, arti_puzzbook1, special);
            return;
        case HEXEN_SPR_ABK2:
            TryPickupArtifact(player, arti_puzzbook2, special);
            return;
        case HEXEN_SPR_ASK2:
            TryPickupArtifact(player, arti_puzzskull2, special);
            return;
        case HEXEN_SPR_AFWP:
            TryPickupArtifact(player, arti_puzzfweapon, special);
            return;
        case HEXEN_SPR_ACWP:
            TryPickupArtifact(player, arti_puzzcweapon, special);
            return;
        case HEXEN_SPR_AMWP:
            TryPickupArtifact(player, arti_puzzmweapon, special);
            return;
        case HEXEN_SPR_AGER:
            TryPickupArtifact(player, arti_puzzgear1, special);
            return;
        case HEXEN_SPR_AGR2:
            TryPickupArtifact(player, arti_puzzgear2, special);
            return;
        case HEXEN_SPR_AGR3:
            TryPickupArtifact(player, arti_puzzgear3, special);
            return;
        case HEXEN_SPR_AGR4:
            TryPickupArtifact(player, arti_puzzgear4, special);
            return;

            // Mana
        case HEXEN_SPR_MAN1:
            if (!P_GiveMana(player, MANA_1, 15))
            {
                return;
            }
            P_SetMessage(player, TXT_MANA_1, false);
            break;
        case HEXEN_SPR_MAN2:
            if (!P_GiveMana(player, MANA_2, 15))
            {
                return;
            }
            P_SetMessage(player, TXT_MANA_2, false);
            break;
        case HEXEN_SPR_MAN3:         // Double Mana Dodecahedron
            if (!P_GiveMana(player, MANA_1, 20))
            {
                if (!P_GiveMana(player, MANA_2, 20))
                {
                    return;
                }
            }
            else
            {
                P_GiveMana(player, MANA_2, 20);
            }
            P_SetMessage(player, TXT_MANA_BOTH, false);
            break;

            // 2nd and 3rd Mage Weapons
        case HEXEN_SPR_WMCS:         // Frost Shards
            TryPickupWeapon(player, PCLASS_MAGE, WP_SECOND,
                            special, TXT_WEAPON_M2);
            return;
        case HEXEN_SPR_WMLG:         // Arc of Death
            TryPickupWeapon(player, PCLASS_MAGE, WP_THIRD,
                            special, TXT_WEAPON_M3);
            return;

            // 2nd and 3rd Fighter Weapons
        case HEXEN_SPR_WFAX:         // Timon's Axe
            TryPickupWeapon(player, PCLASS_FIGHTER, WP_SECOND,
                            special, TXT_WEAPON_F2);
            return;
        case HEXEN_SPR_WFHM:         // Hammer of Retribution
            TryPickupWeapon(player, PCLASS_FIGHTER, WP_THIRD,
                            special, TXT_WEAPON_F3);
            return;

            // 2nd and 3rd Cleric Weapons
        case HEXEN_SPR_WCSS:         // Serpent Staff
            TryPickupWeapon(player, PCLASS_CLERIC, WP_SECOND,
                            special, TXT_WEAPON_C2);
            return;
        case HEXEN_SPR_WCFM:         // Firestorm
            TryPickupWeapon(player, PCLASS_CLERIC, WP_THIRD,
                            special, TXT_WEAPON_C3);
            return;

            // Fourth Weapon Pieces
        case HEXEN_SPR_WFR1:
            TryPickupWeaponPiece(player, PCLASS_FIGHTER, WPIECE1, special);
            return;
        case HEXEN_SPR_WFR2:
            TryPickupWeaponPiece(player, PCLASS_FIGHTER, WPIECE2, special);
            return;
        case HEXEN_SPR_WFR3:
            TryPickupWeaponPiece(player, PCLASS_FIGHTER, WPIECE3, special);
            return;
        case HEXEN_SPR_WCH1:
            TryPickupWeaponPiece(player, PCLASS_CLERIC, WPIECE1, special);
            return;
        case HEXEN_SPR_WCH2:
            TryPickupWeaponPiece(player, PCLASS_CLERIC, WPIECE2, special);
            return;
        case HEXEN_SPR_WCH3:
            TryPickupWeaponPiece(player, PCLASS_CLERIC, WPIECE3, special);
            return;
        case HEXEN_SPR_WMS1:
            TryPickupWeaponPiece(player, PCLASS_MAGE, WPIECE1, special);
            return;
        case HEXEN_SPR_WMS2:
            TryPickupWeaponPiece(player, PCLASS_MAGE, WPIECE2, special);
            return;
        case HEXEN_SPR_WMS3:
            TryPickupWeaponPiece(player, PCLASS_MAGE, WPIECE3, special);
            return;

        default:
            I_Error("P_SpecialThing: Unknown gettable thing");
    }
    if (special->special)
    {
        P_ExecuteLineSpecial(special->special, special->args, NULL,
                             0, toucher);
        special->special = 0;
    }
    if (deathmatch && respawn && !(special->flags & MF_DROPPED))
    {
        P_HideSpecialThing(special);
    }
    else
    {
        P_RemoveMobj(special);
    }
    player->bonuscount += BONUSADD;
    if (player == &players[consoleplayer])
    {
        S_StartSound(NULL, sound);
        SB_PaletteFlash(false);
    }
}

// Search thinker list for minotaur
mobj_t *ActiveMinotaur(player_t * master)
{
    mobj_t *mo;
    player_t *plr;
    thinker_t *think;
    unsigned int *starttime;

    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != P_MobjThinker)
            continue;
        mo = (mobj_t *) think;
        if (mo->type != HEXEN_MT_MINOTAUR)
            continue;
        if (mo->health <= 0)
            continue;
        if (!(mo->flags & MF_COUNTKILL))
            continue;           // for morphed minotaurs
        if (mo->flags & MF_CORPSE)
            continue;
        starttime = (unsigned int *) mo->args;
        if ((leveltime - *starttime) >= MAULATORTICS)
            continue;
        plr = mo->special1.m->player;
        if (plr == master)
            return (mo);
    }
    return (NULL);
}


//---------------------------------------------------------------------------
//
// PROC P_KillMobj
//
//---------------------------------------------------------------------------

void P_KillMobj(mobj_t * source, mobj_t * target)
{
    byte dummyArgs[3] = {0, 0, 0};
    mobj_t *master;

    target->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY | MF_NOGRAVITY);
    target->flags |= MF_CORPSE | MF_DROPOFF;
    target->flags2 &= ~MF2_PASSMOBJ;
    target->height >>= 2;
    if ((target->flags & MF_COUNTKILL || target->type == HEXEN_MT_ZBELL)
        && target->special)
    {                           // Initiate monster death actions
        if (target->type == HEXEN_MT_SORCBOSS)
        {
            P_StartACS(target->special, 0, dummyArgs, target, NULL, 0);
        }
        else
        {
            P_ExecuteLineSpecial(target->special, target->args,
                                 NULL, 0, target);
        }
    }
    if (source && source->player)
    {                           // Check for frag changes
        if (target->player)
        {
            if (target == source)
            {                   // Self-frag
                target->player->frags[target->player - players]--;
                if (cmdfrag && netgame
                    && source->player == &players[consoleplayer])
                {               // Send out a frag count packet
                    NET_SendFrags(source->player);
                }
            }
            else
            {
                source->player->frags[target->player - players]++;
                if (cmdfrag && netgame
                    && source->player == &players[consoleplayer])
                {               // Send out a frag count packet
                    NET_SendFrags(source->player);
                }
            }
        }
    }
    if (target->player)
    {                           // Player death
        if (!source)
        {                       // Self-frag
            target->player->frags[target->player - players]--;
            if (cmdfrag && netgame
                && target->player == &players[consoleplayer])
            {                   // Send out a frag count packet
                NET_SendFrags(target->player);
            }
        }
        target->flags &= ~MF_SOLID;
        target->flags2 &= ~MF2_FLY;
        target->player->powers[pw_flight] = 0;
        target->player->playerstate = PST_DEAD;
        P_DropWeapon(target->player);
        if (target->flags2 & MF2_FIREDAMAGE)
        {                       // Player flame death
            switch (target->player->class)
            {
                case PCLASS_FIGHTER:
                    S_StartSound(target, hexen_sfx_player_fighter_burn_death);
                    P_SetMobjState(target, HEXEN_S_PLAY_F_FDTH1);
                    return;
                case PCLASS_CLERIC:
                    S_StartSound(target, hexen_sfx_player_cleric_burn_death);
                    P_SetMobjState(target, HEXEN_S_PLAY_C_FDTH1);
                    return;
                case PCLASS_MAGE:
                    S_StartSound(target, hexen_sfx_player_mage_burn_death);
                    P_SetMobjState(target, HEXEN_S_PLAY_M_FDTH1);
                    return;
                default:
                    break;
            }
        }
        if (target->flags2 & MF2_ICEDAMAGE)
        {                       // Player ice death
            target->flags &= ~(7 << MF_TRANSSHIFT);     //no translation
            target->flags |= MF_ICECORPSE;
            switch (target->player->class)
            {
                case PCLASS_FIGHTER:
                    P_SetMobjState(target, HEXEN_S_FPLAY_ICE);
                    return;
                case PCLASS_CLERIC:
                    P_SetMobjState(target, HEXEN_S_CPLAY_ICE);
                    return;
                case PCLASS_MAGE:
                    P_SetMobjState(target, HEXEN_S_MPLAY_ICE);
                    return;
                case PCLASS_PIG:
                    P_SetMobjState(target, HEXEN_S_PIG_ICE);
                    return;
                default:
                    break;
            }
        }
    }
    if (target->flags2 & MF2_FIREDAMAGE)
    {
        if (target->type == HEXEN_MT_FIGHTER_BOSS
            || target->type == HEXEN_MT_CLERIC_BOSS || target->type == HEXEN_MT_MAGE_BOSS)
        {
            switch (target->type)
            {
                case HEXEN_MT_FIGHTER_BOSS:
                    S_StartSound(target, hexen_sfx_player_fighter_burn_death);
                    P_SetMobjState(target, HEXEN_S_PLAY_F_FDTH1);
                    return;
                case HEXEN_MT_CLERIC_BOSS:
                    S_StartSound(target, hexen_sfx_player_cleric_burn_death);
                    P_SetMobjState(target, HEXEN_S_PLAY_C_FDTH1);
                    return;
                case HEXEN_MT_MAGE_BOSS:
                    S_StartSound(target, hexen_sfx_player_mage_burn_death);
                    P_SetMobjState(target, HEXEN_S_PLAY_M_FDTH1);
                    return;
                default:
                    break;
            }
        }
        else if (target->type == HEXEN_MT_TREEDESTRUCTIBLE)
        {
            P_SetMobjState(target, HEXEN_S_ZTREEDES_X1);
            target->height = 24 * FRACUNIT;
            S_StartSound(target, hexen_sfx_tree_explode);
            return;
        }
    }
    if (target->flags2 & MF2_ICEDAMAGE)
    {
        target->flags |= MF_ICECORPSE;
        switch (target->type)
        {
            case HEXEN_MT_BISHOP:
                P_SetMobjState(target, HEXEN_S_BISHOP_ICE);
                return;
            case HEXEN_MT_CENTAUR:
            case HEXEN_MT_CENTAURLEADER:
                P_SetMobjState(target, HEXEN_S_CENTAUR_ICE);
                return;
            case HEXEN_MT_DEMON:
            case HEXEN_MT_DEMON2:
                P_SetMobjState(target, HEXEN_S_DEMON_ICE);
                return;
            case HEXEN_MT_SERPENT:
            case HEXEN_MT_SERPENTLEADER:
                P_SetMobjState(target, HEXEN_S_SERPENT_ICE);
                return;
            case HEXEN_MT_WRAITH:
            case HEXEN_MT_WRAITHB:
                P_SetMobjState(target, HEXEN_S_WRAITH_ICE);
                return;
            case HEXEN_MT_ETTIN:
                P_SetMobjState(target, HEXEN_S_ETTIN_ICE1);
                return;
            case HEXEN_MT_FIREDEMON:
                P_SetMobjState(target, HEXEN_S_FIRED_ICE1);
                return;
            case HEXEN_MT_FIGHTER_BOSS:
                P_SetMobjState(target, HEXEN_S_FIGHTER_ICE);
                return;
            case HEXEN_MT_CLERIC_BOSS:
                P_SetMobjState(target, HEXEN_S_CLERIC_ICE);
                return;
            case HEXEN_MT_MAGE_BOSS:
                P_SetMobjState(target, HEXEN_S_MAGE_ICE);
                return;
            case HEXEN_MT_PIG:
                P_SetMobjState(target, HEXEN_S_PIG_ICE);
                return;
            default:
                target->flags &= ~MF_ICECORPSE;
                break;
        }
    }

    if (target->type == HEXEN_MT_MINOTAUR)
    {
        master = target->special1.m;
        if (master->health > 0)
        {
            if (!ActiveMinotaur(master->player))
            {
                master->player->powers[pw_minotaur] = 0;
            }
        }
    }
    else if (target->type == HEXEN_MT_TREEDESTRUCTIBLE)
    {
        target->height = 24 * FRACUNIT;
    }
    if (target->health < -(target->info->spawnhealth >> 1)
        && target->info->xdeathstate)
    {                           // Extreme death
        P_SetMobjState(target, target->info->xdeathstate);
    }
    else
    {                           // Normal death
        if ((target->type == HEXEN_MT_FIREDEMON) &&
            (target->z <= target->floorz + 2 * FRACUNIT) &&
            (target->info->xdeathstate))
        {
            // This is to fix the imps' staying in fall state
            P_SetMobjState(target, target->info->xdeathstate);
        }
        else
        {
            P_SetMobjState(target, target->info->deathstate);
        }
    }
    target->tics -= P_Random(pr_hexen) & 3;
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
    player->class = PCLASS_PIG;
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
        savedPercent = AutoArmorSave[player->class]
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
                                          ArmorIncrement[player->class][i]),
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
            SB_PaletteFlash(false);
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
            (source->player->readyweapon == WP_FOURTH))
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
// P_FallingDamage
//
//==========================================================================

void P_FallingDamage(player_t * player)
{
    int damage;
    int mom;
    int dist;

    mom = abs(player->mo->momz);
    dist = FixedMul(mom, 16 * FRACUNIT / 23);

    if (mom >= 63 * FRACUNIT)
    {                           // automatic death
        P_DamageMobj(player->mo, NULL, NULL, 10000);
        return;
    }
    damage = ((FixedMul(dist, dist) / 10) >> FRACBITS) - 24;
    if (player->mo->momz > -39 * FRACUNIT && damage > player->mo->health
        && player->mo->health != 1)
    {                           // No-death threshold
        damage = player->mo->health - 1;
    }
    S_StartSound(player->mo, hexen_sfx_player_land);
    P_DamageMobj(player->mo, NULL, NULL, damage);
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

//==========================================================================
//
// P_PoisonDamage - Similar to P_DamageMobj
//
//==========================================================================

void P_PoisonDamage(player_t * player, mobj_t * source, int damage,
                    dboolean playPainSound)
{
    mobj_t *target;
    mobj_t *inflictor;

    target = player->mo;
    inflictor = source;
    if (target->health <= 0)
    {
        return;
    }
    if (target->flags2 & MF2_INVULNERABLE && damage < 10000)
    {                           // mobj is invulnerable
        return;
    }
    if (gameskill == sk_baby)
    {
        // Take half damage in trainer mode
        damage >>= 1;
    }
    if (damage < 1000 && ((player->cheats & CF_GODMODE)
                          || player->powers[pw_invulnerability]))
    {
        return;
    }
    if (damage >= player->health
        && ((gameskill == sk_baby) || deathmatch) && !player->morphTics)
    {                           // Try to use some inventory health
        P_AutoUseHealth(player, damage - player->health + 1);
    }
    player->health -= damage;   // mirror mobj health here for Dave
    if (player->health < 0)
    {
        player->health = 0;
    }
    player->attacker = source;

    //
    // do the damage
    //
    target->health -= damage;
    if (target->health <= 0)
    {                           // Death
        target->special1.i = damage;
        if (inflictor && !player->morphTics)
        {                       // Check for flame death
            if ((inflictor->flags2 & MF2_FIREDAMAGE)
                && (target->health > -50) && (damage > 25))
            {
                target->flags2 |= MF2_FIREDAMAGE;
            }
            if (inflictor->flags2 & MF2_ICEDAMAGE)
            {
                target->flags2 |= MF2_ICEDAMAGE;
            }
        }
        P_KillMobj(source, target);
        return;
    }
    if (!(leveltime & 63) && playPainSound)
    {
        P_SetMobjState(target, target->info->painstate);
    }
/*
	if((P_Random(pr_hexen) < target->info->painchance)
		&& !(target->flags&MF_SKULLFLY))
	{
		target->flags |= MF_JUSTHIT; // fight back!
		P_SetMobjState(target, target->info->painstate);
	}
*/
}
