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

//=========================================================================
//
// EV_FloorCrushStop
//
//=========================================================================

int EV_FloorCrushStop(line_t * line, byte * args)
{
    thinker_t *think;
    floormove_t *floor;
    dboolean rtn;

    rtn = 0;
    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != T_MoveFloor)
        {
            continue;
        }
        floor = (floormove_t *) think;
        if (floor->type != FLEV_RAISEFLOORCRUSH)
        {
            continue;
        }
        // Completely remove the crushing floor
        SN_StopSequence((mobj_t *) & floor->sector->soundorg);
        floor->sector->floordata = NULL;
        P_TagFinished(floor->sector->tag);
        P_RemoveThinker(&floor->thinker);
        rtn = 1;
    }
    return rtn;
}

//==========================================================================
//
// T_FloorWaggle
//
//==========================================================================

#define WGLSTATE_EXPAND 1
#define WGLSTATE_STABLE 2
#define WGLSTATE_REDUCE 3

extern fixed_t FloatBobOffsets[64];

void T_FloorWaggle(floorWaggle_t * waggle)
{
    switch (waggle->state)
    {
        case WGLSTATE_EXPAND:
            if ((waggle->scale += waggle->scaleDelta) >= waggle->targetScale)
            {
                waggle->scale = waggle->targetScale;
                waggle->state = WGLSTATE_STABLE;
            }
            break;
        case WGLSTATE_REDUCE:
            if ((waggle->scale -= waggle->scaleDelta) <= 0)
            {                   // Remove
                waggle->sector->floorheight = waggle->originalHeight;
                P_ChangeSector(waggle->sector, true);
                waggle->sector->floordata = NULL;
                P_TagFinished(waggle->sector->tag);
                P_RemoveThinker(&waggle->thinker);
                return;
            }
            break;
        case WGLSTATE_STABLE:
            if (waggle->ticker != -1)
            {
                if (!--waggle->ticker)
                {
                    waggle->state = WGLSTATE_REDUCE;
                }
            }
            break;
    }
    waggle->accumulator += waggle->accDelta;
    waggle->sector->floorheight = waggle->originalHeight
        + FixedMul(FloatBobOffsets[(waggle->accumulator >> FRACBITS) & 63],
                   waggle->scale);
    P_ChangeSector(waggle->sector, true);
}

//==========================================================================
//
// EV_StartFloorWaggle
//
//==========================================================================

dboolean EV_StartFloorWaggle(int tag, int height, int speed, int offset,
                            int timer)
{
    int sectorIndex;
    sector_t *sector;
    floorWaggle_t *waggle;
    dboolean retCode;

    retCode = false;
    sectorIndex = -1;
    while ((sectorIndex = P_FindSectorFromTag(tag, sectorIndex)) >= 0)
    {
        sector = &sectors[sectorIndex];
        if (sector->floordata)
        {                       // Already busy with another thinker
            continue;
        }
        retCode = true;
        waggle = Z_Malloc(sizeof(*waggle), PU_LEVEL, 0);
        sector->floordata = waggle;
        waggle->thinker.function = T_FloorWaggle;
        waggle->sector = sector;
        waggle->originalHeight = sector->floorheight;
        waggle->accumulator = offset * FRACUNIT;
        waggle->accDelta = speed << 10;
        waggle->scale = 0;
        waggle->targetScale = height << 10;
        waggle->scaleDelta = waggle->targetScale
            / (35 + ((3 * 35) * height) / 255);
        waggle->ticker = timer ? timer * 35 : -1;
        waggle->state = WGLSTATE_EXPAND;
        P_AddThinker(&waggle->thinker);
    }
    return retCode;
}
