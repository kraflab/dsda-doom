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
#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"

//==================================================================
//==================================================================
//
//                                                              FLOORS
//
//==================================================================
//==================================================================

//==================================================================
//
//      MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
//==================================================================
void T_MoveFloor(floormove_t * floor)
{
    result_e res;

    res = T_MovePlane(floor->sector, floor->speed,
                      floor->floordestheight, floor->crush, 0,
                      floor->direction);
    if (!(leveltime & 7))
    {
        S_StartSound(&floor->sector->soundorg, sfx_dormov);
    }

    if (res == pastdest)
    {
        floor->sector->specialdata = NULL;
        if (floor->type == raiseBuildStep)
        {
            S_StartSound(&floor->sector->soundorg, sfx_pstop);
        }
        if (floor->direction == 1)
            switch (floor->type)
            {
                case donutRaise:
                    floor->sector->special = floor->newspecial;
                    floor->sector->floorpic = floor->texture;
                default:
                    break;
            }
        else if (floor->direction == -1)
            switch (floor->type)
            {
                case lowerAndChange:
                    floor->sector->special = floor->newspecial;
                    floor->sector->floorpic = floor->texture;
                default:
                    break;
            }
        P_RemoveThinker(&floor->thinker);
    }

}

//==================================================================
//
//      HANDLE FLOOR TYPES
//
//==================================================================
int EV_DoFloor(line_t * line, floor_e floortype)
{
    int secnum;
    int rtn;
    int i;
    sector_t *sec;
    floormove_t *floor;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {
        sec = &sectors[secnum];

        //      ALREADY MOVING?  IF SO, KEEP GOING...
        if (sec->specialdata)
            continue;

        //
        //      new floor thinker
        //
        rtn = 1;
        floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker(&floor->thinker);
        sec->specialdata = floor;
        floor->thinker.function = T_MoveFloor;
        floor->type = floortype;
        floor->crush = false;
        switch (floortype)
        {
            case lowerFloor:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindHighestFloorSurrounding(sec);
                break;
            case lowerFloorToLowest:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindLowestFloorSurrounding(sec);
                break;
            case turboLower:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED * 4;
                floor->floordestheight = (8 * FRACUNIT) +
                    P_FindHighestFloorSurrounding(sec);
                break;
            case raiseFloorCrush:
                floor->crush = true;
            case raiseFloor:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
                if (floor->floordestheight > sec->ceilingheight)
                    floor->floordestheight = sec->ceilingheight;
                floor->floordestheight -= (8 * FRACUNIT) *
                    (floortype == raiseFloorCrush);
                break;
            case raiseFloorToNearest:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight =
                    P_FindNextHighestFloor(sec, sec->floorheight);
                break;
            case raiseFloor24:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = floor->sector->floorheight +
                    24 * FRACUNIT;
                break;
            case raiseFloor24AndChange:
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = floor->sector->floorheight +
                    24 * FRACUNIT;
                sec->floorpic = line->frontsector->floorpic;
                sec->special = line->frontsector->special;
                break;
            case raiseToTexture:
                {
                    int minsize = INT_MAX;
                    side_t *side;

                    floor->direction = 1;
                    floor->sector = sec;
                    floor->speed = FLOORSPEED;
                    for (i = 0; i < sec->linecount; i++)
                        if (twoSided(secnum, i))
                        {
                            side = getSide(secnum, i, 0);
                            if (side->bottomtexture >= 0)
                                if (textureheight[side->bottomtexture] <
                                    minsize)
                                    minsize =
                                        textureheight[side->bottomtexture];
                            side = getSide(secnum, i, 1);
                            if (side->bottomtexture >= 0)
                                if (textureheight[side->bottomtexture] <
                                    minsize)
                                    minsize =
                                        textureheight[side->bottomtexture];
                        }
                    floor->floordestheight = floor->sector->floorheight +
                        minsize;
                }
                break;
            case lowerAndChange:
                floor->direction = -1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;
                floor->floordestheight = P_FindLowestFloorSurrounding(sec);
                floor->texture = sec->floorpic;
                for (i = 0; i < sec->linecount; i++)
                    if (twoSided(secnum, i))
                    {
                        if (getSide(secnum, i, 0)->sector - sectors == secnum)
                        {
                            sec = getSector(secnum, i, 1);
                            floor->texture = sec->floorpic;
                            floor->newspecial = sec->special;
                            break;
                        }
                        else
                        {
                            sec = getSector(secnum, i, 0);
                            floor->texture = sec->floorpic;
                            floor->newspecial = sec->special;
                            break;
                        }
                    }
            default:
                break;
        }
    }
    return rtn;
}
