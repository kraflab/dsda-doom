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

void T_PlatRaise(plat_t * plat)
{
    result_e res;

    switch (plat->status)
    {
        case up:
            res = T_MovePlane(plat->sector, plat->speed,
                              plat->high, plat->crush, 0, 1);
            if (res == crushed && (!plat->crush))
            {
                plat->count = plat->wait;
                plat->status = down;
                SN_StartSequence((mobj_t *) & plat->sector->soundorg,
                                 SEQ_PLATFORM + plat->sector->seqType);
            }
            else if (res == pastdest)
            {
                plat->count = plat->wait;
                plat->status = waiting;
                SN_StopSequence((mobj_t *) & plat->sector->soundorg);
                switch (plat->type)
                {
                    case PLAT_DOWNWAITUPSTAY:
                    case PLAT_DOWNBYVALUEWAITUPSTAY:
                        P_RemoveActivePlat(plat);
                        break;
                    default:
                        break;
                }
            }
            break;
        case down:
            res =
                T_MovePlane(plat->sector, plat->speed, plat->low, false, 0,
                            -1);
            if (res == pastdest)
            {
                plat->count = plat->wait;
                plat->status = waiting;
                switch (plat->type)
                {
                    case PLAT_UPWAITDOWNSTAY:
                    case PLAT_UPBYVALUEWAITDOWNSTAY:
                        P_RemoveActivePlat(plat);
                        break;
                    default:
                        break;
                }
                SN_StopSequence((mobj_t *) & plat->sector->soundorg);
            }
            break;
        case waiting:
            if (!--plat->count)
            {
                if (plat->sector->floorheight == plat->low)
                    plat->status = up;
                else
                    plat->status = down;
                SN_StartSequence((mobj_t *) & plat->sector->soundorg,
                                 SEQ_PLATFORM + plat->sector->seqType);
            }
    }
}

//==================================================================
//
//      Do Platforms
//      "amount" is only used for SOME platforms.
//
//==================================================================
int EV_DoPlat(line_t * line, byte * args, plattype_e type, int amount)
{
    plat_t *plat;
    int secnum;
    int rtn;
    sector_t *sec;

    secnum = -1;
    rtn = 0;

    while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
    {
        sec = &sectors[secnum];
        if (sec->specialdata)
            continue;

        //
        // Find lowest & highest floors around sector
        //
        rtn = 1;
        plat = Z_Malloc(sizeof(*plat), PU_LEVEL, 0);
        P_AddThinker(&plat->thinker);

        plat->type = type;
        plat->sector = sec;
        plat->sector->specialdata = plat;
        plat->thinker.function = T_PlatRaise;
        plat->crush = false;
        plat->tag = args[0];
        plat->speed = args[1] * (FRACUNIT / 8);
        switch (type)
        {
            case PLAT_DOWNWAITUPSTAY:
                plat->low = P_FindLowestFloorSurrounding(sec) + 8 * FRACUNIT;
                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;
                plat->high = sec->floorheight;
                plat->wait = args[2];
                plat->status = down;
                break;
            case PLAT_DOWNBYVALUEWAITUPSTAY:
                plat->low = sec->floorheight - args[3] * 8 * FRACUNIT;
                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;
                plat->high = sec->floorheight;
                plat->wait = args[2];
                plat->status = down;
                break;
            case PLAT_UPWAITDOWNSTAY:
                plat->high = P_FindHighestFloorSurrounding(sec);
                if (plat->high < sec->floorheight)
                    plat->high = sec->floorheight;
                plat->low = sec->floorheight;
                plat->wait = args[2];
                plat->status = up;
                break;
            case PLAT_UPBYVALUEWAITDOWNSTAY:
                plat->high = sec->floorheight + args[3] * 8 * FRACUNIT;
                if (plat->high < sec->floorheight)
                    plat->high = sec->floorheight;
                plat->low = sec->floorheight;
                plat->wait = args[2];
                plat->status = up;
                break;
            case PLAT_PERPETUALRAISE:
                plat->low = P_FindLowestFloorSurrounding(sec) + 8 * FRACUNIT;
                if (plat->low > sec->floorheight)
                    plat->low = sec->floorheight;
                plat->high = P_FindHighestFloorSurrounding(sec);
                if (plat->high < sec->floorheight)
                    plat->high = sec->floorheight;
                plat->wait = args[2];
                plat->status = P_Random(pr_hexen) & 1;
                break;
        }
        P_AddActivePlat(plat);
        SN_StartSequence((mobj_t *) & sec->soundorg,
                         SEQ_PLATFORM + sec->seqType);
    }
    return rtn;
}

void EV_StopPlat(line_t * line, byte * args)
{
    int i;

    for (i = 0; i < MAXPLATS; i++)
    {
        activeplats[i]->tag = args[0];

        if (activeplats[i]->tag != 0)
        {
            activeplats[i]->sector->specialdata = NULL;
            P_TagFinished(activeplats[i]->sector->tag);
            P_RemoveThinker(&activeplats[i]->thinker);
            activeplats[i] = NULL;

            return;
        }
    }
}
