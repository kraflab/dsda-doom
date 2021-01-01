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

// P_telept.c

#include "doomdef.h"
#include "p_local.h"
#include "s_sound.h"
#include "v_video.h"

//----------------------------------------------------------------------------
//
// FUNC EV_Teleport
//
//----------------------------------------------------------------------------

boolean EV_Teleport(line_t * line, int side, mobj_t * thing)
{
    int i;
    int tag;
    mobj_t *m;
    thinker_t *thinker;
    sector_t *sector;

    if (thing->flags2 & MF2_NOTELEPORT)
    {
        return (false);
    }
    if (side == 1)
    {                           // Don't teleport when crossing back side
        return (false);
    }
    tag = line->tag;
    for (i = 0; i < numsectors; i++)
    {
        if (sectors[i].tag == tag)
        {
            for (thinker = thinkercap.next; thinker != &thinkercap;
                 thinker = thinker->next)
            {
                if (thinker->function != P_MobjThinker)
                {               // Not a mobj
                    continue;
                }
                m = (mobj_t *) thinker;
                if (m->type != MT_TELEPORTMAN)
                {               // Not a teleportman
                    continue;
                }
                sector = m->subsector->sector;
                if (sector - sectors != i)
                {               // Wrong sector
                    continue;
                }
                return (P_Teleport(thing, m->x, m->y, m->angle));
            }
        }
    }
    return (false);
}
