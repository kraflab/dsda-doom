
/*
==================
=
= P_ThingHeightClip
=
= Takes a valid thing and adjusts the thing->floorz, thing->ceilingz,
= anf possibly thing->z
=
= This is called for all nearby monsters whenever a sector changes height
=
= If the thing doesn't fit, the z will be set to the lowest value and
= false will be returned
==================
*/

boolean P_ThingHeightClip(mobj_t * thing)
{
    boolean onfloor;

    onfloor = (thing->z == thing->floorz);

    P_CheckPosition(thing, thing->x, thing->y);
    // what about stranding a monster partially off an edge?

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;

    if (onfloor)
        // walking monsters rise and fall with the floor
        thing->z = thing->floorz;
    else
    {                           // don't adjust a floating monster unless forced to
        if (thing->z + thing->height > thing->ceilingz)
            thing->z = thing->ceilingz - thing->height;
    }

    if (thing->ceilingz - thing->floorz < thing->height)
        return false;

    return true;
}


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

boolean PTR_SlideTraverse(intercept_t * in)
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
    {                           // the move most have hit the middle, so stairstep
      stairstep:
        if (!P_TryMove(mo, mo->x, mo->y + mo->momy))
            P_TryMove(mo, mo->x + mo->momx, mo->y);
        return;
    }

    bestslidefrac -= 0x800;     // fudge a bit to make sure it doesn't hit
    if (bestslidefrac > 0)
    {
        newx = FixedMul(mo->momx, bestslidefrac);
        newy = FixedMul(mo->momy, bestslidefrac);
        if (!P_TryMove(mo, mo->x + newx, mo->y + newy))
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

    if (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove))
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

boolean PTR_AimTraverse(intercept_t * in)
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
        return true;            // corpse or something
    if (th->type == MT_POD)
    {                           // Can't auto-aim at pods
        return (true);
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

boolean PTR_ShootTraverse(intercept_t * in)
{
    fixed_t x, y, z;
    fixed_t frac;
    line_t *li;
    mobj_t *th;
    fixed_t slope;
    fixed_t dist;
    fixed_t thingtopslope, thingbottomslope;
    mobj_t *mo;

    if (in->isaline)
    {
        li = in->d.line;
        if (li->special)
            P_ShootSpecialLine(shootthing, li);
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
    if (th->flags & MF_SHADOW && shootthing->player->readyweapon == wp_staff)
    {
        return (true);
    }

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
    if (PuffType == MT_BLASTERPUFF1)
    {                           // Make blaster big puff
        mo = P_SpawnMobj(x, y, z, MT_BLASTERPUFF2);
        S_StartSound(mo, sfx_blshit);
    }
    else
    {
        P_SpawnPuff(x, y, z);
    }
    if (la_damage)
    {
        if (!(in->d.thing->flags & MF_NOBLOOD) && P_Random() < 192)
        {
            P_BloodSplatter(x, y, z, in->d.thing);
        }
        P_DamageMobj(th, shootthing, shootthing, la_damage);
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
    if (t1->flags2 & MF2_FEETARECLIPPED)
    {
        shootz -= FOOTCLIPSIZE;
    }
    attackrange = distance;
    aimslope = slope;

    P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS,
                   PTR_ShootTraverse);
}



/*
==============================================================================

							USE LINES

==============================================================================
*/

mobj_t *usething;

boolean PTR_UseTraverse(intercept_t * in)
{
    if (!in->d.line->special)
    {
        P_LineOpening(in->d.line);
        if (openrange <= 0)
        {
            //S_StartSound (usething, sfx_noway);
            return false;       // can't use through a wall
        }
        return true;            // not a special line, but keep checking
    }

    if (P_PointOnLineSide(usething->x, usething->y, in->d.line) == 1)
        return false;           // don't use back sides

    P_UseSpecialLine(usething, in->d.line);

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



/*
==============================================================================

							RADIUS ATTACK

==============================================================================
*/

mobj_t *bombsource;
mobj_t *bombspot;
int bombdamage;

/*
=================
=
= PIT_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

boolean PIT_RadiusAttack(mobj_t * thing)
{
    fixed_t dx, dy, dist;

    if (!(thing->flags & MF_SHOOTABLE))
    {
        return true;
    }
    if (thing->type == MT_MINOTAUR || thing->type == MT_SORCERER1
        || thing->type == MT_SORCERER2)
    {                           // Episode 2 and 3 bosses take no damage from PIT_RadiusAttack
        return (true);
    }
    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);
    dist = dx > dy ? dx : dy;
    dist = (dist - thing->radius) >> FRACBITS;
    if (dist < 0)
    {
        dist = 0;
    }
    if (dist >= bombdamage)
    {                           // Out of range
        return true;
    }
    if (P_CheckSight(thing, bombspot))
    {                           // OK to damage, target is in direct path
        P_DamageMobj(thing, bombspot, bombsource, bombdamage - dist);
    }
    return (true);
}

/*
=================
=
= P_RadiusAttack
=
= Source is the creature that casued the explosion at spot
=================
*/

void P_RadiusAttack(mobj_t * spot, mobj_t * source, int damage)
{
    int x, y, xl, xh, yl, yh;
    fixed_t dist;

    dist = (damage + MAXRADIUS) << FRACBITS;
    yh = (spot->y + dist - bmaporgy) >> MAPBLOCKSHIFT;
    yl = (spot->y - dist - bmaporgy) >> MAPBLOCKSHIFT;
    xh = (spot->x + dist - bmaporgx) >> MAPBLOCKSHIFT;
    xl = (spot->x - dist - bmaporgx) >> MAPBLOCKSHIFT;
    bombspot = spot;
    if (spot->type == MT_POD && spot->target)
    {
        bombsource = spot->target;
    }
    else
    {
        bombsource = source;
    }
    bombdamage = damage;
    for (y = yl; y <= yh; y++)
        for (x = xl; x <= xh; x++)
            P_BlockThingsIterator(x, y, PIT_RadiusAttack);
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

boolean crushchange;
boolean nofit;

/*
===============
=
= PIT_ChangeSector
=
===============
*/

boolean PIT_ChangeSector(mobj_t * thing)
{
    mobj_t *mo;

    if (P_ThingHeightClip(thing))
        return true;            // keep checking

    // crunch bodies to giblets
    if (thing->health <= 0)
    {
        //P_SetMobjState (thing, S_GIBS);
        thing->height = 0;
        thing->radius = 0;
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
        P_DamageMobj(thing, NULL, NULL, 10);
        // spray blood in a random direction
        mo = P_SpawnMobj(thing->x, thing->y, thing->z + thing->height / 2,
                         MT_BLOOD);
        mo->momx = P_SubRandom() << 12;
        mo->momy = P_SubRandom() << 12;
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

boolean P_ChangeSector(sector_t * sector, boolean crunch)
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
