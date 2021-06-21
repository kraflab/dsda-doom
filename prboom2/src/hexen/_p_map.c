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

							USE LINES

==============================================================================
*/

mobj_t *usething;

dboolean PTR_UseTraverse(intercept_t * in)
{
    int sound;
    fixed_t pheight;

    if (!in->d.line->special)
    {
        P_LineOpening(in->d.line);
        if (openrange <= 0)
        {
            if (usething->player)
            {
                switch (usething->player->pclass)
                {
                    case PCLASS_FIGHTER:
                        sound = hexen_sfx_player_fighter_failed_use;
                        break;
                    case PCLASS_CLERIC:
                        sound = hexen_sfx_player_cleric_failed_use;
                        break;
                    case PCLASS_MAGE:
                        sound = hexen_sfx_player_mage_failed_use;
                        break;
                    case PCLASS_PIG:
                        sound = hexen_sfx_pig_active1;
                        break;
                    default:
                        sound = hexen_sfx_None;
                        break;
                }
                S_StartSound(usething, sound);
            }
            return false;       // can't use through a wall
        }
        if (usething->player)
        {
            pheight = usething->z + (usething->height / 2);
            if ((opentop < pheight) || (openbottom > pheight))
            {
                switch (usething->player->pclass)
                {
                    case PCLASS_FIGHTER:
                        sound = hexen_sfx_player_fighter_failed_use;
                        break;
                    case PCLASS_CLERIC:
                        sound = hexen_sfx_player_cleric_failed_use;
                        break;
                    case PCLASS_MAGE:
                        sound = hexen_sfx_player_mage_failed_use;
                        break;
                    case PCLASS_PIG:
                        sound = hexen_sfx_pig_active1;
                        break;
                    default:
                        sound = hexen_sfx_None;
                        break;
                }
                S_StartSound(usething, sound);
            }
        }
        return true;            // not a special line, but keep checking
    }

    if (P_PointOnLineSide(usething->x, usething->y, in->d.line) == 1)
        return false;           // don't use back sides

//      P_UseSpecialLine (usething, in->d.line);
    P_ActivateLine(in->d.line, usething, 0, SPAC_USE);

    return false;               // can't use for than one special line in a row
}


/*
================
=
= P_UseLines
=
= Looks for special lines in front of the player to activate
================
*/

void P_UseLines(player_t * player)
{
    int angle;
    fixed_t x1, y1, x2, y2;

    usething = player->mo;

    angle = player->mo->angle >> ANGLETOFINESHIFT;
    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE >> FRACBITS) * finecosine[angle];
    y2 = y1 + (USERANGE >> FRACBITS) * finesine[angle];

    P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse);
}

//==========================================================================
//
// PTR_PuzzleItemTraverse
//
//==========================================================================

#define USE_PUZZLE_ITEM_SPECIAL 129

static mobj_t *PuzzleItemUser;
static int PuzzleItemType;
static dboolean PuzzleActivated;

dboolean PTR_PuzzleItemTraverse(intercept_t * in)
{
    mobj_t *mobj;
    byte args[3];
    int sound;

    if (in->isaline)
    {                           // Check line
        if (in->d.line->special != USE_PUZZLE_ITEM_SPECIAL)
        {
            P_LineOpening(in->d.line);
            if (openrange <= 0)
            {
                sound = hexen_sfx_None;
                if (PuzzleItemUser->player)
                {
                    switch (PuzzleItemUser->player->pclass)
                    {
                        case PCLASS_FIGHTER:
                            sound = hexen_sfx_puzzle_fail_fighter;
                            break;
                        case PCLASS_CLERIC:
                            sound = hexen_sfx_puzzle_fail_cleric;
                            break;
                        case PCLASS_MAGE:
                            sound = hexen_sfx_puzzle_fail_mage;
                            break;
                        default:
                            sound = hexen_sfx_None;
                            break;
                    }
                }
                S_StartSound(PuzzleItemUser, sound);
                return false;   // can't use through a wall
            }
            return true;        // Continue searching
        }
        if (P_PointOnLineSide(PuzzleItemUser->x, PuzzleItemUser->y,
                              in->d.line) == 1)
        {                       // Don't use back sides
            return false;
        }
        if (PuzzleItemType != in->d.line->arg1)
        {                       // Item type doesn't match
            return false;
        }

        // Construct an args[] array that would contain the values from
        // the line that would be passed by Vanilla Hexen.
        args[0] = in->d.line->arg3;
        args[1] = in->d.line->arg4;
        args[2] = in->d.line->arg5;

        P_StartACS(in->d.line->arg2, 0, args, PuzzleItemUser, in->d.line, 0);
        in->d.line->special = 0;
        PuzzleActivated = true;
        return false;           // Stop searching
    }
    // Check thing
    mobj = in->d.thing;
    if (mobj->special != USE_PUZZLE_ITEM_SPECIAL)
    {                           // Wrong special
        return true;
    }
    if (PuzzleItemType != mobj->args[0])
    {                           // Item type doesn't match
        return true;
    }

    P_StartACS(mobj->args[1], 0, &mobj->args[2], PuzzleItemUser, NULL, 0);
    mobj->special = 0;
    PuzzleActivated = true;
    return false;               // Stop searching
}

//==========================================================================
//
// P_UsePuzzleItem
//
// Returns true if the puzzle item was used on a line or a thing.
//
//==========================================================================

dboolean P_UsePuzzleItem(player_t * player, int itemType)
{
    int angle;
    fixed_t x1, y1, x2, y2;

    PuzzleItemType = itemType;
    PuzzleItemUser = player->mo;
    PuzzleActivated = false;
    angle = player->mo->angle >> ANGLETOFINESHIFT;
    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE >> FRACBITS) * finecosine[angle];
    y2 = y1 + (USERANGE >> FRACBITS) * finesine[angle];
    P_PathTraverse(x1, y1, x2, y2, PT_ADDLINES | PT_ADDTHINGS,
                   PTR_PuzzleItemTraverse);
    return PuzzleActivated;
}

/*
==============================================================================

							RADIUS ATTACK

==============================================================================
*/

mobj_t *bombsource;
mobj_t *bombspot;
int bombdamage;
int bombdistance;
dboolean DamageSource;

/*
=================
=
= PIT_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

dboolean PIT_RadiusAttack(mobj_t * thing)
{
    fixed_t dx, dy, dist;
    int damage;

    if (!(thing->flags & MF_SHOOTABLE))
    {
        return true;
    }
//      if(thing->flags2&MF2_BOSS)
//      {       // Bosses take no damage from PIT_RadiusAttack
//              return(true);
//      }
    if (!DamageSource && thing == bombsource)
    {                           // don't damage the source of the explosion
        return true;
    }
    if (abs((thing->z - bombspot->z) >> FRACBITS) > 2 * bombdistance)
    {                           // too high/low
        return true;
    }
    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);
    dist = dx > dy ? dx : dy;
    dist = (dist - thing->radius) >> FRACBITS;
    if (dist < 0)
    {
        dist = 0;
    }
    if (dist >= bombdistance)
    {                           // Out of range
        return true;
    }
    if (P_CheckSight(thing, bombspot))
    {                           // OK to damage, target is in direct path
        damage = (bombdamage * (bombdistance - dist) / bombdistance) + 1;
        if (thing->player)
        {
            damage >>= 2;
        }
        P_DamageMobj(thing, bombspot, bombsource, damage);
    }
    return (true);
}

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
