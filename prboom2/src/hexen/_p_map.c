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

						SECTOR HEIGHT CHANGING

= After modifying a sectors floor or ceiling height, call this
= routine to adjust the positions of all things that touch the
= sector.
=
= If anything doesn't fit anymore, true will be returned.
= If crunch is true, they will take damage as they are being crushed
= If Crunch is false, you should set the sector height back the way it
= was and call P_ChangeSector again to undo the changes
==============================================================================
*/

int crushchange;
dboolean nofit;

/*
===============
=
= PIT_ChangeSector
=
===============
*/

dboolean PIT_ChangeSector(mobj_t * thing)
{
    mobj_t *mo;

    if (P_ThingHeightClip(thing))
        return true;            // keep checking

    // crunch bodies to giblets
    if ((thing->flags & MF_CORPSE) && (thing->health <= 0))
    {
        if (thing->flags & MF_NOBLOOD)
        {
            P_RemoveMobj(thing);
        }
        else
        {
            if (thing->state != &states[HEXEN_S_GIBS1])
            {
                P_SetMobjState(thing, HEXEN_S_GIBS1);
                thing->height = 0;
                thing->radius = 0;
                S_StartSound(thing, hexen_sfx_player_falling_splat);
            }
        }
        return true;            // keep checking
    }

    // crunch dropped items
    if (thing->flags & MF_DROPPED)
    {
        P_RemoveMobj(thing);
        return true;            // keep checking
    }

    if (!(thing->flags & MF_SHOOTABLE))
        return true;            // assume it is bloody gibs or something

    nofit = true;
    if (crushchange && !(leveltime & 3))
    {
        P_DamageMobj(thing, NULL, NULL, crushchange);
        // spray blood in a random direction
        if ((!(thing->flags & MF_NOBLOOD)) &&
            (!(thing->flags2 & MF2_INVULNERABLE)))
        {
            mo = P_SpawnMobj(thing->x, thing->y, thing->z + thing->height / 2,
                             HEXEN_MT_BLOOD);
            mo->momx = P_SubRandom() << 12;
            mo->momy = P_SubRandom() << 12;
        }
    }

    return true;                // keep checking (crush other things)
}

/*
===============
=
= P_ChangeSector
=
===============
*/

dboolean P_ChangeSector(sector_t * sector, int crunch)
{
    int x, y;

    nofit = false;
    crushchange = crunch;

// recheck heights for all things near the moving sector

    for (x = sector->blockbox[BOXLEFT]; x <= sector->blockbox[BOXRIGHT]; x++)
        for (y = sector->blockbox[BOXBOTTOM]; y <= sector->blockbox[BOXTOP];
             y++)
            P_BlockThingsIterator(x, y, PIT_ChangeSector);


    return nofit;
}
