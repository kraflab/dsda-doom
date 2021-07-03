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

// Position indicator for cooperative net-play reborn
int RebornPosition;

int LeaveMap;
static int LeavePosition;

//=============================================================================

/*
==============
=
= G_DoLoadLevel
=
==============
*/

void G_DoLoadLevel(void)
{
    int i;

    levelstarttic = gametic;    // for time calculation
    gamestate = GS_LEVEL;
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i] && players[i].playerstate == PST_DEAD)
            players[i].playerstate = PST_REBORN;
        memset(players[i].frags, 0, sizeof(players[i].frags));
    }

    SN_StopAllSequences();
    P_SetupLevel(gameepisode, gamemap, 0, gameskill);
    displayplayer = consoleplayer;      // view the guy you are playing
    gameaction = ga_nothing;
    Z_CheckHeap();

//
// clear cmd building stuff
//

    memset(gamekeydown, 0, sizeof(gamekeydown));
    joyxmove = joyymove = joystrafemove = joylook = 0;
    mousex = mousey = 0;
    sendpause = sendsave = paused = false;
    memset(mousearray, 0, sizeof(mousearray));
    memset(joyarray, 0, sizeof(joyarray));
}

/*
===============================================================================
=
= G_Responder
=
= get info needed to make ticcmd_ts for the players
=
===============================================================================
*/

dboolean G_Responder(event_t * ev)
{
    player_t *plr;
    extern dboolean MenuActive;

    plr = &players[consoleplayer];
    if (ev->type == ev_keyup && ev->data1 == key_useartifact)
    {                           // flag to denote that it's okay to use an artifact
        if (!inventory)
        {
            plr->readyArtifact = plr->inventory[inv_ptr].type;
        }
        usearti = true;
    }

    // Check for spy mode player cycle
    if (gamestate == GS_LEVEL && ev->type == ev_keydown
        && ev->data1 == key_spy && !deathmatch)
    {                           // Cycle the display player
        do
        {
            displayplayer++;
            if (displayplayer == MAXPLAYERS)
            {
                displayplayer = 0;
            }
        }
        while (!playeringame[displayplayer]
               && displayplayer != consoleplayer);
        return (true);
    }

    if (CT_Responder(ev))
    {                           // Chat ate the event
        return (true);
    }
    if (gamestate == GS_LEVEL)
    {
        if (SB_Responder(ev))
        {                       // Status bar ate the event
            return (true);
        }
        if (AM_Responder(ev))
        {                       // Automap ate the event
            return (true);
        }
    }

    if (ev->type == ev_keydown && ev->data1 == key_prevweapon)
    {
        next_weapon = -1;
    }
    else if (ev->type == ev_keydown && ev->data1 == key_nextweapon)
    {
        next_weapon = 1;
    }

    switch (ev->type)
    {
        case ev_keydown:
            if (ev->data1 == key_invleft)
            {
                inventoryTics = 5 * 35;
                if (!inventory)
                {
                    inventory = true;
                    break;
                }
                inv_ptr--;
                if (inv_ptr < 0)
                {
                    inv_ptr = 0;
                }
                else
                {
                    curpos--;
                    if (curpos < 0)
                    {
                        curpos = 0;
                    }
                }
                return (true);
            }
            if (ev->data1 == key_invright)
            {
                inventoryTics = 5 * 35;
                if (!inventory)
                {
                    inventory = true;
                    break;
                }
                inv_ptr++;
                if (inv_ptr >= plr->inventorySlotNum)
                {
                    inv_ptr--;
                    if (inv_ptr < 0)
                        inv_ptr = 0;
                }
                else
                {
                    curpos++;
                    if (curpos > 6)
                    {
                        curpos = 6;
                    }
                }
                return (true);
            }
            if (ev->data1 == key_pause && !MenuActive)
            {
                sendpause = true;
                return (true);
            }
            if (ev->data1 < NUMKEYS)
            {
                gamekeydown[ev->data1] = true;
            }
            return (true);      // eat key down events

        case ev_keyup:
            if (ev->data1 < NUMKEYS)
            {
                gamekeydown[ev->data1] = false;
            }
            return (false);     // always let key up events filter down

        case ev_mouse:
            SetMouseButtons(ev->data1);
            mousex = ev->data2 * (mouseSensitivity + 5) / 10;
            mousey = ev->data3 * (mouseSensitivity + 5) / 10;
            return (true);      // eat events

        case ev_joystick:
            SetJoyButtons(ev->data1);
            joyxmove = ev->data2;
            joyymove = ev->data3;
            joystrafemove = ev->data4;
            joylook = ev->data5;
            return (true);      // eat events

        default:
            break;
    }
    return (false);
}


//==========================================================================
//
// G_Ticker
//
//==========================================================================

void G_Ticker(void)
{
    int i, buf;
    ticcmd_t *cmd = NULL;

//
// do player reborns if needed
//
    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i] && players[i].playerstate == PST_REBORN)
            G_DoReborn(i);

//
// do things to change the game state
//
    while (gameaction != ga_nothing)
    {
        switch (gameaction)
        {
            case ga_loadlevel:
                G_DoLoadLevel();
                break;
            case ga_newgame:
                G_DoNewGame();
                break;
            case ga_loadgame:
                G_DoLoadGame();
                break;
            case ga_savegame:
                G_DoSaveGame();
                break;
            case ga_singlereborn:
                G_DoSingleReborn();
                break;
            case ga_playdemo:
                G_DoPlayDemo();
                break;
            case ga_screenshot:
                V_ScreenShot("HEXEN%02i.%s");
                P_SetMessage(&players[consoleplayer], "SCREEN SHOT", false);
                gameaction = ga_nothing;
                break;
            case ga_leavemap:
                G_DoTeleportNewMap();
                break;
            case ga_completed:
                G_DoCompleted();
                break;
            case ga_worlddone:
                G_DoWorldDone();
                break;
            case ga_victory:
                F_StartFinale();
                break;
            default:
                break;
        }
    }


//
// get commands, check consistancy, and build new consistancy check
//
    //buf = gametic%BACKUPTICS;
    buf = (gametic / ticdup) % BACKUPTICS;

    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i])
        {
            cmd = &players[i].cmd;

            memcpy(cmd, &netcmds[i], sizeof(ticcmd_t));

            if (demoplayback)
                G_ReadDemoTiccmd(cmd);
            if (demorecording)
                G_WriteDemoTiccmd(cmd);

            if (netgame && !(gametic % ticdup))
            {
                if (gametic > BACKUPTICS
                    && consistancy[i][buf] != cmd->consistancy)
                {
                    I_Error("consistency failure (%i should be %i)",
                            cmd->consistancy, consistancy[i][buf]);
                }
                if (players[i].mo)
                    consistancy[i][buf] = players[i].mo->x;
                else
                    consistancy[i][buf] = rndindex;
            }
        }

//
// check for special buttons
//
    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i])
        {
            if (players[i].cmd.buttons & BT_SPECIAL)
            {
                switch (players[i].cmd.buttons & BT_SPECIALMASK)
                {
                    case BTS_PAUSE:
                        paused ^= 1;
                        if (paused)
                        {
                            S_PauseSound();
                        }
                        else
                        {
                            S_ResumeSound();
                        }
                        break;

                    case BTS_SAVEGAME:
                        if (!savedescription[0])
                        {
                            if (netgame)
                            {
                                M_StringCopy(savedescription, "NET GAME",
                                             sizeof(savedescription));
                            }
                            else
                            {
                                M_StringCopy(savedescription, "SAVE GAME",
                                             sizeof(savedescription));
                            }
                        }
                        savegameslot =
                            (players[i].cmd.
                             buttons & BTS_SAVEMASK) >> BTS_SAVESHIFT;
                        gameaction = ga_savegame;
                        break;
                }
            }
        }
    // turn inventory off after a certain amount of time
    if (inventory && !(--inventoryTics))
    {
        players[consoleplayer].readyArtifact =
            players[consoleplayer].inventory[inv_ptr].type;
        inventory = false;
        cmd->arti = 0;
    }
//
// do main actions
//
//
// do main actions
//
    switch (gamestate)
    {
        case GS_LEVEL:
            P_Ticker();
            SB_Ticker();
            AM_Ticker();
            CT_Ticker();
            break;
        case GS_INTERMISSION:
            IN_Ticker();
            break;
        case GS_FINALE:
            F_Ticker();
            break;
        case GS_DEMOSCREEN:
            H2_PageTicker();
            break;
    }
}


/*
==============================================================================

						PLAYER STRUCTURE FUNCTIONS

also see P_SpawnPlayer in P_Things
==============================================================================
*/

//==========================================================================
//
// G_PlayerExitMap
//
// Called when the player leaves a map.
//
//==========================================================================

void G_PlayerExitMap(int playerNumber)
{
    int i;
    player_t *player;
    int flightPower;

    player = &players[playerNumber];

    // Strip all current powers (retain flight)
    flightPower = player->powers[pw_flight];
    memset(player->powers, 0, sizeof(player->powers));
    player->powers[pw_flight] = flightPower;

    if (deathmatch)
    {
        player->powers[pw_flight] = 0;
    }
    else
    {
        if (P_GetMapCluster(gamemap) != P_GetMapCluster(LeaveMap))
        {                       // Entering new cluster
            // Strip all keys
            for (i = 0; i < NUMCARDS; ++i)
              player->cards[i] = 0;

            // Strip flight artifact
            for (i = 0; i < 25; i++)
            {
                player->powers[pw_flight] = 0;
                P_PlayerUseArtifact(player, hexen_arti_fly);
            }
            player->powers[pw_flight] = 0;
        }
    }

    if (player->morphTics)
    {
        player->readyweapon = player->mo->special1.i;     // Restore weapon
        player->morphTics = 0;
    }
    player->messageTics = 0;
    player->lookdir = 0;
    player->mo->flags &= ~MF_SHADOW;    // Remove invisibility
    player->extralight = 0;     // Remove weapon flashes
    player->fixedcolormap = 0;  // Remove torch
    player->damagecount = 0;    // No palette changes
    player->bonuscount = 0;
    player->poisoncount = 0;
    if (player == &players[consoleplayer])
    {
        SB_state = -1;          // refresh the status bar
        viewangleoffset = 0;
    }
}

//==========================================================================
//
// G_PlayerReborn
//
// Called after a player dies.  Almost everything is cleared and
// initialized.
//
//==========================================================================

void G_PlayerReborn(int player)
{
    player_t *p;
    int frags[MAXPLAYERS];
    int killcount, itemcount, secretcount;
    unsigned int worldTimer;

    memcpy(frags, players[player].frags, sizeof(frags));
    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;
    worldTimer = players[player].worldTimer;

    p = &players[player];
    memset(p, 0, sizeof(*p));

    memcpy(players[player].frags, frags, sizeof(players[player].frags));
    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;
    players[player].worldTimer = worldTimer;
    players[player].pclass = PlayerClass[player];

    p->usedown = p->attackdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = MAXHEALTH;
    p->readyweapon = p->pendingweapon = wp_first;
    p->weaponowned[wp_first] = true;
    p->messageTics = 0;
    p->lookdir = 0;
    localQuakeHappening[player] = false;
    if (p == &players[consoleplayer])
    {
        SB_state = -1;          // refresh the status bar
        inv_ptr = 0;            // reset the inventory pointer
        curpos = 0;
        viewangleoffset = 0;
    }
}

/*
====================
=
= G_CheckSpot
=
= Returns false if the player cannot be respawned at the given mapthing_t spot
= because something is occupying it
====================
*/

void P_SpawnPlayer(mapthing_t * mthing);

dboolean G_CheckSpot(int playernum, mapthing_t * mthing)
{
    fixed_t x, y;
    subsector_t *ss;
    unsigned an;
    mobj_t *mo;

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    players[playernum].mo->flags2 &= ~MF2_PASSMOBJ;
    if (!P_CheckPosition(players[playernum].mo, x, y))
    {
        players[playernum].mo->flags2 |= MF2_PASSMOBJ;
        return false;
    }
    players[playernum].mo->flags2 |= MF2_PASSMOBJ;

// spawn a teleport fog
    ss = R_PointInSubsector(x, y);
    an = ((unsigned) ANG45 * (mthing->angle / 45)) >> ANGLETOFINESHIFT;

    mo = P_SpawnMobj(x + 20 * finecosine[an], y + 20 * finesine[an],
                     ss->sector->floorheight + TELEFOGHEIGHT, HEXEN_MT_TFOG);
    if (players[consoleplayer].viewz != 1)
        S_StartSound(mo, hexen_sfx_teleport); // don't start sound on first frame

    return true;
}

/*
====================
=
= G_DeathMatchSpawnPlayer
=
= Spawns a player at one of the random death match spots
= called at level load and each death
====================
*/

void G_DeathMatchSpawnPlayer(int playernum)
{
    int i, j;
    int selections;

    selections = deathmatch_p - deathmatchstarts;

    for (j = 0; j < 20; j++)
    {
        i = P_Random(pr_hexen) % selections;
        if (G_CheckSpot(playernum, &deathmatchstarts[i]))
        {
            deathmatchstarts[i].type = playernum + 1;
            P_SpawnPlayer(&deathmatchstarts[i]);
            return;
        }
    }

// no good spot, so the player will probably get stuck
    P_SpawnPlayer(&playerstarts[0][playernum]);
}

//==========================================================================
//
// G_DoReborn
//
//==========================================================================

void G_DoReborn(int playernum)
{
    int i;
    dboolean oldWeaponowned[NUMWEAPONS];
    dboolean oldKeys[NUMCARDS];
    int oldPieces;
    dboolean foundSpot;
    int bestWeapon;

    // quit demo unless -demoextend
    if (!demoextend && G_CheckDemoStatus())
    {
        return;
    }
    if (!netgame)
    {
        if (SV_RebornSlotAvailable())
        {                       // Use the reborn code if the slot is available
            gameaction = ga_singlereborn;
        }
        else
        {                       // Start a new game if there's no reborn info
            gameaction = ga_newgame;
        }
    }
    else
    {                           // Net-game
        players[playernum].mo->player = NULL;   // Dissassociate the corpse

        if (deathmatch)
        {                       // Spawn at random spot if in death match
            G_DeathMatchSpawnPlayer(playernum);
            return;
        }

        // Cooperative net-play, retain keys and weapons
        for (i = 0; i < NUMCARDS; ++i)
          oldKeys[i] = players[playernum].cards[i];
        oldPieces = players[playernum].pieces;
        for (i = 0; i < NUMWEAPONS; i++)
        {
            oldWeaponowned[i] = players[playernum].weaponowned[i];
        }

        foundSpot = false;
        if (G_CheckSpot(playernum, &playerstarts[RebornPosition][playernum]))
        {                       // Appropriate player start spot is open
            P_SpawnPlayer(&playerstarts[RebornPosition][playernum]);
            foundSpot = true;
        }
        else
        {
            // Try to spawn at one of the other player start spots
            for (i = 0; i < MAXPLAYERS; i++)
            {
                if (G_CheckSpot(playernum, &playerstarts[RebornPosition][i]))
                {               // Found an open start spot

                    // Fake as other player
                    playerstarts[RebornPosition][i].type = playernum + 1;
                    P_SpawnPlayer(&playerstarts[RebornPosition][i]);

                    // Restore proper player type
                    playerstarts[RebornPosition][i].type = i + 1;

                    foundSpot = true;
                    break;
                }
            }
        }

        if (foundSpot == false)
        {                       // Player's going to be inside something
            P_SpawnPlayer(&playerstarts[RebornPosition][playernum]);
        }

        // Restore keys and weapons
        for (i = 0; i < NUMCARDS; ++i)
          players[playernum].cards[i] = oldKeys[i];
        players[playernum].pieces = oldPieces;
        for (bestWeapon = 0, i = 0; i < NUMWEAPONS; i++)
        {
            if (oldWeaponowned[i])
            {
                bestWeapon = i;
                players[playernum].weaponowned[i] = true;
            }
        }
        players[playernum].ammo[MANA_1] = 25;
        players[playernum].ammo[MANA_2] = 25;
        if (bestWeapon)
        {                       // Bring up the best weapon
            players[playernum].pendingweapon = bestWeapon;
        }
    }
}

void G_ScreenShot(void)
{
    gameaction = ga_screenshot;
}

//==========================================================================
//
// G_TeleportNewMap
//
// Only called by the warp cheat code.  Works just like normal map to map
// teleporting, but doesn't do any interlude stuff.
//
//==========================================================================

void G_TeleportNewMap(int map, int position)
{
    gameaction = ga_leavemap;
    LeaveMap = map;
    LeavePosition = position;
}

//==========================================================================
//
// G_DoTeleportNewMap
//
//==========================================================================

void G_DoTeleportNewMap(void)
{
    SV_MapTeleport(LeaveMap, LeavePosition);
    gamestate = GS_LEVEL;
    gameaction = ga_nothing;
    RebornPosition = LeavePosition;
}

void G_DoCompleted(void)
{
    int i;

    gameaction = ga_nothing;

    // quit demo unless -demoextend
    if (!demoextend && G_CheckDemoStatus())
    {
        return;
    }
    for (i = 0; i < MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
            G_PlayerExitMap(i);
        }
    }
    if (LeaveMap == -1 && LeavePosition == -1)
    {
        gameaction = ga_victory;
        return;
    }
    else
    {
        gamestate = GS_INTERMISSION;
        IN_Start();
    }
}

//============================================================================
//
// G_WorldDone
//
//============================================================================

void G_WorldDone(void)
{
    gameaction = ga_worlddone;
}

//============================================================================
//
// G_DoWorldDone
//
//============================================================================

void G_DoWorldDone(void)
{
    gamestate = GS_LEVEL;
    G_DoLoadLevel();
    gameaction = ga_nothing;
    viewactive = true;
}

//==========================================================================
//
// G_DoSingleReborn
//
// Called by G_Ticker based on gameaction.  Loads a game from the reborn
// save slot.
//
//==========================================================================

void G_DoSingleReborn(void)
{
    gameaction = ga_nothing;
    SV_LoadGame(SV_GetRebornSlot());
    SB_SetClassData();
}
