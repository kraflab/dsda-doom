/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2004 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Movement, collision handling.
 *  Shooting and aiming.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "r_main.h"
#include "p_mobj.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_inter.h"
#include "m_random.h"
#include "m_bbox.h"
#include "lprintf.h"
#include "g_game.h"
#include "p_tick.h"
#include "g_overflow.h"
#include "am_map.h"

#include "e6y.h"//e6y

#include "dsda.h"
#include "dsda/destructible.h"
#include "dsda/excmd.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"

#include "heretic/def.h"

static mobj_t    *tmthing;
static mobj_t    *tsthing; // hexen
static fixed_t   tmx;
static fixed_t   tmy;
static int pe_x; // Pain Elemental position for Lost Soul checks // phares
static int pe_y; // Pain Elemental position for Lost Soul checks // phares
static int ls_x; // Lost Soul position for Lost Soul checks      // phares
static int ls_y; // Lost Soul position for Lost Soul checks      // phares

//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//  to undo the changes.
//

static int crushchange;
static dboolean nofit;

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".

dboolean   floatok;

/* killough 11/98: if "felldown" true, object was pushed down ledge */
dboolean   felldown;

// The tm* items are used to hold information globally, usually for
// line or object intersection checking

fixed_t   tmbbox[4];  // bounding box for line intersection checks
fixed_t   tmfloorz;   // floor you'd hit if free to fall
fixed_t   tmceilingz; // ceiling of sector you're in
fixed_t   tmdropoffz; // dropoff on other side of line you're crossing

// heretic
int tmflags;

// hexen
int tmfloorpic;
mobj_t *BlockingMobj;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls

line_t        *ceilingline;
line_t        *blockline;    /* killough 8/11/98: blocking linedef */
line_t        *floorline;    /* killough 8/1/98: Highest touched floor */
static int    tmunstuck;     /* killough 8/1/98: whether to allow unsticking */

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid

// 1/11/98 killough: removed limit on special lines crossed
line_t **spechit;                // new code -- killough
static int spechit_max;          // killough

int numspechit;

// Temporary holder for thing_sectorlist threads
msecnode_t* sector_list = NULL;                             // phares 3/16/98

//
// TELEPORT MOVE
//

//
// PIT_StompThing
//

static dboolean telefrag;   /* killough 8/9/98: whether to telefrag at exit */

dboolean PIT_StompThing (mobj_t* thing)
{
  fixed_t blockdist;

  // phares 9/10/98: moved this self-check to start of routine

  // don't clip against self

  if (thing == tmthing)
    return true;

  if (!(thing->flags & MF_SHOOTABLE)) // Can't shoot it? Can't stomp it!
    return true;

  blockdist = thing->radius + tmthing->radius;

  if (D_abs(thing->x - tmx) >= blockdist || D_abs(thing->y - tmy) >= blockdist)
    return true; // didn't hit it

  // monsters don't stomp things except on boss level
  // killough 8/9/98: make consistent across all levels
  if (!telefrag &&
      !(map_info.flags & MI_ALLOW_MONSTER_TELEFRAGS) &&
      !(tmthing->flags2 & MF2_TELESTOMP))
    return false;

  P_DamageMobj (thing, tmthing, tmthing, 10000); // Stomp!

  return true;
}


/*
 * killough 8/28/98:
 *
 * P_GetFriction()
 *
 * Returns the friction associated with a particular mobj.
 */

int P_GetFriction(const mobj_t *mo, int *frictionfactor)
{
  int friction = ORIG_FRICTION;
  int movefactor = ORIG_FRICTION_FACTOR;
  const msecnode_t *m;
  const sector_t *sec;

  /* Assign the friction value to objects on the floor, non-floating,
   * and clipped. Normally the object's friction value is kept at
   * ORIG_FRICTION and this thinker changes it for icy or muddy floors.
   *
   * When the object is straddling sectors with the same
   * floorheight that have different frictions, use the lowest
   * friction value (muddy has precedence over icy).
   */

  if (mo->flags & MF_FLY)
  {
    friction = FRICTION_FLY;
  }
  else
  {
    if (
      !(mo->flags & (MF_NOCLIP | MF_NOGRAVITY)) &&
      (mbf_features || (mo->player && !compatibility)) &&
      variable_friction
    )
      for (m = mo->touching_sectorlist; m; m = m->m_tnext)
        if (
          (sec = m->m_sector)->flags & SECF_FRICTION &&
          (sec->friction < friction || friction == ORIG_FRICTION) &&
          (
            mo->z <= sec->floorheight ||
            (
              sec->heightsec != -1 &&
              mo->z <= sectors[sec->heightsec].floorheight &&
              mbf_features
            )
          )
        )
        {
          friction = sec->friction;
          movefactor = sec->movefactor;
        }
  }

  if (frictionfactor)
    *frictionfactor = movefactor;

  return friction;
}

/* phares 3/19/98
 * P_GetMoveFactor() returns the value by which the x,y
 * movements are multiplied to add to player movement.
 *
 * killough 8/28/98: rewritten
 */

int P_GetMoveFactor(mobj_t *mo, int *frictionp)
{
  int movefactor, friction;

  //e6y
  if (!mbf_features && !prboom_comp[PC_PRBOOM_FRICTION].state)
  {
    int momentum;

    movefactor = ORIG_FRICTION_FACTOR;

    if (!compatibility && variable_friction &&
      !(mo->flags & (MF_NOGRAVITY | MF_NOCLIP)))
    {
      friction = mo->friction;
      if (friction == ORIG_FRICTION)            // normal floor
        ;
      else if (friction > ORIG_FRICTION)        // ice
      {
        movefactor = mo->movefactor;
        ((mobj_t*)mo)->movefactor = ORIG_FRICTION_FACTOR;  // reset
      }
      else                                      // sludge
      {

        // phares 3/11/98: you start off slowly, then increase as
        // you get better footing

        momentum = (P_AproxDistance(mo->momx,mo->momy));
        movefactor = mo->movefactor;
        if (momentum > MORE_FRICTION_MOMENTUM<<2)
          movefactor <<= 3;

        else if (momentum > MORE_FRICTION_MOMENTUM<<1)
          movefactor <<= 2;

        else if (momentum > MORE_FRICTION_MOMENTUM)
          movefactor <<= 1;

        ((mobj_t*)mo)->movefactor = ORIG_FRICTION_FACTOR;  // reset
      }
    }                                                       //     ^

    return(movefactor);                                       //     |
  }

  // If the floor is icy or muddy, it's harder to get moving. This is where
  // the different friction factors are applied to 'trying to move'. In
  // p_mobj.c, the friction factors are applied as you coast and slow down.

  if ((friction = P_GetFriction(mo, &movefactor)) < ORIG_FRICTION)
    {
      // phares 3/11/98: you start off slowly, then increase as
      // you get better footing

     int momentum = P_AproxDistance(mo->momx,mo->momy);

     if (momentum > MORE_FRICTION_MOMENTUM<<2)
       movefactor <<= 3;
     else if (momentum > MORE_FRICTION_MOMENTUM<<1)
       movefactor <<= 2;
     else if (momentum > MORE_FRICTION_MOMENTUM)
       movefactor <<= 1;
    }

  if (frictionp)
    *frictionp = friction;

  return movefactor;
}

dboolean P_MoveThing(mobj_t *thing, fixed_t x, fixed_t y, fixed_t z, dboolean fog)
{
  sector_t *newsec;
  fixed_t oldx, oldy, oldz;
  fixed_t oldfloorz, oldceilingz, olddropoffz;

  oldx = thing->x;
  oldy = thing->y;
  oldz = thing->z;
  oldfloorz = thing->floorz;
  oldceilingz = thing->ceilingz;
  olddropoffz = thing->dropoffz;

  newsec = R_PointInSector(x, y);

  thing->x = x;
  thing->y = y;
  thing->z = z;
  thing->floorz = newsec->floorheight;
  thing->ceilingz = newsec->ceilingheight;
  thing->dropoffz = thing->floorz;

  if (P_TestMobjLocation(thing))
  {
    P_UnsetThingPosition(thing);
    P_SetThingPosition(thing);

    if (fog)
    {
      mobj_t *telefog;

      telefog = P_SpawnMobj(oldx,
                            oldy,
                            oldfloorz + g_telefog_height,
                            g_mt_tfog);
      S_StartMobjSound(telefog, g_sfx_telept);
      telefog = P_SpawnMobj(thing->x,
                            thing->y,
                            thing->floorz + g_telefog_height,
                            g_mt_tfog);
      S_StartMobjSound(telefog, g_sfx_telept);
    }

    thing->PrevX = x;
    thing->PrevY = y;
    thing->PrevZ = z;

    return true;
  }
  else
  {
    thing->x = oldx;
    thing->y = oldy;
    thing->z = oldz;
    thing->floorz = oldfloorz;
    thing->ceilingz = oldceilingz;
    thing->dropoffz = olddropoffz;

    return false;
  }
}

void P_UnqualifiedMove(mobj_t *thing, fixed_t x, fixed_t y)
{
  sector_t *sector;

  P_UnsetThingPosition(thing);
  thing->x = x;
  thing->y = y;
  sector = R_PointInSector(thing->x, thing->y);
  thing->z = thing->floorz = sector->floorheight;
  thing->ceilingz = sector->ceilingheight;
  P_SetThingPosition(thing);
}

//
// P_TeleportMove
//

dboolean P_TeleportMove (mobj_t* thing,fixed_t x,fixed_t y, dboolean boss)
{
  int     xl;
  int     xh;
  int     yl;
  int     yh;
  int     bx;
  int     by;

  sector_t*  newsec;

  /* killough 8/9/98: make telefragging more consistent, preserve compatibility */
  telefrag = !raven &&
    (thing->player || (!comp[comp_telefrag] ? boss : (gamemap==30)));

  // kill anything occupying the position

  tmthing = thing;
  tmflags = thing->flags;

  tmx = x;
  tmy = y;

  tmbbox[BOXTOP] = y + tmthing->radius;
  tmbbox[BOXBOTTOM] = y - tmthing->radius;
  tmbbox[BOXRIGHT] = x + tmthing->radius;
  tmbbox[BOXLEFT] = x - tmthing->radius;

  newsec = R_PointInSector (x,y);
  ceilingline = NULL;

  // The base floor/ceiling is from the sector
  // that contains the point.
  // Any contacted lines the step closer together
  // will adjust them.

  tmfloorz = tmdropoffz = newsec->floorheight;
  tmceilingz = newsec->ceilingheight;
  tmfloorpic = newsec->floorpic;

  validcount++;
  numspechit = 0;

  // stomp on any things contacted

  xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS);
  xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS);
  yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS);
  yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy + MAXRADIUS);

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockThingsIterator(bx,by,PIT_StompThing))
        return false;

  // the move is ok,
  // so unlink from the old position & link into the new position

  P_UnsetThingPosition (thing);

  thing->floorz = tmfloorz;
  thing->ceilingz = tmceilingz;
  thing->dropoffz = tmdropoffz;        // killough 11/98

  thing->x = x;
  thing->y = y;

  P_SetThingPosition (thing);

  thing->PrevX = x;
  thing->PrevY = y;
  thing->PrevZ = thing->floorz;

  return true;
}


//
// MOVEMENT ITERATOR FUNCTIONS
//

//                                                                  // phares
// PIT_CrossLine                                                    //   |
// Checks to see if a PE->LS trajectory line crosses a blocking     //   V
// line. Returns false if it does.
//
// tmbbox holds the bounding box of the trajectory. If that box
// does not touch the bounding box of the line in question,
// then the trajectory is not blocked. If the PE is on one side
// of the line and the LS is on the other side, then the
// trajectory is blocked.
//
// Currently this assumes an infinite line, which is not quite
// correct. A more correct solution would be to check for an
// intersection of the trajectory and the line, but that takes
// longer and probably really isn't worth the effort.
//

static // killough 3/26/98: make static
dboolean PIT_CrossLine (line_t* ld)
{
  if (!(ld->flags & ML_TWOSIDED) ||
      (ld->flags & (ML_BLOCKING|ML_BLOCKMONSTERS)))
    if (!(tmbbox[BOXLEFT]   > ld->bbox[BOXRIGHT]  ||
          tmbbox[BOXRIGHT]  < ld->bbox[BOXLEFT]   ||
          tmbbox[BOXTOP]    < ld->bbox[BOXBOTTOM] ||
          tmbbox[BOXBOTTOM] > ld->bbox[BOXTOP]))
      if (P_PointOnLineSide(pe_x,pe_y,ld) != P_PointOnLineSide(ls_x,ls_y,ld))
        return(false);  // line blocks trajectory                   //   ^
  return(true); // line doesn't block trajectory                    //   |
}                                                                   // phares


/* killough 8/1/98: used to test intersection between thing and line
 * assuming NO movement occurs -- used to avoid sticky situations.
 */

static int untouched(line_t *ld)
{
  fixed_t x, y, tmbbox[4];
  return
    (tmbbox[BOXRIGHT] = (x=tmthing->x)+tmthing->radius) <= ld->bbox[BOXLEFT] ||
    (tmbbox[BOXLEFT] = x-tmthing->radius) >= ld->bbox[BOXRIGHT] ||
    (tmbbox[BOXTOP] = (y=tmthing->y)+tmthing->radius) <= ld->bbox[BOXBOTTOM] ||
    (tmbbox[BOXBOTTOM] = y-tmthing->radius) >= ld->bbox[BOXTOP] ||
    P_BoxOnLineSide(tmbbox, ld) != -1;
}

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//

static void CheckForDamageSpecial(line_t *line, mobj_t *mo)
{
  int damage;

  // TODO: bouncing against a damage line
  // TODO: lost souls don't damage walls in gzdoom
  if (
    !line->health ||
    !(mo->flags & (/* MF_SKULLFLY |*/ MF_MISSILE /*| MF_BOUNCES */)) ||
    !mo->info->damage
  )
  {
    return;
  }

  damage = ((P_Random(pr_damage) % 8) + 1) * mo->info->damage;
  dsda_DamageLinedef(line, mo->target, damage);
}

static void CheckForPushSpecial(line_t * line, int side, mobj_t * mobj);

static // killough 3/26/98: make static
dboolean PIT_CheckLine (line_t* ld)
{
  dboolean rail = false;

  if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
   || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
   || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
   || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
    return true; // didn't hit it

  if (P_BoxOnLineSide(tmbbox, ld) != -1)
    return true; // didn't hit it

  // A line has been hit

  // The moving thing's destination position will cross the given line.
  // If this should not be allowed, return false.
  // If the line is special, keep track of it
  // to process later if the move is proven ok.
  // NOTE: specials are NOT sorted by order,
  // so two special lines that are only 8 pixels apart
  // could be crossed in either order.

  // killough 7/24/98: allow player to move out of 1s wall, to prevent sticking
  if (!ld->backsector) // one sided line
  {
    if (heretic)
    {
      if (tmthing->flags & MF_MISSILE)
      {                       // Missiles can trigger impact specials
        if (ld->special)
        {
          P_AppendSpecHit(ld);
        }
      }
    }
    else if (map_format.hexen)
    {
      if (tmthing->flags2 & MF2_BLASTED)
      {
        P_DamageMobj(tmthing, NULL, NULL, tmthing->info->mass >> 5);
      }
      CheckForPushSpecial(ld, 0, tmthing);
      CheckForDamageSpecial(ld, tmthing);
    }
    blockline = ld;
    return tmunstuck && !untouched(ld) &&
           FixedMul(tmx-tmthing->x,ld->dy) > FixedMul(tmy-tmthing->y,ld->dx);
  }

  // killough 8/10/98: allow bouncing objects to pass through as missiles
  if (!(tmthing->flags & (MF_MISSILE | MF_BOUNCES)) ||
      ld->flags & (ML_BLOCKPROJECTILES | ML_BLOCKEVERYTHING))
  {
    if (ld->flags & ML_JUMPOVER)
    {
      rail = true;
    }
    else
    {
      // explicitly blocking everything
      // or blocking player
      // or blocking projectile
      if (
        ld->flags & ML_BLOCKING ||
        (mbf21 && tmthing->player && ld->flags & ML_BLOCKPLAYERS) ||
        tmthing->flags & (MF_MISSILE | MF_BOUNCES)
      )
      {
        if (map_format.hexen)
        {
          if (tmthing->flags2 & MF2_BLASTED)
          {
            P_DamageMobj(tmthing, NULL, NULL, tmthing->info->mass >> 5);
          }
          CheckForPushSpecial(ld, 0, tmthing);
          CheckForDamageSpecial(ld, tmthing);
        }
        return tmunstuck && !untouched(ld);  // killough 8/1/98: allow escape
      }

      // killough 8/9/98: monster-blockers don't affect friends
      if (
        !(tmthing->flags & MF_FRIEND || tmthing->player) &&
        (
          ld->flags & ML_BLOCKMONSTERS ||
          (mbf21 && ld->flags & ML_BLOCKLANDMONSTERS && !(tmthing->flags & MF_FLOAT)) ||
          (ld->flags & ML_BLOCKFLOATERS && tmthing->flags & MF_FLOAT)
        ) &&
        (!heretic || tmthing->type != HERETIC_MT_POD)
      )
      {
        if (tmthing->flags2 & MF2_BLASTED)
        {
          P_DamageMobj(tmthing, NULL, NULL, tmthing->info->mass >> 5);
        }
        return false; // block monsters only
      }
    }
  }

  // defines a 'window' from one sector to another across this line

  P_LineOpening (ld, tmthing);

  if (rail && line_opening.bottom == tmfloorz)
  {
    line_opening.bottom += (32 << FRACBITS);
  }

  // adjust floor & ceiling heights

  if (line_opening.top < tmceilingz)
  {
    tmceilingz = line_opening.top;
    ceilingline = ld;
    blockline = ld;
  }

  if (line_opening.bottom > tmfloorz)
  {
    tmfloorz = line_opening.bottom;
    floorline = ld;          // killough 8/1/98: remember floor linedef
    blockline = ld;
  }

  if (line_opening.lowfloor < tmdropoffz)
    tmdropoffz = line_opening.lowfloor;

  // if contacted a special line, add it to the list
  if (ld->special)
    P_AppendSpecHit(ld);

  return true;
}

//
// PIT_CheckThing
//

static dboolean P_ProjectileImmune(mobj_t *target, mobj_t *source)
{
  return
    ( // PG_GROUPLESS means no immunity, even to own species
      mobjinfo[target->type].projectile_group != PG_GROUPLESS ||
      target == source
    ) &&
    (
      ( // target type has default behaviour, and things are the same type
        mobjinfo[target->type].projectile_group == PG_DEFAULT &&
        source->type == target->type
      ) ||
      ( // target type has special behaviour, and things have the same group
        mobjinfo[target->type].projectile_group != PG_DEFAULT &&
        mobjinfo[target->type].projectile_group == mobjinfo[source->type].projectile_group
      )
    );
}

static dboolean PIT_CheckThing(mobj_t *thing) // killough 3/26/98: make static
{
  fixed_t blockdist;
  int damage;

  // killough 11/98: add touchy things
  if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE|MF_TOUCHY)))
    return true;

  blockdist = thing->radius + tmthing->radius;

  if (D_abs(thing->x - tmx) >= blockdist || D_abs(thing->y - tmy) >= blockdist)
    return true; // didn't hit it

  // killough 11/98:
  //
  // This test has less information content (it's almost always false), so it
  // should not be moved up to first, as it adds more overhead than it removes.

  // don't clip against self

  if (thing == tmthing)
    return true;

  if (map_format.hexen)
    BlockingMobj = thing;

  /* killough 11/98:
   *
   * TOUCHY flag, for mines or other objects which die on contact with solids.
   * If a solid object of a different type comes in contact with a touchy
   * thing, and the touchy thing is not the sole one moving relative to fixed
   * surroundings such as walls, then the touchy thing dies immediately.
   */

  if (thing->flags & MF_TOUCHY &&                  // touchy object
      tmthing->flags & MF_SOLID &&                 // solid object touches it
      thing->health > 0 &&                         // touchy object is alive
      (thing->intflags & MIF_ARMED ||              // Thing is an armed mine
       sentient(thing)) &&                         // ... or a sentient thing
      (thing->type != tmthing->type ||             // only different species
       thing->type == g_mt_player) &&                // ... or different players
      thing->z + thing->height >= tmthing->z &&    // touches vertically
      tmthing->z + tmthing->height >= thing->z &&
      (thing->type ^ MT_PAIN) |                    // PEs and lost souls
      (tmthing->type ^ MT_SKULL) &&                // are considered same
      (thing->type ^ MT_SKULL) |                   // (but Barons & Knights
      (tmthing->type ^ MT_PAIN))                   // are intentionally not)
    {
      P_DamageMobj(thing, NULL, NULL, thing->health);  // kill object
      return true;
    }

  if (tmthing->flags2 & MF2_PASSMOBJ)
  {                           // check if a mobj passed over/under another object
    if (raven)
    {
      if ((tmthing->type == HERETIC_MT_IMP || tmthing->type == HERETIC_MT_WIZARD)
          && (thing->type == HERETIC_MT_IMP || thing->type == HERETIC_MT_WIZARD))
      {                       // don't let imps/wizards fly over other imps/wizards
        return false;
      }

      if (tmthing->type == HEXEN_MT_BISHOP && thing->type == HEXEN_MT_BISHOP)
      {                       // don't let bishops fly over other bishops
        return false;
      }
    }

    if (
      (map_format.hexen ? tmthing->z >= thing->z + thing->height
                        : tmthing->z >  thing->z + thing->height)
      && !(thing->flags & MF_SPECIAL)
    )
    {
      return (true);
    }
    else if (
      (map_format.zdoom ? tmthing->z + tmthing->height <= thing->z
                        : tmthing->z + tmthing->height <  thing->z)
      && !(thing->flags & MF_SPECIAL))
    {                       // under thing
      return (true);
    }
  }

  // check for skulls slamming into things

  if (tmthing->flags & MF_SKULLFLY)
  {
    // A flying skull is smacking something.
    // Determine damage amount, and the skull comes to a dead stop.

    int new_state;
    int damage;

    if (hexen)
    {
      if (tmthing->type == HEXEN_MT_MINOTAUR)
      {
        // Slamming minotaurs shouldn't move non-creatures
        if (!(thing->flags & MF_COUNTKILL))
        {
          return (false);
        }
      }
      else if (tmthing->type == HEXEN_MT_HOLY_FX)
      {
        if (thing->flags & MF_SHOOTABLE && thing != tmthing->target)
        {
          if (netgame && !deathmatch && thing->player)
          {               // don't attack other co-op players
            return true;
          }
          if (thing->flags2 & MF2_REFLECTIVE
              && (thing->player || thing->flags2 & MF2_BOSS))
          {
            P_SetTarget(&tmthing->special1.m, tmthing->target);
            P_SetTarget(&tmthing->target, thing);
            return true;
          }
          if (thing->flags & MF_COUNTKILL || thing->player)
          {
            P_SetTarget(&tmthing->special1.m, thing);
          }
          if (P_Random(pr_hexen) < 96)
          {
            damage = 12;
            if (thing->player || thing->flags2 & MF2_BOSS)
            {
              damage = 3;
              // ghost burns out faster when attacking players/bosses
              tmthing->health -= 6;
            }
            P_DamageMobj(thing, tmthing, tmthing->target, damage);
            if (P_Random(pr_hexen) < 128)
            {
              P_SpawnMobj(tmthing->x, tmthing->y, tmthing->z,
                          HEXEN_MT_HOLY_PUFF);
              S_StartMobjSound(tmthing, hexen_sfx_spirit_attack);
              if (thing->flags & MF_COUNTKILL && P_Random(pr_hexen) < 128
                  && !S_GetSoundPlayingInfo(thing, hexen_sfx_puppybeat))
              {
                if ((thing->type == HEXEN_MT_CENTAUR) ||
                    (thing->type == HEXEN_MT_CENTAURLEADER) ||
                    (thing->type == HEXEN_MT_ETTIN))
                {
                  S_StartMobjSound(thing, hexen_sfx_puppybeat);
                }
              }
            }
          }
          if (thing->health <= 0)
          {
            tmthing->special1.i = 0;
            P_SetTarget(&tmthing->special1.m, NULL);
          }
        }
        return true;
      }
    }

    damage = ((P_Random(pr_skullfly) % 8) + 1) * tmthing->info->damage;

    P_DamageMobj (thing, tmthing, tmthing, damage);

    tmthing->flags &= ~MF_SKULLFLY;
    tmthing->momx = tmthing->momy = tmthing->momz = 0;

    if (raven)
      new_state = tmthing->info->seestate;
    else
      new_state = tmthing->info->spawnstate;

    P_SetMobjState (tmthing, new_state);

    return false;   // stop moving
  }

  // Check for blasted thing running into another
  if (tmthing->flags2 & MF2_BLASTED && thing->flags & MF_SHOOTABLE)
  {
    if (!(thing->flags2 & MF2_BOSS) && (thing->flags & MF_COUNTKILL))
    {
      thing->momx += tmthing->momx;
      thing->momy += tmthing->momy;
      if ((thing->momx + thing->momy) > 3 * FRACUNIT)
      {
          damage = (tmthing->info->mass / 100) + 1;
          P_DamageMobj(thing, tmthing, tmthing, damage);
          damage = (thing->info->mass / 100) + 1;
          P_DamageMobj(tmthing, thing, thing, damage >> 2);
      }
      return (false);
    }
  }

  // missiles can hit other things
  // killough 8/10/98: bouncing non-solid things can hit other things too

  if (tmthing->flags & MF_MISSILE ||
     (tmthing->flags & MF_BOUNCES && !(tmthing->flags & MF_SOLID)))
  {
    // Check for a non-shootable mobj
    if (thing->flags2 & MF2_NONSHOOTABLE)
    {
        return true;
    }

    // Check for passing through a ghost
    if ((thing->flags & MF_SHADOW) && (tmthing->flags2 & MF2_THRUGHOST))
    {
        return (true);
    }

    // see if it went over / under

    if (tmthing->z > thing->z + thing->height)
      return true;    // overhead

    if (tmthing->z + tmthing->height < thing->z)
      return true;    // underneath

    if (hexen)
    {
      if (tmthing->flags2 & MF2_FLOORBOUNCE)
      {
        if (tmthing->target == thing || !(thing->flags & MF_SOLID))
        {
          return true;
        }
        else
        {
          return false;
        }
      }

      if (tmthing->type == HEXEN_MT_LIGHTNING_FLOOR
          || tmthing->type == HEXEN_MT_LIGHTNING_CEILING)
      {
        if (thing->flags & MF_SHOOTABLE && thing != tmthing->target)
        {
          if (thing->info->mass != INT_MAX)
          {
            thing->momx += tmthing->momx >> 4;
            thing->momy += tmthing->momy >> 4;
          }
          if ((!thing->player && !(thing->flags2 & MF2_BOSS))
              || !(leveltime & 1))
          {
            if (thing->type == HEXEN_MT_CENTAUR
                || thing->type == HEXEN_MT_CENTAURLEADER)
            {           // Lightning does more damage to centaurs
              P_DamageMobj(thing, tmthing, tmthing->target, 9);
            }
            else
            {
              P_DamageMobj(thing, tmthing, tmthing->target, 3);
            }
            if (!(S_GetSoundPlayingInfo(tmthing,
                                        hexen_sfx_mage_lightning_zap)))
            {
              S_StartMobjSound(tmthing, hexen_sfx_mage_lightning_zap);
            }
            if (thing->flags & MF_COUNTKILL && P_Random(pr_hexen) < 64
                && !S_GetSoundPlayingInfo(thing, hexen_sfx_puppybeat))
            {
              if ((thing->type == HEXEN_MT_CENTAUR) ||
                  (thing->type == HEXEN_MT_CENTAURLEADER) ||
                  (thing->type == HEXEN_MT_ETTIN))
              {
                S_StartMobjSound(thing, hexen_sfx_puppybeat);
              }
            }
          }
          tmthing->health--;
          if (tmthing->health <= 0 || thing->health <= 0)
          {
            return false;
          }
          if (tmthing->type == HEXEN_MT_LIGHTNING_FLOOR)
          {
            if (tmthing->special2.m
                && !tmthing->special2.m->special1.m)
            {
              P_SetTarget(&tmthing->special2.m->special1.m, thing);
            }
          }
          else if (!tmthing->special1.m)
          {
            P_SetTarget(&tmthing->special1.m, thing);
          }
        }
        return true;        // lightning zaps through all sprites
      }
      else if (tmthing->type == HEXEN_MT_LIGHTNING_ZAP)
      {
        mobj_t *lmo;

        if (thing->flags & MF_SHOOTABLE && thing != tmthing->target)
        {
          lmo = tmthing->special2.m;
          if (lmo)
          {
            if (lmo->type == HEXEN_MT_LIGHTNING_FLOOR)
            {
              if (lmo->special2.m
                  && !lmo->special2.m->special1.m)
              {
                P_SetTarget(&lmo->special2.m->special1.m, thing);
              }
            }
            else if (!lmo->special1.m)
            {
              P_SetTarget(&lmo->special1.m, thing);
            }
            if (!(leveltime & 3))
            {
              lmo->health--;
            }
          }
        }
      }
      else if (tmthing->type == HEXEN_MT_MSTAFF_FX2 && thing != tmthing->target)
      {
        if (!thing->player && !(thing->flags2 & MF2_BOSS))
        {
          switch (thing->type)
          {
            case HEXEN_MT_FIGHTER_BOSS:      // these not flagged boss
            case HEXEN_MT_CLERIC_BOSS:       // so they can be blasted
            case HEXEN_MT_MAGE_BOSS:
              break;
            default:
              P_DamageMobj(thing, tmthing, tmthing->target, 10);
              return true;
              break;
          }
        }
      }
    }

    if (tmthing->target && P_ProjectileImmune(thing, tmthing->target))
    {
      if (thing == tmthing->target)
        return true;                // Don't hit self.
      else
        // e6y: Dehacked support - monsters infight
        if (thing->type != g_mt_player && !monsters_infight) // Explode, but do no damage.
          return false;         // Let players missile other players.
    }

    // killough 8/10/98: if moving thing is not a missile, no damage
    // is inflicted, and momentum is reduced if object hit is solid.

    if (!(tmthing->flags & MF_MISSILE)) {
      if (!(thing->flags & MF_SOLID))
      {
          return true;
      }
      else
      {
        tmthing->momx = -tmthing->momx;
        tmthing->momy = -tmthing->momy;
        if (!(tmthing->flags & MF_NOGRAVITY))
        {
          tmthing->momx >>= 2;
          tmthing->momy >>= 2;
        }
        return false;
      }
    }

    if (!(thing->flags & MF_SHOOTABLE))
      return !(thing->flags & MF_SOLID); // didn't do any damage

    if (tmthing->flags2 & MF2_RIP)
    {
      if (raven)
      {
        if (!(thing->flags & MF_NOBLOOD) &&
            !(thing->flags2 & MF2_REFLECTIVE) &&
            !(thing->flags2 & MF2_INVULNERABLE))
        {                   // Ok to spawn some blood
          P_RipperBlood(tmthing, thing);
        }
        if (heretic) S_StartMobjSound(tmthing, heretic_sfx_ripslop);
        damage = ((P_Random(pr_heretic) & 3) + 2) * tmthing->damage;
      }
      else
      {
        damage = ((P_Random(pr_mbf21) & 3) + 2) * tmthing->info->damage;
        if (!(thing->flags & MF_NOBLOOD))
          P_SpawnBlood(tmthing->x, tmthing->y, tmthing->z, damage, thing);
        if (tmthing->info->ripsound)
          S_StartMobjSound(tmthing, tmthing->info->ripsound);
      }

      P_DamageMobj(thing, tmthing, tmthing->target, damage);
      if (thing->flags2 & MF2_PUSHABLE && !(tmthing->flags2 & MF2_CANNOTPUSH))
      {                   // Push thing
        thing->momx += tmthing->momx >> 2;
        thing->momy += tmthing->momy >> 2;
      }
      numspechit = 0;
      return (true);
    }

    // damage / explode

    damage = raven ? tmthing->damage : tmthing->info->damage;
    damage = ((P_Random(pr_damage) % 8) + 1) * damage;
    if (
      raven &&
      damage &&
      !(thing->flags & MF_NOBLOOD) &&
      !(thing->flags2 & MF2_REFLECTIVE) &&
      !(thing->flags2 & MF2_INVULNERABLE) &&
      !(tmthing->type == HEXEN_MT_TELOTHER_FX1) &&
      !(tmthing->type == HEXEN_MT_TELOTHER_FX2) &&
      !(tmthing->type == HEXEN_MT_TELOTHER_FX3) &&
      !(tmthing->type == HEXEN_MT_TELOTHER_FX4) &&
      !(tmthing->type == HEXEN_MT_TELOTHER_FX5) &&
      P_Random(pr_heretic) < 192
    )
    {
      P_BloodSplatter(tmthing->x, tmthing->y, tmthing->z, thing);
    }
    if (!raven || damage)
      P_DamageMobj(thing, tmthing, tmthing->target, damage);

    // don't traverse any more
    return false;
  }

  if (thing->flags2 & MF2_PUSHABLE && !(tmthing->flags2 & MF2_CANNOTPUSH))
  {                           // Push thing
      thing->momx += tmthing->momx >> 2;
      thing->momy += tmthing->momy >> 2;
  }

  // check for special pickup

  if (thing->flags & MF_SPECIAL)
  {
    uint64_t solid = thing->flags & MF_SOLID;
    if (tmthing->flags & MF_PICKUP) // hexen_note: can probably use tmflags here?
      P_TouchSpecialThing(thing, tmthing); // can remove thing
    return !solid;
  }

  // RjY
  // comperr_hangsolid, an attempt to handle blocking hanging bodies
  // A solid hanging body will allow sufficiently small things underneath it.
  if (comperr(comperr_hangsolid) &&
      !((~thing->flags) & (MF_SOLID | MF_SPAWNCEILING)) // solid and hanging
      // invert everything, then both bits should be clear
      && tmthing->z + tmthing->height <= thing->z) // head height <= base
      // top of thing trying to move under the body <= bottom of body
  {
    tmceilingz = thing->z; // pretend ceiling height is at body's base
    return true;
  }

  // killough 3/16/98: Allow non-solid moving objects to move through solid
  // ones, by allowing the moving thing (tmthing) to move if it's non-solid,
  // despite another solid thing being in the way.
  // killough 4/11/98: Treat no-clipping things as not blocking
  // ...but not in demo_compatibility mode

  // e6y
  // Correction of wrong return value with demo_compatibility.
  // There is no more synch on http://www.doomworld.com/sda/dwdemo/w303-115.zip
  // (with correction in setMobjInfoValue)
  if (demo_compatibility && !prboom_comp[PC_TREAT_NO_CLIPPING_THINGS_AS_NOT_BLOCKING].state)
    return !(thing->flags & MF_SOLID);
  else
    return !((thing->flags & MF_SOLID && !(thing->flags & MF_NOCLIP))
           && (tmthing->flags & MF_SOLID || demo_compatibility));

  // return !(thing->flags & MF_SOLID);   // old code -- killough
}

// This routine checks for Lost Souls trying to be spawned      // phares
// across 1-sided lines, impassible lines, or "monsters can't   //   |
// cross" lines. Draw an imaginary line between the PE          //   V
// and the new Lost Soul spawn spot. If that line crosses
// a 'blocking' line, then disallow the spawn. Only search
// lines in the blocks of the blockmap where the bounding box
// of the trajectory line resides. Then check bounding box
// of the trajectory vs. the bounding box of each blocking
// line to see if the trajectory and the blocking line cross.
// Then check the PE and LS to see if they're on different
// sides of the blocking line. If so, return true, otherwise
// false.

dboolean Check_Sides(mobj_t* actor, int x, int y)
{
  int bx,by,xl,xh,yl,yh;

  pe_x = actor->x;
  pe_y = actor->y;
  ls_x = x;
  ls_y = y;

  // Here is the bounding box of the trajectory

  tmbbox[BOXLEFT]   = pe_x < x ? pe_x : x;
  tmbbox[BOXRIGHT]  = pe_x > x ? pe_x : x;
  tmbbox[BOXTOP]    = pe_y > y ? pe_y : y;
  tmbbox[BOXBOTTOM] = pe_y < y ? pe_y : y;

  // Determine which blocks to look in for blocking lines

  xl = P_GetSafeBlockX(tmbbox[BOXLEFT]   - bmaporgx);
  xh = P_GetSafeBlockX(tmbbox[BOXRIGHT]  - bmaporgx);
  yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy);
  yh = P_GetSafeBlockY(tmbbox[BOXTOP]    - bmaporgy);

  // xl->xh, yl->yh determine the mapblock set to search

  validcount++; // prevents checking same line twice
  for (bx = xl ; bx <= xh ; bx++)
    for (by = yl ; by <= yh ; by++)
      if (!P_BlockLinesIterator(bx,by,PIT_CrossLine))
        return true;                                                //   ^
  return(false);                                                    //   |
}                                                                   // phares

//
// MOVEMENT CLIPPING
//

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
//
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  floorz
//  ceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//

dboolean P_CheckPosition (mobj_t* thing,fixed_t x,fixed_t y)
{
  int     xl;
  int     xh;
  int     yl;
  int     yh;
  int     bx;
  int     by;
  sector_t*  newsec;

  tmthing = thing;
  tmflags = thing->flags;

  tmx = x;
  tmy = y;

  tmbbox[BOXTOP] = y + tmthing->radius;
  tmbbox[BOXBOTTOM] = y - tmthing->radius;
  tmbbox[BOXRIGHT] = x + tmthing->radius;
  tmbbox[BOXLEFT] = x - tmthing->radius;

  newsec = R_PointInSector (x,y);
  floorline = blockline = ceilingline = NULL; // killough 8/1/98

  // Whether object can get out of a sticky situation:
  tmunstuck = thing->player &&          /* only players */
    thing->player->mo == thing &&       /* not voodoo dolls */
    mbf_features; /* not under old demos */

  // The base floor / ceiling is from the sector
  // that contains the point.
  // Any contacted lines the step closer together
  // will adjust them.

  tmfloorz = tmdropoffz = newsec->floorheight;
  tmceilingz = newsec->ceilingheight;
  tmfloorpic = newsec->floorpic;
  validcount++;
  numspechit = 0;

  if (tmflags & MF_NOCLIP && (!hexen || !(tmflags & MF_SKULLFLY)))
    return true;

  // Check things first, possibly picking things up.
  // The bounding box is extended by MAXRADIUS
  // because mobj_ts are grouped into mapblocks
  // based on their origin point, and can overlap
  // into adjacent blocks by up to MAXRADIUS units.

  xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS);
  xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS);
  yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS);
  yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy + MAXRADIUS);

  BlockingMobj = NULL;

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockThingsIterator(bx,by,PIT_CheckThing))
        return false;

  if (hexen && tmflags & MF_NOCLIP)
  {
      return true;
  }

  BlockingMobj = NULL;

  // check lines

  xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx);
  xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx);
  yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy);
  yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy);

  // Fixes a vanilla bug where this is incremented in the wrong place
  // Prevents edge cases where lines aren't checked when they should be
  if (mbf21)
  {
    validcount++;
  }

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      if (!P_BlockLinesIterator (bx,by,PIT_CheckLine))
        return false; // doesn't fit

  return true;
}

void P_AdjustZLimits(mobj_t *thing)
{
  int xl, xh;
  int yl, yh;
  int bx, by;
  fixed_t bbox[4];

  bbox[BOXTOP] = thing->y + thing->radius;
  bbox[BOXBOTTOM] = thing->y - thing->radius;
  bbox[BOXRIGHT] = thing->x + thing->radius;
  bbox[BOXLEFT] = thing->x - thing->radius;

  validcount++;

  xl = P_GetSafeBlockX(bbox[BOXLEFT] - bmaporgx);
  xh = P_GetSafeBlockX(bbox[BOXRIGHT] - bmaporgx);
  yl = P_GetSafeBlockY(bbox[BOXBOTTOM] - bmaporgy);
  yh = P_GetSafeBlockY(bbox[BOXTOP] - bmaporgy);

  for (bx = xl; bx <= xh; ++bx)
    for (by = yl; by <= yh; ++by)
    {
      int offset;
      const int *list;

      if (bx < 0 || by < 0 || bx >= bmapwidth || by >= bmapheight)
        continue;

      offset = by * bmapwidth + bx;
      offset = *(blockmap + offset);
      list = blockmaplump + offset;

      if (skipblstart)
        list++;

      for (; *list != -1; list++)
      {
        line_t *ld;

        ld = &lines[*list];
        if (ld->validcount == validcount)
          continue; // line has already been checked
        ld->validcount = validcount;

        if (bbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
            || bbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
            || bbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
            || bbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
          continue; // didn't hit it

        if (P_BoxOnLineSide(bbox, ld) != -1)
          continue; // didn't hit it

        if (!ld->backsector || !ld->frontsector || !(ld->flags & ML_3DMIDTEX))
          continue; // not relevant

        P_LineOpening(ld, thing);

        if (line_opening.bottom > thing->floorz)
          thing->floorz = line_opening.bottom;

        if (line_opening.top < thing->ceilingz)
          thing->ceilingz = line_opening.top;
      }
    }
}

//
// P_TryMove
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
//

static dboolean Hexen_P_TryMove(mobj_t* thing, fixed_t x, fixed_t y);

void P_CheckCompatibleImpact(mobj_t *thing)
{
  // nothing in doom
}

void P_CheckHereticImpact(mobj_t *thing)
{
  int i;

  if (!numspechit || !(thing->flags & MF_MISSILE) || !thing->target || !thing->target->player)
  {
    return;
  }

  for (i = numspechit - 1; i >= 0; i--)
  {
    map_format.shoot_special_line(thing->target, spechit[i]);
  }
}

void P_CheckZDoomImpact(mobj_t *thing)
{
  if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)))
  {
    int i, side;
    line_t *ld;

    if (tmthing->flags2 & MF2_BLASTED)
    {
      P_DamageMobj(tmthing, NULL, NULL, tmthing->info->mass >> 5);
    }

    for (i = numspechit - 1; i >= 0; i--)
    {
      // see if the line was crossed
      ld = spechit[i];
      side = P_PointOnLineSide(thing->x, thing->y, ld);
      CheckForPushSpecial(ld, side, thing);
      CheckForDamageSpecial(ld, tmthing);
    }
  }
}

void P_IterateCompatibleSpecHit(mobj_t *thing, fixed_t oldx, fixed_t oldy)
{
  while (numspechit--)
    if (spechit[numspechit]->special)  // see if the line was crossed
    {
      int oldside = P_PointOnLineSide(oldx, oldy, spechit[numspechit]);
      if (oldside != P_PointOnLineSide(thing->x, thing->y, spechit[numspechit]))
        map_format.cross_special_line(spechit[numspechit], oldside, thing, false);
    }
}

void P_IterateZDoomSpecHit(mobj_t *thing, fixed_t oldx, fixed_t oldy)
{
  // In hexen format, crossing a special line can trigger a missile spawn,
  //   which will trigger a check that resets numspechit.
  // We must store the index separately in order to check everything
  int tempnumspechit = numspechit;

  while (tempnumspechit--)
    if (spechit[tempnumspechit]->special)  // see if the line was crossed
    {
      int oldside = P_PointOnLineSide(oldx, oldy, spechit[tempnumspechit]);
      if (oldside != P_PointOnLineSide(thing->x, thing->y, spechit[tempnumspechit]))
        map_format.cross_special_line(spechit[tempnumspechit], oldside, thing, false);
    }
}

dboolean P_TryMove(mobj_t* thing,fixed_t x,fixed_t y,
                  dboolean dropoff) // killough 3/15/98: allow dropoff as option
{
  fixed_t oldx;
  fixed_t oldy;

  if (map_trail_mode == map_trail_mode_include_collisions &&
      thing->player && thing->player->mo == thing)
  {
    AM_updatePlayerTrail(x, y);
  }

  if (hexen) return Hexen_P_TryMove(thing, x, y);

  felldown = floatok = false;               // killough 11/98

  if (!P_CheckPosition(thing, x, y))
  {                           // Solid wall or thing
    if (heretic || !BlockingMobj || BlockingMobj->player || !thing->player)
      map_format.check_impact(thing);
    return false;
  }

  if (!(thing->flags & MF_NOCLIP))
  {
    if (thing->flags & MF_FLY)
    {
      // When flying, slide up or down blocking lines until the actor
      // is not blocked.
      if (thing->z+thing->height > tmceilingz)
      {
        thing->momz = -8*FRACUNIT;
        return false;
      }
      else if (thing->z < tmfloorz && tmfloorz-tmdropoffz > 24*FRACUNIT)
      {
        thing->momz = 8*FRACUNIT;
        return false;
      }
    }
    // killough 7/26/98: reformatted slightly
    // killough 8/1/98: Possibly allow escape if otherwise stuck

    if (
      tmceilingz - tmfloorz < thing->height ||     // doesn't fit
      // mobj must lower to fit
      (
        floatok = true,
        !(thing->flags & MF_TELEPORT) &&
        tmceilingz - thing->z < thing->height &&
        !(thing->flags & MF_FLY) &&
        !(thing->flags2 & MF2_FLY)
      )
    )
    {
      map_format.check_impact(thing);
      return tmunstuck
        && !(ceilingline && untouched(ceilingline))
        && !(  floorline && untouched(  floorline));
    }

    if (thing->flags2 & MF2_FLY)
    {
      if (thing->z + thing->height > tmceilingz)
      {
        thing->momz = -8 * FRACUNIT;
        map_format.check_impact(thing);
        return false;
      }
      else if (thing->z < tmfloorz
               && tmfloorz - tmdropoffz > 24 * FRACUNIT)
      {
        thing->momz = 8 * FRACUNIT;
        map_format.check_impact(thing);
        return false;
      }
    }

    if (
      !(thing->flags & MF_TELEPORT) &&
      (!heretic || thing->type != HERETIC_MT_MNTRFX2) &&
      tmfloorz - thing->z > 24*FRACUNIT
    )
    {
      dsda_WatchLedgeImpact(thing, tmfloorz);

      map_format.check_impact(thing);
      return tmunstuck
        && !(ceilingline && untouched(ceilingline))
        && !(  floorline && untouched(  floorline));
    }

    if (heretic && tmfloorz > thing->z)
    {
      map_format.check_impact(thing);
    }

    /* killough 3/15/98: Allow certain objects to drop off
     * killough 7/24/98, 8/1/98:
     * Prevent monsters from getting stuck hanging off ledges
     * killough 10/98: Allow dropoffs in controlled circumstances
     * killough 11/98: Improve symmetry of clipping on stairs
     */
    if (!(thing->flags & (MF_DROPOFF|MF_FLOAT)))
    {
      dboolean ledgeblock = comp[comp_ledgeblock] &&
                            !(mbf21 && thing->intflags & MIF_SCROLLING);

      if (comp[comp_dropoff] || ledgeblock)
      {
        // e6y
        // Fix demosync bug in mbf compatibility mode
        // There is no more desync on v2-2822.lmp/vrack2.wad
        // -force_no_dropoff command-line switch is for mbf_compatibility demos
        // recorded with prboom 2.2.2 - 2.4.7
        // Links:
        // http://competn.doom2.net/pub/sda/t-z/v2-2822.zip
        // http://www.doomworld.com/idgames/index.php?id=11138
        if (
          (
            ledgeblock ||
            !dropoff ||
            (
              !prboom_comp[PC_NO_DROPOFF].state &&
              mbf_features &&
              compatibility_level <= prboom_2_compatibility
            )
          ) &&
          (tmfloorz - tmdropoffz > 24*FRACUNIT)
        )
          return false;                      // don't stand over a dropoff
      }
      else
        if (!dropoff || (dropoff==2 &&  // large jump down (e.g. dogs)
             (tmfloorz-tmdropoffz > 128*FRACUNIT ||
              !thing->target || thing->target->z >tmdropoffz)))
        {
            if (!monkeys || !mbf_features ?
                tmfloorz - tmdropoffz > 24*FRACUNIT :
                thing->floorz  - tmfloorz > 24*FRACUNIT ||
                thing->dropoffz - tmdropoffz > 24*FRACUNIT)
              return false;
        }
        else
        { /* dropoff allowed -- check for whether it fell more than 24 */
          felldown = !(thing->flags & MF_NOGRAVITY) && thing->z - tmfloorz > 24*FRACUNIT;
        }
    }

    if (thing->flags & MF_BOUNCES &&    // killough 8/13/98
        !(thing->flags & (MF_MISSILE|MF_NOGRAVITY)) &&
        !sentient(thing) && tmfloorz - thing->z > 16*FRACUNIT)
      return false; // too big a step up for bouncers under gravity

    // killough 11/98: prevent falling objects from going up too many steps
    if (thing->intflags & MIF_FALLING && tmfloorz - thing->z >
        FixedMul(thing->momx, thing->momx) + FixedMul(thing->momy, thing->momy))
      return false;
  }

  // the move is ok,
  // so unlink from the old position and link into the new position

  P_UnsetThingPosition (thing);

  oldx = thing->x;
  oldy = thing->y;
  thing->floorz = tmfloorz;
  thing->ceilingz = tmceilingz;
  thing->dropoffz = tmdropoffz;      // killough 11/98: keep track of dropoffs
  thing->x = x;
  thing->y = y;

  P_SetThingPosition (thing);

  if (thing->flags2 & MF2_FOOTCLIP
      && P_GetThingFloorType(thing) != FLOOR_SOLID)
  {
    thing->flags2 |= MF2_FEETARECLIPPED;
  }
  else if (thing->flags2 & MF2_FEETARECLIPPED)
  {
    thing->flags2 &= ~MF2_FEETARECLIPPED;
  }

  if (map_trail_mode == map_trail_mode_ignore_collisions &&
      thing->player && thing->player->mo == thing)
  {
    AM_updatePlayerTrail(x, y);
  }

  // if any special lines were hit, do the effect

  if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)))
  {
    map_format.iterate_spechit(thing, oldx, oldy);
  }

  return true;
}

/*
 * killough 9/12/98:
 *
 * Apply "torque" to objects hanging off of ledges, so that they
 * fall off. It's not really torque, since Doom has no concept of
 * rotation, but it's a convincing effect which avoids anomalies
 * such as lifeless objects hanging more than halfway off of ledges,
 * and allows objects to roll off of the edges of moving lifts, or
 * to slide up and then back down stairs, or to fall into a ditch.
 * If more than one linedef is contacted, the effects are cumulative,
 * so balancing is possible.
 */

static dboolean PIT_ApplyTorque(line_t *ld)
{
  if (ld->backsector &&       // If thing touches two-sided pivot linedef
      (ld->dx || ld->dy) && // Torque is undefined if the line has no length
      tmbbox[BOXRIGHT]  > ld->bbox[BOXLEFT]  &&
      tmbbox[BOXLEFT]   < ld->bbox[BOXRIGHT] &&
      tmbbox[BOXTOP]    > ld->bbox[BOXBOTTOM] &&
      tmbbox[BOXBOTTOM] < ld->bbox[BOXTOP] &&
      P_BoxOnLineSide(tmbbox, ld) == -1)
  {
    mobj_t *mo = tmthing;

    fixed_t dist =                               // lever arm
                 + (ld->dx >> FRACBITS) * (mo->y >> FRACBITS)
                 - (ld->dy >> FRACBITS) * (mo->x >> FRACBITS)
                 - (ld->dx >> FRACBITS) * (ld->v1->y >> FRACBITS)
                 + (ld->dy >> FRACBITS) * (ld->v1->x >> FRACBITS);

    if (dist < 0 ?                               // dropoff direction
        ld->frontsector->floorheight < mo->z &&
        ld->backsector->floorheight >= mo->z :
        ld->backsector->floorheight < mo->z &&
        ld->frontsector->floorheight >= mo->z)
    {
      /* At this point, we know that the object straddles a two-sided
        * linedef, and that the object's center of mass is above-ground.
        */

      fixed_t x = D_abs(ld->dx), y = D_abs(ld->dy);

      if (y > x)
      {
        fixed_t t = x;
        x = y;
        y = t;
      }

      y = finesine[(tantoangle[FixedDiv(y,x)>>DBITS] + ANG90) >> ANGLETOFINESHIFT];

      /* Momentum is proportional to distance between the
        * object's center of mass and the pivot linedef.
        *
        * It is scaled by 2^(OVERDRIVE - gear). When gear is
        * increased, the momentum gradually decreases to 0 for
        * the same amount of pseudotorque, so that oscillations
        * are prevented, yet it has a chance to reach equilibrium.
        */
      dist = FixedDiv(FixedMul(dist, (mo->gear < OVERDRIVE) ?
                      y << -(mo->gear - OVERDRIVE) :
                      y >> +(mo->gear - OVERDRIVE)), x);

      /* Apply momentum away from the pivot linedef. */

      x = FixedMul(ld->dy, dist);
      y = FixedMul(ld->dx, dist);

      /* Avoid moving too fast all of a sudden (step into "overdrive") */

      dist = FixedMul(x,x) + FixedMul(y,y);

      while (dist > FRACUNIT*4 && mo->gear < MAXGEAR)
        ++mo->gear, x >>= 1, y >>= 1, dist >>= 1;

      mo->momx -= x;
      mo->momy += y;
    }
  }
  return true;
}

/*
 * killough 9/12/98
 *
 * Applies "torque" to objects, based on all contacted linedefs
 */

void P_ApplyTorque(mobj_t *mo)
{
  int xl = P_GetSafeBlockX((tmbbox[BOXLEFT] =
       mo->x - mo->radius) - bmaporgx);
  int xh = P_GetSafeBlockX((tmbbox[BOXRIGHT] =
       mo->x + mo->radius) - bmaporgx);
  int yl = P_GetSafeBlockY((tmbbox[BOXBOTTOM] =
       mo->y - mo->radius) - bmaporgy);
  int yh = P_GetSafeBlockY((tmbbox[BOXTOP] =
       mo->y + mo->radius) - bmaporgy);
  int bx,by,flags = mo->intflags; //Remember the current state, for gear-change

  tmthing = mo;
  validcount++; /* prevents checking same line twice */

  for (bx = xl ; bx <= xh ; bx++)
    for (by = yl ; by <= yh ; by++)
      P_BlockLinesIterator(bx, by, PIT_ApplyTorque);

  /* If any momentum, mark object as 'falling' using engine-internal flags */
  if (mo->momx | mo->momy)
    mo->intflags |= MIF_FALLING;
  else  // Clear the engine-internal flag indicating falling object.
    mo->intflags &= ~MIF_FALLING;

  /* If the object has been moving, step up the gear.
   * This helps reach equilibrium and avoid oscillations.
   *
   * Doom has no concept of potential energy, much less
   * of rotation, so we have to creatively simulate these
   * systems somehow :)
   */

  if (!((mo->intflags | flags) & MIF_FALLING))   // If not falling for a while,
    mo->gear = 0;                                // Reset it to full strength
  else
    if (mo->gear < MAXGEAR)                      // Else if not at max gear,
      mo->gear++;                                // move up a gear
}

//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//

dboolean P_ThingHeightClip (mobj_t* thing)
{
  dboolean   onfloor;

  onfloor = (thing->z == thing->floorz);

  P_CheckPosition (thing, thing->x, thing->y);

  /* what about stranding a monster partially off an edge?
   * killough 11/98: Answer: see below (upset balance if hanging off ledge)
   */

  thing->floorz = tmfloorz;
  thing->ceilingz = tmceilingz;
  thing->dropoffz = tmdropoffz;    /* killough 11/98: remember dropoffs */
  thing->floorpic = tmfloorpic;

  if (onfloor)
  {
    // walking monsters rise and fall with the floor
    if (
      !hexen ||
      (thing->z - thing->floorz < 9 * FRACUNIT) ||
      (thing->flags & MF_NOGRAVITY)
    )
      thing->z = thing->floorz;

    /* killough 11/98: Possibly upset balance of objects hanging off ledges */
    if (thing->intflags & MIF_FALLING && thing->gear >= MAXGEAR)
      thing->gear = 0;
  }
  else
  {
    // don't adjust a floating monster unless forced to

    if (thing->z+thing->height > thing->ceilingz)
      thing->z = thing->ceilingz - thing->height;
  }

  return thing->ceilingz - thing->floorz >= thing->height;
}


//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//

/* killough 8/2/98: make variables static */
static fixed_t   bestslidefrac;
static line_t*   bestslideline;
static mobj_t*   slidemo;
static fixed_t   tmxmove;
static fixed_t   tmymove;

//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
// If the floor is icy, then you can bounce off a wall.             // phares
//

void P_HitSlideLine (line_t* ld)
{
  int     side;
  angle_t lineangle;
  angle_t moveangle;
  angle_t deltaangle;
  fixed_t movelen;
  fixed_t newlen;
  dboolean icyfloor;  // is floor icy?                               // phares
                                                                    //   |
  // Under icy conditions, if the angle of approach to the wall     //   V
  // is more than 45 degrees, then you'll bounce and lose half
  // your momentum. If less than 45 degrees, you'll slide along
  // the wall. 45 is arbitrary and is believable.

  // Check for the special cases of horz or vert walls.

  /* killough 10/98: only bounce if hit hard (prevents wobbling)
   * cph - DEMOSYNC - should only affect players in Boom demos? */

  //e6y
  if (mbf_features || prboom_comp[PC_PRBOOM_FRICTION].state)
  {
    icyfloor =
    P_AproxDistance(tmxmove, tmymove) > 4*FRACUNIT &&
    variable_friction &&  // killough 8/28/98: calc friction on demand
    slidemo->z <= slidemo->floorz &&
    P_GetFriction(slidemo, NULL) > ORIG_FRICTION;
  }
  else
  {
    extern dboolean onground;
    icyfloor = !compatibility &&
               variable_friction &&
               slidemo->player &&
               onground &&
               slidemo->friction > ORIG_FRICTION;
  }

  if (ld->slopetype == ST_HORIZONTAL)
  {
    if (icyfloor && (D_abs(tmymove) > D_abs(tmxmove)))
    {
      tmxmove /= 2; // absorb half the momentum
      tmymove = -tmymove/2;
      S_StartMobjSound(slidemo,sfx_oof); // oooff!
    }
    else
      tmymove = 0; // no more movement in the Y direction
    return;
  }

  if (ld->slopetype == ST_VERTICAL)
  {
    if (icyfloor && (D_abs(tmxmove) > D_abs(tmymove)))
    {
      tmxmove = -tmxmove/2; // absorb half the momentum
      tmymove /= 2;
      S_StartMobjSound(slidemo,sfx_oof); // oooff!                      //   ^
    }                                                               //   |
    else                                                            // phares
      tmxmove = 0; // no more movement in the X direction
    return;
  }

  // The wall is angled. Bounce if the angle of approach is         // phares
  // less than 45 degrees.                                          // phares

  side = P_PointOnLineSide (slidemo->x, slidemo->y, ld);

  lineangle = R_PointToAngle2 (0,0, ld->dx, ld->dy);
  if (side == 1)
    lineangle += ANG180;
  moveangle = R_PointToAngle2 (0,0, tmxmove, tmymove);

  // killough 3/2/98:
  // The moveangle+=10 breaks v1.9 demo compatibility in
  // some demos, so it needs demo_compatibility switch.

  if (!demo_compatibility)
    moveangle += 10; // prevents sudden path reversal due to        // phares
                     // rounding error                              //   |
  deltaangle = moveangle-lineangle;                                 //   V
  movelen = P_AproxDistance (tmxmove, tmymove);
  if (icyfloor && (deltaangle > ANG45) && (deltaangle < ANG90+ANG45))
  {
    moveangle = lineangle - deltaangle;
    movelen /= 2; // absorb
    S_StartMobjSound(slidemo,sfx_oof); // oooff!
    moveangle >>= ANGLETOFINESHIFT;
    tmxmove = FixedMul (movelen, finecosine[moveangle]);
    tmymove = FixedMul (movelen, finesine[moveangle]);
  }                                                                 //   ^
  else                                                              //   |
  {                                                                 // phares
    if (deltaangle > ANG180)
      deltaangle += ANG180;

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;
    newlen = FixedMul (movelen, finecosine[deltaangle]);
    tmxmove = FixedMul (newlen, finecosine[lineangle]);
    tmymove = FixedMul (newlen, finesine[lineangle]);
  }                                                                 // phares
}


//
// PTR_SlideTraverse
//

dboolean PTR_SlideTraverse (intercept_t* in)
{
  line_t* li;

  if (!in->isaline)
    I_Error ("PTR_SlideTraverse: not a line?");

  li = in->d.line;

  if ( ! (li->flags & ML_TWOSIDED) )
  {
    if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
      return true; // don't hit the back side
    goto isblocking;
  }

  // defines a 'window' from one sector to another across a line

  P_LineOpening (li, slidemo);

  if (line_opening.range < slidemo->height)
    goto isblocking;  // doesn't fit

  if (line_opening.top - slidemo->z < slidemo->height)
    goto isblocking;  // mobj is too high

  if (line_opening.bottom - slidemo->z > 24*FRACUNIT )
    goto isblocking;  // too big a step up

  // this line doesn't block movement

  return true;

  // the line does block movement,
  // see if it is closer than best so far

isblocking:

  if (in->frac < bestslidefrac)
  {
    bestslidefrac = in->frac;
    bestslideline = li;
  }

  return false; // stop
}


//
// P_SlideMove
// The momx / momy move is bad, so try to slide
// along a wall.
// Find the first line hit, move flush to it,
// and slide along it
//
// This is a kludgy mess.
//
// killough 11/98: reformatted

void P_SlideMove(mobj_t *mo)
{
  int hitcount = 3;

  slidemo = mo; // the object that's sliding

  do
  {
    fixed_t leadx, leady, trailx, traily;

    if (!--hitcount)
      goto stairstep;   // don't loop forever

    // trace along the three leading corners

    if (mo->momx > 0)
      leadx = mo->x + mo->radius, trailx = mo->x - mo->radius;
    else
      leadx = mo->x - mo->radius, trailx = mo->x + mo->radius;

    if (mo->momy > 0)
      leady = mo->y + mo->radius, traily = mo->y - mo->radius;
    else
      leady = mo->y - mo->radius, traily = mo->y + mo->radius;

    bestslidefrac = FRACUNIT+1;

    P_PathTraverse(leadx, leady, leadx+mo->momx, leady+mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(trailx, leady, trailx+mo->momx, leady+mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);
    P_PathTraverse(leadx, traily, leadx+mo->momx, traily+mo->momy,
                   PT_ADDLINES, PTR_SlideTraverse);

      // move up to the wall

    if (bestslidefrac == FRACUNIT+1)
    {
      // the move must have hit the middle, so stairstep

    stairstep:

      /* killough 3/15/98: Allow objects to drop off ledges
       *
       * phares 5/4/98: kill momentum if you can't move at all
       * This eliminates player bobbing if pressed against a wall
       * while on ice.
       *
       * killough 10/98: keep buggy code around for old Boom demos
       *
       * cph 2000/09//23: buggy code was only in Boom v2.01
       */

      if (!P_TryMove(mo, mo->x, mo->y + mo->momy, true))
        if (!P_TryMove(mo, mo->x + mo->momx, mo->y, true))
          if (compatibility_level == boom_201_compatibility)
            mo->momx = mo->momy = 0;

      break;
    }

    // fudge a bit to make sure it doesn't hit

    if ((bestslidefrac -= 0x800) > 0)
    {
      fixed_t newx = FixedMul(mo->momx, bestslidefrac);
      fixed_t newy = FixedMul(mo->momy, bestslidefrac);

      // killough 3/15/98: Allow objects to drop off ledges

      if (!P_TryMove(mo, mo->x+newx, mo->y+newy, true))
        goto stairstep;
    }

    // Now continue along the wall.
    // First calculate remainder.

    bestslidefrac = FRACUNIT-(bestslidefrac+0x800);

    if (bestslidefrac > FRACUNIT)
      bestslidefrac = FRACUNIT;

    if (bestslidefrac <= 0)
      break;

    tmxmove = FixedMul(mo->momx, bestslidefrac);
    tmymove = FixedMul(mo->momy, bestslidefrac);

    P_HitSlideLine(bestslideline); // clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;

    /* killough 10/98: affect the bobbing the same way (but not voodoo dolls)
     * cph - DEMOSYNC? */
    if (!raven && mo->player && mo->player->mo == mo)
    {
      if (D_abs(mo->player->momx) > D_abs(tmxmove))
        mo->player->momx = tmxmove;
      if (D_abs(mo->player->momy) > D_abs(tmymove))
        mo->player->momy = tmymove;
    }
  }  // killough 3/15/98: Allow objects to drop off ledges:
  while (!P_TryMove(mo, mo->x+tmxmove, mo->y+tmymove, true));
}

//
// P_LineAttack
//
mobj_t*   linetarget; // who got hit (or NULL)
mobj_t*   crosshair_target;
static mobj_t*   shootthing;

/* killough 8/2/98: for more intelligent autoaiming */
static uint64_t aim_flags_mask;

// Height if not aiming up or down
fixed_t   shootz;

int       la_damage;
fixed_t   attackrange;

static fixed_t   aimslope;

// slopes to top and bottom of target
// killough 4/20/98: make static instead of using ones in p_sight.c

static fixed_t  topslope;
static fixed_t  bottomslope;


//
// PTR_AimTraverse
// Sets linetaget and aimslope when a target is aimed at.
//
dboolean PTR_AimTraverse (intercept_t* in)
{
  line_t* li;
  mobj_t* th;
  fixed_t slope;
  fixed_t thingtopslope;
  fixed_t thingbottomslope;
  fixed_t dist;

  if (in->isaline)
  {
    li = in->d.line;

    if ( !(li->flags & (ML_TWOSIDED | ML_BLOCKEVERYTHING)) )
      return false;   // stop

    // Crosses a two sided line.
    // A two sided line will restrict
    // the possible target ranges.

    P_LineOpening (li, NULL);

    if (line_opening.bottom >= line_opening.top)
      return false;   // stop

    dist = FixedMul (attackrange, in->frac);

    // e6y: emulation of missed back side on two-sided lines.
    // backsector can be NULL if overrun_missedbackside_emulate is 1
    if (!li->backsector || li->frontsector->floorheight != li->backsector->floorheight)
    {
      slope = FixedDiv (line_opening.bottom - shootz , dist);
      if (slope > bottomslope)
        bottomslope = slope;
    }

    // e6y: emulation of missed back side on two-sided lines.
    if (!li->backsector || li->frontsector->ceilingheight != li->backsector->ceilingheight)
    {
      slope = FixedDiv (line_opening.top - shootz , dist);
      if (slope < topslope)
        topslope = slope;
    }

    if (topslope <= bottomslope)
      return false;   // stop

    return true;    // shot continues
  }

  // shoot a thing

  th = in->d.thing;
  if (th == shootthing)
    return true;    // can't shoot self

  if (!(th->flags&MF_SHOOTABLE))
    return true;    // corpse or something

  if (heretic && th->type == HERETIC_MT_POD)
    return true;    // Can't auto-aim at pods

  if (hexen && th->player && netgame && !deathmatch)
  {                           // don't aim at fellow co-op players
      return true;
  }

  /* killough 7/19/98, 8/2/98:
   * friends don't aim at friends (except players), at least not first
   */
  if (th->flags & shootthing->flags & aim_flags_mask && !th->player)
    return true;

  // check angles to see if the thing can be aimed at

  dist = FixedMul (attackrange, in->frac);
  thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

  if (thingtopslope < bottomslope)
    return true;    // shot over the thing

  thingbottomslope = FixedDiv (th->z - shootz, dist);

  if (thingbottomslope > topslope)
    return true;    // shot under the thing

  // this thing can be hit!

  if (thingtopslope > topslope)
    thingtopslope = topslope;

  if (thingbottomslope < bottomslope)
    thingbottomslope = bottomslope;

  aimslope = (thingtopslope+thingbottomslope)/2;
  linetarget = th;

  return false;   // don't go any farther
}

// heretic
extern mobjtype_t PuffType;

//
// PTR_ShootTraverse
//
dboolean PTR_ShootTraverse (intercept_t* in)
{
  fixed_t x;
  fixed_t y;
  fixed_t z;
  fixed_t frac;

  mobj_t* th;

  fixed_t slope;
  fixed_t dist;
  fixed_t thingtopslope;
  fixed_t thingbottomslope;

  if (in->isaline)
  {
    line_t *li = in->d.line;

    if (li->special)
      map_format.shoot_special_line(shootthing, li);

    if (li->flags & ML_TWOSIDED &&
        !(li->flags & (ML_BLOCKEVERYTHING | ML_BLOCKHITSCAN)))
    {  // crosses a two sided (really 2s) line
      P_LineOpening (li, NULL);
      dist = FixedMul(attackrange, in->frac);

      // killough 11/98: simplify

      // e6y: emulation of missed back side on two-sided lines.
      // backsector can be NULL if overrun_missedbackside_emulate is 1
      if (!li->backsector)
      {
        if ((slope = FixedDiv(line_opening.bottom - shootz , dist)) <= aimslope &&
            (slope = FixedDiv(line_opening.top - shootz , dist)) >= aimslope)
          return true;      // shot continues
      }
      else
        if ((li->frontsector->floorheight==li->backsector->floorheight ||
             (slope = FixedDiv(line_opening.bottom - shootz , dist)) <= aimslope) &&
            (li->frontsector->ceilingheight==li->backsector->ceilingheight ||
             (slope = FixedDiv (line_opening.top - shootz , dist)) >= aimslope))
          return true;      // shot continues
    }

    // hit line
    // position a bit closer

    if (dsda_FreeAim())
    {
      int64_t real_z;
      int side = P_PointOnLineSide(trace.x, trace.y, li);
      sector_t *sec = side ? li->backsector : li->frontsector;

      real_z = (int64_t) shootz + FixedMul64(aimslope, FixedMul(in->frac, attackrange));
      z = real_z > INT_MAX ? INT_MAX :
          real_z < INT_MIN ? INT_MIN :
          real_z;

      if (sec && sec->floorheight > z)
      {
        fixed_t dist;

        if (sec->floorpic == skyflatnum)
          return false;

        z = sec->floorheight;
        dist = FixedDiv(z - shootz, aimslope);
        frac = FixedDiv(dist, attackrange);
      }
      else if (sec && sec->ceilingheight < z)
      {
        fixed_t dist;

        if (sec->ceilingpic == skyflatnum)
          return false;

        // puff spawn height is +/- (255 << 10)
        z = sec->ceilingheight - mobjinfo[MT_PUFF].height - (255 << 10);
        dist = FixedDiv(z - shootz, aimslope);
        frac = FixedDiv(dist, attackrange);
      }
      else
      {
        frac = in->frac - FixedDiv(4 * FRACUNIT, attackrange);
        z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));
      }
    }
    else
    {
      frac = in->frac - FixedDiv(4 * FRACUNIT, attackrange);
      z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

      if (li->frontsector->ceilingpic == skyflatnum)
      {
        // don't shoot the sky!

        if (z > li->frontsector->ceilingheight)
          return false;

        // it's a sky hack wall

        if  (li->backsector && li->backsector->ceilingpic == skyflatnum)

          // fix bullet-eaters -- killough:
          // WARNING: Almost all demos will lose sync without this
          // demo_compatibility flag check!!! killough 1/18/98
          if (demo_compatibility || li->backsector->ceilingheight < z)
            return false;
      }
    }

    x = trace.x + FixedMul (trace.dx, frac);
    y = trace.y + FixedMul (trace.dy, frac);

    if (li->health)
    {
      dsda_DamageLinedef(li, shootthing, la_damage);
    }

    // Spawn bullet puffs.

    P_SpawnPuff (x,y,z);

    // don't go any farther

    return false;
  }

  // shoot a thing

  th = in->d.thing;
  if (th == shootthing)
    return true;  // can't shoot self

  if (!(th->flags&MF_SHOOTABLE))
    return true;  // corpse or something

  if (heretic && th->flags & MF_SHADOW && shootthing->player->readyweapon == wp_staff)
    return true;

  // check angles to see if the thing can be aimed at

  dist = FixedMul (attackrange, in->frac);
  thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

  if (thingtopslope < aimslope)
    return true;  // shot over the thing

  thingbottomslope = FixedDiv (th->z - shootz, dist);

  if (thingbottomslope > aimslope)
    return true;  // shot under the thing

  // hit thing
  // position a bit closer

  frac = in->frac - FixedDiv (10*FRACUNIT,attackrange);

  x = trace.x + FixedMul (trace.dx, frac);
  y = trace.y + FixedMul (trace.dy, frac);
  z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

  // Spawn bullet puffs or blod spots,
  // depending on target type.
  if (heretic && PuffType == HERETIC_MT_BLASTERPUFF1)
  {                           // Make blaster big puff
    mobj_t* mo;
    mo = P_SpawnMobj(x, y, z, HERETIC_MT_BLASTERPUFF2);
    S_StartMobjSound(mo, heretic_sfx_blshit);
  }
  else
  {
    if (raven || in->d.thing->flags & MF_NOBLOOD)
      P_SpawnPuff (x,y,z);
    else
      P_SpawnBlood (x,y,z, la_damage, th);
  }

  if (la_damage)
  {
    if (
      raven &&
      !(in->d.thing->flags & MF_NOBLOOD) &&
      !(in->d.thing->flags2 & MF2_INVULNERABLE)
    )
    {
      if (PuffType == HEXEN_MT_AXEPUFF || PuffType == HEXEN_MT_AXEPUFF_GLOW)
      {
        P_BloodSplatter2(x, y, z, in->d.thing);
      }
      if (P_Random(pr_heretic) < 192)
      {
        P_BloodSplatter(x, y, z, in->d.thing);
      }
    }

    if (hexen && PuffType == HEXEN_MT_FLAMEPUFF2)
    {                       // Cleric FlameStrike does fire damage
      extern mobj_t LavaInflictor;

      P_DamageMobj(th, &LavaInflictor, shootthing, la_damage);
    }
    else
    {
      P_DamageMobj(th, shootthing, shootthing, la_damage);
    }
  }

  // don't go any farther
  return false;
}


//
// P_AimLineAttack
//
fixed_t P_AimLineAttack(mobj_t* t1,angle_t angle,fixed_t distance, uint64_t mask)
{
  fixed_t x2;
  fixed_t y2;

  t1 = P_SubstNullMobj(t1);

  angle >>= ANGLETOFINESHIFT;
  shootthing = t1;

  x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
  y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
  shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

  // can't shoot outside view angles

  topslope = 100*FRACUNIT/160;
  bottomslope = -100*FRACUNIT/160;

  attackrange = distance;
  linetarget = NULL;

  /* killough 8/2/98: prevent friends from aiming at friends */
  aim_flags_mask = mask;

  P_PathTraverse(t1->x,t1->y,x2,y2,PT_ADDLINES|PT_ADDTHINGS,PTR_AimTraverse);

  if (linetarget)
    return aimslope;

  return 0;
}


//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
//

void P_LineAttack(mobj_t* t1, angle_t angle, fixed_t distance, fixed_t slope,
                  int damage)
{
  fixed_t x2;
  fixed_t y2;

  angle >>= ANGLETOFINESHIFT;
  shootthing = t1;
  la_damage = damage;
  x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
  y2 = t1->y + (distance>>FRACBITS)*finesine[angle];
  shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
  if (hexen)
  {
    shootz -= t1->floorclip;
  }
  else if (t1->flags2 & MF2_FEETARECLIPPED)
  {
    shootz -= FOOTCLIPSIZE;
  }
  attackrange = distance;
  aimslope = slope;

  if (P_PathTraverse(t1->x,t1->y,x2,y2,PT_ADDLINES|PT_ADDTHINGS,PTR_ShootTraverse))
  {
    if (hexen)
    {
      switch (PuffType)
      {
        case HEXEN_MT_PUNCHPUFF:
          S_StartMobjSound(t1, hexen_sfx_fighter_punch_miss);
          break;
        case HEXEN_MT_HAMMERPUFF:
        case HEXEN_MT_AXEPUFF:
        case HEXEN_MT_AXEPUFF_GLOW:
          S_StartMobjSound(t1, hexen_sfx_fighter_hammer_miss);
          break;
        case HEXEN_MT_FLAMEPUFF:
          P_SpawnPuff(x2, y2, shootz + FixedMul(slope, distance));
          break;
        default:
          break;
      }
    }
  }
}


//
// USE LINES
//

mobj_t*   usething;

dboolean PTR_UseTraverse (intercept_t* in)
{
  int side;

  if (!in->d.line->special)
  {
    int sound;

    if (in->d.line->flags & (ML_BLOCKEVERYTHING | ML_BLOCKUSE))
    {
      line_opening.range = 0;
    }
    else
    {
      P_LineOpening (in->d.line, NULL);
    }

    if (line_opening.range <= 0)
    {
      if (hexen && usething->player)
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
        S_StartMobjSound(usething, sound);
      }
      else if (!heretic)
      {
        S_StartSound (usething, sfx_noway);
      }

      // can't use through a wall
      return false;
    }

    if (hexen && usething->player)
    {
      fixed_t pheight = usething->z + (usething->height / 2);
      if ((line_opening.top < pheight) || (line_opening.bottom > pheight))
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
        S_StartMobjSound(usething, sound);
      }
    }

    // not a special line, but keep checking

    return true;
  }

  side = 0;
  if (P_PointOnLineSide (usething->x, usething->y, in->d.line) == 1)
    side = 1;

  //  return false;   // don't use back side

  P_UseSpecialLine (usething, in->d.line, side, false);

  //WAS can't use for than one special line in a row
  //jff 3/21/98 NOW multiple use allowed with enabling line flag

  return (!demo_compatibility && ((in->d.line->flags&ML_PASSUSE) || comperr(comperr_passuse)))?//e6y
          true : false;
}

// Returns false if a "oof" sound should be made because of a blocking
// linedef. Makes 2s middles which are impassable, as well as 2s uppers
// and lowers which block the player, cause the sound effect when the
// player tries to activate them. Specials are excluded, although it is
// assumed that all special linedefs within reach have been considered
// and rejected already (see P_UseLines).
//
// by Lee Killough
//

dboolean PTR_NoWayTraverse(intercept_t* in)
{
  line_t *ld = in->d.line;
                                           // This linedef
  return ld->special || !(                 // Ignore specials
   ld->flags & ML_BLOCKING || (            // Always blocking
   P_LineOpening(ld, NULL),                // Find openings
   line_opening.range <= 0 ||                       // No opening
   line_opening.bottom > usething->z+24*FRACUNIT || // Too high it blocks
   line_opening.top < usething->z+usething->height  // Too low it blocks
  )
  );
}

//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines (player_t*  player)
{
  int     angle;
  fixed_t x1;
  fixed_t y1;
  fixed_t x2;
  fixed_t y2;

  usething = player->mo;

  angle = player->mo->angle >> ANGLETOFINESHIFT;

  x1 = player->mo->x;
  y1 = player->mo->y;
  x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
  y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];

  // old code:
  //
  // P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse );
  //
  // This added test makes the "oof" sound work on 2s lines -- killough:

  if (P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse ))
    if (!comp[comp_sound] && !P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_NoWayTraverse ))
      S_StartSound (usething, sfx_noway);
}


//
// RADIUS ATTACK
//

bomb_t bomb;

//
// PIT_RadiusAttack
// "bomb.source" is the creature
// that caused the explosion at "bomb.spot".
//

static dboolean P_SplashImmune(mobj_t *target, mobj_t *spot)
{
  return // not default behaviour and same group
    mobjinfo[target->type].splash_group != SG_DEFAULT &&
    mobjinfo[target->type].splash_group == mobjinfo[spot->type].splash_group;
}

int P_SplashDamage(fixed_t dist)
{
  int damage;

  // [XA] independent damage/distance calculation.
  //      same formula as eternity; thanks Quas :P
  if (!hexen && bomb.damage == bomb.distance)
    damage = bomb.damage - dist;
  else
    damage = (bomb.damage * (bomb.distance - dist) / bomb.distance) + 1;

  return damage;
}

dboolean PIT_RadiusAttack (mobj_t* thing)
{
  fixed_t dx;
  fixed_t dy;
  fixed_t dist;
  int damage;

  /* killough 8/20/98: allow bouncers to take damage
   * (missile bouncers are already excluded with MF_NOBLOCKMAP)
   */

  if (!(thing->flags & (MF_SHOOTABLE | MF_BOUNCES)))
    return true;

  if (P_SplashImmune(thing, bomb.spot))
    return true;

  if (hexen)
  {
    if (!(bomb.flags & BF_DAMAGESOURCE) && thing == bomb.source)
    {                           // don't damage the source of the explosion
      return true;
    }
    if (D_abs((thing->z - bomb.spot->z) >> FRACBITS) > 2 * bomb.distance)
    {                           // too high/low
      return true;
    }
  }
  else
  {
    // Boss spider and cyborg
    // take no damage from concussion.

    // killough 8/10/98: allow grenades to hurt anyone, unless
    // fired by Cyberdemons, in which case it won't hurt Cybers.

    if (bomb.spot->flags & MF_BOUNCES ?
        thing->type == MT_CYBORG && bomb.source->type == MT_CYBORG :
        thing->flags2 & (MF2_NORADIUSDMG | MF2_BOSS) &&
        !(bomb.spot->flags2 & MF2_FORCERADIUSDMG))
      return true;
  }

  dx = D_abs(thing->x - bomb.spot->x);
  dy = D_abs(thing->y - bomb.spot->y);

  dist = dx > dy ? dx : dy;

  if (map_info.flags & MI_EXPLODE_IN_3D &&
      (bomb.spot->z < thing->z || bomb.spot->z >= thing->z + thing->height))
  {
    fixed_t dz;

    if (bomb.spot->z > thing->z)
    {
      dz = bomb.spot->z - thing->z - thing->height;
    }
    else
    {
      dz = thing->z - bomb.spot->z;
    }

    if (dist <= thing->radius)
    {
      dist = dz;
    }
    else
    {
      dist -= thing->radius;
      dist = P_AproxDistance(dist, dz);
    }
  }
  else
  {
    dist -= thing->radius;

    if (dist < 0)
      dist = 0;
  }

  dist >>= FRACBITS;

  if (dist >= bomb.distance)
    return true;  // out of range

  if ( P_CheckSight (thing, bomb.spot) )
  {
    // must be in direct path

    damage = P_SplashDamage(dist);

    if (hexen && thing->player)
    {
      damage >>= 2;
    }

    P_DamageMobj (thing, bomb.spot, bomb.source, damage);

    if (map_info.flags & MI_VERTICAL_EXPLOSION_THRUST && !(bomb.flags & BF_HORIZONTAL))
    {
      fixed_t thrust;
      fixed_t dxy, dz;
      angle_t an;

      dxy = P_AproxDistance(dx, dy);
      dz = thing->z + thing->height / 2 - bomb.spot->z;
      an = R_PointToAngle2(0, 0, dxy, dz);

      thrust = damage * (FRACUNIT >> 3) * g_thrust_factor / thing->info->mass;

      thing->momz += FixedMul(thrust, finesine[an >> ANGLETOFINESHIFT]);
    }
  }

  return true;
}

//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void P_RadiusAttack(mobj_t* spot,mobj_t* source, int damage, int distance, int flags)
{
  int x;
  int y;

  int xl;
  int xh;
  int yl;
  int yh;

  fixed_t dist;

  dist = (distance+MAXRADIUS)<<FRACBITS;
  yh = P_GetSafeBlockY(spot->y + dist - bmaporgy);
  yl = P_GetSafeBlockY(spot->y - dist - bmaporgy);
  xh = P_GetSafeBlockX(spot->x + dist - bmaporgx);
  xl = P_GetSafeBlockX(spot->x - dist - bmaporgx);

  bomb.spot = spot;
  if (heretic && spot->type == HERETIC_MT_POD && spot->target)
  {
    bomb.source = spot->target;
  }
  else
  {
    bomb.source = source;
  }
  bomb.damage = damage;
  bomb.distance = distance;
  bomb.flags = flags;

  for (y=yl ; y<=yh ; y++)
    for (x=xl ; x<=xh ; x++)
      P_BlockThingsIterator (x, y, PIT_RadiusAttack );

  if (map_format.zdoom)
  {
    dsda_RadiusAttackDestructibles(xl, xh, yl, yh);
  }
}


//
// PIT_ChangeSector
//

dboolean PIT_ChangeSector (mobj_t* thing)
{
  mobj_t* mo;

  if (P_ThingHeightClip (thing))
    return true; // keep checking

  // crunch bodies to giblets

  if (thing->health <= 0)
  {
    if (hexen)
    {
      if ((thing->flags & MF_CORPSE))
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
            S_StartMobjSound(thing, hexen_sfx_player_falling_splat);
          }
        }
        return true;            // keep checking
      }
    }
    else
    {
      if (!heretic) P_SetMobjState (thing, S_GIBS);

      if (compatibility_level != doom_12_compatibility)
      {
        thing->flags &= ~MF_SOLID;
      }
      thing->height = 0;
      thing->radius = 0;
      thing->color = thing->info->bloodcolor;
      return true; // keep checking
    }
  }

  // crunch dropped items

  if (thing->flags & MF_DROPPED)
  {
    P_RemoveMobj (thing);

    // keep checking
    return true;
  }

  /* killough 11/98: kill touchy things immediately */
  if (thing->flags & MF_TOUCHY &&
      (thing->intflags & MIF_ARMED || sentient(thing)))
  {
    P_DamageMobj(thing, NULL, NULL, thing->health);  // kill object
    return true;   // keep checking
  }

  if (! (thing->flags & MF_SHOOTABLE) )
  {
    // assume it is bloody gibs or something
    return true;
  }

  nofit = true;

  if (crushchange > 0 && !(leveltime & 3)) {
    int t;

    P_DamageMobj(thing, NULL, NULL, crushchange);
    dsda_WatchCrush(thing, crushchange);

    if (
      !hexen ||
      (
        !(thing->flags & MF_NOBLOOD) &&
        !(thing->flags2 & MF2_INVULNERABLE)
      )
    )
    {
      // spray blood in a random direction
      mo = P_SpawnMobj (thing->x,
                        thing->y,
                        thing->z + thing->height / 2, g_mt_blood);
      mo->color = thing->info->bloodcolor;

      /* killough 8/10/98: remove dependence on order of evaluation */
      t = P_Random(pr_crush);
      mo->momx = (t - P_Random (pr_crush)) << 12;
      t = P_Random(pr_crush);
      mo->momy = (t - P_Random (pr_crush)) << 12;
    }
  }

  // keep checking (crush other things)
  return true;
}


//
// P_ChangeSector
//
dboolean P_ChangeSector(sector_t* sector, int crunch)
{
  int   x;
  int   y;

  nofit = false;
  crushchange = crunch;

  if (crushchange == STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE)
    crushchange = DOOM_CRUSH;

  // ARRGGHHH!!!!
  // This is horrendously slow!!!
  // killough 3/14/98

  // re-check heights for all things near the moving sector

  for (x=sector->blockbox[BOXLEFT] ; x<= sector->blockbox[BOXRIGHT] ; x++)
    for (y=sector->blockbox[BOXBOTTOM];y<= sector->blockbox[BOXTOP] ; y++)
      P_BlockThingsIterator (x, y, PIT_ChangeSector);

  return nofit;
}

void P_InitSectorSearch(mobj_in_sector_t *data, sector_t *sector)
{
  data->sector = sector;

  for (data->node = data->sector->touching_thinglist;
       data->node;
       data->node = data->node->m_snext)
    data->node->visited = false;

  data->node = data->sector->touching_thinglist;
}

mobj_t *P_FindMobjInSector(mobj_in_sector_t *data)
{
  while (data->node && data->node->visited)
    data->node = data->node->m_snext;

  if (!data->node)
    return NULL;

  data->node->visited = true;

  return data->node->m_thing;
}

//
// P_CheckSector
// jff 3/19/98 added to just check monsters on the periphery
// of a moving sector instead of all in bounding box of the
// sector. Both more accurate and faster.
//

dboolean P_CheckSector(sector_t* sector, int crunch)
{
  msecnode_t *n;

  if (comp[comp_floors]) /* use the old routine for old demos though */
    return P_ChangeSector(sector,crunch);

  nofit = false;
  crushchange = crunch;

  if (crushchange == STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE)
    crushchange = DOOM_CRUSH;

  // killough 4/4/98: scan list front-to-back until empty or exhausted,
  // restarting from beginning after each thing is processed. Avoids
  // crashes, and is sure to examine all things in the sector, and only
  // the things which are in the sector, until a steady-state is reached.
  // Things can arbitrarily be inserted and removed and it won't mess up.
  //
  // killough 4/7/98: simplified to avoid using complicated counter

  // Mark all things invalid

  for (n=sector->touching_thinglist; n; n=n->m_snext)
    n->visited = false;

  do
    for (n=sector->touching_thinglist; n; n=n->m_snext)  // go through list
      if (!n->visited)               // unprocessed thing found
        {
        n->visited  = true;          // mark thing as processed
        if (!(n->m_thing->flags & MF_NOBLOCKMAP)) //jff 4/7/98 don't do these
          PIT_ChangeSector(n->m_thing);    // process it
        break;                 // exit and start over
        }
  while (n);  // repeat from scratch until all things left are marked valid

  return nofit;
}


#define USE_BLOCK_MEMORY_ALLOCATOR
#ifdef USE_BLOCK_MEMORY_ALLOCATOR
// CPhipps -
// Use block memory allocator here

#include "z_bmalloc.h"

IMPLEMENT_BLOCK_MEMORY_ALLOC_ZONE(secnodezone, sizeof(msecnode_t), 256, "SecNodes");

//
// P_FreeSecNodeList
//
void P_FreeSecNodeList(void)
{
  DECLARE_BLOCK_MEMORY_ALLOC_ZONE(secnodezone);
  NULL_BLOCK_MEMORY_ALLOC_ZONE(secnodezone);
}

msecnode_t* P_GetSecnode(void)
{
  return (msecnode_t*)Z_BMalloc(&secnodezone);
}

// P_PutSecnode() returns a node to the freelist.

inline static void P_PutSecnode(msecnode_t* node)
{
  Z_BFree(&secnodezone, node);
}
#else // USE_BLOCK_MEMORY_ALLOCATOR
// phares 3/21/98
//
// Maintain a freelist of msecnode_t's to reduce memory allocs and frees.

msecnode_t *headsecnode = NULL;

//
// P_FreeSecNodeList
//
void P_FreeSecNodeList(void)
{
   headsecnode = NULL; // this is all thats needed to fix the bug
}

//
// P_GetSecnode
//
// Retrieves a node from the freelist. The calling routine should make sure it
// sets all fields properly.
//
// killough 11/98: reformatted
//
msecnode_t *P_GetSecnode(void)
{
  msecnode_t *node;

  return headsecnode ?
    node = headsecnode, headsecnode = node->m_snext, node :
  (msecnode_t *)(Z_MallocLevel(sizeof *node));
}

//
// P_PutSecnode
//
// Returns a node to the freelist.
//
static void P_PutSecnode(msecnode_t *node)
{
  node->m_snext = headsecnode;
  headsecnode = node;
}
#endif // USE_BLOCK_MEMORY_ALLOCATOR

// phares 3/16/98
//
// P_AddSecnode() searches the current list to see if this sector is
// already there. If not, it adds a sector node at the head of the list of
// sectors this object appears in. This is called when creating a list of
// nodes that will get linked in later. Returns a pointer to the new node.

msecnode_t* P_AddSecnode(sector_t* s, mobj_t* thing, msecnode_t* nextnode)
{
  msecnode_t* node;

  node = nextnode;
  while (node)
    {
    if (node->m_sector == s)   // Already have a node for this sector?
      {
      node->m_thing = thing; // Yes. Setting m_thing says 'keep it'.
      return(nextnode);
      }
    node = node->m_tnext;
    }

  // Couldn't find an existing node for this sector. Add one at the head
  // of the list.

  node = P_GetSecnode();

  // killough 4/4/98, 4/7/98: mark new nodes unvisited.
  node->visited = 0;

  node->m_sector = s;       // sector
  node->m_thing  = thing;     // mobj
  node->m_tprev  = NULL;    // prev node on Thing thread
  node->m_tnext  = nextnode;  // next node on Thing thread
  if (nextnode)
    nextnode->m_tprev = node; // set back link on Thing

  // Add new node at head of sector thread starting at s->touching_thinglist

  node->m_sprev  = NULL;    // prev node on sector thread
  node->m_snext  = s->touching_thinglist; // next node on sector thread
  if (s->touching_thinglist)
    node->m_snext->m_sprev = node;
  s->touching_thinglist = node;
  return(node);
}


// P_DelSecnode() deletes a sector node from the list of
// sectors this object appears in. Returns a pointer to the next node
// on the linked list, or NULL.

msecnode_t* P_DelSecnode(msecnode_t* node)
{
  msecnode_t* tp;  // prev node on thing thread
  msecnode_t* tn;  // next node on thing thread
  msecnode_t* sp;  // prev node on sector thread
  msecnode_t* sn;  // next node on sector thread

  if (node)
    {

    // Unlink from the Thing thread. The Thing thread begins at
    // sector_list and not from mobj_t->touching_sectorlist.

    tp = node->m_tprev;
    tn = node->m_tnext;
    if (tp)
      tp->m_tnext = tn;
    if (tn)
      tn->m_tprev = tp;

    // Unlink from the sector thread. This thread begins at
    // sector_t->touching_thinglist.

    sp = node->m_sprev;
    sn = node->m_snext;
    if (sp)
      sp->m_snext = sn;
    else
      node->m_sector->touching_thinglist = sn;
    if (sn)
      sn->m_sprev = sp;

    // Return this node to the freelist

    P_PutSecnode(node);
    return(tn);
    }
  return(NULL);
}                               // phares 3/13/98

// Delete an entire sector list

void P_DelSeclist(msecnode_t* node)

  {
  while (node)
    node = P_DelSecnode(node);
  }


// phares 3/14/98
//
// PIT_GetSectors
// Locates all the sectors the object is in by looking at the lines that
// cross through it. You have already decided that the object is allowed
// at this location, so don't bother with checking impassable or
// blocking lines.

dboolean PIT_GetSectors(line_t* ld)
{
  if (tmbbox[BOXRIGHT]  <= ld->bbox[BOXLEFT]   ||
      tmbbox[BOXLEFT]   >= ld->bbox[BOXRIGHT]  ||
      tmbbox[BOXTOP]    <= ld->bbox[BOXBOTTOM] ||
      tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
    return true;

  if (P_BoxOnLineSide(tmbbox, ld) != -1)
    return true;

  // This line crosses through the object.

  // Collect the sector(s) from the line and add to the
  // sector_list you're examining. If the Thing ends up being
  // allowed to move to this position, then the sector_list
  // will be attached to the Thing's mobj_t at touching_sectorlist.

  sector_list = P_AddSecnode(ld->frontsector,tmthing,sector_list);

  /* Don't assume all lines are 2-sided, since some Things
   * like MT_TFOG are allowed regardless of whether their radius takes
   * them beyond an impassable linedef.
   *
   * killough 3/27/98, 4/4/98:
   * Use sidedefs instead of 2s flag to determine two-sidedness.
   * killough 8/1/98: avoid duplicate if same sector on both sides
   * cph - DEMOSYNC? */

  if (ld->backsector && ld->backsector != ld->frontsector)
    sector_list = P_AddSecnode(ld->backsector, tmthing, sector_list);

  return true;
}


// phares 3/14/98
//
// P_CreateSecNodeList alters/creates the sector_list that shows what sectors
// the object resides in.

void P_CreateSecNodeList(mobj_t* thing,fixed_t x,fixed_t y)
{
  int xl;
  int xh;
  int yl;
  int yh;
  int bx;
  int by;
  msecnode_t* node;
  mobj_t* saved_tmthing = tmthing; /* cph - see comment at func end */
  fixed_t saved_tmx = tmx, saved_tmy = tmy; /* ditto */

  // First, clear out the existing m_thing fields. As each node is
  // added or verified as needed, m_thing will be set properly. When
  // finished, delete all nodes where m_thing is still NULL. These
  // represent the sectors the Thing has vacated.

  node = sector_list;
  while (node)
    {
    node->m_thing = NULL;
    node = node->m_tnext;
    }

  tmthing = thing;

  tmx = x;
  tmy = y;

  tmbbox[BOXTOP]  = y + tmthing->radius;
  tmbbox[BOXBOTTOM] = y - tmthing->radius;
  tmbbox[BOXRIGHT]  = x + tmthing->radius;
  tmbbox[BOXLEFT]   = x - tmthing->radius;

  validcount2++; // used to make sure we only process a line once

  xl = P_GetSafeBlockX(tmbbox[BOXLEFT] - bmaporgx);
  xh = P_GetSafeBlockX(tmbbox[BOXRIGHT] - bmaporgx);
  yl = P_GetSafeBlockY(tmbbox[BOXBOTTOM] - bmaporgy);
  yh = P_GetSafeBlockY(tmbbox[BOXTOP] - bmaporgy);

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      P_BlockLinesIterator2(bx,by,PIT_GetSectors);

  // Add the sector of the (x,y) point to sector_list.

  sector_list = P_AddSecnode(thing->subsector->sector,thing,sector_list);

  // Now delete any nodes that won't be used. These are the ones where
  // m_thing is still NULL.

  node = sector_list;
  while (node)
    {
    if (node->m_thing == NULL)
      {
      if (node == sector_list)
        sector_list = node->m_tnext;
      node = P_DelSecnode(node);
      }
    else
      node = node->m_tnext;
    }

  /* cph -
   * This is the strife we get into for using global variables. tmthing
   *  is being used by several different functions calling
   *  P_BlockThingIterator, including functions that can be called *from*
   *  P_BlockThingIterator. Using a global tmthing is not reentrant.
   * OTOH for Boom/MBF demos we have to preserve the buggy behavior.
   *  Fun. We restore its previous value unless we're in a Boom/MBF demo.
   */
  if (!prboom_comp[PC_FORCE_LXDOOM_DEMO_COMPATIBILITY].state)
  if ((compatibility_level < boom_compatibility_compatibility) ||
      (compatibility_level >= prboom_3_compatibility))
    tmthing = saved_tmthing;
  /* And, duh, the same for tmx/y - cph 2002/09/22
   * And for tmbbox - cph 2003/08/10 */
  if ((compatibility_level < boom_compatibility_compatibility) /* ||
      (compatibility_level >= prboom_4_compatibility) */) {
    tmx = saved_tmx, tmy = saved_tmy;
    if (tmthing) {
      tmbbox[BOXTOP]  = tmy + tmthing->radius;
      tmbbox[BOXBOTTOM] = tmy - tmthing->radius;
      tmbbox[BOXRIGHT]  = tmx + tmthing->radius;
      tmbbox[BOXLEFT]   = tmx - tmthing->radius;
    }
  }
}

/* cphipps 2004/08/30 -
 * Must clear tmthing at tic end, as it might contain a pointer to a removed thinker, or the level might have ended/been ended and we clear the objects it was pointing too. Hopefully we don't need to carry this between tics for sync. */
void P_MapStart(void) {
  if (tmthing) I_Error("P_MapStart: tmthing set!");
}
void P_MapEnd(void) {
  tmthing = NULL;
}

// heretic

mobj_t *onmobj; // generic global onmobj...used for landing on pods/players

dboolean P_TestMobjLocation(mobj_t * mobj)
{
    int flags;

    flags = mobj->flags;
    mobj->flags &= ~MF_PICKUP;
    if (P_CheckPosition(mobj, mobj->x, mobj->y))
    {                           // XY is ok, now check Z
        mobj->flags = flags;
        if ((mobj->z < mobj->floorz)
            || (mobj->z + mobj->height > mobj->ceilingz))
        {                       // Bad Z
            return (false);
        }
        return (true);
    }
    mobj->flags = flags;
    return (false);
}

dboolean PIT_CheckOnmobjZ(mobj_t * thing)
{
    fixed_t blockdist;

    if (!(thing->flags & (MF_SOLID | MF_SPECIAL | MF_SHOOTABLE)))
    {                           // Can't hit thing
        return (true);
    }
    blockdist = thing->radius + tmthing->radius;
    if (abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist)
    {                           // Didn't hit thing
        return (true);
    }
    if (thing == tmthing)
    {                           // Don't clip against self
        return (true);
    }
    if (tmthing->z > thing->z + thing->height)
    {
        return (true);
    }
    else if (tmthing->z + tmthing->height < thing->z)
    {                           // under thing
        return (true);
    }
    if (thing->flags & MF_SOLID)
    {
        onmobj = thing;
    }
    return (!(thing->flags & MF_SOLID));
}

// Checks if the new Z position is legal
mobj_t *P_CheckOnmobj(mobj_t * thing)
{
    int xl, xh, yl, yh, bx, by;
    sector_t *newsec;
    fixed_t x;
    fixed_t y;
    mobj_t oldmo;

    x = thing->x;
    y = thing->y;
    tmthing = thing;
    tmflags = thing->flags;
    oldmo = *thing;             // save the old mobj before the fake zmovement
    P_FakeZMovement(tmthing);

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsec = R_PointInSector(x, y);
    ceilingline = NULL;

//
// the base floor / ceiling is from the subsector that contains the
// point.  Any contacted lines the step closer together will adjust them
//
    tmfloorz = tmdropoffz = newsec->floorheight;
    tmceilingz = newsec->ceilingheight;
    tmfloorpic = newsec->floorpic;

    validcount++;
    numspechit = 0;

    if (tmflags & MF_NOCLIP)
        return NULL;

//
// check things first, possibly picking things up
// the bounding box is extended by MAXRADIUS because mobj_ts are grouped
// into mapblocks based on their origin point, and can overlap into adjacent
// blocks by up to MAXRADIUS units
//
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_CheckOnmobjZ))
            {
                *tmthing = oldmo;
                return onmobj;
            }
    *tmthing = oldmo;
    return NULL;
}

void P_FakeZMovement(mobj_t * mo)
{
    int dist;
    int delta;
//
// adjust height
//
    mo->z += mo->momz;
    if (mo->flags & MF_FLOAT && mo->target)
    {                           // float down towards target if too close
        if (!(mo->flags & MF_SKULLFLY) && !(mo->flags & MF_INFLOAT))
        {
            dist =
                P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y);
            delta = (mo->target->z + (mo->height >> 1)) - mo->z;
            if (delta < 0 && dist < -(delta * 3))
                mo->z -= FLOATSPEED;
            else if (delta > 0 && dist < (delta * 3))
                mo->z += FLOATSPEED;
        }
    }
    if (mo->player && mo->flags2 & MF2_FLY && !(mo->z <= mo->floorz)
        && leveltime & 2)
    {
        mo->z += finesine[(FINEANGLES / 20 * leveltime >> 2) & FINEMASK];
    }

//
// clip movement
//
    if (mo->z <= mo->floorz)
    {                           // Hit the floor
        mo->z = mo->floorz;
        if (mo->momz < 0)
        {
            mo->momz = 0;
        }
        if (mo->flags & MF_SKULLFLY)
        {                       // The skull slammed into something
            mo->momz = -mo->momz;
        }
        if (mo->info->crashstate && (mo->flags & MF_CORPSE))
        {
            return;
        }
    }
    else if (mo->flags2 & MF2_LOGRAV)
    {
        fixed_t gravity = P_MobjGravity(mo);

        if (mo->momz == 0)
            mo->momz = -(gravity >> 3) * 2;
        else
            mo->momz -= gravity >> 3;
    }
    else if (!(mo->flags & MF_NOGRAVITY))
    {
        fixed_t gravity = P_MobjGravity(mo);

        if (mo->momz == 0)
            mo->momz = -gravity * 2;
        else
            mo->momz -= gravity;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {                           // hit the ceiling
        if (mo->momz > 0)
            mo->momz = 0;
        mo->z = mo->ceilingz - mo->height;
        if (mo->flags & MF_SKULLFLY)
        {                       // the skull slammed into something
            mo->momz = -mo->momz;
        }
    }
}

void P_AppendSpecHit(line_t * ld)
{
  // 1/11/98 killough: remove limit on lines hit, by array doubling
  if (numspechit >= spechit_max) {
    spechit_max = spechit_max ? spechit_max*2 : 8;
    spechit = Z_Realloc(spechit,sizeof *spechit*spechit_max); // killough
  }
  spechit[numspechit++] = ld;
  // e6y: Spechits overrun emulation code
  if (numspechit > 8 && demo_compatibility)
  {
    static spechit_overrun_param_t spechit_overrun_param = {
      NULL,          // line_t *line;

      &spechit,      // line_t **spechit;
      &numspechit,   // int *numspechit;

      tmbbox,        // fixed_t *tmbbox[4];
      &tmfloorz,     // fixed_t *tmfloorz;
      &tmceilingz,   // fixed_t *tmceilingz;

      &crushchange,  // int *crushchange;
      &nofit,        // dboolean *nofit;
    };
    spechit_overrun_param.line = ld;
    SpechitOverrun(&spechit_overrun_param);
  }
}

// hexen

dboolean PTR_BounceTraverse(intercept_t * in)
{
    line_t *li;

    if (!in->isaline)
        I_Error("PTR_BounceTraverse: not a line?");

    li = in->d.line;

    if (li->flags & ML_BLOCKEVERYTHING)
        goto bounceblocking;

    if (!(li->flags & ML_TWOSIDED))
    {
        if (P_PointOnLineSide(slidemo->x, slidemo->y, li))
            return true;        // don't hit the back side
        goto bounceblocking;
    }

    P_LineOpening(li, slidemo);          // set open
    if (line_opening.range < slidemo->height)
        goto bounceblocking;    // doesn't fit

    if (line_opening.top - slidemo->z < slidemo->height)
        goto bounceblocking;    // mobj is too high
    return true;                // this line doesn't block movement

// the line does block movement, see if it is closer than best so far
  bounceblocking:
    if (in->frac < bestslidefrac)
    {
        bestslidefrac = in->frac;
        bestslideline = li;
    }
    return false;               // stop
}

void P_BounceWall(mobj_t * mo)
{
    fixed_t leadx, leady;
    int side;
    angle_t lineangle, moveangle, deltaangle;
    fixed_t movelen;

    slidemo = mo;

    //
    // trace along the three leading corners
    //
    if (mo->momx > 0)
    {
        leadx = mo->x + mo->radius;
    }
    else
    {
        leadx = mo->x - mo->radius;
    }
    if (mo->momy > 0)
    {
        leady = mo->y + mo->radius;
    }
    else
    {
        leady = mo->y - mo->radius;
    }
    bestslidefrac = FRACUNIT + 1;
    P_PathTraverse(leadx, leady, leadx + mo->momx, leady + mo->momy,
                   PT_ADDLINES, PTR_BounceTraverse);

    side = P_PointOnLineSide(mo->x, mo->y, bestslideline);
    lineangle = R_PointToAngle2(0, 0, bestslideline->dx, bestslideline->dy);
    if (side == 1)
        lineangle += ANG180;
    moveangle = R_PointToAngle2(0, 0, mo->momx, mo->momy);
    deltaangle = (2 * lineangle) - moveangle;

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_AproxDistance(mo->momx, mo->momy);
    movelen = FixedMul(movelen, 0.75 * FRACUNIT);       // friction
    if (movelen < FRACUNIT)
        movelen = 2 * FRACUNIT;
    mo->momx = FixedMul(movelen, finecosine[deltaangle]);
    mo->momy = FixedMul(movelen, finesine[deltaangle]);
}

dboolean PIT_ThrustStompThing(mobj_t * thing)
{
    fixed_t blockdist;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    blockdist = thing->radius + tsthing->radius;
    if (abs(thing->x - tsthing->x) >= blockdist ||
        abs(thing->y - tsthing->y) >= blockdist ||
        (thing->z > tsthing->z + tsthing->height))
        return true;            // didn't hit it

    if (thing == tsthing)
        return true;            // don't clip against self

    P_DamageMobj(thing, tsthing, tsthing, 10001);
    tsthing->special_args[1] = 1;       // Mark thrust thing as bloody

    return true;
}

void PIT_ThrustSpike(mobj_t * actor)
{
    int xl, xh, yl, yh, bx, by;
    int x0, x2, y0, y2;

    tsthing = actor;

    x0 = actor->x - actor->info->radius;
    x2 = actor->x + actor->info->radius;
    y0 = actor->y - actor->info->radius;
    y2 = actor->y + actor->info->radius;

    xl = (x0 - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (x2 - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (y0 - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (y2 - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    // stomp on any things contacted
    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            P_BlockThingsIterator(bx, by, PIT_ThrustStompThing);
}

static void CheckForPushSpecial(line_t * line, int side, mobj_t * mobj)
{
    if (line->special)
    {
        if (mobj->flags2 & MF2_PUSHWALL)
        {
            P_ActivateLine(line, mobj, side, SPAC_PUSH);
        }
        else if (mobj->flags2 & MF2_IMPACT)
        {
            if (map_info.flags & MI_MISSILES_ACTIVATE_IMPACT_LINES ||
                !(mobj->flags & MF_MISSILE) ||
                !mobj->target)
            {
              P_ActivateLine(line, mobj, side, SPAC_IMPACT);
            }
            else
            {
              P_ActivateLine(line, mobj->target, side, SPAC_IMPACT);
            }
        }
    }
}

static dboolean Hexen_P_TryMove(mobj_t* thing, fixed_t x, fixed_t y)
{
    fixed_t oldx, oldy;
    int side, oldside;
    line_t *ld;

    floatok = false;
    if (!P_CheckPosition(thing, x, y))
    {                           // Solid wall or thing
        if (!BlockingMobj || BlockingMobj->player || !thing->player)
        {
            goto pushline;
        }
        else if (BlockingMobj->z + BlockingMobj->height - thing->z
                 > 24 * FRACUNIT
                 || (BlockingMobj->subsector->sector->ceilingheight
                     - (BlockingMobj->z + BlockingMobj->height) <
                     thing->height)
                 || (tmceilingz - (BlockingMobj->z + BlockingMobj->height) <
                     thing->height))
        {
            goto pushline;
        }
    }
    if (!(thing->flags & MF_NOCLIP))
    {
        if (tmceilingz - tmfloorz < thing->height)
        {                       // Doesn't fit
            goto pushline;
        }
        floatok = true;
        if (!(thing->flags & MF_TELEPORT)
            && tmceilingz - thing->z < thing->height
            && thing->type != HEXEN_MT_LIGHTNING_CEILING
            && !(thing->flags2 & MF2_FLY))
        {                       // mobj must lower itself to fit
            goto pushline;
        }
        if (thing->flags2 & MF2_FLY)
        {
            if (thing->z + thing->height > tmceilingz)
            {
                thing->momz = -8 * FRACUNIT;
                goto pushline;
            }
            else if (thing->z < tmfloorz
                     && tmfloorz - tmdropoffz > 24 * FRACUNIT)
            {
                thing->momz = 8 * FRACUNIT;
                goto pushline;
            }
        }
        if (!(thing->flags & MF_TELEPORT)
            // The Minotaur floor fire (HEXEN_MT_MNTRFX2) can step up any amount
            && thing->type != HEXEN_MT_MNTRFX2 && thing->type != HEXEN_MT_LIGHTNING_FLOOR
            && tmfloorz - thing->z > 24 * FRACUNIT)
        {
            goto pushline;
        }
        if (!(thing->flags & (MF_DROPOFF | MF_FLOAT)) &&
            (tmfloorz - tmdropoffz > 24 * FRACUNIT) &&
            !(thing->flags2 & MF2_BLASTED))
        {                       // Can't move over a dropoff unless it's been blasted
            return (false);
        }
        if (thing->flags2 & MF2_CANTLEAVEFLOORPIC
            && (tmfloorpic != thing->subsector->sector->floorpic
                || tmfloorz - thing->z != 0))
        {                       // must stay within a sector of a certain floor type
            return false;
        }
    }

    //
    // the move is ok, so link the thing into its new position
    //
    P_UnsetThingPosition(thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->floorpic = tmfloorpic;
    thing->x = x;
    thing->y = y;

    P_SetThingPosition(thing);

    if (thing->flags2 & MF2_FOOTCLIP)
    {
        if (thing->z == thing->subsector->sector->floorheight
            && P_GetThingFloorType(thing) >= FLOOR_LIQUID)
        {
            thing->floorclip = 10 * FRACUNIT;
        }
        else
        {
            thing->floorclip = 0;
        }
    }

    //
    // if any special lines were hit, do the effect
    //
    if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)))
    {
        while (numspechit > 0)
        {
            numspechit--;
            // see if the line was crossed
            ld = spechit[numspechit];
            side = P_PointOnLineSide(thing->x, thing->y, ld);
            oldside = P_PointOnLineSide(oldx, oldy, ld);
            if (side != oldside)
            {
                if (ld->special)
                {
                    map_format.cross_special_line(ld, oldside, thing, false);
                }
            }
        }
    }
    return true;

  pushline:
    if (!(thing->flags & (MF_TELEPORT | MF_NOCLIP)))
    {
        int numSpecHitTemp;

        if (tmthing->flags2 & MF2_BLASTED)
        {
            P_DamageMobj(tmthing, NULL, NULL, tmthing->info->mass >> 5);
        }
        numSpecHitTemp = numspechit;
        while (numSpecHitTemp > 0)
        {
            numSpecHitTemp--;
            // see if the line was crossed
            ld = spechit[numSpecHitTemp];
            side = P_PointOnLineSide(thing->x, thing->y, ld);
            CheckForPushSpecial(ld, side, thing);
        }
    }
    return false;
}

#define USE_PUZZLE_ITEM_SPECIAL 129

static mobj_t *PuzzleItemUser;
static int PuzzleItemType;
static dboolean PuzzleActivated;

#include "hexen/p_acs.h"

dboolean PTR_PuzzleItemTraverse(intercept_t * in)
{
    mobj_t *mobj;
    byte args[3];
    int sound;

    if (in->isaline)
    {                           // Check line
        if (in->d.line->special != USE_PUZZLE_ITEM_SPECIAL)
        {
            P_LineOpening(in->d.line, NULL);
            if (line_opening.range <= 0)
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
                S_StartMobjSound(PuzzleItemUser, sound);
                return false;   // can't use through a wall
            }
            return true;        // Continue searching
        }
        if (P_PointOnLineSide(PuzzleItemUser->x, PuzzleItemUser->y,
                              in->d.line) == 1)
        {                       // Don't use back sides
            return false;
        }
        if (PuzzleItemType != in->d.line->special_args[0])
        {                       // Item type doesn't match
            return false;
        }

        // Construct an args[] array that would contain the values from
        // the line that would be passed by Vanilla Hexen.
        args[0] = in->d.line->special_args[2];
        args[1] = in->d.line->special_args[3];
        args[2] = in->d.line->special_args[4];

        P_StartACS(in->d.line->special_args[1], 0, args, PuzzleItemUser, in->d.line, 0);
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
    if (PuzzleItemType != mobj->special_args[0])
    {                           // Item type doesn't match
        return true;
    }

    args[0] = mobj->special_args[2];
    args[1] = mobj->special_args[3];
    args[2] = mobj->special_args[4];

    P_StartACS(mobj->special_args[1], 0, args, PuzzleItemUser, NULL, 0);
    mobj->special = 0;
    PuzzleActivated = true;
    return false;               // Stop searching
}

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
