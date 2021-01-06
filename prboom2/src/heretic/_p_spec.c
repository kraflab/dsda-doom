
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
