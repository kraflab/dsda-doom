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
            case 1:            // Phased light
                // Hardcoded base, use sector->lightlevel as the index
                P_SpawnPhasedLight(sector, 80, -1);
                break;
            case 2:            // Phased light sequence start
                P_SpawnLightSequence(sector, 1);
                break;
                // Specials 3 & 4 are used by the phased light sequences

                /*
                   case 1:         // FLICKERING LIGHTS
                   P_SpawnLightFlash (sector);
                   break;
                   case 2:         // STROBE FAST
                   P_SpawnStrobeFlash(sector,FASTDARK,0);
                   break;
                   case 3:         // STROBE SLOW
                   P_SpawnStrobeFlash(sector,SLOWDARK,0);
                   break;
                   case 4:         // STROBE FAST/DEATH SLIME
                   P_SpawnStrobeFlash(sector,FASTDARK,0);
                   sector->special = 4;
                   break;
                   case 8:         // GLOWING LIGHT
                   P_SpawnGlowingLight(sector);
                   break;
                   case 9:         // SECRET SECTOR
                   totalsecret++;
                   break;
                   case 10:        // DOOR CLOSE IN 30 SECONDS
                   P_SpawnDoorCloseIn30 (sector);
                   break;
                   case 12:        // SYNC STROBE SLOW
                   P_SpawnStrobeFlash (sector, SLOWDARK, 1);
                   break;
                   case 13:        // SYNC STROBE FAST
                   P_SpawnStrobeFlash (sector, FASTDARK, 1);
                   break;
                   case 14:        // DOOR RAISE IN 5 MINUTES
                   P_SpawnDoorRaiseIn5Mins (sector, i);
                   break;
                 */
        }
    }


    //
    //      Init line EFFECTs
    //
    numlinespecials = 0;
    TaggedLineCount = 0;
    for (i = 0; i < numlines; i++)
    {
        switch (lines[i].special)
        {
            case 100:          // Scroll_Texture_Left
            case 101:          // Scroll_Texture_Right
            case 102:          // Scroll_Texture_Up
            case 103:          // Scroll_Texture_Down
                linespeciallist[numlinespecials] = &lines[i];
                numlinespecials++;
                break;
            case 121:          // Line_SetIdentification
                if (lines[i].arg1)
                {
                    if (TaggedLineCount == MAX_TAGGED_LINES)
                    {
                        I_Error("P_SpawnSpecials: MAX_TAGGED_LINES "
                                "(%d) exceeded.", MAX_TAGGED_LINES);
                    }
                    TaggedLines[TaggedLineCount].line = &lines[i];
                    TaggedLines[TaggedLineCount++].lineTag = lines[i].arg1;
                }
                lines[i].special = 0;
                break;
        }
    }

    //
    //      Init other misc stuff
    //
    P_RemoveAllActiveCeilings();
    P_RemoveAllActivePlats();
    for (i = 0; i < MAXBUTTONS; i++)
        memset(&buttonlist[i], 0, sizeof(button_t));

    // Initialize flat and texture animations
    P_InitFTAnims();
}
