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

							SLIDE MOVE

Allows the player to slide along any angled walls

==============================================================================
*/

fixed_t bestslidefrac, secondslidefrac;
line_t *bestslideline, *secondslideline;
mobj_t *slidemo;

fixed_t tmxmove, tmymove;

/*
==================
=
= P_HitSlideLine
=
= Adjusts the xmove / ymove so that the next move will slide along the wall
==================
*/

void P_HitSlideLine(line_t * ld)
{
    int side;
    angle_t lineangle, moveangle, deltaangle;
    fixed_t movelen, newlen;


    if (ld->slopetype == ST_HORIZONTAL)
    {
        tmymove = 0;
        return;
    }
    if (ld->slopetype == ST_VERTICAL)
    {
        tmxmove = 0;
        return;
    }

    side = P_PointOnLineSide(slidemo->x, slidemo->y, ld);

    lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);
    if (side == 1)
        lineangle += ANG180;
    moveangle = R_PointToAngle2(0, 0, tmxmove, tmymove);
    deltaangle = moveangle - lineangle;
    if (deltaangle > ANG180)
        deltaangle += ANG180;
//              I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_AproxDistance(tmxmove, tmymove);
    newlen = FixedMul(movelen, finecosine[deltaangle]);
    tmxmove = FixedMul(newlen, finecosine[lineangle]);
    tmymove = FixedMul(newlen, finesine[lineangle]);
}

/*
==============
=
= PTR_SlideTraverse
=
==============
*/

dboolean PTR_SlideTraverse(intercept_t * in)
{
    line_t *li;

    if (!in->isaline)
        I_Error("PTR_SlideTraverse: not a line?");

    li = in->d.line;
    if (!(li->flags & ML_TWOSIDED))
    {
        if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
            return true;        // don't hit the back side
        goto isblocking;
    }

    P_LineOpening(li);          // set openrange, opentop, openbottom
    if (openrange < slidemo->height)
        goto isblocking;        // doesn't fit

    if (opentop - slidemo->z < slidemo->height)
        goto isblocking;        // mobj is too high

    if (openbottom - slidemo->z > 24 * FRACUNIT)
        goto isblocking;        // too big a step up

    return true;                // this line doesn't block movement

// the line does block movement, see if it is closer than best so far
  isblocking:
    if (in->frac < bestslidefrac)
    {
        secondslidefrac = bestslidefrac;
        secondslideline = bestslideline;
        bestslidefrac = in->frac;
        bestslideline = li;
    }

    return false;               // stop
}


/*
==================
=
= P_SlideMove
=
= The momx / momy move is bad, so try to slide along a wall
=
= Find the first line hit, move flush to it, and slide along it
=
= This is a kludgy mess.
==================
*/

void P_SlideMove(mobj_t * mo)
{
    fixed_t leadx, leady;
    fixed_t trailx, traily;
    fixed_t newx, newy;
    int hitcount;

    slidemo = mo;
    hitcount = 0;
  retry:
    if (++hitcount == 3)
        goto stairstep;         // don't loop forever

//
// trace along the three leading corners
//
    if (mo->momx > 0)
    {
        leadx = mo->x + mo->radius;
        trailx = mo->x - mo->radius;
    }
    else
    {
        leadx = mo->x - mo->radius;
        trailx = mo->x + mo->radius;
    }

    if (mo->momy > 0)
    {
        leady = mo->y + mo->radius;
        traily = mo->y - mo->radius;
    }
    else
    {
        leady = mo->y - mo->radius;
        traily = mo->y + mo->radius;
    }

    bestslidefrac = FRACUNIT + 1;

    P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(trailx, leady, trailx + mo->momx, leady + mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(leadx, traily, leadx + mo->momx, traily + mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);

//
// move up to the wall
//
    if (bestslidefrac == FRACUNIT + 1)
    {                           // the move must have hit the middle, so stairstep
      stairstep:
        if (!P_TryMove(mo, mo->x, mo->y + mo->momy, false))
        {
            P_TryMove(mo, mo->x + mo->momx, mo->y, false);
        }
        return;
    }

    bestslidefrac -= 0x800;     // fudge a bit to make sure it doesn't hit
    if (bestslidefrac > 0)
    {
        newx = FixedMul(mo->momx, bestslidefrac);
        newy = FixedMul(mo->momy, bestslidefrac);
        if (!P_TryMove(mo, mo->x + newx, mo->y + newy, false))
            goto stairstep;
    }

//
// now continue along the wall
//
    bestslidefrac = FRACUNIT - (bestslidefrac + 0x800); // remainder
    if (bestslidefrac > FRACUNIT)
        bestslidefrac = FRACUNIT;
    if (bestslidefrac <= 0)
        return;

    tmxmove = FixedMul(mo->momx, bestslidefrac);
    tmymove = FixedMul(mo->momy, bestslidefrac);

    P_HitSlideLine(bestslideline);      // clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;

    if (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove, false))
    {
        goto retry;
    }
}

/*
==============================================================================

							P_LineAttack

==============================================================================
*/


mobj_t *linetarget;             // who got hit (or NULL)
mobj_t *shootthing;
fixed_t shootz;                 // height if not aiming up or down
                                                                        // ???: use slope for monsters?
int la_damage;
fixed_t attackrange;

fixed_t aimslope;

extern fixed_t topslope, bottomslope;   // slopes to top and bottom of target

/*
===============================================================================
=
= PTR_AimTraverse
=
= Sets linetaget and aimslope when a target is aimed at
===============================================================================
*/

dboolean PTR_AimTraverse(intercept_t * in)
{
    line_t *li;
    mobj_t *th;
    fixed_t slope, thingtopslope, thingbottomslope;
    fixed_t dist;

    if (in->isaline)
    {
        li = in->d.line;
        if (!(li->flags & ML_TWOSIDED))
            return false;       // stop
//
// crosses a two sided line
// a two sided line will restrict the possible target ranges
        P_LineOpening(li);

        if (openbottom >= opentop)
            return false;       // stop

        dist = FixedMul(attackrange, in->frac);

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv(openbottom - shootz, dist);
            if (slope > bottomslope)
                bottomslope = slope;
        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv(opentop - shootz, dist);
            if (slope < topslope)
                topslope = slope;
        }

        if (topslope <= bottomslope)
            return false;       // stop

        return true;            // shot continues
    }

//
// shoot a thing
//
    th = in->d.thing;
    if (th == shootthing)
        return true;            // can't shoot self
    if (!(th->flags & MF_SHOOTABLE))
    {                           // corpse or something
        return true;
    }
    if (th->player && netgame && !deathmatch)
    {                           // don't aim at fellow co-op players
        return true;
    }

// check angles to see if the thing can be aimed at

    dist = FixedMul(attackrange, in->frac);
    thingtopslope = FixedDiv(th->z + th->height - shootz, dist);
    if (thingtopslope < bottomslope)
        return true;            // shot over the thing
    thingbottomslope = FixedDiv(th->z - shootz, dist);
    if (thingbottomslope > topslope)
        return true;            // shot under the thing

//
// this thing can be hit!
//
    if (thingtopslope > topslope)
        thingtopslope = topslope;
    if (thingbottomslope < bottomslope)
        thingbottomslope = bottomslope;

    aimslope = (thingtopslope + thingbottomslope) / 2;
    linetarget = th;

    return false;               // don't go any farther
}


/*
==============================================================================
=
= PTR_ShootTraverse
=
==============================================================================
*/

dboolean PTR_ShootTraverse(intercept_t * in)
{
    fixed_t x, y, z;
    fixed_t frac;
    line_t *li;
    mobj_t *th;
    fixed_t slope;
    fixed_t dist;
    fixed_t thingtopslope, thingbottomslope;

    extern mobj_t LavaInflictor;

    if (in->isaline)
    {
        li = in->d.line;
        if (li->special)
        {
            P_ActivateLine(li, shootthing, 0, SPAC_IMPACT);
//                      P_ShootSpecialLine (shootthing, li);
        }
        if (!(li->flags & ML_TWOSIDED))
            goto hitline;

//
// crosses a two sided line
//
        P_LineOpening(li);

        dist = FixedMul(attackrange, in->frac);

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv(openbottom - shootz, dist);
            if (slope > aimslope)
                goto hitline;
        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv(opentop - shootz, dist);
            if (slope < aimslope)
                goto hitline;
        }

        return true;            // shot continues
//
// hit line
//
      hitline:
        // position a bit closer
        frac = in->frac - FixedDiv(4 * FRACUNIT, attackrange);
        x = trace.x + FixedMul(trace.dx, frac);
        y = trace.y + FixedMul(trace.dy, frac);
        z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));

        if (li->frontsector->ceilingpic == skyflatnum)
        {
            if (z > li->frontsector->ceilingheight)
                return false;   // don't shoot the sky!
            if (li->backsector && li->backsector->ceilingpic == skyflatnum)
                return false;   // it's a sky hack wall
        }

        P_SpawnPuff(x, y, z);
        return false;           // don't go any farther
    }

//
// shoot a thing
//
    th = in->d.thing;
    if (th == shootthing)
        return true;            // can't shoot self
    if (!(th->flags & MF_SHOOTABLE))
        return true;            // corpse or something

//
// check for physical attacks on a ghost
//
/*  FIX:  Impliment Heretic 2 weapons here
	if(th->flags&MF_SHADOW && shootthing->player->readyweapon == wp_staff)
	{
		return(true);
	}
*/

// check angles to see if the thing can be aimed at
    dist = FixedMul(attackrange, in->frac);
    thingtopslope = FixedDiv(th->z + th->height - shootz, dist);
    if (thingtopslope < aimslope)
        return true;            // shot over the thing
    thingbottomslope = FixedDiv(th->z - shootz, dist);
    if (thingbottomslope > aimslope)
        return true;            // shot under the thing

//
// hit thing
//
    // position a bit closer
    frac = in->frac - FixedDiv(10 * FRACUNIT, attackrange);
    x = trace.x + FixedMul(trace.dx, frac);
    y = trace.y + FixedMul(trace.dy, frac);
    z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));
    P_SpawnPuff(x, y, z);
    if (la_damage)
    {
        if (!(in->d.thing->flags & MF_NOBLOOD) &&
            !(in->d.thing->flags2 & MF2_INVULNERABLE))
        {
            if (PuffType == HEXEN_MT_AXEPUFF || PuffType == HEXEN_MT_AXEPUFF_GLOW)
            {
                P_BloodSplatter2(x, y, z, in->d.thing);
            }
            if (P_Random(pr_hexen) < 192)
            {
                P_BloodSplatter(x, y, z, in->d.thing);
            }
        }
        if (PuffType == HEXEN_MT_FLAMEPUFF2)
        {                       // Cleric FlameStrike does fire damage
            P_DamageMobj(th, &LavaInflictor, shootthing, la_damage);
        }
        else
        {
            P_DamageMobj(th, shootthing, shootthing, la_damage);
        }
    }
    return (false);             // don't go any farther
}

/*
=================
=
= P_AimLineAttack
=
=================
*/

fixed_t P_AimLineAttack(mobj_t * t1, angle_t angle, fixed_t distance)
{
    fixed_t x2, y2;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
    topslope = 100 * FRACUNIT / 160;    // can't shoot outside view angles
    bottomslope = -100 * FRACUNIT / 160;
    attackrange = distance;
    linetarget = NULL;

    P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS,
                   PTR_AimTraverse);

    if (linetarget)
        return aimslope;
    return 0;
}



/*
=================
=
= P_LineAttack
=
= if damage == 0, it is just a test trace that will leave linetarget set
=
=================
*/

void P_LineAttack(mobj_t * t1, angle_t angle, fixed_t distance, fixed_t slope,
                  int damage)
{
    fixed_t x2, y2;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    la_damage = damage;
    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
    shootz -= t1->floorclip;
    attackrange = distance;
    aimslope = slope;

    if (P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS,
                       PTR_ShootTraverse))
    {
        switch (PuffType)
        {
            case HEXEN_MT_PUNCHPUFF:
                S_StartSound(t1, hexen_sfx_fighter_punch_miss);
                break;
            case HEXEN_MT_HAMMERPUFF:
            case HEXEN_MT_AXEPUFF:
            case HEXEN_MT_AXEPUFF_GLOW:
                S_StartSound(t1, hexen_sfx_fighter_hammer_miss);
                break;
            case HEXEN_MT_FLAMEPUFF:
                P_SpawnPuff(x2, y2, shootz + FixedMul(slope, distance));
                break;
            default:
                break;
        }
    }
}

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
