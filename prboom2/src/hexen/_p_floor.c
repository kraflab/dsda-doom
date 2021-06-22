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
// T_BuildPillar
//
//=========================================================================

void T_BuildPillar(pillar_t * pillar)
{
    result_e res1;
    result_e res2;

    // First, raise the floor
    res1 = T_MovePlane(pillar->sector, pillar->floorSpeed, pillar->floordest, pillar->crush, 0, pillar->direction);     // floorOrCeiling, direction
    // Then, lower the ceiling
    res2 = T_MovePlane(pillar->sector, pillar->ceilingSpeed,
                       pillar->ceilingdest, pillar->crush, 1,
                       -pillar->direction);
    if (res1 == pastdest && res2 == pastdest)
    {
        pillar->sector->floordata = NULL;
        SN_StopSequence((mobj_t *) & pillar->sector->soundorg);
        P_TagFinished(pillar->sector->tag);
        P_RemoveThinker(&pillar->thinker);
    }
}

//=========================================================================
//
// EV_BuildPillar
//
//=========================================================================

int EV_BuildPillar(line_t * line, byte * args, dboolean crush)
{
    int secnum;
    sector_t *sec;
    pillar_t *pillar;
    int newHeight;
    int rtn;

    rtn = 0;
    secnum = -1;
    while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->floordata)
            continue;           // already moving
        if (sec->floorheight == sec->ceilingheight)
        {                       // pillar is already closed
            continue;
        }
        rtn = 1;
        if (!args[2])
        {
            newHeight = sec->floorheight +
                ((sec->ceilingheight - sec->floorheight) / 2);
        }
        else
        {
            newHeight = sec->floorheight + (args[2] << FRACBITS);
        }

        pillar = Z_Malloc(sizeof(*pillar), PU_LEVEL, 0);
        sec->floordata = pillar;
        P_AddThinker(&pillar->thinker);
        pillar->thinker.function = T_BuildPillar;
        pillar->sector = sec;
        if (!args[2])
        {
            pillar->ceilingSpeed = pillar->floorSpeed =
                args[1] * (FRACUNIT / 8);
        }
        else if (newHeight - sec->floorheight >
                 sec->ceilingheight - newHeight)
        {
            pillar->floorSpeed = args[1] * (FRACUNIT / 8);
            pillar->ceilingSpeed = FixedMul(sec->ceilingheight - newHeight,
                                            FixedDiv(pillar->floorSpeed,
                                                     newHeight -
                                                     sec->floorheight));
        }
        else
        {
            pillar->ceilingSpeed = args[1] * (FRACUNIT / 8);
            pillar->floorSpeed = FixedMul(newHeight - sec->floorheight,
                                          FixedDiv(pillar->ceilingSpeed,
                                                   sec->ceilingheight -
                                                   newHeight));
        }
        pillar->floordest = newHeight;
        pillar->ceilingdest = newHeight;
        pillar->direction = 1;
        pillar->crush = crush * args[3];
        SN_StartSequence((mobj_t *) & pillar->sector->soundorg,
                         SEQ_PLATFORM + pillar->sector->seqType);
    }
    return rtn;
}

//=========================================================================
//
// EV_OpenPillar
//
//=========================================================================

int EV_OpenPillar(line_t * line, byte * args)
{
    int secnum;
    sector_t *sec;
    pillar_t *pillar;
    int rtn;

    rtn = 0;
    secnum = -1;
    while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->floordata)
            continue;           // already moving
        if (sec->floorheight != sec->ceilingheight)
        {                       // pillar isn't closed
            continue;
        }
        rtn = 1;
        pillar = Z_Malloc(sizeof(*pillar), PU_LEVEL, 0);
        sec->floordata = pillar;
        P_AddThinker(&pillar->thinker);
        pillar->thinker.function = T_BuildPillar;
        pillar->sector = sec;
        if (!args[2])
        {
            pillar->floordest = P_FindLowestFloorSurrounding(sec);
        }
        else
        {
            pillar->floordest = sec->floorheight - (args[2] << FRACBITS);
        }
        if (!args[3])
        {
            pillar->ceilingdest = P_FindHighestCeilingSurrounding(sec);
        }
        else
        {
            pillar->ceilingdest = sec->ceilingheight + (args[3] << FRACBITS);
        }
        if (sec->floorheight - pillar->floordest >= pillar->ceilingdest -
            sec->ceilingheight)
        {
            pillar->floorSpeed = args[1] * (FRACUNIT / 8);
            pillar->ceilingSpeed = FixedMul(sec->ceilingheight -
                                            pillar->ceilingdest,
                                            FixedDiv(pillar->floorSpeed,
                                                     pillar->floordest -
                                                     sec->floorheight));
        }
        else
        {
            pillar->ceilingSpeed = args[1] * (FRACUNIT / 8);
            pillar->floorSpeed =
                FixedMul(pillar->floordest - sec->floorheight,
                         FixedDiv(pillar->ceilingSpeed,
                                  sec->ceilingheight - pillar->ceilingdest));
        }
        pillar->direction = -1; // open the pillar
        SN_StartSequence((mobj_t *) & pillar->sector->soundorg,
                         SEQ_PLATFORM + pillar->sector->seqType);
    }
    return rtn;
}

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
