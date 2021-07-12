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

#include "doomstat.h"
#include "p_setup.h"
#include "d_player.h"
#include "p_mobj.h"
#include "p_map.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "lprintf.h"

#include "hexen/p_acs.h"

#include "sv_save.h"

#define BASE_SLOT 6
#define REBORN_SLOT 7
#define REBORN_DESCRIPTION "TEMP GAME"
#define DEFAULT_SAVEPATH "hexndata/"

const char *SavePath = DEFAULT_SAVEPATH;

int vanilla_savegame_limit = 1;

static int MobjCount;
static mobj_t **MobjList;
static mobj_t ***TargetPlayerAddrs;
static int TargetPlayerCount;
static dboolean SavingPlayers;
static FILE *SavingFP;

extern int inv_ptr;
extern int curpos;

void SV_SaveMap(dboolean savePlayers)
{
  return;
}

static void ClearSaveSlot(int slot)
{
  return;
}

void SV_LoadMap(void)
{
  return;
}

static dboolean ExistingFile(char *name)
{
  return false;
}

void SV_SaveGame(int slot, const char *description)
{
  return;
}

void SV_MapTeleport(int map, int position)
{
    int i;
    int j;
    int key_i;
    char fileName[100];
    player_t playerBackup[MAX_MAXPLAYERS];
    mobj_t *targetPlayerMobj;
    mobj_t *mobj;
    int inventoryPtr;
    int currentInvPos;
    dboolean rClass;
    dboolean playerWasReborn;
    dboolean oldWeaponowned[HEXEN_NUMWEAPONS];
    int oldKeys[NUMCARDS];
    int oldPieces = 0;
    int bestWeapon;

    if (!deathmatch)
    {
        if (P_GetMapCluster(gamemap) == P_GetMapCluster(map))
        {                       // Same cluster - save map without saving player mobjs
            SV_SaveMap(false);
        }
        else
        {                       // Entering new cluster - clear base slot
            ClearSaveSlot(BASE_SLOT);
        }
    }

    // Store player structs for later
    rClass = randomclass;
    randomclass = false;
    for (i = 0; i < g_maxplayers; i++)
    {
        playerBackup[i] = players[i];
    }

    // Save some globals that get trashed during the load
    inventoryPtr = inv_ptr;
    currentInvPos = curpos;

    // Only SV_LoadMap() uses TargetPlayerAddrs, so it's NULLed here
    // for the following check (player mobj redirection)
    TargetPlayerAddrs = NULL;

    gamemap = map;
    doom_snprintf(fileName, sizeof(fileName), "%shex6%02d.hxs", SavePath, gamemap);
    if (!deathmatch && ExistingFile(fileName))
    {                           // Unarchive map
        SV_LoadMap();
        P_MapStart();
    }
    else
    {                           // New map
        G_InitNew(gameskill, gameepisode, gamemap);

        P_MapStart();

        // Destroy all freshly spawned players
        for (i = 0; i < g_maxplayers; i++)
        {
            if (playeringame[i])
            {
                P_RemoveMobj(players[i].mo);
            }
        }
    }

    // Restore player structs
    targetPlayerMobj = NULL;
    for (i = 0; i < g_maxplayers; i++)
    {
        if (!playeringame[i])
        {
            continue;
        }
        players[i] = playerBackup[i];
        ClearMessage();
        players[i].attacker = NULL;
        players[i].poisoner = NULL;

        if (netgame)
        {
            if (players[i].playerstate == PST_DEAD)
            {                   // In a network game, force all players to be alive
                players[i].playerstate = PST_REBORN;
            }
            if (!deathmatch)
            {                   // Cooperative net-play, retain keys and weapons
                for (key_i = 0; key_i < NUMCARDS; ++key_i)
                  oldKeys[key_i] = players[i].cards[key_i];
                oldPieces = players[i].pieces;
                for (j = 0; j < HEXEN_NUMWEAPONS; j++)
                {
                    oldWeaponowned[j] = players[i].weaponowned[j];
                }
            }
        }
        playerWasReborn = (players[i].playerstate == PST_REBORN);
        if (deathmatch)
        {
            memset(players[i].frags, 0, sizeof(players[i].frags));
            mobj = P_SpawnMobj(playerstarts[0][i].x << 16,
                               playerstarts[0][i].y << 16, 0,
                               HEXEN_MT_PLAYER_FIGHTER);
            players[i].mo = mobj;
            G_DeathMatchSpawnPlayer(i);
            P_RemoveMobj(mobj);
        }
        else
        {
            P_SpawnPlayer(i, &playerstarts[position][i]);
        }

        if (playerWasReborn && netgame && !deathmatch)
        {                       // Restore keys and weapons when reborn in co-op
            for (key_i = 0; key_i < NUMCARDS; ++key_i)
              players[i].cards[key_i] = oldKeys[key_i];
            players[i].pieces = oldPieces;
            for (bestWeapon = 0, j = 0; j < HEXEN_NUMWEAPONS; j++)
            {
                if (oldWeaponowned[j])
                {
                    bestWeapon = j;
                    players[i].weaponowned[j] = true;
                }
            }
            players[i].ammo[MANA_1] = 25;
            players[i].ammo[MANA_2] = 25;
            if (bestWeapon)
            {                   // Bring up the best weapon
                players[i].pendingweapon = bestWeapon;
            }
        }

        if (targetPlayerMobj == NULL)
        {                       // The poor sap
            targetPlayerMobj = players[i].mo;
        }
    }
    randomclass = rClass;

    // Redirect anything targeting a player mobj
    if (TargetPlayerAddrs)
    {
        for (i = 0; i < TargetPlayerCount; i++)
        {
            *TargetPlayerAddrs[i] = targetPlayerMobj;
        }
        Z_Free(TargetPlayerAddrs);
    }

    // Destroy all things touching players
    for (i = 0; i < g_maxplayers; i++)
    {
        if (playeringame[i])
        {
            P_TeleportMove(players[i].mo, players[i].mo->x, players[i].mo->y, false);
        }
    }

    // Restore trashed globals
    inv_ptr = inventoryPtr;
    curpos = currentInvPos;

    // Launch waiting scripts
    if (!deathmatch)
    {
        P_CheckACSStore();
    }

    P_MapEnd();
}
