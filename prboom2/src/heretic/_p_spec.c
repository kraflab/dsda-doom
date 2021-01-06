
/*
===============================================================================
=
= P_CrossSpecialLine - TRIGGER
=
= Called every time a thing origin is about to cross
= a line with a non 0 special
=
===============================================================================
*/

void P_CrossSpecialLine(int linenum, int side, mobj_t * thing)
{
    line_t *line;

    line = &lines[linenum];
    if (!thing->player)
    {                           // Check if trigger allowed by non-player mobj
        switch (line->special)
        {
            case 39:           // Trigger_TELEPORT
            case 97:           // Retrigger_TELEPORT
            case 4:            // Trigger_Raise_Door
                //case 10:      // PLAT DOWN-WAIT-UP-STAY TRIGGER
                //case 88:      // PLAT DOWN-WAIT-UP-STAY RETRIGGER
                break;
            default:
                return;
                break;
        }
    }
    switch (line->special)
    {
            //====================================================
            // TRIGGERS
            //====================================================
        case 2:                // Open Door
            EV_DoDoor(line, vld_open, VDOORSPEED);
            line->special = 0;
            break;
        case 3:                // Close Door
            EV_DoDoor(line, vld_close, VDOORSPEED);
            line->special = 0;
            break;
        case 4:                // Raise Door
            EV_DoDoor(line, vld_normal, VDOORSPEED);
            line->special = 0;
            break;
        case 5:                // Raise Floor
            EV_DoFloor(line, raiseFloor);
            line->special = 0;
            break;
        case 6:                // Fast Ceiling Crush & Raise
            EV_DoCeiling(line, fastCrushAndRaise);
            line->special = 0;
            break;
        case 8:                // Trigger_Build_Stairs (8 pixel steps)
            EV_BuildStairs(line, 8 * FRACUNIT);
            line->special = 0;
            break;
        case 106:              // Trigger_Build_Stairs_16 (16 pixel steps)
            EV_BuildStairs(line, 16 * FRACUNIT);
            line->special = 0;
            break;
        case 10:               // PlatDownWaitUp
            EV_DoPlat(line, downWaitUpStay, 0);
            line->special = 0;
            break;
        case 12:               // Light Turn On - brightest near
            EV_LightTurnOn(line, 0);
            line->special = 0;
            break;
        case 13:               // Light Turn On 255
            EV_LightTurnOn(line, 255);
            line->special = 0;
            break;
        case 16:               // Close Door 30
            EV_DoDoor(line, vld_close30ThenOpen, VDOORSPEED);
            line->special = 0;
            break;
        case 17:               // Start Light Strobing
            EV_StartLightStrobing(line);
            line->special = 0;
            break;
        case 19:               // Lower Floor
            EV_DoFloor(line, lowerFloor);
            line->special = 0;
            break;
        case 22:               // Raise floor to nearest height and change texture
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            line->special = 0;
            break;
        case 25:               // Ceiling Crush and Raise
            EV_DoCeiling(line, crushAndRaise);
            line->special = 0;
            break;
        case 30:               // Raise floor to shortest texture height
            // on either side of lines
            EV_DoFloor(line, raiseToTexture);
            line->special = 0;
            break;
        case 35:               // Lights Very Dark
            EV_LightTurnOn(line, 35);
            line->special = 0;
            break;
        case 36:               // Lower Floor (TURBO)
            EV_DoFloor(line, turboLower);
            line->special = 0;
            break;
        case 37:               // LowerAndChange
            EV_DoFloor(line, lowerAndChange);
            line->special = 0;
            break;
        case 38:               // Lower Floor To Lowest
            EV_DoFloor(line, lowerFloorToLowest);
            line->special = 0;
            break;
        case 39:               // TELEPORT!
            EV_Teleport(line, side, thing);
            line->special = 0;
            break;
        case 40:               // RaiseCeilingLowerFloor
            EV_DoCeiling(line, raiseToHighest);
            EV_DoFloor(line, lowerFloorToLowest);
            line->special = 0;
            break;
        case 44:               // Ceiling Crush
            EV_DoCeiling(line, lowerAndCrush);
            line->special = 0;
            break;
        case 52:               // EXIT!
            G_ExitLevel();
            line->special = 0;
            break;
        case 53:               // Perpetual Platform Raise
            EV_DoPlat(line, perpetualRaise, 0);
            line->special = 0;
            break;
        case 54:               // Platform Stop
            EV_StopPlat(line);
            line->special = 0;
            break;
        case 56:               // Raise Floor Crush
            EV_DoFloor(line, raiseFloorCrush);
            line->special = 0;
            break;
        case 57:               // Ceiling Crush Stop
            EV_CeilingCrushStop(line);
            line->special = 0;
            break;
        case 58:               // Raise Floor 24
            EV_DoFloor(line, raiseFloor24);
            line->special = 0;
            break;
        case 59:               // Raise Floor 24 And Change
            EV_DoFloor(line, raiseFloor24AndChange);
            line->special = 0;
            break;
        case 104:              // Turn lights off in sector(tag)
            EV_TurnTagLightsOff(line);
            line->special = 0;
            break;
        case 105:              // Trigger_SecretExit
            G_SecretExitLevel();
            line->special = 0;
            break;

            //====================================================
            // RE-DOABLE TRIGGERS
            //====================================================

        case 72:               // Ceiling Crush
            EV_DoCeiling(line, lowerAndCrush);
            break;
        case 73:               // Ceiling Crush and Raise
            EV_DoCeiling(line, crushAndRaise);
            break;
        case 74:               // Ceiling Crush Stop
            EV_CeilingCrushStop(line);
            break;
        case 75:               // Close Door
            EV_DoDoor(line, vld_close, VDOORSPEED);
            break;
        case 76:               // Close Door 30
            EV_DoDoor(line, vld_close30ThenOpen, VDOORSPEED);
            break;
        case 77:               // Fast Ceiling Crush & Raise
            EV_DoCeiling(line, fastCrushAndRaise);
            break;
        case 79:               // Lights Very Dark
            EV_LightTurnOn(line, 35);
            break;
        case 80:               // Light Turn On - brightest near
            EV_LightTurnOn(line, 0);
            break;
        case 81:               // Light Turn On 255
            EV_LightTurnOn(line, 255);
            break;
        case 82:               // Lower Floor To Lowest
            EV_DoFloor(line, lowerFloorToLowest);
            break;
        case 83:               // Lower Floor
            EV_DoFloor(line, lowerFloor);
            break;
        case 84:               // LowerAndChange
            EV_DoFloor(line, lowerAndChange);
            break;
        case 86:               // Open Door
            EV_DoDoor(line, vld_open, VDOORSPEED);
            break;
        case 87:               // Perpetual Platform Raise
            EV_DoPlat(line, perpetualRaise, 0);
            break;
        case 88:               // PlatDownWaitUp
            EV_DoPlat(line, downWaitUpStay, 0);
            break;
        case 89:               // Platform Stop
            EV_StopPlat(line);
            break;
        case 90:               // Raise Door
            EV_DoDoor(line, vld_normal, VDOORSPEED);
            break;
        case 100:              // Retrigger_Raise_Door_Turbo
            EV_DoDoor(line, vld_normal, VDOORSPEED * 3);
            break;
        case 91:               // Raise Floor
            EV_DoFloor(line, raiseFloor);
            break;
        case 92:               // Raise Floor 24
            EV_DoFloor(line, raiseFloor24);
            break;
        case 93:               // Raise Floor 24 And Change
            EV_DoFloor(line, raiseFloor24AndChange);
            break;
        case 94:               // Raise Floor Crush
            EV_DoFloor(line, raiseFloorCrush);
            break;
        case 95:               // Raise floor to nearest height and change texture
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            break;
        case 96:               // Raise floor to shortest texture height
            // on either side of lines
            EV_DoFloor(line, raiseToTexture);
            break;
        case 97:               // TELEPORT!
            EV_Teleport(line, side, thing);
            break;
        case 98:               // Lower Floor (TURBO)
            EV_DoFloor(line, turboLower);
            break;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_ShootSpecialLine
//
// Called when a thing shoots a special line.
//
//----------------------------------------------------------------------------

void P_ShootSpecialLine(mobj_t * thing, line_t * line)
{
    if (!thing->player)
    {                           // Check if trigger allowed by non-player mobj
        switch (line->special)
        {
            case 46:           // Impact_OpenDoor
                break;
            default:
                return;
                break;
        }
    }
    switch (line->special)
    {
        case 24:               // Impact_RaiseFloor
            EV_DoFloor(line, raiseFloor);
            P_ChangeSwitchTexture(line, 0);
            break;
        case 46:               // Impact_OpenDoor
            EV_DoDoor(line, vld_open, VDOORSPEED);
            P_ChangeSwitchTexture(line, 1);
            break;
        case 47:               // Impact_RaiseFloorNear&Change
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            P_ChangeSwitchTexture(line, 0);
            break;
    }
}

//----------------------------------------------------------------------------
//
// PROC P_PlayerInSpecialSector
//
// Called every tic frame that the player origin is in a special sector.
//
//----------------------------------------------------------------------------

void P_PlayerInSpecialSector(player_t * player)
{
    extern boolean messageson;
    sector_t *sector;
    static int pushTab[5] = {
        2048 * 5,
        2048 * 10,
        2048 * 25,
        2048 * 30,
        2048 * 35
    };

    sector = player->mo->subsector->sector;
    if (player->mo->z != sector->floorheight)
    {                           // Player is not touching the floor
        return;
    }
    switch (sector->special)
    {
        case 7:                // Damage_Sludge
            if (!(leveltime & 31))
            {
                P_DamageMobj(player->mo, NULL, NULL, 4);
            }
            break;
        case 5:                // Damage_LavaWimpy
            if (!(leveltime & 15))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 5);
                P_HitFloor(player->mo);
            }
            break;
        case 16:               // Damage_LavaHefty
            if (!(leveltime & 15))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 8);
                P_HitFloor(player->mo);
            }
            break;
        case 4:                // Scroll_EastLavaDamage
            P_Thrust(player, 0, 2048 * 28);
            if (!(leveltime & 15))
            {
                P_DamageMobj(player->mo, &LavaInflictor, NULL, 5);
                P_HitFloor(player->mo);
            }
            break;
        case 9:                // SecretArea
            player->secretcount++;
            // [crispy] Show centered "Secret Revealed!" message
            if (messageson && crispy->secretmessage && player == &players[consoleplayer]) 
            {
                static char str_count[32];
                
                M_snprintf(str_count, sizeof(str_count), "SECRET %d OF %d REVEALED!", player->secretcount, totalsecret);

                P_SetCenterMessage(&players[consoleplayer],
                    (crispy->secretmessage == SECRETMESSAGE_COUNT) ? str_count : HUSTR_SECRETFOUND);
                // Play the chat sound when secret found, it's a message after all
                S_StartSound(NULL, sfx_chat);
            }
            sector->special = 0;
            break;
        case 11:               // Exit_SuperDamage (DOOM E1M8 finale)
            /*
               player->cheats &= ~CF_GODMODE;
               if(!(leveltime&0x1f))
               {
               P_DamageMobj(player->mo, NULL, NULL, 20);
               }
               if(player->health <= 10)
               {
               G_ExitLevel();
               }
             */
            break;

        case 25:
        case 26:
        case 27:
        case 28:
        case 29:               // Scroll_North
            P_Thrust(player, ANG90, pushTab[sector->special - 25]);
            break;
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:               // Scroll_East
            P_Thrust(player, 0, pushTab[sector->special - 20]);
            break;
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:               // Scroll_South
            P_Thrust(player, ANG270, pushTab[sector->special - 30]);
            break;
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:               // Scroll_West
            P_Thrust(player, ANG180, pushTab[sector->special - 35]);
            break;

        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
            // Wind specials are handled in (P_mobj):P_XYMovement
            break;

        case 15:               // Friction_Low
            // Only used in (P_mobj):P_XYMovement and (P_user):P_Thrust
            break;

        default:
            I_Error("P_PlayerInSpecialSector: "
                    "unknown special %i", sector->special);
    }
}

//----------------------------------------------------------------------------
//
// PROC P_UpdateSpecials
//
// Animate planes, scroll walls, etc.
//
//----------------------------------------------------------------------------

void P_UpdateSpecials(void)
{
    int i;
    int pic;
    anim_t *anim;
    line_t *line;

    // Animate flats and textures
    for (anim = anims; anim < lastanim; anim++)
    {
        for (i = anim->basepic; i < anim->basepic + anim->numpics; i++)
        {
            pic =
                anim->basepic +
                ((leveltime / anim->speed + i) % anim->numpics);
            if (anim->istexture)
            {
                texturetranslation[i] = pic;
            }
            else
            {
                flattranslation[i] = pic;
            }
        }
    }
    // Update scrolling texture offsets
    for (i = 0; i < numlinespecials; i++)
    {
        line = linespeciallist[i];
        switch (line->special)
        {
            case 48:           // Effect_Scroll_Left
                // [crispy] smooth texture scrolling
                sides[line->sidenum[0]].basetextureoffset += FRACUNIT;
                sides[line->sidenum[0]].textureoffset =
                    sides[line->sidenum[0]].basetextureoffset;
                break;
            case 99:           // Effect_Scroll_Right
                // [crispy] smooth texture scrolling
                sides[line->sidenum[0]].basetextureoffset -= FRACUNIT;
                sides[line->sidenum[0]].textureoffset =
                    sides[line->sidenum[0]].basetextureoffset;
                break;
        }
    }
    // Handle buttons
    for (i = 0; i < MAXBUTTONS; i++)
    {
        if (buttonlist[i].btimer)
        {
            buttonlist[i].btimer--;
            if (!buttonlist[i].btimer)
            {
                switch (buttonlist[i].where)
                {
                    case top:
                        sides[buttonlist[i].line->sidenum[0]].toptexture =
                            buttonlist[i].btexture;
                        break;
                    case middle:
                        sides[buttonlist[i].line->sidenum[0]].midtexture =
                            buttonlist[i].btexture;
                        break;
                    case bottom:
                        sides[buttonlist[i].line->sidenum[0]].bottomtexture =
                            buttonlist[i].btexture;
                        break;
                }
                S_StartSound(buttonlist[i].soundorg, sfx_switch);
                memset(&buttonlist[i], 0, sizeof(button_t));
            }
        }
    }
}

//============================================================
//
//      Special Stuff that can't be categorized
//
//============================================================
int EV_DoDonut(line_t * line)
{
    sector_t *s1;
    sector_t *s2;
    sector_t *s3;
    int secnum;
    int rtn;
    int i;
    floormove_t *floor;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        s1 = &sectors[secnum];

        //      ALREADY MOVING?  IF SO, KEEP GOING...
        if (s1->specialdata)
            continue;

        rtn = 1;
        s2 = getNextSector(s1->lines[0], s1);
        for (i = 0; i < s2->linecount; i++)
        {
            // Note: This was originally part of the following test:
            //   (!s2->lines[i]->flags & ML_TWOSIDED) ||
            // Due to the apparent mistaken formatting, this can never be
            // true.

            if (s2->lines[i]->backsector == s1)
                continue;
            s3 = s2->lines[i]->backsector;

            //
            //      Spawn rising slime
            //
            floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
            P_AddThinker(&floor->thinker);
            s2->specialdata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = donutRaise;
            floor->crush = false;
            floor->direction = 1;
            floor->sector = s2;
            floor->speed = FLOORSPEED / 2;
            floor->texture = s3->floorpic;
            floor->newspecial = 0;
            floor->floordestheight = s3->floorheight;

            //
            //      Spawn lowering donut-hole
            //
            floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
            P_AddThinker(&floor->thinker);
            s1->specialdata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = lowerFloor;
            floor->crush = false;
            floor->direction = -1;
            floor->sector = s1;
            floor->speed = FLOORSPEED / 2;
            floor->floordestheight = s3->floorheight;
            break;
        }
    }
    return rtn;
}

/*
==============================================================================

							SPECIAL SPAWNING

==============================================================================
*/
/*
================================================================================
= P_SpawnSpecials
=
= After the map has been loaded, scan for specials that
= spawn thinkers
=
===============================================================================
*/

short numlinespecials;
line_t *linespeciallist[MAXLINEANIMS];

void P_SpawnSpecials(void)
{
    sector_t *sector;
    int i;

    //
    //      Init special SECTORs
    //
    sector = sectors;
    for (i = 0; i < numsectors; i++, sector++)
    {
        if (!sector->special)
            continue;
        switch (sector->special)
        {
            case 1:            // FLICKERING LIGHTS
                P_SpawnLightFlash(sector);
                break;
            case 2:            // STROBE FAST
                P_SpawnStrobeFlash(sector, FASTDARK, 0);
                break;
            case 3:            // STROBE SLOW
                P_SpawnStrobeFlash(sector, SLOWDARK, 0);
                break;
            case 4:            // STROBE FAST/DEATH SLIME
                P_SpawnStrobeFlash(sector, FASTDARK, 0);
                sector->special = 4;
                break;
            case 8:            // GLOWING LIGHT
                P_SpawnGlowingLight(sector);
                break;
            case 9:            // SECRET SECTOR
                totalsecret++;
                break;
            case 10:           // DOOR CLOSE IN 30 SECONDS
                P_SpawnDoorCloseIn30(sector);
                break;
            case 12:           // SYNC STROBE SLOW
                P_SpawnStrobeFlash(sector, SLOWDARK, 1);
                break;
            case 13:           // SYNC STROBE FAST
                P_SpawnStrobeFlash(sector, FASTDARK, 1);
                break;
            case 14:           // DOOR RAISE IN 5 MINUTES
                P_SpawnDoorRaiseIn5Mins(sector, i);
                break;
        }
    }


    //
    //      Init line EFFECTs
    //
    numlinespecials = 0;
    for (i = 0; i < numlines; i++)
        switch (lines[i].special)
        {
            case 48:           // Effect_Scroll_Left
            case 99:           // Effect_Scroll_Right
                linespeciallist[numlinespecials] = &lines[i];
                numlinespecials++;
                break;
        }

    //
    //      Init other misc stuff
    //
    for (i = 0; i < MAXCEILINGS; i++)
        activeceilings[i] = NULL;
    for (i = 0; i < MAXPLATS; i++)
        activeplats[i] = NULL;
    for (i = 0; i < MAXBUTTONS; i++)
        memset(&buttonlist[i], 0, sizeof(button_t));
}
