/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000,2002 by
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
 *      Enemy thinking, AI.
 *      Action Pointer Functions
 *      that are associated with states/frames.
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "m_random.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "s_sound.h"
#include "sounds.h"
#include "p_inter.h"
#include "g_game.h"
#include "p_enemy.h"
#include "p_tick.h"
#include "i_sound.h"
#include "m_bbox.h"
#include "hu_stuff.h"
#include "lprintf.h"
#include "e6y.h"//e6y
#include "dsda.h"

static mobj_t *current_actor;

typedef enum {
  DI_EAST,
  DI_NORTHEAST,
  DI_NORTH,
  DI_NORTHWEST,
  DI_WEST,
  DI_SOUTHWEST,
  DI_SOUTH,
  DI_SOUTHEAST,
  DI_NODIR,
  NUMDIRS
} dirtype_e;

typedef int dirtype_t;

static void P_NewChaseDir(mobj_t *actor);
void P_ZBumpCheck(mobj_t *);                                        // phares

//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//

//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//
// killough 5/5/98: reformatted, cleaned up

static void P_RecursiveSound(sector_t *sec, int soundblocks,
           mobj_t *soundtarget)
{
  int i;

  // wake up all monsters in this sector
  if (sec->validcount == validcount && sec->soundtraversed <= soundblocks+1)
    return;             // already flooded

  sec->validcount = validcount;
  sec->soundtraversed = soundblocks+1;
  P_SetTarget(&sec->soundtarget, soundtarget);

  for (i=0; i<sec->linecount; i++)
    {
      sector_t *other;
      line_t *check = sec->lines[i];

      if (!(check->flags & ML_TWOSIDED))
        continue;

      P_LineOpening(check);

      if (openrange <= 0)
        continue;       // closed door

      other=sides[check->sidenum[sides[check->sidenum[0]].sector==sec]].sector;

      if (!(check->flags & ML_SOUNDBLOCK))
        P_RecursiveSound(other, soundblocks, soundtarget);
      else
        if (!soundblocks)
          P_RecursiveSound(other, 1, soundtarget);
    }
}

//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert(mobj_t *target, mobj_t *emitter)
{
  if (target != NULL && target->player && (target->player->cheats & CF_NOTARGET))
    return;

  validcount++;
  P_RecursiveSound(emitter->subsector->sector, 0, target);
}

//
// P_CheckRange
//

static dboolean P_CheckRange(mobj_t *actor, fixed_t range)
{
  mobj_t *pl = actor->target;

  return  // killough 7/18/98: friendly monsters don't attack other friends
    pl &&
    !(actor->flags & pl->flags & MF_FRIEND) &&
    P_AproxDistance(pl->x-actor->x, pl->y-actor->y) < range &&
    P_CheckSight(actor, actor->target) &&
    ( // finite height!
      !heretic ||
      (
        pl->z <= actor->z + actor->height &&
        actor->z <= pl->z + pl->height
      )
    );
}

//
// P_CheckMeleeRange
//

static dboolean P_CheckMeleeRange(mobj_t *actor)
{
  int range;

  range = actor->info->meleerange;

  if (compatibility_level != doom_12_compatibility)
    range += actor->target->info->radius - 20 * FRACUNIT;

  return P_CheckRange(actor, range);
}

//
// P_HitFriend()
//
// killough 12/98
// This function tries to prevent shooting at friends

static dboolean P_HitFriend(mobj_t *actor)
{
  return actor->flags & MF_FRIEND && actor->target &&
    (P_AimLineAttack(actor,
         R_PointToAngle2(actor->x, actor->y,
             actor->target->x, actor->target->y),
         P_AproxDistance(actor->x-actor->target->x,
             actor->y-actor->target->y), 0),
     linetarget) && linetarget != actor->target &&
    !((linetarget->flags ^ actor->flags) & MF_FRIEND);
}

//
// P_CheckMissileRange
//
static dboolean P_CheckMissileRange(mobj_t *actor)
{
  fixed_t dist;

  if (!P_CheckSight(actor, actor->target))
    return false;

  if (actor->flags & MF_JUSTHIT)
  {      // the target just hit the enemy, so fight back!
    actor->flags &= ~MF_JUSTHIT;

    /* killough 7/18/98: no friendly fire at corpses
     * killough 11/98: prevent too much infighting among friends
     * cph - yikes, talk about fitting everything on one line... */

    return
      !(actor->flags & MF_FRIEND) ||
      (actor->target->health > 0 &&
       (!(actor->target->flags & MF_FRIEND) ||
        (actor->target->player ?
         monster_infighting || P_Random(pr_defect) >128 :
         !(actor->target->flags & MF_JUSTHIT) && P_Random(pr_defect) >128)));
  }

  /* killough 7/18/98: friendly monsters don't attack other friendly
   * monsters or players (except when attacked, and then only once)
   */
  if (actor->flags & actor->target->flags & MF_FRIEND)
    return false;

  if (actor->reactiontime)
    return false;       // do not attack yet

  // OPTIMIZE: get this from a global checksight
  dist = P_AproxDistance ( actor->x-actor->target->x,
                           actor->y-actor->target->y) - 64*FRACUNIT;

  if (!actor->info->meleestate)
    dist -= 128*FRACUNIT;       // no melee attack, so fire more

  dist >>= FRACBITS;

  if (actor->flags2 & MF2_SHORTMRANGE)
    if (dist > 14*64)
      return false;     // too far away

  if (actor->flags2 & MF2_LONGMELEE)
  {
    if (dist < 196)
      return false;   // close for fist attack
  }

  if (actor->flags2 & MF2_RANGEHALF)
    dist >>= 1;

  if (dist > 200)
    dist = 200;

  if (actor->flags2 & MF2_HIGHERMPROB && dist > 160)
    dist = 160;

  if (P_Random(pr_missrange) < dist)
    return false;

  if (P_HitFriend(actor))
    return false;

  return true;
}

/*
 * P_IsOnLift
 *
 * killough 9/9/98:
 *
 * Returns true if the object is on a lift. Used for AI,
 * since it may indicate the need for crowded conditions,
 * or that a monster should stay on the lift for a while
 * while it goes up or down.
 */

static dboolean P_IsOnLift(const mobj_t *actor)
{
  const sector_t *sec = actor->subsector->sector;
  line_t line;
  int l;

  // Short-circuit: it's on a lift which is active.
  if (sec->floordata && ((thinker_t *) sec->floordata)->function==T_PlatRaise)
    return true;

  // Check to see if it's in a sector which can be activated as a lift.
  if ((line.tag = sec->tag))
    for (l = -1; (l = P_FindLineFromLineTag(&line, l)) >= 0;)
      switch (lines[l].special)
  {
  case  10: case  14: case  15: case  20: case  21: case  22:
  case  47: case  53: case  62: case  66: case  67: case  68:
  case  87: case  88: case  95: case 120: case 121: case 122:
  case 123: case 143: case 162: case 163: case 181: case 182:
  case 144: case 148: case 149: case 211: case 227: case 228:
  case 231: case 232: case 235: case 236:
    return true;
  }

  return false;
}

/*
 * P_IsUnderDamage
 *
 * killough 9/9/98:
 *
 * Returns nonzero if the object is under damage based on
 * their current position. Returns 1 if the damage is moderate,
 * -1 if it is serious. Used for AI.
 */

static int P_IsUnderDamage(mobj_t *actor)
{
  const struct msecnode_s *seclist;
  const ceiling_t *cl;             // Crushing ceiling
  int dir = 0;
  for (seclist=actor->touching_sectorlist; seclist; seclist=seclist->m_tnext)
    if ((cl = seclist->m_sector->ceilingdata) &&
  cl->thinker.function == T_MoveCeiling)
      dir |= cl->direction;
  return dir;
}

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//

static fixed_t xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
static fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

// 1/11/98 killough: Limit removed on special lines crossed
extern  line_t **spechit;          // New code -- killough
extern  int    numspechit;

static dboolean P_Move(mobj_t *actor, dboolean dropoff) /* killough 9/12/98 */
{
  fixed_t tryx, tryy, deltax, deltay, origx, origy;
  dboolean try_ok;
  int movefactor = ORIG_FRICTION_FACTOR;    // killough 10/98
  int friction = ORIG_FRICTION;
  int speed;

  if (actor->movedir == DI_NODIR)
    return false;

#ifdef RANGECHECK
  if ((unsigned)actor->movedir >= 8)
    I_Error ("P_Move: Weird actor->movedir!");
#endif

  // killough 10/98: make monsters get affected by ice and sludge too:

  if (monster_friction)
    movefactor = P_GetMoveFactor(actor, &friction);

  speed = actor->info->speed;

  if (friction < ORIG_FRICTION &&     // sludge
      !(speed = ((ORIG_FRICTION_FACTOR - (ORIG_FRICTION_FACTOR-movefactor)/2)
     * speed) / ORIG_FRICTION_FACTOR))
    speed = 1;      // always give the monster a little bit of speed

  tryx = (origx = actor->x) + (deltax = speed * xspeed[actor->movedir]);
  tryy = (origy = actor->y) + (deltay = speed * yspeed[actor->movedir]);

  try_ok = P_TryMove(actor, tryx, tryy, dropoff);

  // killough 10/98:
  // Let normal momentum carry them, instead of steptoeing them across ice.

  if (try_ok && friction > ORIG_FRICTION)
    {
      actor->x = origx;
      actor->y = origy;
      movefactor *= FRACUNIT / ORIG_FRICTION_FACTOR / 4;
      actor->momx += FixedMul(deltax, movefactor);
      actor->momy += FixedMul(deltay, movefactor);
    }

  if (!try_ok)
    {      // open any specials
      int good;

      if (actor->flags & MF_FLOAT && floatok)
        {
          if (actor->z < tmfloorz)          // must adjust height
            actor->z += FLOATSPEED;
          else
            actor->z -= FLOATSPEED;

          actor->flags |= MF_INFLOAT;

    return true;
        }

      if (!numspechit)
        return false;

      actor->movedir = DI_NODIR;

      /* if the special is not a door that can be opened, return false
       *
       * killough 8/9/98: this is what caused monsters to get stuck in
       * doortracks, because it thought that the monster freed itself
       * by opening a door, even if it was moving towards the doortrack,
       * and not the door itself.
       *
       * killough 9/9/98: If a line blocking the monster is activated,
       * return true 90% of the time. If a line blocking the monster is
       * not activated, but some other line is, return false 90% of the
       * time. A bit of randomness is needed to ensure it's free from
       * lockups, but for most cases, it returns the correct result.
       *
       * Do NOT simply return false 1/4th of the time (causes monsters to
       * back out when they shouldn't, and creates secondary stickiness).
       */

      for (good = false; numspechit--; )
        if (P_UseSpecialLine(actor, spechit[numspechit], 0, false))
          good |= spechit[numspechit] == blockline ? 1 : 2;

      if (heretic) return good > 0;

      /* cph - compatibility maze here
       * Boom v2.01 and orig. Doom return "good"
       * Boom v2.02 and LxDoom return good && (P_Random(pr_trywalk)&3)
       * MBF plays even more games
       */
      if (!good || comp[comp_doorstuck]) return good;
      if (!mbf_features)
        return (P_Random(pr_trywalk)&3); /* jff 8/13/98 */
      else /* finally, MBF code */
        return ((P_Random(pr_opendoor) >= 230) ^ (good & 1));
    }
  else
    actor->flags &= ~MF_INFLOAT;

  /* killough 11/98: fall more slowly, under gravity, if felldown==true */
  if (!(actor->flags & MF_FLOAT) && (!felldown || !mbf_features)) {
    if (heretic && actor->z > actor->floorz)
    {
      P_HitFloor(actor);
    }
    actor->z = actor->floorz;
  }

  return true;
}

/*
 * P_SmartMove
 *
 * killough 9/12/98: Same as P_Move, except smarter
 */

static dboolean P_SmartMove(mobj_t *actor)
{
  mobj_t *target = actor->target;
  int on_lift, dropoff = false, under_damage;
  int tmp_monster_avoid_hazards = (prboom_comp[PC_MONSTER_AVOID_HAZARDS].state ?
    true : (demo_compatibility ? false : monster_avoid_hazards));//e6y

  /* killough 9/12/98: Stay on a lift if target is on one */
  on_lift = !comp[comp_staylift]
    && target && target->health > 0
    && target->subsector->sector->tag==actor->subsector->sector->tag &&
    P_IsOnLift(actor);

  under_damage = tmp_monster_avoid_hazards && P_IsUnderDamage(actor);//e6y

  // killough 10/98: allow dogs to drop off of taller ledges sometimes.
  // dropoff==1 means always allow it, dropoff==2 means only up to 128 high,
  // and only if the target is immediately on the other side of the line.

  // haleyjd: allow all friends of HelperType to also jump down

  if ((actor->type == MT_DOGS || (actor->type == (HelperThing-1) && actor->flags&MF_FRIEND))
      && target && dog_jumping &&
      !((target->flags ^ actor->flags) & MF_FRIEND) &&
      P_AproxDistance(actor->x - target->x,
          actor->y - target->y) < FRACUNIT*144 &&
      P_Random(pr_dropoff) < 235)
    dropoff = 2;

  if (!P_Move(actor, dropoff))
    return false;

  // killough 9/9/98: avoid crushing ceilings or other damaging areas
  if (
      (on_lift && P_Random(pr_stayonlift) < 230 &&      // Stay on lift
       !P_IsOnLift(actor))
      ||
      (tmp_monster_avoid_hazards && !under_damage &&//e6y  // Get away from damage
       (under_damage = P_IsUnderDamage(actor)) &&
       (under_damage < 0 || P_Random(pr_avoidcrush) < 200))
      )
    actor->movedir = DI_NODIR;    // avoid the area (most of the time anyway)

  return true;
}

//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//

// HERETIC_NOTE: Quite sure P_SmartMove == P_Move for heretic
static dboolean P_TryWalk(mobj_t *actor)
{
  if (!P_SmartMove(actor))
    return false;
  actor->movecount = P_Random(pr_trywalk)&15;
  return true;
}

//
// P_DoNewChaseDir
//
// killough 9/8/98:
//
// Most of P_NewChaseDir(), except for what
// determines the new direction to take
//

static void P_DoNewChaseDir(mobj_t *actor, fixed_t deltax, fixed_t deltay)
{
  dirtype_t xdir, ydir, tdir;
  dirtype_t olddir = actor->movedir;
  dirtype_t turnaround = olddir;

  if (turnaround != DI_NODIR)         // find reverse direction
    turnaround ^= 4;

  xdir =
    deltax >  10*FRACUNIT ? DI_EAST :
    deltax < -10*FRACUNIT ? DI_WEST : DI_NODIR;

  ydir =
    deltay < -10*FRACUNIT ? DI_SOUTH :
    deltay >  10*FRACUNIT ? DI_NORTH : DI_NODIR;

  // try direct route
  if (xdir != DI_NODIR && ydir != DI_NODIR && turnaround !=
      (actor->movedir = deltay < 0 ? deltax > 0 ? DI_SOUTHEAST : DI_SOUTHWEST :
       deltax > 0 ? DI_NORTHEAST : DI_NORTHWEST) && P_TryWalk(actor))
    return;

  // try other directions
  if (P_Random(pr_newchase) > 200 || D_abs(deltay)>D_abs(deltax))
    tdir = xdir, xdir = ydir, ydir = tdir;

  if ((xdir == turnaround ? xdir = DI_NODIR : xdir) != DI_NODIR &&
      (actor->movedir = xdir, P_TryWalk(actor)))
    return;         // either moved forward or attacked

  if ((ydir == turnaround ? ydir = DI_NODIR : ydir) != DI_NODIR &&
      (actor->movedir = ydir, P_TryWalk(actor)))
    return;

  // there is no direct path to the player, so pick another direction.
  if (olddir != DI_NODIR && (actor->movedir = olddir, P_TryWalk(actor)))
    return;

  // randomly determine direction of search
  if (P_Random(pr_newchasedir) & 1)
    {
      for (tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
        if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
    return;
    }
  else
    for (tdir = DI_SOUTHEAST; tdir != DI_EAST-1; tdir--)
      if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
  return;

  if ((actor->movedir = turnaround) != DI_NODIR && !P_TryWalk(actor))
    actor->movedir = DI_NODIR;
}

//
// killough 11/98:
//
// Monsters try to move away from tall dropoffs.
//
// In Doom, they were never allowed to hang over dropoffs,
// and would remain stuck if involuntarily forced over one.
// This logic, combined with p_map.c (P_TryMove), allows
// monsters to free themselves without making them tend to
// hang over dropoffs.

static fixed_t dropoff_deltax, dropoff_deltay, floorz;

static dboolean PIT_AvoidDropoff(line_t *line)
{
  if (line->backsector                          && // Ignore one-sided linedefs
      tmbbox[BOXRIGHT]  > line->bbox[BOXLEFT]   &&
      tmbbox[BOXLEFT]   < line->bbox[BOXRIGHT]  &&
      tmbbox[BOXTOP]    > line->bbox[BOXBOTTOM] && // Linedef must be contacted
      tmbbox[BOXBOTTOM] < line->bbox[BOXTOP]    &&
      P_BoxOnLineSide(tmbbox, line) == -1)
    {
      fixed_t front = line->frontsector->floorheight;
      fixed_t back  = line->backsector->floorheight;
      angle_t angle;

      // The monster must contact one of the two floors,
      // and the other must be a tall dropoff (more than 24).

      if (back == floorz && front < floorz - FRACUNIT*24)
  angle = R_PointToAngle2(0,0,line->dx,line->dy);   // front side dropoff
      else
  if (front == floorz && back < floorz - FRACUNIT*24)
    angle = R_PointToAngle2(line->dx,line->dy,0,0); // back side dropoff
  else
    return true;

      // Move away from dropoff at a standard speed.
      // Multiple contacted linedefs are cumulative (e.g. hanging over corner)
      dropoff_deltax -= finesine[angle >> ANGLETOFINESHIFT]*32;
      dropoff_deltay += finecosine[angle >> ANGLETOFINESHIFT]*32;
    }
  return true;
}

//
// Driver for above
//

static fixed_t P_AvoidDropoff(mobj_t *actor)
{
  int yh=P_GetSafeBlockY((tmbbox[BOXTOP]   = actor->y+actor->radius)-bmaporgy);
  int yl=P_GetSafeBlockY((tmbbox[BOXBOTTOM]= actor->y-actor->radius)-bmaporgy);
  int xh=P_GetSafeBlockX((tmbbox[BOXRIGHT] = actor->x+actor->radius)-bmaporgx);
  int xl=P_GetSafeBlockX((tmbbox[BOXLEFT]  = actor->x-actor->radius)-bmaporgx);
  int bx, by;

  floorz = actor->z;            // remember floor height

  dropoff_deltax = dropoff_deltay = 0;

  // check lines

  validcount++;
  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      P_BlockLinesIterator(bx, by, PIT_AvoidDropoff);  // all contacted lines

  return dropoff_deltax | dropoff_deltay;   // Non-zero if movement prescribed
}

//
// P_NewChaseDir
//
// killough 9/8/98: Split into two functions
//

static void P_NewChaseDir(mobj_t *actor)
{
  mobj_t *target = actor->target;
  fixed_t deltax = target->x - actor->x;
  fixed_t deltay = target->y - actor->y;

  // killough 8/8/98: sometimes move away from target, keeping distance
  //
  // 1) Stay a certain distance away from a friend, to avoid being in their way
  // 2) Take advantage over an enemy without missiles, by keeping distance

  actor->strafecount = 0;

  if (mbf_features) {
    if (
      actor->floorz - actor->dropoffz > FRACUNIT*24 &&
      actor->z <= actor->floorz &&
      !(actor->flags & (MF_DROPOFF|MF_FLOAT)) &&
      !comp[comp_dropoff] &&
      P_AvoidDropoff(actor)
    ) /* Move away from dropoff */
    {
      P_DoNewChaseDir(actor, dropoff_deltax, dropoff_deltay);

      // If moving away from dropoff, set movecount to 1 so that
      // small steps are taken to get monster away from dropoff.

      actor->movecount = 1;
      return;
    }
    else
    {
      fixed_t dist = P_AproxDistance(deltax, deltay);

      // Move away from friends when too close, except
      // in certain situations (e.g. a crowded lift)

      if (actor->flags & target->flags & MF_FRIEND &&
          distfriend << FRACBITS > dist &&
          !P_IsOnLift(target) && !P_IsUnderDamage(actor))
      {
        deltax = -deltax, deltay = -deltay;
      }
      else if (target->health > 0 && (actor->flags ^ target->flags) & MF_FRIEND)
      {   // Live enemy target
        if (
          monster_backing &&
          actor->info->missilestate &&
          actor->type != MT_SKULL &&
          (
            (!target->info->missilestate && dist < target->info->meleerange*2) ||
            (
              target->player && dist < target->player->mo->info->meleerange*3 &&
              weaponinfo[target->player->readyweapon].flags & WPF_FLEEMELEE
            )
          )
        )
        {       // Back away from melee attacker
          actor->strafecount = P_Random(pr_enemystrafe) & 15;
          deltax = -deltax, deltay = -deltay;
        }
      }
    }
  }

  P_DoNewChaseDir(actor, deltax, deltay);

  // If strafing, set movecount to strafecount so that old Doom
  // logic still works the same, except in the strafing part

  if (actor->strafecount)
    actor->movecount = actor->strafecount;
}

//
// P_IsVisible
//
// killough 9/9/98: whether a target is visible to a monster
//

static dboolean P_IsVisible(mobj_t *actor, mobj_t *mo, dboolean allaround)
{
  if (!allaround)
    {
      angle_t an = R_PointToAngle2(actor->x, actor->y,
           mo->x, mo->y) - actor->angle;
      if (an > ANG90 && an < ANG270 &&
    P_AproxDistance(mo->x-actor->x, mo->y-actor->y) > WAKEUPRANGE)
  return false;
    }
  return P_CheckSight(actor, mo);
}

//
// PIT_FindTarget
//
// killough 9/5/98
//
// Finds monster targets for other monsters
//

static int current_allaround;

static dboolean PIT_FindTarget(mobj_t *mo)
{
  mobj_t *actor = current_actor;

  if (!((mo->flags ^ actor->flags) & MF_FRIEND &&        // Invalid target
  mo->health > 0 && (mo->flags & MF_COUNTKILL || mo->type == MT_SKULL)))
    return true;

  // If the monster is already engaged in a one-on-one attack
  // with a healthy friend, don't attack around 60% the time
  {
    const mobj_t *targ = mo->target;
    if (targ && targ->target == mo &&
  P_Random(pr_skiptarget) > 100 &&
  (targ->flags ^ mo->flags) & MF_FRIEND &&
  targ->health*2 >= targ->info->spawnhealth)
      return true;
  }

  if (!P_IsVisible(actor, mo, current_allaround))
    return true;

  P_SetTarget(&actor->lastenemy, actor->target);  // Remember previous target
  P_SetTarget(&actor->target, mo);                // Found target

  // Move the selected monster to the end of its associated
  // list, so that it gets searched last next time.

  {
    thinker_t *cap = &thinkerclasscap[mo->flags & MF_FRIEND ?
             th_friends : th_enemies];
    (mo->thinker.cprev->cnext = mo->thinker.cnext)->cprev = mo->thinker.cprev;
    (mo->thinker.cprev = cap->cprev)->cnext = &mo->thinker;
    (mo->thinker.cnext = cap)->cprev = &mo->thinker;
  }

  return false;
}

//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//

static dboolean P_LookForPlayers(mobj_t *actor, dboolean allaround)
{
  player_t *player;
  int stop, stopc, c;

  if (heretic) return Heretic_P_LookForPlayers(actor, allaround);

  if (actor->flags & MF_FRIEND)
    {  // killough 9/9/98: friendly monsters go about players differently
      int anyone;

      // Go back to a player, no matter whether it's visible or not
      for (anyone=0; anyone<=1; anyone++)
  for (c=0; c<MAXPLAYERS; c++)
    if (playeringame[c] && players[c].playerstate==PST_LIVE &&
        (anyone || P_IsVisible(actor, players[c].mo, allaround)))
      {
        P_SetTarget(&actor->target, players[c].mo);

        // killough 12/98:
        // get out of refiring loop, to avoid hitting player accidentally

        if (actor->info->missilestate)
    {
      P_SetMobjState(actor, actor->info->seestate);
      actor->flags &= ~MF_JUSTHIT;
    }

        return true;
      }

      return false;
    }

  // Change mask of 3 to (MAXPLAYERS-1) -- killough 2/15/98:
  stop = (actor->lastlook-1)&(MAXPLAYERS-1);

  c = 0;

  stopc = !mbf_features &&
    !demo_compatibility && monsters_remember ?
    MAXPLAYERS : 2;       // killough 9/9/98

  for (;; actor->lastlook = (actor->lastlook+1)&(MAXPLAYERS-1))
    {
      if (!playeringame[actor->lastlook])
  continue;

      // killough 2/15/98, 9/9/98:
      if (c++ == stopc || actor->lastlook == stop)  // done looking
      {
        // e6y
        // Fixed Boom incompatibilities. The following code was missed.
        // There are no more desyncs on Donce's demos on horror.wad

        // Use last known enemy if no players sighted -- killough 2/15/98:
        if (!mbf_features && !demo_compatibility && monsters_remember)
        {
          if (actor->lastenemy && actor->lastenemy->health > 0)
          {
            actor->target = actor->lastenemy;
            actor->lastenemy = NULL;
            return true;
          }
        }

        return false;
      }

      player = &players[actor->lastlook];

      if (player->cheats & CF_NOTARGET)
        continue; // no target

      if (player->health <= 0)
  continue;               // dead

      if (!P_IsVisible(actor, player->mo, allaround))
  continue;

      P_SetTarget(&actor->target, player->mo);

      /* killough 9/9/98: give monsters a threshold towards getting players
       * (we don't want it to be too easy for a player with dogs :)
       */
      if (!comp[comp_pursuit])
  actor->threshold = 60;

      return true;
    }
}

//
// Friendly monsters, by Lee Killough 7/18/98
//
// Friendly monsters go after other monsters first, but
// also return to owner if they cannot find any targets.
// A marine's best friend :)  killough 7/18/98, 9/98
//

static dboolean P_LookForMonsters(mobj_t *actor, dboolean allaround)
{
  thinker_t *cap, *th;

  if (demo_compatibility)
    return false;

  if (actor->lastenemy && actor->lastenemy->health > 0 && monsters_remember &&
      !(actor->lastenemy->flags & actor->flags & MF_FRIEND)) // not friends
    {
      P_SetTarget(&actor->target, actor->lastenemy);
      P_SetTarget(&actor->lastenemy, NULL);
      return true;
    }

  /* Old demos do not support monster-seeking bots */
  if (!mbf_features)
    return false;

  // Search the threaded list corresponding to this object's potential targets
  cap = &thinkerclasscap[actor->flags & MF_FRIEND ? th_enemies : th_friends];

  // Search for new enemy

  if (cap->cnext != cap)        // Empty list? bail out early
    {
      int x = P_GetSafeBlockX(actor->x - bmaporgx);
      int y = P_GetSafeBlockY(actor->y - bmaporgy);
      int d;

      current_actor = actor;
      current_allaround = allaround;

      // Search first in the immediate vicinity.

      if (!P_BlockThingsIterator(x, y, PIT_FindTarget))
  return true;

      for (d=1; d<5; d++)
  {
    int i = 1 - d;
    do
      if (!P_BlockThingsIterator(x+i, y-d, PIT_FindTarget) ||
    !P_BlockThingsIterator(x+i, y+d, PIT_FindTarget))
        return true;
    while (++i < d);
    do
      if (!P_BlockThingsIterator(x-d, y+i, PIT_FindTarget) ||
    !P_BlockThingsIterator(x+d, y+i, PIT_FindTarget))
        return true;
    while (--i + d >= 0);
  }

      {   // Random number of monsters, to prevent patterns from forming
  int n = (P_Random(pr_friends) & 31) + 15;

  for (th = cap->cnext; th != cap; th = th->cnext)
    if (--n < 0)
      {
        // Only a subset of the monsters were searched. Move all of
        // the ones which were searched so far, to the end of the list.

        (cap->cnext->cprev = cap->cprev)->cnext = cap->cnext;
        (cap->cprev = th->cprev)->cnext = cap;
        (th->cprev = cap)->cnext = th;
        break;
     }
    else
      if (!PIT_FindTarget((mobj_t *) th))   // If target sighted
        return true;
      }
    }

  return false;  // No monster found
}

//
// P_LookForTargets
//
// killough 9/5/98: look for targets to go after, depending on kind of monster
//

static dboolean P_LookForTargets(mobj_t *actor, int allaround)
{
  return actor->flags & MF_FRIEND ?
    P_LookForMonsters(actor, allaround) || P_LookForPlayers (actor, allaround):
    P_LookForPlayers (actor, allaround) || P_LookForMonsters(actor, allaround);
}

//
// P_HelpFriend
//
// killough 9/8/98: Help friends in danger of dying
//

static dboolean P_HelpFriend(mobj_t *actor)
{
  thinker_t *cap, *th;

  // If less than 33% health, self-preservation rules
  if (actor->health*3 < actor->info->spawnhealth)
    return false;

  current_actor = actor;
  current_allaround = true;

  // Possibly help a friend under 50% health
  cap = &thinkerclasscap[actor->flags & MF_FRIEND ? th_friends : th_enemies];

  for (th = cap->cnext; th != cap; th = th->cnext)
    if (((mobj_t *) th)->health*2 >= ((mobj_t *) th)->info->spawnhealth)
      {
  if (P_Random(pr_helpfriend) < 180)
    break;
      }
    else
      if (((mobj_t *) th)->flags & MF_JUSTHIT &&
    ((mobj_t *) th)->target &&
    ((mobj_t *) th)->target != actor->target &&
    !PIT_FindTarget(((mobj_t *) th)->target))
  {
    // Ignore any attacking monsters, while searching for friend
    actor->threshold = BASETHRESHOLD;
    return true;
  }

  return false;
}

//
// A_KeenDie
// DOOM II special, map 32.
// Uses special tag 666.
//
void A_KeenDie(mobj_t* mo)
{
  thinker_t *th;
  line_t   junk;

  A_Fall(mo);

  // scan the remaining thinkers to see if all Keens are dead

  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function == P_MobjThinker)
      {
        mobj_t *mo2 = (mobj_t *) th;
        if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
          return;                           // other Keen not dead
      }

  junk.tag = 666;
  EV_DoDoor(&junk,openDoor);
}


//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//

void A_Look(mobj_t *actor)
{
  mobj_t *targ = actor->subsector->sector->soundtarget;
  actor->threshold = 0; // any shot will wake up

  if (targ && targ->player && (targ->player->cheats & CF_NOTARGET))
    return;

  /* killough 7/18/98:
   * Friendly monsters go after other monsters first, but
   * also return to player, without attacking them, if they
   * cannot find any targets. A marine's best friend :)
   */
  actor->pursuecount = 0;

  if (
    !(actor->flags & MF_FRIEND && P_LookForTargets(actor, false)) &&
    !(
      (targ = actor->subsector->sector->soundtarget) &&
      targ->flags & MF_SHOOTABLE &&
      (
        P_SetTarget(&actor->target, targ),
        !(actor->flags & MF_AMBUSH) ||
        P_CheckSight(actor, targ)
      )
    ) &&
    (
      actor->flags & MF_FRIEND || !P_LookForTargets(actor, false)
    )
  )
    return;

  // go into chase state

  if (actor->info->seesound)
  {
    int sound;
    sound = actor->info->seesound;

    if (!heretic)
      switch (sound)
      {
        case sfx_posit1:
        case sfx_posit2:
        case sfx_posit3:
          sound = sfx_posit1 + P_Random(pr_see) % 3;
          break;

        case sfx_bgsit1:
        case sfx_bgsit2:
          sound = sfx_bgsit1 + P_Random(pr_see) % 2;
          break;

        default:
          break;
      }

    if (actor->flags2 & (MF2_BOSS | MF2_FULLVOLSOUNDS))
      S_StartSound(NULL, sound);          // full volume
    else
    {
      S_StartSound(actor, sound);

      // [FG] make seesounds uninterruptible
      if (full_sounds)
        S_UnlinkSound(actor);
    }
  }
  P_SetMobjState(actor, actor->info->seestate);
}

//
// A_KeepChasing
//
// killough 10/98:
// Allows monsters to continue movement while attacking
//

static void A_KeepChasing(mobj_t *actor)
{
  if (actor->movecount)
    {
      actor->movecount--;
      if (actor->strafecount)
        actor->strafecount--;
      P_SmartMove(actor);
    }
}

//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//

void A_Chase(mobj_t *actor)
{
  if (actor->reactiontime)
    actor->reactiontime--;

  if (actor->threshold) { /* modify target threshold */
    if (compatibility_level == doom_12_compatibility)
    {
      actor->threshold--;
    }
    else
    {
      if (!actor->target || actor->target->health <= 0)
        actor->threshold = 0;
      else
        actor->threshold--;
    }
  }

  if (heretic && gameskill == sk_nightmare)
  {                           // Monsters move faster in nightmare mode
      actor->tics -= actor->tics / 2;
      if (actor->tics < 3)
      {
          actor->tics = 3;
      }
  }

  /* turn towards movement direction if not there yet
   * killough 9/7/98: keep facing towards target if strafing or backing out
   */

  if (actor->strafecount)
    A_FaceTarget(actor);
  else if (actor->movedir < 8)
  {
    int delta = (actor->angle &= (7<<29)) - (actor->movedir << 29);
    if (delta > 0)
      actor->angle -= ANG90/2;
    else
      if (delta < 0)
        actor->angle += ANG90/2;
  }

  if (!actor->target || !(actor->target->flags&MF_SHOOTABLE))
  {
    if (!P_LookForTargets(actor,true)) // look for a new target
      P_SetMobjState(actor, actor->info->spawnstate); // no new target
    return;
  }

  // do not attack twice in a row
  if (actor->flags & MF_JUSTATTACKED)
  {
    actor->flags &= ~MF_JUSTATTACKED;
    if (gameskill != sk_nightmare && !fastparm)
      P_NewChaseDir(actor);
    return;
  }

  // check for melee attack
  if (actor->info->meleestate && P_CheckMeleeRange(actor))
  {
    if (actor->info->attacksound)
      S_StartSound(actor, actor->info->attacksound);
    P_SetMobjState(actor, actor->info->meleestate);
    /* killough 8/98: remember an attack
    * cph - DEMOSYNC? */
    if (!actor->info->missilestate && !heretic)
      actor->flags |= MF_JUSTHIT;
    return;
  }

  // check for missile attack
  if (actor->info->missilestate)
    if (!(gameskill < sk_nightmare && !fastparm && actor->movecount))
      if (P_CheckMissileRange(actor))
      {
        P_SetMobjState(actor, actor->info->missilestate);
        actor->flags |= MF_JUSTATTACKED;
        return;
      }

  if (!actor->threshold) {
    if (!mbf_features)
    {   /* killough 9/9/98: for backward demo compatibility */
      if (netgame && !P_CheckSight(actor, actor->target) &&
          P_LookForPlayers(actor, true))
        return;
    }
    /* killough 7/18/98, 9/9/98: new monster AI */
    else if (help_friends && P_HelpFriend(actor))
      return;      /* killough 9/8/98: Help friends in need */
    /* Look for new targets if current one is bad or is out of view */
    else if (actor->pursuecount)
      actor->pursuecount--;
    else {
      /* Our pursuit time has expired. We're going to think about
       * changing targets */
      actor->pursuecount = BASETHRESHOLD;

      // look for new target, unless conditions are met
      if (
        !(
          actor->target &&                       // have a target
          actor->target->health > 0 &&           // and the target is alive
          (
            (comp[comp_pursuit] && !netgame) ||  // and using old pursuit behaviour
            (
              (                                  // or the target is not friendly
                (actor->target->flags ^ actor->flags) & MF_FRIEND ||
                (!(actor->flags & MF_FRIEND) && monster_infighting)
              ) &&
              P_CheckSight(actor, actor->target) // and we can see it
            )
          )
        ) &&
        P_LookForTargets(actor, true)
      )
        return;

      /* (Current target was good, or no new target was found.)
       *
       * If monster is a missile-less friend, give up pursuit and
       * return to player, if no attacks have occurred recently.
       */

      if (!actor->info->missilestate && actor->flags & MF_FRIEND) {
        if (actor->flags & MF_JUSTHIT)          /* if recent action, */
          actor->flags &= ~MF_JUSTHIT;          /* keep fighting */
        else if (P_LookForPlayers(actor, true)) /* else return to player */
          return;
      }
    }
  }

  if (actor->strafecount)
    actor->strafecount--;

  // chase towards player
  // HERETIC_NOTE: Quite sure P_SmartMove == P_Move for heretic
  if (--actor->movecount < 0 || !P_SmartMove(actor))
    P_NewChaseDir(actor);

  // make active sound
  if (actor->info->activesound && P_Random(pr_see) < 3)
  {
    if (actor->type == HERETIC_MT_WIZARD && P_Random(pr_heretic) < 128)
    {
        S_StartSound(actor, actor->info->seesound);
    }
    else if (actor->type == HERETIC_MT_SORCERER2)
    {
        S_StartSound(NULL, actor->info->activesound);
    }
    else
    {
        S_StartSound(actor, actor->info->activesound);
    }
  }
}

//
// A_FaceTarget
//
void A_FaceTarget(mobj_t *actor)
{
  if (!actor->target)
    return;
  actor->flags &= ~MF_AMBUSH;
  actor->angle = R_PointToAngle2(actor->x, actor->y,
                                 actor->target->x, actor->target->y);
  if (actor->target->flags & MF_SHADOW)
    { // killough 5/5/98: remove dependence on order of evaluation:
      int t = P_Random(pr_facetarget);
      actor->angle += (t-P_Random(pr_facetarget))<<21;
    }
}

//
// A_PosAttack
//

void A_PosAttack(mobj_t *actor)
{
  int angle, damage, slope, t;

  if (!actor->target)
    return;
  A_FaceTarget(actor);
  angle = actor->angle;
  slope = P_AimLineAttack(actor, angle, MISSILERANGE, 0); /* killough 8/2/98 */
  S_StartSound(actor, sfx_pistol);

  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random(pr_posattack);
  angle += (t - P_Random(pr_posattack))<<20;
  damage = (P_Random(pr_posattack)%5 + 1)*3;
  P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack(mobj_t* actor)
{
  int i, bangle, slope;

  if (!actor->target)
    return;
  S_StartSound(actor, sfx_shotgn);
  A_FaceTarget(actor);
  bangle = actor->angle;
  slope = P_AimLineAttack(actor, bangle, MISSILERANGE, 0); /* killough 8/2/98 */
  for (i=0; i<3; i++)
    {  // killough 5/5/98: remove dependence on order of evaluation:
      int t = P_Random(pr_sposattack);
      int angle = bangle + ((t - P_Random(pr_sposattack))<<20);
      int damage = ((P_Random(pr_sposattack)%5)+1)*3;
      P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
    }
}

void A_CPosAttack(mobj_t *actor)
{
  int angle, bangle, damage, slope, t;

  if (!actor->target)
    return;
  S_StartSound(actor, sfx_shotgn);
  A_FaceTarget(actor);
  bangle = actor->angle;
  slope = P_AimLineAttack(actor, bangle, MISSILERANGE, 0); /* killough 8/2/98 */

  // killough 5/5/98: remove dependence on order of evaluation:
  t = P_Random(pr_cposattack);
  angle = bangle + ((t - P_Random(pr_cposattack))<<20);
  damage = ((P_Random(pr_cposattack)%5)+1)*3;
  P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire(mobj_t *actor)
{
  // keep firing unless target got out of sight
  A_FaceTarget(actor);

  /* killough 12/98: Stop firing if a friend has gotten in the way */
  if (P_HitFriend(actor))
    goto stop;

  /* killough 11/98: prevent refiring on friends continuously */
  if (P_Random(pr_cposrefire) < 40) {
    if (actor->target && actor->flags & actor->target->flags & MF_FRIEND)
      goto stop;
    else
      return;
  }

  if (!actor->target || actor->target->health <= 0
      || !P_CheckSight(actor, actor->target))
stop:  P_SetMobjState(actor, actor->info->seestate);
}

void A_SpidRefire(mobj_t* actor)
{
  // keep firing unless target got out of sight
  A_FaceTarget(actor);

  /* killough 12/98: Stop firing if a friend has gotten in the way */
  if (P_HitFriend(actor))
    goto stop;

  if (P_Random(pr_spidrefire) < 10)
    return;

  // killough 11/98: prevent refiring on friends continuously
  if (!actor->target || actor->target->health <= 0
      || actor->flags & actor->target->flags & MF_FRIEND
      || !P_CheckSight(actor, actor->target))
    stop: P_SetMobjState(actor, actor->info->seestate);
}

void A_BspiAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  P_SpawnMissile(actor, actor->target, MT_ARACHPLAZ);  // launch a missile
}

//
// A_TroopAttack
//

void A_TroopAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  if (P_CheckMeleeRange(actor))
    {
      int damage;
      S_StartSound(actor, sfx_claw);
      damage = (P_Random(pr_troopattack)%8+1)*3;
      P_DamageMobj(actor->target, actor, actor, damage);
      return;
    }
  P_SpawnMissile(actor, actor->target, MT_TROOPSHOT);  // launch a missile
}

void A_SargAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  if (compatibility_level == doom_12_compatibility)
  {
    int damage = ((P_Random(pr_sargattack)%10)+1)*4;
    P_LineAttack(actor, actor->angle, MELEERANGE, 0, damage);
  }
  else
  {
  if (P_CheckMeleeRange(actor))
    {
      int damage = ((P_Random(pr_sargattack)%10)+1)*4;
      P_DamageMobj(actor->target, actor, actor, damage);
    }
  }
}

void A_HeadAttack(mobj_t * actor)
{
  int i;
  mobj_t *fire;
  mobj_t *baseFire;
  mobj_t *mo;
  mobj_t *target;
  int randAttack;
  static int atkResolve1[] = { 50, 150 };
  static int atkResolve2[] = { 150, 200 };
  int dist;

  // Ice ball     (close 20% : far 60%)
  // Fire column  (close 40% : far 20%)
  // Whirlwind    (close 40% : far 20%)
  // Distance threshold = 8 cells

  target = actor->target;
  if (target == NULL) return;

  A_FaceTarget(actor);

  if (P_CheckMeleeRange(actor))
  {
    int damage = heretic ? HITDICE(6) : (P_Random(pr_headattack) % 6 + 1) * 10;
    P_DamageMobj(target, actor, actor, damage);
    return;
  }

  if (!heretic) {
    P_SpawnMissile(actor, target, MT_HEADSHOT);
    return;
  }

  dist = P_AproxDistance(actor->x - target->x, actor->y - target->y)
         > 8 * 64 * FRACUNIT;
  randAttack = P_Random(pr_heretic);
  if (randAttack < atkResolve1[dist])
  {                           // Ice ball
    P_SpawnMissile(actor, target, HERETIC_MT_HEADFX1);
    S_StartSound(actor, heretic_sfx_hedat2);
  }
  else if (randAttack < atkResolve2[dist])
  {                           // Fire column
    baseFire = P_SpawnMissile(actor, target, HERETIC_MT_HEADFX3);
    if (baseFire != NULL)
    {
      P_SetMobjState(baseFire, HERETIC_S_HEADFX3_4);      // Don't grow
      for (i = 0; i < 5; i++)
      {
        fire = P_SpawnMobj(baseFire->x, baseFire->y,
                           baseFire->z, HERETIC_MT_HEADFX3);
        if (i == 0)
        {
          S_StartSound(actor, heretic_sfx_hedat1);
        }
        P_SetTarget(&fire->target, baseFire->target);
        fire->angle = baseFire->angle;
        fire->momx = baseFire->momx;
        fire->momy = baseFire->momy;
        fire->momz = baseFire->momz;
        fire->damage = 0;
        fire->health = (i + 1) * 2;
        P_CheckMissileSpawn(fire);
      }
    }
  }
  else
  {                           // Whirlwind
    mo = P_SpawnMissile(actor, target, HERETIC_MT_WHIRLWIND);
    if (mo != NULL)
    {
      mo->z -= 32 * FRACUNIT;
      mo->special1.m = target;
      mo->special2.i = 50;  // Timer for active sound
      mo->health = 20 * TICRATE;       // Duration
      S_StartSound(actor, heretic_sfx_hedat3);
    }
  }
}

void A_CyberAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  P_SpawnMissile(actor, actor->target, MT_ROCKET);
}

void A_BruisAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  if (P_CheckMeleeRange(actor))
    {
      int damage;
      S_StartSound(actor, sfx_claw);
      damage = (P_Random(pr_bruisattack)%8+1)*10;
      P_DamageMobj(actor->target, actor, actor, damage);
      return;
    }
  P_SpawnMissile(actor, actor->target, MT_BRUISERSHOT);  // launch a missile
}

//
// A_SkelMissile
//

void A_SkelMissile(mobj_t *actor)
{
  mobj_t *mo;

  if (!actor->target)
    return;

  A_FaceTarget (actor);
  actor->z += 16*FRACUNIT;      // so missile spawns higher
  mo = P_SpawnMissile (actor, actor->target, MT_TRACER);
  actor->z -= 16*FRACUNIT;      // back to normal

  mo->x += mo->momx;
  mo->y += mo->momy;
  P_SetTarget(&mo->tracer, actor->target);
}

int     TRACEANGLE = 0xc000000;

void A_Tracer(mobj_t *actor)
{
  angle_t       exact;
  fixed_t       dist;
  fixed_t       slope;
  mobj_t        *dest;
  mobj_t        *th;

  /* killough 1/18/98: this is why some missiles do not have smoke
   * and some do. Also, internal demos start at random gametics, thus
   * the bug in which revenants cause internal demos to go out of sync.
   *
   * killough 3/6/98: fix revenant internal demo bug by subtracting
   * levelstarttic from gametic.
   *
   * killough 9/29/98: use new "basetic" so that demos stay in sync
   * during pauses and menu activations, while retaining old demo sync.
   *
   * leveltime would have been better to use to start with in Doom, but
   * since old demos were recorded using gametic, we must stick with it,
   * and improvise around it (using leveltime causes desync across levels).
   */

  if ((gametic-basetic) & 3)
    return;

  // spawn a puff of smoke behind the rocket
  P_SpawnPuff(actor->x, actor->y, actor->z);

  th = P_SpawnMobj (actor->x-actor->momx,
                    actor->y-actor->momy,
                    actor->z, MT_SMOKE);

  th->momz = FRACUNIT;
  th->tics -= P_Random(pr_tracer) & 3;
  if (th->tics < 1)
    th->tics = 1;

  // adjust direction
  dest = actor->tracer;

  if (!dest || dest->health <= 0)
    return;

  // change angle
  exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

  if (exact != actor->angle) {
    if (exact - actor->angle > 0x80000000)
      {
        actor->angle -= TRACEANGLE;
        if (exact - actor->angle < 0x80000000)
          actor->angle = exact;
      }
    else
      {
        actor->angle += TRACEANGLE;
        if (exact - actor->angle > 0x80000000)
          actor->angle = exact;
      }
  }

  exact = actor->angle>>ANGLETOFINESHIFT;
  actor->momx = FixedMul(actor->info->speed, finecosine[exact]);
  actor->momy = FixedMul(actor->info->speed, finesine[exact]);

  // change slope
  dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);

  dist = dist / actor->info->speed;

  if (dist < 1)
    dist = 1;

  slope = (dest->z+40*FRACUNIT - actor->z) / dist;

  if (slope < actor->momz)
    actor->momz -= FRACUNIT/8;
  else
    actor->momz += FRACUNIT/8;
}

void A_SkelWhoosh(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  S_StartSound(actor,sfx_skeswg);
}

void A_SkelFist(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  if (P_CheckMeleeRange(actor))
    {
      int damage = ((P_Random(pr_skelfist)%10)+1)*6;
      S_StartSound(actor, sfx_skepch);
      P_DamageMobj(actor->target, actor, actor, damage);
    }
}

//
// PIT_VileCheck
// Detect a corpse that could be raised.
//

mobj_t* corpsehit;
mobj_t* vileobj;
fixed_t viletryx;
fixed_t viletryy;
int viletryradius;

static dboolean PIT_VileCheck(mobj_t *thing)
{
  int     maxdist;
  dboolean check;

  if (!(thing->flags & MF_CORPSE) )
    return true;        // not a monster

  if (thing->tics != -1)
    return true;        // not lying still yet

  if (thing->info->raisestate == g_s_null)
    return true;        // monster doesn't have a raise state

  maxdist = thing->info->radius + viletryradius;

  if (D_abs(thing->x-viletryx) > maxdist || D_abs(thing->y-viletryy) > maxdist)
    return true;                // not actually touching

// Check to see if the radius and height are zero. If they are      // phares
// then this is a crushed monster that has been turned into a       //   |
// gib. One of the options may be to ignore this guy.               //   V

// Option 1: the original, buggy method, -> ghost (compatibility)
// Option 2: ressurect the monster, but not as a ghost
// Option 3: ignore the gib

//    if (Option3)                                                  //   ^
//        if ((thing->height == 0) && (thing->radius == 0))         //   |
//            return true;                                          // phares

    corpsehit = thing;
    corpsehit->momx = corpsehit->momy = 0;
    if (comp[comp_vile])                                            // phares
      {                                                             //   |
        corpsehit->height <<= 2;                                    //   V
        check = P_CheckPosition(corpsehit,corpsehit->x,corpsehit->y);
        corpsehit->height >>= 2;
      }
    else
      {
        int height,radius;

        height = corpsehit->height; // save temporarily
        radius = corpsehit->radius; // save temporarily
        corpsehit->height = corpsehit->info->height;
        corpsehit->radius = corpsehit->info->radius;
        corpsehit->flags |= MF_SOLID;
        check = P_CheckPosition(corpsehit,corpsehit->x,corpsehit->y);
        corpsehit->height = height; // restore
        corpsehit->radius = radius; // restore                      //   ^
        corpsehit->flags &= ~MF_SOLID;
      }                                                             //   |
                                                                    // phares
    if (!check)
      return true;              // doesn't fit here
    return false;               // got one, so stop checking
}

//
// P_HealCorpse
// Check for ressurecting a body
//

static dboolean P_HealCorpse(mobj_t* actor, int radius, statenum_t healstate, sfxenum_t healsound)
{
  int xl, xh;
  int yl, yh;
  int bx, by;

  if (actor->movedir != DI_NODIR)
    {
      // check for corpses to raise
      viletryx =
        actor->x + actor->info->speed*xspeed[actor->movedir];
      viletryy =
        actor->y + actor->info->speed*yspeed[actor->movedir];

      xl = P_GetSafeBlockX(viletryx - bmaporgx - MAXRADIUS*2);
      xh = P_GetSafeBlockX(viletryx - bmaporgx + MAXRADIUS*2);
      yl = P_GetSafeBlockY(viletryy - bmaporgy - MAXRADIUS*2);
      yh = P_GetSafeBlockY(viletryy - bmaporgy + MAXRADIUS*2);

      vileobj = actor;
      viletryradius = radius;
      for (bx=xl ; bx<=xh ; bx++)
        {
          for (by=yl ; by<=yh ; by++)
            {
              // Call PIT_VileCheck to check
              // whether object is a corpse
              // that canbe raised.
              if (!P_BlockThingsIterator(bx,by,PIT_VileCheck))
                {
      mobjinfo_t *info;

                  // got one!
                  mobj_t* temp = actor->target;
                  actor->target = corpsehit;
                  A_FaceTarget(actor);
                  actor->target = temp;

                  P_SetMobjState(actor, healstate);
                  S_StartSound(corpsehit, healsound);
                  info = corpsehit->info;

                  P_SetMobjState(corpsehit,info->raisestate);

                  if (comp[comp_vile])                              // phares
                    corpsehit->height <<= 2;                        //   |
                  else                                              //   V
                    {
                      corpsehit->height = info->height; // fix Ghost bug
                      corpsehit->radius = info->radius; // fix Ghost bug
                    }                                               // phares

      /* killough 7/18/98:
       * friendliness is transferred from AV to raised corpse
       */
      corpsehit->flags =
        (info->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND);
      corpsehit->flags = corpsehit->flags | MF_RESSURECTED;//e6y

      dsda_WatchResurrection(corpsehit);

		  if (!((corpsehit->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
		    totallive++;

                  corpsehit->health = info->spawnhealth;
      P_SetTarget(&corpsehit->target, NULL);  // killough 11/98

      if (mbf_features)
        {         /* kilough 9/9/98 */
          P_SetTarget(&corpsehit->lastenemy, NULL);
          corpsehit->flags &= ~MF_JUSTHIT;
        }

      /* killough 8/29/98: add to appropriate thread */
      P_UpdateThinker(&corpsehit->thinker);

                  return true;
                }
            }
        }
    }
  return false;
}

//
// A_VileChase
//

void A_VileChase(mobj_t* actor)
{
  if (!P_HealCorpse(actor, mobjinfo[MT_VILE].radius, S_VILE_HEAL1, sfx_slop))
    A_Chase(actor);  // Return to normal attack.
}

//
// A_VileStart
//

void A_VileStart(mobj_t *actor)
{
  S_StartSound(actor, sfx_vilatk);
}

//
// A_Fire
// Keep fire in front of player unless out of sight
//

void A_StartFire(mobj_t *actor)
{
  S_StartSound(actor,sfx_flamst);
  A_Fire(actor);
}

void A_FireCrackle(mobj_t* actor)
{
  S_StartSound(actor,sfx_flame);
  A_Fire(actor);
}

void A_Fire(mobj_t *actor)
{
  mobj_t* target;
  unsigned an;
  mobj_t *dest = actor->tracer;

  if (!dest)
    return;

  target = P_SubstNullMobj(actor->target);

  // don't move it if the vile lost sight
  if (!P_CheckSight(target, dest) )
    return;

  an = dest->angle >> ANGLETOFINESHIFT;

  P_UnsetThingPosition(actor);
  actor->x = dest->x + FixedMul(24*FRACUNIT, finecosine[an]);
  actor->y = dest->y + FixedMul(24*FRACUNIT, finesine[an]);
  actor->z = dest->z;
  P_SetThingPosition(actor);
}

//
// A_VileTarget
// Spawn the hellfire
//

void A_VileTarget(mobj_t *actor)
{
  mobj_t *fog;

  if (!actor->target)
    return;

  A_FaceTarget(actor);

  // killough 12/98: fix Vile fog coordinates // CPhipps - compatibility optioned
  fog = P_SpawnMobj(actor->target->x,
    (compatibility_level < lxdoom_1_compatibility) ? actor->target->x : actor->target->y,
                    actor->target->z,MT_FIRE);

  P_SetTarget(&actor->tracer, fog);
  P_SetTarget(&fog->target, actor);
  P_SetTarget(&fog->tracer, actor->target);
  A_Fire(fog);
}

//
// A_VileAttack
//

void A_VileAttack(mobj_t *actor)
{
  mobj_t *fire;
  int    an;

  if (!actor->target)
    return;

  A_FaceTarget(actor);

  if (!P_CheckSight(actor, actor->target))
    return;

  S_StartSound(actor, sfx_barexp);
  P_DamageMobj(actor->target, actor, actor, 20);
  actor->target->momz = 1000*FRACUNIT/actor->target->info->mass;

  an = actor->angle >> ANGLETOFINESHIFT;

  fire = actor->tracer;

  if (!fire)
    return;

  // move the fire between the vile and the player
  fire->x = actor->target->x - FixedMul (24*FRACUNIT, finecosine[an]);
  fire->y = actor->target->y - FixedMul (24*FRACUNIT, finesine[an]);
  P_RadiusAttack(fire, actor, 70, 70);
}

//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it.
//

#define FATSPREAD       (ANG90/8)

void A_FatRaise(mobj_t *actor)
{
  A_FaceTarget(actor);
  S_StartSound(actor, sfx_manatk);
}

void A_FatAttack1(mobj_t *actor)
{
  mobj_t *mo;
  mobj_t* target;
  int    an;

  if (!actor->target)
    return;

  A_FaceTarget(actor);

  // Change direction  to ...
  actor->angle += FATSPREAD;
  target = P_SubstNullMobj(actor->target);
  P_SpawnMissile(actor, target, MT_FATSHOT);

  mo = P_SpawnMissile (actor, target, MT_FATSHOT);
  mo->angle += FATSPREAD;
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack2(mobj_t *actor)
{
  mobj_t *mo;
  mobj_t* target;
  int    an;

  if (!actor->target)
    return;

  A_FaceTarget(actor);
  // Now here choose opposite deviation.
  actor->angle -= FATSPREAD;
  target = P_SubstNullMobj(actor->target);
  P_SpawnMissile(actor, target, MT_FATSHOT);

  mo = P_SpawnMissile(actor, target, MT_FATSHOT);
  mo->angle -= FATSPREAD*2;
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);
}

void A_FatAttack3(mobj_t *actor)
{
  mobj_t *mo;
  mobj_t* target;
  int    an;

  if (!actor->target)
    return;

  A_FaceTarget(actor);

  target = P_SubstNullMobj(actor->target);

  mo = P_SpawnMissile(actor, target, MT_FATSHOT);
  mo->angle -= FATSPREAD/2;
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);

  mo = P_SpawnMissile(actor, target, MT_FATSHOT);
  mo->angle += FATSPREAD/2;
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);
}


//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED              (20*FRACUNIT)

void A_SkullAttack(mobj_t *actor)
{
  mobj_t  *dest;
  angle_t an;
  int     dist;

  if (!actor->target)
    return;

  dest = actor->target;
  actor->flags |= MF_SKULLFLY;

  S_StartSound(actor, actor->info->attacksound);
  A_FaceTarget(actor);
  an = actor->angle >> ANGLETOFINESHIFT;
  actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
  actor->momy = FixedMul(SKULLSPEED, finesine[an]);
  dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
  dist = dist / SKULLSPEED;

  if (dist < 1)
    dist = 1;
  actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
}

void A_BetaSkullAttack(mobj_t *actor)
{
  int damage;

  if (compatibility_level < mbf_compatibility)
    return;

  if (!actor->target || actor->target->type == MT_SKULL)
    return;

  S_StartSound(actor, actor->info->attacksound);
  A_FaceTarget(actor);
  damage = (P_Random(pr_skullfly)%8+1)*actor->info->damage;
  P_DamageMobj(actor->target, actor, actor, damage);
}

void A_Stop(mobj_t *actor)
{
  if (compatibility_level < mbf_compatibility)
    return;

  actor->momx = actor->momy = actor->momz = 0;
}

//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//

static void A_PainShootSkull(mobj_t *actor, angle_t angle)
{
  fixed_t       x,y,z;
  mobj_t        *newmobj;
  angle_t       an;
  int           prestep;

// The original code checked for 20 skulls on the level,            // phares
// and wouldn't spit another one if there were. If not in           // phares
// compatibility mode, we remove the limit.                         // phares
                                                                    // phares
  if (comp[comp_pain]) /* killough 10/98: compatibility-optioned */
    {
      // count total number of skulls currently on the level
      int count = 0;
      thinker_t *currentthinker = NULL;
      while ((currentthinker = P_NextThinker(currentthinker,th_all)) != NULL)
        if ((currentthinker->function == P_MobjThinker)
            && ((mobj_t *)currentthinker)->type == MT_SKULL)
          count++;
      if (count > 20)                                               // phares
        return;                                                     // phares
    }

  // okay, there's room for another one

  an = angle >> ANGLETOFINESHIFT;

  prestep = 4*FRACUNIT + 3*(actor->info->radius + mobjinfo[MT_SKULL].radius)/2;

  x = actor->x + FixedMul(prestep, finecosine[an]);
  y = actor->y + FixedMul(prestep, finesine[an]);
  z = actor->z + 8*FRACUNIT;

  if (comp[comp_skull])   /* killough 10/98: compatibility-optioned */
    newmobj = P_SpawnMobj(x, y, z, MT_SKULL);                     // phares
  else                                                            //   V
    {
      // Check whether the Lost Soul is being fired through a 1-sided
      // wall or an impassible line, or a "monsters can't cross" line.
      // If it is, then we don't allow the spawn. This is a bug fix, but
      // it should be considered an enhancement, since it may disturb
      // existing demos, so don't do it in compatibility mode.

      if (Check_Sides(actor,x,y))
        return;

      newmobj = P_SpawnMobj(x, y, z, MT_SKULL);

      // Check to see if the new Lost Soul's z value is above the
      // ceiling of its new sector, or below the floor. If so, kill it.

      if ((newmobj->z >
           (newmobj->subsector->sector->ceilingheight - newmobj->height)) ||
          (newmobj->z < newmobj->subsector->sector->floorheight))
        {
          // kill it immediately
          P_DamageMobj(newmobj,actor,actor,10000);
          return;                                                 //   ^
        }                                                         //   |
     }                                                            // phares

  /* killough 7/20/98: PEs shoot lost souls with the same friendliness */
  newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND);

  /* killough 8/29/98: add to appropriate thread */
  P_UpdateThinker(&newmobj->thinker);

  // Check for movements.
  // killough 3/15/98: don't jump over dropoffs:

  if (!P_TryMove(newmobj, newmobj->x, newmobj->y, false))
    {
      // kill it immediately
      P_DamageMobj(newmobj, actor, actor, 10000);
      return;
    }

  P_SetTarget(&newmobj->target, actor->target);
  A_SkullAttack(newmobj);
}

//
// A_PainAttack
// Spawn a lost soul and launch it at the target
//

void A_PainAttack(mobj_t *actor)
{
  if (!actor->target)
    return;
  A_FaceTarget(actor);
  A_PainShootSkull(actor, actor->angle);
}

void A_PainDie(mobj_t *actor)
{
  A_Fall(actor);
  A_PainShootSkull(actor, actor->angle+ANG90);
  A_PainShootSkull(actor, actor->angle+ANG180);
  A_PainShootSkull(actor, actor->angle+ANG270);
}

void A_Scream(mobj_t *actor)
{
  int sound;

  if (heretic) return Heretic_A_Scream(actor);

  switch (actor->info->deathsound)
    {
    case 0:
      return;

    case sfx_podth1:
    case sfx_podth2:
    case sfx_podth3:
      sound = sfx_podth1 + P_Random(pr_scream)%3;
      break;

    case sfx_bgdth1:
    case sfx_bgdth2:
      sound = sfx_bgdth1 + P_Random(pr_scream)%2;
      break;

    default:
      sound = actor->info->deathsound;
      break;
    }

  // Check for bosses.
  if (actor->flags2 & (MF2_BOSS | MF2_FULLVOLSOUNDS))
    S_StartSound(NULL, sound); // full volume
  else
    S_StartSound(actor, sound);
}

void A_XScream(mobj_t *actor)
{
  S_StartSound(actor, sfx_slop);
}

void A_SkullPop(mobj_t *actor)
{
  mobj_t *mo;
  player_t *player;
  int sfx_id;

  if (!heretic && (demorecording || demoplayback))
    return;

  if (!heretic) {
    sfx_id = (I_GetSfxLumpNum(&S_sfx[sfx_gibdth]) < 0 ? sfx_pldeth : sfx_gibdth);
    S_StartSound(actor, sfx_id);
  }

  actor->flags &= ~MF_SOLID;
  mo = P_SpawnMobj(actor->x, actor->y, actor->z + 48 * FRACUNIT, g_skullpop_mt);
  //mo->target = actor;
  mo->momx = P_SubRandom() << 9;
  mo->momy = P_SubRandom() << 9;
  mo->momz = FRACUNIT * 2 + (P_Random(pr_heretic) << 6);
  // Attach player mobj to bloody skull
  player = actor->player;
  actor->player = NULL;
  mo->player = player;
  mo->health = actor->health;
  mo->angle = actor->angle;
  mo->pitch = 0;
  if (player)
  {
    player->mo = mo;
    player->lookdir = 0;
    player->damagecount = 32;
  }
}

void A_Pain(mobj_t *actor)
{
  if (actor->info->painsound)
    S_StartSound(actor, actor->info->painsound);
}

void A_Fall(mobj_t *actor)
{
  // actor is on ground, it can be walked over
  actor->flags &= ~MF_SOLID;
}

//
// A_Explode
//
void A_Explode(mobj_t *thingy)
{
  int damage;

  damage = 128;
  switch (thingy->type)
  {
    case HERETIC_MT_FIREBOMB:      // Time Bombs
      thingy->z += 32 * FRACUNIT;
      thingy->flags &= ~MF_SHADOW;
      break;
    case HERETIC_MT_MNTRFX2:       // Minotaur floor fire
      damage = 24;
      break;
    case HERETIC_MT_SOR2FX1:       // D'Sparil missile
      damage = 80 + (P_Random(pr_heretic) & 31);
      break;
    default:
      break;
  }

  P_RadiusAttack(thingy, thingy->target, damage, damage);
  if (heretic) P_HitFloor(thingy);
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//

void A_BossDeath(mobj_t *mo)
{
  thinker_t *th;
  line_t    junk;
  int       i;

  // heretic_note: probably we can adopt the clean heretic style and merge
  if (heretic) return Heretic_A_BossDeath(mo);

  // numbossactions == 0 means to use the defaults.
  // numbossactions == -1 means to do nothing.
  // positive values mean to check the list of boss actions and run all that apply.
  if (gamemapinfo && gamemapinfo->numbossactions != 0)
  {
	  if (gamemapinfo->numbossactions < 0) return;

	  // make sure there is a player alive for victory
	  for (i=0; i<MAXPLAYERS; i++)
		if (playeringame[i] && players[i].health > 0)
		  break;

	  if (i==MAXPLAYERS)
		return;     // no one left alive, so do not end game

	  for (i = 0; i < gamemapinfo->numbossactions; i++)
	  {
		  if (gamemapinfo->bossactions[i].type == mo->type)
			  break;
	  }
	  if (i >= gamemapinfo->numbossactions)
		  return;	// no matches found

		// scan the remaining thinkers to see
		// if all bosses are dead
	  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
		if (th->function == P_MobjThinker)
		  {
			mobj_t *mo2 = (mobj_t *) th;
			if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
			  return;         // other boss not dead
      }
	  for (i = 0; i < gamemapinfo->numbossactions; i++)
	  {
		  if (gamemapinfo->bossactions[i].type == mo->type)
		  {
			  junk = *lines;
			  junk.special = (short)gamemapinfo->bossactions[i].special;
			  junk.tag = (short)gamemapinfo->bossactions[i].tag;
			  // use special semantics for line activation to block problem types.
			  if (!P_UseSpecialLine(mo, &junk, 0, true))
				  P_CrossSpecialLine(&junk, 0, mo, true);
		  }
	  }

	  return;
  }

  if (gamemode == commercial)
    {
      if (gamemap != 7)
        return;

      if (!(mo->flags2 & (MF2_MAP07BOSS1 | MF2_MAP07BOSS2)))
        return;
    }
  else
    {
      // e6y
      // Additional check of gameepisode is necessary, because
      // there is no right or wrong solution for E4M6 in original EXEs,
      // there's nothing to emulate.
      if (comp[comp_666] && gameepisode < 4)
      {
        // e6y
        // Only following checks are present in doom2.exe ver. 1.666 and 1.9
        // instead of separate checks for each episode in doomult.exe, plutonia.exe and tnt.exe
        // There is no more desync on doom.wad\episode3.lmp
        // http://www.doomworld.com/idgames/index.php?id=6909
        if (gamemap != 8)
          return;
        if (mo->flags2 & MF2_E1M8BOSS && gameepisode != 1)
          return;
      }
      else
      {
      switch(gameepisode)
        {
        case 1:
          if (gamemap != 8)
            return;

          if (!(mo->flags2 & MF2_E1M8BOSS))
            return;
          break;

        case 2:
          if (gamemap != 8)
            return;

          if (!(mo->flags2 & MF2_E2M8BOSS))
            return;
          break;

        case 3:
          if (gamemap != 8)
            return;

          if (!(mo->flags2 & MF2_E3M8BOSS))
            return;

          break;

        case 4:
          switch(gamemap)
            {
            case 6:
              if (!(mo->flags2 & MF2_E4M6BOSS))
                return;
              break;

            case 8:
              if (!(mo->flags2 & MF2_E4M8BOSS))
                return;
              break;

            default:
              return;
              break;
            }
          break;

        default:
          if (gamemap != 8)
            return;
          break;
        }
      }

    }

  // make sure there is a player alive for victory
  for (i=0; i<MAXPLAYERS; i++)
    if (playeringame[i] && players[i].health > 0)
      break;

  if (i==MAXPLAYERS)
    return;     // no one left alive, so do not end game

    // scan the remaining thinkers to see
    // if all bosses are dead
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function == P_MobjThinker)
      {
        mobj_t *mo2 = (mobj_t *) th;
        if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
          return;         // other boss not dead
      }

  // victory!
  if ( gamemode == commercial)
    {
      if (gamemap == 7)
        {
          if (mo->flags2 & MF2_MAP07BOSS1)
            {
              junk.tag = 666;
              EV_DoFloor(&junk,lowerFloorToLowest);
              return;
            }

          if (mo->flags2 & MF2_MAP07BOSS2)
            {
              junk.tag = 667;
              EV_DoFloor(&junk,raiseToTexture);
              return;
            }
        }
    }
  else
    {
      switch(gameepisode)
        {
        case 1:
          junk.tag = 666;
          EV_DoFloor(&junk, lowerFloorToLowest);
          return;
          break;

        case 4:
          switch(gamemap)
            {
            case 6:
              junk.tag = 666;
              EV_DoDoor(&junk, blazeOpen);
              return;
              break;

            case 8:
              junk.tag = 666;
              EV_DoFloor(&junk, lowerFloorToLowest);
              return;
              break;
            }
        }
    }
  G_ExitLevel();
}


void A_Hoof (mobj_t* mo)
{
    S_StartSound(mo, sfx_hoof);
    A_Chase(mo);
}

void A_Metal(mobj_t *mo)
{
  S_StartSound(mo, sfx_metal);
  A_Chase(mo);
}

void A_BabyMetal(mobj_t *mo)
{
  S_StartSound(mo, sfx_bspwlk);
  A_Chase(mo);
}

void A_OpenShotgun2(player_t *player, pspdef_t *psp)
{
  S_StartSound(player->mo, sfx_dbopn);
}

void A_LoadShotgun2(player_t *player, pspdef_t *psp)
{
  S_StartSound(player->mo, sfx_dbload);
}

void A_CloseShotgun2(player_t *player, pspdef_t *psp)
{
  S_StartSound(player->mo, sfx_dbcls);
  A_ReFire(player,psp);
}

// killough 2/7/98: Remove limit on icon landings:
mobj_t **braintargets;
int    numbraintargets_alloc;
int    numbraintargets;

struct brain_s brain;   // killough 3/26/98: global state of boss brain

// killough 3/26/98: initialize icon landings at level startup,
// rather than at boss wakeup, to prevent savegame-related crashes

void P_SpawnBrainTargets(void)  // killough 3/26/98: renamed old function
{
  thinker_t *thinker;

  // find all the target spots
  numbraintargets = 0;
  brain.targeton = 0;
  brain.easy = 0;           // killough 3/26/98: always init easy to 0

  for (thinker = thinkercap.next ;
       thinker != &thinkercap ;
       thinker = thinker->next)
    if (thinker->function == P_MobjThinker)
      {
        mobj_t *m = (mobj_t *) thinker;

        if (m->type == MT_BOSSTARGET )
          {   // killough 2/7/98: remove limit on icon landings:
            if (numbraintargets >= numbraintargets_alloc)
              braintargets = realloc(braintargets,
                      (numbraintargets_alloc = numbraintargets_alloc ?
                       numbraintargets_alloc*2 : 32) *sizeof *braintargets);
            braintargets[numbraintargets++] = m;
          }
      }
}

void A_BrainAwake(mobj_t *mo)
{
  //e6y
  if (demo_compatibility && !prboom_comp[PC_BOOM_BRAINAWAKE].state)
  {
    brain.targeton = 0;
    brain.easy = 0;
  }

  S_StartSound(NULL,sfx_bossit); // killough 3/26/98: only generates sound now
}

void A_BrainPain(mobj_t *mo)
{
  S_StartSound(NULL,sfx_bospn);
}

void A_BrainScream(mobj_t *mo)
{
  int x;
  for (x=mo->x - 196*FRACUNIT ; x< mo->x + 320*FRACUNIT ; x+= FRACUNIT*8)
    {
      int y = mo->y - 320*FRACUNIT;
      int z = 128 + P_Random(pr_brainscream)*2*FRACUNIT;
      mobj_t *th = P_SpawnMobj (x,y,z, MT_ROCKET);
      th->momz = P_Random(pr_brainscream)*512;
      P_SetMobjState(th, S_BRAINEXPLODE1);
      th->tics -= P_Random(pr_brainscream)&7;
      if (th->tics < 1)
        th->tics = 1;
    }
  S_StartSound(NULL,sfx_bosdth);
}

void A_BrainExplode(mobj_t *mo)
{  // killough 5/5/98: remove dependence on order of evaluation:
  int t = P_Random(pr_brainexp);
  int x = mo->x + (t - P_Random(pr_brainexp))*2048;
  int y = mo->y;
  int z = 128 + P_Random(pr_brainexp)*2*FRACUNIT;
  mobj_t *th = P_SpawnMobj(x,y,z, MT_ROCKET);
  th->momz = P_Random(pr_brainexp)*512;
  P_SetMobjState(th, S_BRAINEXPLODE1);
  th->tics -= P_Random(pr_brainexp)&7;
  if (th->tics < 1)
    th->tics = 1;
}

void A_BrainDie(mobj_t *mo)
{
  G_ExitLevel();
}

void A_BrainSpit(mobj_t *mo)
{
  mobj_t *targ, *newmobj;

  if (!numbraintargets)     // killough 4/1/98: ignore if no targets
    return;

  brain.easy ^= 1;          // killough 3/26/98: use brain struct
  if (gameskill <= sk_easy && !brain.easy)
    return;

  // shoot a cube at current target
  targ = braintargets[brain.targeton++]; // killough 3/26/98:
  brain.targeton %= numbraintargets;     // Use brain struct for targets

  // spawn brain missile
  newmobj = P_SpawnMissile(mo, targ, MT_SPAWNSHOT);

  // e6y: do not crash with 'incorrect' DEHs
  if (!newmobj || !newmobj->state || newmobj->momy == 0 || newmobj->state->tics == 0)
    I_Error("A_BrainSpit: can't spawn brain missile (incorrect DEH)");

  P_SetTarget(&newmobj->target, targ);
  newmobj->reactiontime = (short)(((targ->y-mo->y)/newmobj->momy)/newmobj->state->tics);

  // killough 7/18/98: brain friendliness is transferred
  newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);

  // killough 8/29/98: add to appropriate thread
  P_UpdateThinker(&newmobj->thinker);

  S_StartSound(NULL, sfx_bospit);
}

// travelling cube sound
void A_SpawnSound(mobj_t *mo)
{
  S_StartSound(mo,sfx_boscub);
  A_SpawnFly(mo);
}

void A_SpawnFly(mobj_t *mo)
{
  mobj_t *newmobj;
  mobj_t *fog;
  mobj_t *targ;
  int    r;
  mobjtype_t type;

  if (--mo->reactiontime)
    return;     // still flying

  targ = P_SubstNullMobj(mo->target);

  // First spawn teleport fog.
  fog = P_SpawnMobj(targ->x, targ->y, targ->z, MT_SPAWNFIRE);
  S_StartSound(fog, sfx_telept);

  // Randomly select monster to spawn.
  r = P_Random(pr_spawnfly);

  // Probability distribution (kind of :), decreasing likelihood.
  if ( r<50 )
    type = MT_TROOP;
  else if (r<90)
    type = MT_SERGEANT;
  else if (r<120)
    type = MT_SHADOWS;
  else if (r<130)
    type = MT_PAIN;
  else if (r<160)
    type = MT_HEAD;
  else if (r<162)
    type = MT_VILE;
  else if (r<172)
    type = MT_UNDEAD;
  else if (r<192)
    type = MT_BABY;
  else if (r<222)
    type = MT_FATSO;
  else if (r<246)
    type = MT_KNIGHT;
  else
    type = MT_BRUISER;

  newmobj = P_SpawnMobj(targ->x, targ->y, targ->z, type);

  /* killough 7/18/98: brain friendliness is transferred */
  newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);

  //e6y: monsters spawned by Icon of Sin should not be countable for total killed.
  newmobj->flags |= MF_RESSURECTED;

  dsda_WatchIconSpawn(newmobj);

  /* killough 8/29/98: add to appropriate thread */
  P_UpdateThinker(&newmobj->thinker);

  if (P_LookForTargets(newmobj,true))      /* killough 9/4/98 */
    P_SetMobjState(newmobj, newmobj->info->seestate);

    // telefrag anything in this spot
  P_TeleportMove(newmobj, newmobj->x, newmobj->y, true); /* killough 8/9/98 */

  // remove self (i.e., cube).
  P_RemoveMobj(mo);
}

void A_PlayerScream(mobj_t *mo)
{
  int sound = sfx_pldeth;  // Default death sound.
  if (gamemode != shareware && mo->health < -50)
    sound = sfx_pdiehi;   // IF THE PLAYER DIES LESS THAN -50% WITHOUT GIBBING
  S_StartSound(mo, sound);
}

/* cph - MBF-added codepointer functions */

// killough 11/98: kill an object
void A_Die(mobj_t *actor)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  P_DamageMobj(actor, NULL, NULL, actor->health);
}

//
// A_Detonate
// killough 8/9/98: same as A_Explode, except that the damage is variable
//

void A_Detonate(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  P_RadiusAttack(mo, mo->target, mo->info->damage, mo->info->damage);
}

//
// killough 9/98: a mushroom explosion effect, sorta :)
// Original idea: Linguica
//

void A_Mushroom(mobj_t *actor)
{
  int i, j, n;

  // Mushroom parameters are part of code pointer's state
  dboolean use_misc;
  fixed_t misc1, misc2;

  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  use_misc = mbf21 || (
    compatibility_level == mbf_compatibility &&
    !prboom_comp[PC_DO_NOT_USE_MISC12_FRAME_PARAMETERS_IN_A_MUSHROOM].state
  );
  misc1 = ((use_misc && actor->state->misc1) ? actor->state->misc1 : FRACUNIT * 4);
  misc2 = ((use_misc && actor->state->misc2) ? actor->state->misc2 : FRACUNIT / 2);
  n = actor->info->damage;

  A_Explode(actor);  // First make normal explosion

  // Now launch mushroom cloud
  for (i = -n; i <= n; i += 8)
    for (j = -n; j <= n; j += 8)
    {
      mobj_t target = *actor, *mo;
      target.x += i << FRACBITS;    // Aim in many directions from source
      target.y += j << FRACBITS;
      target.z += P_AproxDistance(i,j) * misc1;         // Aim up fairly high
      mo = P_SpawnMissile(actor, &target, MT_FATSHOT);  // Launch fireball
      mo->momx = FixedMul(mo->momx, misc2);
      mo->momy = FixedMul(mo->momy, misc2);             // Slow down a bit
      mo->momz = FixedMul(mo->momz, misc2);
      mo->flags &= ~MF_NOGRAVITY;   // Make debris fall under gravity
    }
}

//
// killough 11/98
//
// The following were inspired by Len Pitre
//
// A small set of highly-sought-after code pointers
//

void A_Spawn(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  if (mo->state->misc1)
  {
    mobj_t *newmobj =
      P_SpawnMobj(mo->x, mo->y, (mo->state->misc2 << FRACBITS) + mo->z, mo->state->misc1 - 1);

    if (
      mbf_features &&
      comp[comp_friendlyspawn] &&
      !prboom_comp[PC_DO_NOT_INHERIT_FRIENDLYNESS_FLAG_ON_SPAWN].state
    )
      newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);
  }
}

void A_Turn(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  mo->angle += (unsigned int)(((uint_64_t) mo->state->misc1 << 32) / 360);
}

void A_Face(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  mo->angle = (unsigned int)(((uint_64_t) mo->state->misc1 << 32) / 360);
}

void A_Scratch(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  mo->target && (A_FaceTarget(mo), P_CheckMeleeRange(mo)) ?
    mo->state->misc2 ? S_StartSound(mo, mo->state->misc2) : (void) 0,
    P_DamageMobj(mo->target, mo, mo, mo->state->misc1) : (void) 0;
}

void A_PlaySound(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  S_StartSound(mo->state->misc2 ? NULL : mo, mo->state->misc1);
}

void A_RandomJump(mobj_t *mo)
{
  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  if (P_Random(pr_randomjump) < mo->state->misc2)
    P_SetMobjState(mo, mo->state->misc1);
}

//
// This allows linedef effects to be activated inside deh frames.
//

void A_LineEffect(mobj_t *mo)
{
  static line_t junk;
  player_t player;
  player_t *oldplayer;

  if (compatibility_level < lxdoom_1_compatibility &&
      !prboom_comp[PC_APPLY_MBF_CODEPOINTERS_TO_ANY_COMPLEVEL].state)
    return;

  junk = *lines;
  oldplayer = mo->player;
  mo->player = &player;
  player.health = 100;
  junk.special = (short)mo->state->misc1;
  if (!junk.special)
    return;
  junk.tag = (short)mo->state->misc2;
  if (!P_UseSpecialLine(mo, &junk, 0, false))
    P_CrossSpecialLine(&junk, 0, mo, false);
  mo->state->misc1 = junk.special;
  mo->player = oldplayer;
}

//
// [XA] New mbf21 codepointers
//

//
// A_SpawnObject
// Basically just A_Spawn with better behavior and more args.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling actor's angle
//   args[2]: X spawn offset (fixed point), relative to calling actor
//   args[3]: Y spawn offset (fixed point), relative to calling actor
//   args[4]: Z spawn offset (fixed point), relative to calling actor
//   args[5]: X velocity (fixed point)
//   args[6]: Y velocity (fixed point)
//   args[7]: Z velocity (fixed point)
//
void A_SpawnObject(mobj_t *actor)
{
  int type, angle, ofs_x, ofs_y, ofs_z, vel_x, vel_y, vel_z;
  angle_t an;
  int fan, dx, dy;
  mobj_t *mo;

  if (!mbf21 || !actor->state->args[0])
    return;

  type  = actor->state->args[0] - 1;
  angle = actor->state->args[1];
  ofs_x = actor->state->args[2];
  ofs_y = actor->state->args[3];
  ofs_z = actor->state->args[4];
  vel_x = actor->state->args[5];
  vel_y = actor->state->args[6];
  vel_z = actor->state->args[7];

  // calculate position offsets
  an = actor->angle + (unsigned int)(((int_64_t)angle << 16) / 360);
  fan = an >> ANGLETOFINESHIFT;
  dx = FixedMul(ofs_x, finecosine[fan]) - FixedMul(ofs_y, finesine[fan]  );
  dy = FixedMul(ofs_x, finesine[fan]  ) + FixedMul(ofs_y, finecosine[fan]);

  // spawn it, yo
  mo = P_SpawnMobj(actor->x + dx, actor->y + dy, actor->z + ofs_z, type);
  if (!mo)
    return;

  // angle dangle
  mo->angle = an;

  // set velocity
  mo->momx = FixedMul(vel_x, finecosine[fan]) - FixedMul(vel_y, finesine[fan]  );
  mo->momy = FixedMul(vel_x, finesine[fan]  ) + FixedMul(vel_y, finecosine[fan]);
  mo->momz = vel_z;

  // if spawned object is a missile, set target+tracer
  if (mo->flags & (MF_MISSILE | MF_BOUNCES))
  {
    // if spawner is also a missile, copy 'em
    if (actor->flags & (MF_MISSILE | MF_BOUNCES))
    {
      P_SetTarget(&mo->target, actor->target);
      P_SetTarget(&mo->tracer, actor->tracer);
    }
    // otherwise, set 'em as if a monster fired 'em
    else
    {
      P_SetTarget(&mo->target, actor);
      P_SetTarget(&mo->tracer, actor->target);
    }
  }

  // [XA] don't bother with the dont-inherit-friendliness hack
  // that exists in A_Spawn, 'cause WTF is that about anyway?
}

//
// A_MonsterProjectile
// A parameterized monster projectile attack.
//   args[0]: Type of actor to spawn
//   args[1]: Angle (degrees, in fixed point), relative to calling actor's angle
//   args[2]: Pitch (degrees, in fixed point), relative to calling actor's pitch; approximated
//   args[3]: X/Y spawn offset, relative to calling actor's angle
//   args[4]: Z spawn offset, relative to actor's default projectile fire height
//
void A_MonsterProjectile(mobj_t *actor)
{
  int type, angle, pitch, spawnofs_xy, spawnofs_z;
  mobj_t *mo;
  int an;

  if (!mbf21 || !actor->target || !actor->state->args[0])
    return;

  type        = actor->state->args[0] - 1;
  angle       = actor->state->args[1];
  pitch       = actor->state->args[2];
  spawnofs_xy = actor->state->args[3];
  spawnofs_z  = actor->state->args[4];

  A_FaceTarget(actor);
  mo = P_SpawnMissile(actor, actor->target, type);
  if (!mo)
    return;

  // adjust angle
  mo->angle += (unsigned int)(((int_64_t)angle << 16) / 360);
  an = mo->angle >> ANGLETOFINESHIFT;
  mo->momx = FixedMul(mo->info->speed, finecosine[an]);
  mo->momy = FixedMul(mo->info->speed, finesine[an]);

  // adjust pitch (approximated, using Doom's ye olde
  // finetangent table; same method as monster aim)
  mo->momz += FixedMul(mo->info->speed, DegToSlope(pitch));

  // adjust position
  an = (actor->angle - ANG90) >> ANGLETOFINESHIFT;
  mo->x += FixedMul(spawnofs_xy, finecosine[an]);
  mo->y += FixedMul(spawnofs_xy, finesine[an]);
  mo->z += spawnofs_z;

  // always set the 'tracer' field, so this pointer
  // can be used to fire seeker missiles at will.
  P_SetTarget(&mo->tracer, actor->target);
}

//
// A_MonsterBulletAttack
// A parameterized monster bullet attack.
//   args[0]: Horizontal spread (degrees, in fixed point)
//   args[1]: Vertical spread (degrees, in fixed point)
//   args[2]: Number of bullets to fire; if not set, defaults to 1
//   args[3]: Base damage of attack (e.g. for 3d5, customize the 3); if not set, defaults to 3
//   args[4]: Attack damage modulus (e.g. for 3d5, customize the 5); if not set, defaults to 5
//
void A_MonsterBulletAttack(mobj_t *actor)
{
  int hspread, vspread, numbullets, damagebase, damagemod;
  int aimslope, i, damage, angle, slope;

  if (!mbf21 || !actor->target)
    return;

  hspread    = actor->state->args[0];
  vspread    = actor->state->args[1];
  numbullets = actor->state->args[2];
  damagebase = actor->state->args[3];
  damagemod  = actor->state->args[4];

  A_FaceTarget(actor);
  S_StartSound(actor, actor->info->attacksound);

  aimslope = P_AimLineAttack(actor, actor->angle, MISSILERANGE, 0);

  for (i = 0; i < numbullets; i++)
  {
    damage = (P_Random(pr_mbf21) % damagemod + 1) * damagebase;
    angle = (int)actor->angle + P_RandomHitscanAngle(pr_mbf21, hspread);
    slope = aimslope + P_RandomHitscanSlope(pr_mbf21, vspread);

    P_LineAttack(actor, angle, MISSILERANGE, slope, damage);
  }
}

//
// A_MonsterMeleeAttack
// A parameterized monster melee attack.
//   args[0]: Base damage of attack (e.g. for 3d8, customize the 3); if not set, defaults to 3
//   args[1]: Attack damage modulus (e.g. for 3d8, customize the 8); if not set, defaults to 8
//   args[2]: Sound to play if attack hits
//   args[3]: Range (fixed point); if not set, defaults to monster's melee range
//
void A_MonsterMeleeAttack(mobj_t *actor)
{
  int damagebase, damagemod, hitsound, range;
  int damage;

  if (!mbf21 || !actor->target)
    return;

  damagebase = actor->state->args[0];
  damagemod  = actor->state->args[1];
  hitsound   = actor->state->args[2];
  range      = actor->state->args[3];

  if (range == 0)
    range = actor->info->meleerange;

  range += actor->target->info->radius - 20 * FRACUNIT;

  A_FaceTarget(actor);
  if (!P_CheckRange(actor, range))
    return;

  S_StartSound(actor, hitsound);

  damage = (P_Random(pr_mbf21) % damagemod + 1) * damagebase;
  P_DamageMobj(actor->target, actor, actor, damage);
}

//
// A_RadiusDamage
// A parameterized version of A_Explode. Friggin' finally. :P
//   args[0]: Damage (int)
//   args[1]: Radius (also int; no real need for fractional precision here)
//
void A_RadiusDamage(mobj_t *actor)
{
  if (!mbf21 || !actor->state)
    return;

  P_RadiusAttack(actor, actor->target, actor->state->args[0], actor->state->args[1]);
}

//
// A_NoiseAlert
// Alerts nearby monsters (via sound) to the calling actor's target's presence.
//
void A_NoiseAlert(mobj_t *actor)
{
  if (!mbf21 || !actor->target)
    return;

  P_NoiseAlert(actor->target, actor);
}

//
// A_HealChase
// A parameterized version of A_VileChase.
//   args[0]: State to jump to on the calling actor when resurrecting a corpse
//   args[1]: Sound to play when resurrecting a corpse
//
void A_HealChase(mobj_t* actor)
{
  int state, sound;

  if (!mbf21 || !actor)
    return;

  state = actor->state->args[0];
  sound = actor->state->args[1];

  if (!P_HealCorpse(actor, actor->info->radius, state, sound))
    A_Chase(actor);
}

//
// A_SeekTracer
// A parameterized seeker missile function.
//   args[0]: direct-homing threshold angle (degrees, in fixed point)
//   args[1]: maximum turn angle (degrees, in fixed point)
//
void A_SeekTracer(mobj_t *actor)
{
  angle_t threshold, maxturnangle;

  if (!mbf21 || !actor)
    return;

  threshold    = FixedToAngle(actor->state->args[0]);
  maxturnangle = FixedToAngle(actor->state->args[1]);

  P_SeekerMissile(actor, &actor->tracer, threshold, maxturnangle, true);
}

//
// A_FindTracer
// Search for a valid tracer (seek target), if the calling actor doesn't already have one.
//   args[0]: field-of-view to search in (degrees, in fixed point); if zero, will search in all directions
//   args[1]: distance to search (map blocks, i.e. 128 units)
//
void A_FindTracer(mobj_t *actor)
{
  angle_t fov;
  int dist;

  if (!mbf21 || !actor || actor->tracer)
    return;

  fov  = FixedToAngle(actor->state->args[0]);
  dist =             (actor->state->args[1]);

  actor->tracer = P_RoughTargetSearch(actor, fov, dist);
}

//
// A_ClearTracer
// Clear current tracer (seek target).
//
void A_ClearTracer(mobj_t *actor)
{
  if (!mbf21 || !actor)
    return;

  actor->tracer = NULL;
}

//
// A_JumpIfHealthBelow
// Jumps to a state if caller's health is below the specified threshold.
//   args[0]: State to jump to
//   args[1]: Health threshold
//
void A_JumpIfHealthBelow(mobj_t* actor)
{
  int state, health;

  if (!mbf21 || !actor)
    return;

  state  = actor->state->args[0];
  health = actor->state->args[1];

  if (actor->health < health)
    P_SetMobjState(actor, state);
}

//
// A_JumpIfTargetInSight
// Jumps to a state if caller's target is in line-of-sight.
//   args[0]: State to jump to
//   args[1]: Field-of-view to check (degrees, in fixed point); if zero, will check in all directions
//
void A_JumpIfTargetInSight(mobj_t* actor)
{
  int state;
  angle_t fov;

  if (!mbf21 || !actor || !actor->target)
    return;

  state =             (actor->state->args[0]);
  fov   = FixedToAngle(actor->state->args[1]);

  // Check FOV first since it's faster
  if (fov > 0 && !P_CheckFov(actor, actor->target, fov))
    return;

  if (P_CheckSight(actor, actor->target))
    P_SetMobjState(actor, state);
}

//
// A_JumpIfTargetCloser
// Jumps to a state if caller's target is closer than the specified distance.
//   args[0]: State to jump to
//   args[1]: Distance threshold
//
void A_JumpIfTargetCloser(mobj_t* actor)
{
  int state, distance;

  if (!mbf21 || !actor || !actor->target)
    return;

  state    = actor->state->args[0];
  distance = actor->state->args[1];

  if (distance > P_AproxDistance(actor->x - actor->target->x,
                                 actor->y - actor->target->y))
    P_SetMobjState(actor, state);
}

//
// A_JumpIfTracerInSight
// Jumps to a state if caller's tracer (seek target) is in line-of-sight.
//   args[0]: State to jump to
//   args[1]: Field-of-view to check (degrees, in fixed point); if zero, will check in all directions
//
void A_JumpIfTracerInSight(mobj_t* actor)
{
  angle_t fov;
  int state;

  if (!mbf21 || !actor || !actor->tracer)
    return;

  state =             (actor->state->args[0]);
  fov   = FixedToAngle(actor->state->args[1]);

  // Check FOV first since it's faster
  if (fov > 0 && !P_CheckFov(actor, actor->tracer, fov))
    return;

  if (P_CheckSight(actor, actor->tracer))
    P_SetMobjState(actor, state);
}

//
// A_JumpIfTracerCloser
// Jumps to a state if caller's tracer (seek target) is closer than the specified distance.
//   args[0]: State to jump to
//   args[1]: Distance threshold (fixed point)
//
void A_JumpIfTracerCloser(mobj_t* actor)
{
  int state, distance;

  if (!mbf21 || !actor || !actor->tracer)
    return;

  state    = actor->state->args[0];
  distance = actor->state->args[1];

  if (distance > P_AproxDistance(actor->x - actor->tracer->x,
                                 actor->y - actor->tracer->y))
    P_SetMobjState(actor, state);
}

//
// A_JumpIfFlagsSet
// Jumps to a state if caller has the specified thing flags set.
//   args[0]: State to jump to
//   args[1]: Standard Flag(s) to check
//   args[2]: MBF21 Flag(s) to check
//
void A_JumpIfFlagsSet(mobj_t* actor)
{
  int state;
  unsigned int flags, flags2;

  if (!mbf21 || !actor)
    return;

  state  = actor->state->args[0];
  flags  = actor->state->args[1];
  flags2 = actor->state->args[2];

  if ((actor->flags & flags) == flags &&
      (actor->flags2 & flags2) == flags2)
    P_SetMobjState(actor, state);
}

//
// A_AddFlags
// Adds the specified thing flags to the caller.
//   args[0]: Standard Flag(s) to add
//   args[1]: MBF21 Flag(s) to add
//
void A_AddFlags(mobj_t* actor)
{
  unsigned int flags, flags2;

  if (!mbf21 || !actor)
    return;

  flags  = actor->state->args[0];
  flags2 = actor->state->args[1];

  actor->flags  |= flags;
  actor->flags2 |= flags2;
}

//
// A_RemoveFlags
// Removes the specified thing flags from the caller.
//   args[0]: Flag(s) to remove
//   args[1]: MBF21 Flag(s) to remove
//
void A_RemoveFlags(mobj_t* actor)
{
  unsigned int flags, flags2;

  if (!mbf21 || !actor)
    return;

  flags  = actor->state->args[0];
  flags2 = actor->state->args[1];

  actor->flags  &= ~flags;
  actor->flags2 &= ~flags2;
}



// heretic

#include "heretic/def.h"

#define MAX_BOSS_SPOTS 8

typedef struct
{
    fixed_t x;
    fixed_t y;
    angle_t angle;
} BossSpot_t;

static int BossSpotCount;
static BossSpot_t BossSpots[MAX_BOSS_SPOTS];

void P_InitMonsters(void)
{
    BossSpotCount = 0;
}

void P_AddBossSpot(fixed_t x, fixed_t y, angle_t angle)
{
    if (BossSpotCount == MAX_BOSS_SPOTS)
    {
        I_Error("Too many boss spots.");
    }
    BossSpots[BossSpotCount].x = x;
    BossSpots[BossSpotCount].y = y;
    BossSpots[BossSpotCount].angle = angle;
    BossSpotCount++;
}

void A_DripBlood(mobj_t * actor)
{
    mobj_t *mo;
    int r1,r2;

    r1 = P_SubRandom();
    r2 = P_SubRandom();

    mo = P_SpawnMobj(actor->x + (r2 << 11),
                     actor->y + (r1 << 11), actor->z,
                     HERETIC_MT_BLOOD);
    mo->momx = P_SubRandom() << 10;
    mo->momy = P_SubRandom() << 10;
    mo->flags2 |= MF2_LOGRAV;
}

void A_KnightAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(3));
        S_StartSound(actor, heretic_sfx_kgtat2);
        return;
    }
    // Throw axe
    S_StartSound(actor, actor->info->attacksound);
    if (actor->type == HERETIC_MT_KNIGHTGHOST || P_Random(pr_heretic) < 40)
    {                           // Red axe
        P_SpawnMissile(actor, actor->target, HERETIC_MT_REDAXE);
        return;
    }
    // Green axe
    P_SpawnMissile(actor, actor->target, HERETIC_MT_KNIGHTAXE);
}

void A_ImpExplode(mobj_t * actor)
{
    mobj_t *mo;

    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_IMPCHUNK1);
    mo->momx = P_SubRandom() << 10;
    mo->momy = P_SubRandom() << 10;
    mo->momz = 9 * FRACUNIT;
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_IMPCHUNK2);
    mo->momx = P_SubRandom() << 10;
    mo->momy = P_SubRandom() << 10;
    mo->momz = 9 * FRACUNIT;
    if (actor->special1.i == 666)
    {                           // Extreme death crash
        P_SetMobjState(actor, HERETIC_S_IMP_XCRASH1);
    }
}

void A_BeastPuff(mobj_t * actor)
{
    if (P_Random(pr_heretic) > 64)
    {
        int r1,r2,r3;
        r1 = P_SubRandom();
        r2 = P_SubRandom();
        r3 = P_SubRandom();
        P_SpawnMobj(actor->x + (r3 << 10),
                    actor->y + (r2 << 10),
                    actor->z + (r1 << 10), HERETIC_MT_PUFFY);
    }
}

void A_ImpMeAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    S_StartSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, 5 + (P_Random(pr_heretic) & 7));
    }
}

void A_ImpMsAttack(mobj_t * actor)
{
    mobj_t *dest;
    angle_t an;
    int dist;

    if (!actor->target || P_Random(pr_heretic) > 64)
    {
        P_SetMobjState(actor, actor->info->seestate);
        return;
    }
    dest = actor->target;
    actor->flags |= MF_SKULLFLY;
    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(12 * FRACUNIT, finecosine[an]);
    actor->momy = FixedMul(12 * FRACUNIT, finesine[an]);
    dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
    dist = dist / (12 * FRACUNIT);
    if (dist < 1)
    {
        dist = 1;
    }
    actor->momz = (dest->z + (dest->height >> 1) - actor->z) / dist;
}

void A_ImpMsAttack2(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    S_StartSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, 5 + (P_Random(pr_heretic) & 7));
        return;
    }
    P_SpawnMissile(actor, actor->target, HERETIC_MT_IMPBALL);
}

void A_ImpDeath(mobj_t * actor)
{
    actor->flags &= ~MF_SOLID;
    actor->flags2 |= MF2_FOOTCLIP;
    if (actor->z <= actor->floorz)
    {
        P_SetMobjState(actor, HERETIC_S_IMP_CRASH1);
    }
}

void A_ImpXDeath1(mobj_t * actor)
{
    actor->flags &= ~MF_SOLID;
    actor->flags |= MF_NOGRAVITY;
    actor->flags2 |= MF2_FOOTCLIP;
    actor->special1.i = 666;      // Flag the crash routine
}

void A_ImpXDeath2(mobj_t * actor)
{
    actor->flags &= ~MF_NOGRAVITY;
    if (actor->z <= actor->floorz)
    {
        P_SetMobjState(actor, HERETIC_S_IMP_CRASH1);
    }
}

dboolean P_UpdateChicken(mobj_t * actor, int tics)
{
    mobj_t *fog;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    mobjtype_t moType;
    mobj_t *mo;
    mobj_t oldChicken;

    actor->special1.i -= tics;
    if (actor->special1.i > 0)
    {
        return (false);
    }
    moType = actor->special2.i;
    x = actor->x;
    y = actor->y;
    z = actor->z;
    oldChicken = *actor;
    P_SetMobjState(actor, HERETIC_S_FREETARGMOBJ);
    mo = P_SpawnMobj(x, y, z, moType);
    if (P_TestMobjLocation(mo) == false)
    {                           // Didn't fit
        P_RemoveMobj(mo);
        mo = P_SpawnMobj(x, y, z, HERETIC_MT_CHICKEN);
        mo->angle = oldChicken.angle;
        mo->flags = oldChicken.flags;
        mo->health = oldChicken.health;
        P_SetTarget(&mo->target, oldChicken.target);
        mo->special1.i = 5 * 35;  // Next try in 5 seconds
        mo->special2.i = moType;
        return (false);
    }
    mo->angle = oldChicken.angle;
    P_SetTarget(&mo->target, oldChicken.target);
    fog = P_SpawnMobj(x, y, z + TELEFOGHEIGHT, HERETIC_MT_TFOG);
    S_StartSound(fog, heretic_sfx_telept);
    return (true);
}

void A_ChicAttack(mobj_t * actor)
{
    if (P_UpdateChicken(actor, 18))
    {
        return;
    }
    if (!actor->target)
    {
        return;
    }
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, 1 + (P_Random(pr_heretic) & 1));
    }
}

void A_ChicLook(mobj_t * actor)
{
    if (P_UpdateChicken(actor, 10))
    {
        return;
    }
    A_Look(actor);
}

void A_ChicChase(mobj_t * actor)
{
    if (P_UpdateChicken(actor, 3))
    {
        return;
    }
    A_Chase(actor);
}

void A_ChicPain(mobj_t * actor)
{
    if (P_UpdateChicken(actor, 10))
    {
        return;
    }
    S_StartSound(actor, actor->info->painsound);
}

void A_Feathers(mobj_t * actor)
{
    int i;
    int count;
    mobj_t *mo;

    if (actor->health > 0)
    {                           // Pain
        count = P_Random(pr_heretic) < 32 ? 2 : 1;
    }
    else
    {                           // Death
        count = 5 + (P_Random(pr_heretic) & 3);
    }
    for (i = 0; i < count; i++)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z + 20 * FRACUNIT,
                         HERETIC_MT_FEATHER);
        P_SetTarget(&mo->target, actor);
        mo->momx = P_SubRandom() << 8;
        mo->momy = P_SubRandom() << 8;
        mo->momz = FRACUNIT + (P_Random(pr_heretic) << 9);
        P_SetMobjState(mo, HERETIC_S_FEATHER1 + (P_Random(pr_heretic) & 7));
    }
}

void A_MummyAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    S_StartSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(2));
        S_StartSound(actor, heretic_sfx_mumat2);
        return;
    }
    S_StartSound(actor, heretic_sfx_mumat1);
}

void A_MummyAttack2(mobj_t * actor)
{
    mobj_t *mo;

    if (!actor->target)
    {
        return;
    }

    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(2));
        return;
    }
    mo = P_SpawnMissile(actor, actor->target, HERETIC_MT_MUMMYFX1);

    if (mo != NULL)
    {
        mo->special1.m = actor->target;
    }
}

void A_MummyFX1Seek(mobj_t * actor)
{
    P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 10, ANG1_X * 20, false);
}

void A_MummySoul(mobj_t * mummy)
{
    mobj_t *mo;

    mo = P_SpawnMobj(mummy->x, mummy->y, mummy->z + 10 * FRACUNIT,
                     HERETIC_MT_MUMMYSOUL);
    mo->momz = FRACUNIT;
}

void A_Sor1Pain(mobj_t * actor)
{
    actor->special1.i = 20;       // Number of steps to walk fast
    A_Pain(actor);
}

void A_Sor1Chase(mobj_t * actor)
{
    if (actor->special1.i)
    {
        actor->special1.i--;
        actor->tics -= 3;
    }
    A_Chase(actor);
}

void A_Srcr1Attack(mobj_t * actor)
{
    mobj_t *mo;
    fixed_t momz;
    angle_t angle;

    if (!actor->target)
    {
        return;
    }
    S_StartSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(8));
        return;
    }
    if (actor->health > (actor->info->spawnhealth / 3) * 2)
    {                           // Spit one fireball
        P_SpawnMissile(actor, actor->target, HERETIC_MT_SRCRFX1);
    }
    else
    {                           // Spit three fireballs
        mo = P_SpawnMissile(actor, actor->target, HERETIC_MT_SRCRFX1);
        if (mo)
        {
            momz = mo->momz;
            angle = mo->angle;
            P_SpawnMissileAngle(actor, HERETIC_MT_SRCRFX1, angle - ANG1_X * 3, momz);
            P_SpawnMissileAngle(actor, HERETIC_MT_SRCRFX1, angle + ANG1_X * 3, momz);
        }
        if (actor->health < actor->info->spawnhealth / 3)
        {                       // Maybe attack again
            if (actor->special1.i)
            {                   // Just attacked, so don't attack again
                actor->special1.i = 0;
            }
            else
            {                   // Set state to attack again
                actor->special1.i = 1;
                P_SetMobjState(actor, HERETIC_S_SRCR1_ATK4);
            }
        }
    }
}

void A_SorcererRise(mobj_t * actor)
{
    mobj_t *mo;

    actor->flags &= ~MF_SOLID;
    mo = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_SORCERER2);
    P_SetMobjState(mo, HERETIC_S_SOR2_RISE1);
    mo->angle = actor->angle;
    P_SetTarget(&mo->target, actor->target);
}

void P_DSparilTeleport(mobj_t * actor)
{
    int i;
    fixed_t x;
    fixed_t y;
    fixed_t prevX;
    fixed_t prevY;
    fixed_t prevZ;
    mobj_t *mo;

    if (!BossSpotCount)
    {                           // No spots
        return;
    }
    i = P_Random(pr_heretic);
    do
    {
        i++;
        x = BossSpots[i % BossSpotCount].x;
        y = BossSpots[i % BossSpotCount].y;
    }
    while (P_AproxDistance(actor->x - x, actor->y - y) < 128 * FRACUNIT);
    prevX = actor->x;
    prevY = actor->y;
    prevZ = actor->z;
    if (P_TeleportMove(actor, x, y, false))
    {
        mo = P_SpawnMobj(prevX, prevY, prevZ, HERETIC_MT_SOR2TELEFADE);
        S_StartSound(mo, heretic_sfx_telept);
        P_SetMobjState(actor, HERETIC_S_SOR2_TELE1);
        S_StartSound(actor, heretic_sfx_telept);
        actor->z = actor->floorz;
        actor->angle = BossSpots[i % BossSpotCount].angle;
        actor->momx = actor->momy = actor->momz = 0;
    }
}


void A_Srcr2Decide(mobj_t * actor)
{
    static int chance[] = {
        192, 120, 120, 120, 64, 64, 32, 16, 0
    };

    if (!BossSpotCount)
    {                           // No spots
        return;
    }
    if (P_Random(pr_heretic) < chance[actor->health / (actor->info->spawnhealth / 8)])
    {
        P_DSparilTeleport(actor);
    }
}

void A_Srcr2Attack(mobj_t * actor)
{
    int chance;

    if (!actor->target)
    {
        return;
    }
    S_StartSound(NULL, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(20));
        return;
    }
    chance = actor->health < actor->info->spawnhealth / 2 ? 96 : 48;
    if (P_Random(pr_heretic) < chance)
    {                           // Wizard spawners
        P_SpawnMissileAngle(actor, HERETIC_MT_SOR2FX2,
                            actor->angle - ANG45, FRACUNIT / 2);
        P_SpawnMissileAngle(actor, HERETIC_MT_SOR2FX2,
                            actor->angle + ANG45, FRACUNIT / 2);
    }
    else
    {                           // Blue bolt
        P_SpawnMissile(actor, actor->target, HERETIC_MT_SOR2FX1);
    }
}

void A_BlueSpark(mobj_t * actor)
{
    int i;
    mobj_t *mo;

    for (i = 0; i < 2; i++)
    {
        mo = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_SOR2FXSPARK);
        mo->momx = P_SubRandom() << 9;
        mo->momy = P_SubRandom() << 9;
        mo->momz = FRACUNIT + (P_Random(pr_heretic) << 8);
    }
}

void A_GenWizard(mobj_t * actor)
{
    mobj_t *mo;
    mobj_t *fog;

    mo = P_SpawnMobj(actor->x, actor->y,
                     actor->z - mobjinfo[HERETIC_MT_WIZARD].height / 2, HERETIC_MT_WIZARD);
    if (P_TestMobjLocation(mo) == false)
    {                           // Didn't fit
        P_RemoveMobj(mo);
        return;
    }
    actor->momx = actor->momy = actor->momz = 0;
    P_SetMobjState(actor, mobjinfo[actor->type].deathstate);
    actor->flags &= ~MF_MISSILE;
    fog = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_TFOG);
    S_StartSound(fog, heretic_sfx_telept);
}

void P_Massacre(void)
{
    mobj_t *mo;
    thinker_t *think;

    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != P_MobjThinker)
        {                       // Not a mobj thinker
            continue;
        }
        mo = (mobj_t *) think;
        if ((mo->flags & MF_COUNTKILL) && (mo->health > 0))
        {
            P_DamageMobj(mo, NULL, NULL, 10000);
        }
    }
}

void A_Sor2DthInit(mobj_t * actor)
{
    actor->special1.i = 7;        // Animation loop counter
    P_Massacre();               // Kill monsters early
}

void A_Sor2DthLoop(mobj_t * actor)
{
    if (--actor->special1.i)
    {                           // Need to loop
        P_SetMobjState(actor, HERETIC_S_SOR2_DIE4);
    }
}

void A_SorZap(mobj_t * actor)
{
    S_StartSound(NULL, heretic_sfx_sorzap);
}

void A_SorRise(mobj_t * actor)
{
    S_StartSound(NULL, heretic_sfx_sorrise);
}

void A_SorDSph(mobj_t * actor)
{
    S_StartSound(NULL, heretic_sfx_sordsph);
}

void A_SorDExp(mobj_t * actor)
{
    S_StartSound(NULL, heretic_sfx_sordexp);
}

void A_SorDBon(mobj_t * actor)
{
    S_StartSound(NULL, heretic_sfx_sordbon);
}

void A_SorSightSnd(mobj_t * actor)
{
    S_StartSound(NULL, heretic_sfx_sorsit);
}

void A_MinotaurAtk1(mobj_t * actor)
{
    player_t *player;

    if (!actor->target)
    {
        return;
    }
    S_StartSound(actor, heretic_sfx_stfpow);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(4));
        if ((player = actor->target->player) != NULL)
        {                       // Squish the player
            player->deltaviewheight = -16 * FRACUNIT;
        }
    }
}

#define MNTR_CHARGE_SPEED (13*FRACUNIT)

void A_MinotaurDecide(mobj_t * actor)
{
    angle_t angle;
    mobj_t *target;
    int dist;

    target = actor->target;
    if (!target)
    {
        return;
    }
    S_StartSound(actor, heretic_sfx_minsit);
    dist = P_AproxDistance(actor->x - target->x, actor->y - target->y);
    if (target->z + target->height > actor->z
        && target->z + target->height < actor->z + actor->height
        && dist < 8 * 64 * FRACUNIT
        && dist > 1 * 64 * FRACUNIT && P_Random(pr_heretic) < 150)
    {                           // Charge attack
        // Don't call the state function right away
        P_SetMobjStateNF(actor, HERETIC_S_MNTR_ATK4_1);
        actor->flags |= MF_SKULLFLY;
        A_FaceTarget(actor);
        angle = actor->angle >> ANGLETOFINESHIFT;
        actor->momx = FixedMul(MNTR_CHARGE_SPEED, finecosine[angle]);
        actor->momy = FixedMul(MNTR_CHARGE_SPEED, finesine[angle]);
        actor->special1.i = 35 / 2;       // Charge duration
    }
    else if (target->z == target->floorz
             && dist < 9 * 64 * FRACUNIT && P_Random(pr_heretic) < 220)
    {                           // Floor fire attack
        P_SetMobjState(actor, HERETIC_S_MNTR_ATK3_1);
        actor->special2.i = 0;
    }
    else
    {                           // Swing attack
        A_FaceTarget(actor);
        // Don't need to call P_SetMobjState because the current state
        // falls through to the swing attack
    }
}

void A_MinotaurCharge(mobj_t * actor)
{
    mobj_t *puff;

    if (actor->special1.i)
    {
        puff = P_SpawnMobj(actor->x, actor->y, actor->z, HERETIC_MT_PHOENIXPUFF);
        puff->momz = 2 * FRACUNIT;
        actor->special1.i--;
    }
    else
    {
        actor->flags &= ~MF_SKULLFLY;
        P_SetMobjState(actor, actor->info->seestate);
    }
}

void A_MinotaurAtk2(mobj_t * actor)
{
    mobj_t *mo;
    angle_t angle;
    fixed_t momz;

    if (!actor->target)
    {
        return;
    }
    S_StartSound(actor, heretic_sfx_minat2);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(5));
        return;
    }
    mo = P_SpawnMissile(actor, actor->target, HERETIC_MT_MNTRFX1);
    if (mo)
    {
        S_StartSound(mo, heretic_sfx_minat2);
        momz = mo->momz;
        angle = mo->angle;
        P_SpawnMissileAngle(actor, HERETIC_MT_MNTRFX1, angle - (ANG45 / 8), momz);
        P_SpawnMissileAngle(actor, HERETIC_MT_MNTRFX1, angle + (ANG45 / 8), momz);
        P_SpawnMissileAngle(actor, HERETIC_MT_MNTRFX1, angle - (ANG45 / 16), momz);
        P_SpawnMissileAngle(actor, HERETIC_MT_MNTRFX1, angle + (ANG45 / 16), momz);
    }
}

void A_MinotaurAtk3(mobj_t * actor)
{
    mobj_t *mo;
    player_t *player;

    if (!actor->target)
    {
        return;
    }
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(5));
        if ((player = actor->target->player) != NULL)
        {                       // Squish the player
            player->deltaviewheight = -16 * FRACUNIT;
        }
    }
    else
    {
        mo = P_SpawnMissile(actor, actor->target, HERETIC_MT_MNTRFX2);
        if (mo != NULL)
        {
            S_StartSound(mo, heretic_sfx_minat1);
        }
    }
    if (P_Random(pr_heretic) < 192 && actor->special2.i == 0)
    {
        P_SetMobjState(actor, HERETIC_S_MNTR_ATK3_4);
        actor->special2.i = 1;
    }
}

void A_MntrFloorFire(mobj_t * actor)
{
    mobj_t *mo;
    int r1, r2;

    r1 = P_SubRandom();
    r2 = P_SubRandom();

    actor->z = actor->floorz;
    mo = P_SpawnMobj(actor->x + (r2 << 10),
                     actor->y + (r1 << 10), ONFLOORZ,
                     HERETIC_MT_MNTRFX3);
    P_SetTarget(&mo->target, actor->target);
    mo->momx = 1;               // Force block checking
    P_CheckMissileSpawn(mo);
}

void A_BeastAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        return;
    }
    S_StartSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(3));
        return;
    }
    P_SpawnMissile(actor, actor->target, HERETIC_MT_BEASTBALL);
}

void A_WhirlwindSeek(mobj_t * actor)
{
    actor->health -= 3;
    if (actor->health < 0)
    {
        actor->momx = actor->momy = actor->momz = 0;
        P_SetMobjState(actor, mobjinfo[actor->type].deathstate);
        actor->flags &= ~MF_MISSILE;
        return;
    }
    if ((actor->special2.i -= 3) < 0)
    {
        actor->special2.i = 58 + (P_Random(pr_heretic) & 31);
        S_StartSound(actor, heretic_sfx_hedat3);
    }
    if (actor->special1.m
        && (((mobj_t *) (actor->special1.m))->flags & MF_SHADOW))
    {
        return;
    }
    P_SeekerMissile(actor, &actor->special1.m, ANG1_X * 10, ANG1_X * 30, false);
}

void A_HeadIceImpact(mobj_t * ice)
{
    unsigned int i;
    angle_t angle;
    mobj_t *shard;

    for (i = 0; i < 8; i++)
    {
        shard = P_SpawnMobj(ice->x, ice->y, ice->z, HERETIC_MT_HEADFX2);
        angle = i * ANG45;
        P_SetTarget(&shard->target, ice->target);
        shard->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        shard->momx = FixedMul(shard->info->speed, finecosine[angle]);
        shard->momy = FixedMul(shard->info->speed, finesine[angle]);
        shard->momz = (fixed_t)(-.6 * FRACUNIT);
        P_CheckMissileSpawn(shard);
    }
}

void A_HeadFireGrow(mobj_t * fire)
{
    fire->health--;
    fire->z += 9 * FRACUNIT;
    if (fire->health == 0)
    {
        fire->damage = fire->info->damage;
        P_SetMobjState(fire, HERETIC_S_HEADFX3_4);
    }
}

void A_SnakeAttack(mobj_t * actor)
{
    if (!actor->target)
    {
        P_SetMobjState(actor, HERETIC_S_SNAKE_WALK1);
        return;
    }
    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    P_SpawnMissile(actor, actor->target, HERETIC_MT_SNAKEPRO_A);
}

void A_SnakeAttack2(mobj_t * actor)
{
    if (!actor->target)
    {
        P_SetMobjState(actor, HERETIC_S_SNAKE_WALK1);
        return;
    }
    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);
    P_SpawnMissile(actor, actor->target, HERETIC_MT_SNAKEPRO_B);
}

void A_ClinkAttack(mobj_t * actor)
{
    int damage;

    if (!actor->target)
    {
        return;
    }
    S_StartSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        damage = ((P_Random(pr_heretic) % 7) + 3);
        P_DamageMobj(actor->target, actor, actor, damage);
    }
}

void A_GhostOff(mobj_t * actor)
{
    actor->flags &= ~MF_SHADOW;
}

void A_WizAtk1(mobj_t * actor)
{
    A_FaceTarget(actor);
    actor->flags &= ~MF_SHADOW;
}

void A_WizAtk2(mobj_t * actor)
{
    A_FaceTarget(actor);
    actor->flags |= MF_SHADOW;
}

void A_WizAtk3(mobj_t * actor)
{
    mobj_t *mo;
    angle_t angle;
    fixed_t momz;

    actor->flags &= ~MF_SHADOW;
    if (!actor->target)
    {
        return;
    }
    S_StartSound(actor, actor->info->attacksound);
    if (P_CheckMeleeRange(actor))
    {
        P_DamageMobj(actor->target, actor, actor, HITDICE(4));
        return;
    }
    mo = P_SpawnMissile(actor, actor->target, HERETIC_MT_WIZFX1);
    if (mo)
    {
        momz = mo->momz;
        angle = mo->angle;
        P_SpawnMissileAngle(actor, HERETIC_MT_WIZFX1, angle - (ANG45 / 8), momz);
        P_SpawnMissileAngle(actor, HERETIC_MT_WIZFX1, angle + (ANG45 / 8), momz);
    }
}

void P_DropItem(mobj_t * source, mobjtype_t type, int special, int chance)
{
    mobj_t *mo;

    if (P_Random(pr_heretic) > chance)
    {
        return;
    }
    mo = P_SpawnMobj(source->x, source->y,
                     source->z + (source->height >> 1), type);
    mo->momx = P_SubRandom() << 8;
    mo->momy = P_SubRandom() << 8;
    mo->momz = FRACUNIT * 5 + (P_Random(pr_heretic) << 10);
    mo->flags |= MF_DROPPED;
    mo->health = special;
}

void A_NoBlocking(mobj_t * actor)
{
    actor->flags &= ~MF_SOLID;
    // Check for monsters dropping things
    switch (actor->type)
    {
        case HERETIC_MT_MUMMY:
        case HERETIC_MT_MUMMYLEADER:
        case HERETIC_MT_MUMMYGHOST:
        case HERETIC_MT_MUMMYLEADERGHOST:
            P_DropItem(actor, HERETIC_MT_AMGWNDWIMPY, 3, 84);
            break;
        case HERETIC_MT_KNIGHT:
        case HERETIC_MT_KNIGHTGHOST:
            P_DropItem(actor, HERETIC_MT_AMCBOWWIMPY, 5, 84);
            break;
        case HERETIC_MT_WIZARD:
            P_DropItem(actor, HERETIC_MT_AMBLSRWIMPY, 10, 84);
            P_DropItem(actor, HERETIC_MT_ARTITOMEOFPOWER, 0, 4);
            break;
        case HERETIC_MT_HEAD:
            P_DropItem(actor, HERETIC_MT_AMBLSRWIMPY, 10, 84);
            P_DropItem(actor, HERETIC_MT_ARTIEGG, 0, 51);
            break;
        case HERETIC_MT_BEAST:
            P_DropItem(actor, HERETIC_MT_AMCBOWWIMPY, 10, 84);
            break;
        case HERETIC_MT_CLINK:
            P_DropItem(actor, HERETIC_MT_AMSKRDWIMPY, 20, 84);
            break;
        case HERETIC_MT_SNAKE:
            P_DropItem(actor, HERETIC_MT_AMPHRDWIMPY, 5, 84);
            break;
        case HERETIC_MT_MINOTAUR:
            P_DropItem(actor, HERETIC_MT_ARTISUPERHEAL, 0, 51);
            P_DropItem(actor, HERETIC_MT_AMPHRDWIMPY, 10, 84);
            break;
        default:
            break;
    }
}

void A_PodPain(mobj_t * actor)
{
    int i;
    int count;
    int chance;
    mobj_t *goo;

    chance = P_Random(pr_heretic);
    if (chance < 128)
    {
        return;
    }
    count = chance > 240 ? 2 : 1;
    for (i = 0; i < count; i++)
    {
        goo = P_SpawnMobj(actor->x, actor->y,
                          actor->z + 48 * FRACUNIT, HERETIC_MT_PODGOO);
        P_SetTarget(&goo->target, actor);
        goo->momx = P_SubRandom() << 9;
        goo->momy = P_SubRandom() << 9;
        goo->momz = FRACUNIT / 2 + (P_Random(pr_heretic) << 9);
    }
}

void A_RemovePod(mobj_t * actor)
{
    mobj_t *mo;

    if (actor->special2.m)
    {
        mo = (mobj_t *) actor->special2.m;
        if (mo->special1.i > 0)
        {
            mo->special1.i--;
        }
    }
}

#define MAX_GEN_PODS 16

void A_MakePod(mobj_t * actor)
{
    mobj_t *mo;
    fixed_t x;
    fixed_t y;

    if (actor->special1.i == MAX_GEN_PODS)
    {                           // Too many generated pods
        return;
    }
    x = actor->x;
    y = actor->y;
    mo = P_SpawnMobj(x, y, ONFLOORZ, HERETIC_MT_POD);
    if (P_CheckPosition(mo, x, y) == false)
    {                           // Didn't fit
        P_RemoveMobj(mo);
        return;
    }
    P_SetMobjState(mo, HERETIC_S_POD_GROW1);
    P_ThrustMobj(mo, P_Random(pr_heretic) << 24, (fixed_t) (4.5 * FRACUNIT));
    S_StartSound(mo, heretic_sfx_newpod);
    actor->special1.i++;          // Increment generated pod count
    mo->special2.m = actor;       // Link the generator to the pod
    return;
}

void A_ESound(mobj_t * mo)
{
    int sound = heretic_sfx_None;

    switch (mo->type)
    {
        case HERETIC_MT_SOUNDWATERFALL:
            sound = heretic_sfx_waterfl;
            break;
        case HERETIC_MT_SOUNDWIND:
            sound = heretic_sfx_wind;
            break;
        default:
            break;
    }
    S_StartSound(mo, sound);
}

void A_SpawnTeleGlitter(mobj_t * actor)
{
    mobj_t *mo;
    int r1, r2;

    r1 = P_Random(pr_heretic);
    r2 = P_Random(pr_heretic);
    mo = P_SpawnMobj(actor->x + ((r2 & 31) - 16) * FRACUNIT,
                     actor->y + ((r1 & 31) - 16) * FRACUNIT,
                     actor->subsector->sector->floorheight, HERETIC_MT_TELEGLITTER);
    mo->momz = FRACUNIT / 4;
}

void A_SpawnTeleGlitter2(mobj_t * actor)
{
    mobj_t *mo;
    int r1, r2;

    r1 = P_Random(pr_heretic);
    r2 = P_Random(pr_heretic);
    mo = P_SpawnMobj(actor->x + ((r2 & 31) - 16) * FRACUNIT,
                     actor->y + ((r1 & 31) - 16) * FRACUNIT,
                     actor->subsector->sector->floorheight, HERETIC_MT_TELEGLITTER2);
    mo->momz = FRACUNIT / 4;
}

void A_AccTeleGlitter(mobj_t * actor)
{
    if (++actor->health > 35)
    {
        actor->momz += actor->momz / 2;
    }
}

void A_InitKeyGizmo(mobj_t * gizmo)
{
    mobj_t *mo;
    statenum_t state = g_s_null;

    switch (gizmo->type)
    {
        case HERETIC_MT_KEYGIZMOBLUE:
            state = HERETIC_S_KGZ_BLUEFLOAT1;
            break;
        case HERETIC_MT_KEYGIZMOGREEN:
            state = HERETIC_S_KGZ_GREENFLOAT1;
            break;
        case HERETIC_MT_KEYGIZMOYELLOW:
            state = HERETIC_S_KGZ_YELLOWFLOAT1;
            break;
        default:
            break;
    }
    mo = P_SpawnMobj(gizmo->x, gizmo->y, gizmo->z + 60 * FRACUNIT,
                     HERETIC_MT_KEYGIZMOFLOAT);
    P_SetMobjState(mo, state);
}

void A_VolcanoSet(mobj_t * volcano)
{
    volcano->tics = 105 + (P_Random(pr_heretic) & 127);
}

void A_VolcanoBlast(mobj_t * volcano)
{
    int i;
    int count;
    mobj_t *blast;
    angle_t angle;

    count = 1 + (P_Random(pr_heretic) % 3);
    for (i = 0; i < count; i++)
    {
        blast = P_SpawnMobj(volcano->x, volcano->y, volcano->z + 44 * FRACUNIT, HERETIC_MT_VOLCANOBLAST);
        P_SetTarget(&blast->target, volcano);
        angle = P_Random(pr_heretic) << 24;
        blast->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        blast->momx = FixedMul(1 * FRACUNIT, finecosine[angle]);
        blast->momy = FixedMul(1 * FRACUNIT, finesine[angle]);
        blast->momz = (fixed_t)(2.5 * FRACUNIT) + (P_Random(pr_heretic) << 10);
        S_StartSound(blast, heretic_sfx_volsht);
        P_CheckMissileSpawn(blast);
    }
}

void A_VolcBallImpact(mobj_t * ball)
{
    unsigned int i;
    mobj_t *tiny;
    angle_t angle;

    if (ball->z <= ball->floorz)
    {
        ball->flags |= MF_NOGRAVITY;
        ball->flags2 &= ~MF2_LOGRAV;
        ball->z += 28 * FRACUNIT;
        //ball->momz = 3*FRACUNIT;
    }
    P_RadiusAttack(ball, ball->target, 25, 25);
    for (i = 0; i < 4; i++)
    {
        tiny = P_SpawnMobj(ball->x, ball->y, ball->z, HERETIC_MT_VOLCANOTBLAST);
        P_SetTarget(&tiny->target, ball);
        angle = i * ANG90;
        tiny->angle = angle;
        angle >>= ANGLETOFINESHIFT;
        tiny->momx = FixedMul((fixed_t)(FRACUNIT * .7), finecosine[angle]);
        tiny->momy = FixedMul((fixed_t)(FRACUNIT * .7), finesine[angle]);
        tiny->momz = FRACUNIT + (P_Random(pr_heretic) << 9);
        P_CheckMissileSpawn(tiny);
    }
}

void A_CheckSkullFloor(mobj_t * actor)
{
    if (actor->z <= actor->floorz)
    {
        P_SetMobjState(actor, HERETIC_S_BLOODYSKULLX1);
    }
}

void A_CheckSkullDone(mobj_t * actor)
{
    if (actor->special2.i == 666)
    {
        P_SetMobjState(actor, HERETIC_S_BLOODYSKULLX2);
    }
}

void A_CheckBurnGone(mobj_t * actor)
{
    if (actor->special2.i == 666)
    {
        P_SetMobjState(actor, HERETIC_S_PLAY_FDTH20);
    }
}

void A_FreeTargMobj(mobj_t * mo)
{
    mo->momx = mo->momy = mo->momz = 0;
    mo->z = mo->ceilingz + 4 * FRACUNIT;
    mo->flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY | MF_SOLID);
    mo->flags |= MF_CORPSE | MF_DROPOFF | MF_NOGRAVITY;
    mo->flags2 &= ~(MF2_PASSMOBJ | MF2_LOGRAV);
    mo->player = NULL;
}

extern int bodyqueslot, bodyquesize;
extern mobj_t** bodyque;

void A_AddPlayerCorpse(mobj_t * actor)
{
    if (bodyquesize > 0)
    {
      static int queuesize;
      if (queuesize < bodyquesize)
    	{
    	  bodyque = realloc(bodyque, bodyquesize * sizeof(*bodyque));
    	  memset(bodyque+queuesize, 0, (bodyquesize - queuesize) * sizeof(*bodyque));
    	  queuesize = bodyquesize;
    	}
      if (bodyqueslot >= bodyquesize)
    	  P_RemoveMobj(bodyque[bodyqueslot % bodyquesize]);
      bodyque[bodyqueslot++ % bodyquesize] = actor;
    }
}

void A_FlameSnd(mobj_t * actor)
{
    S_StartSound(actor, heretic_sfx_hedat1);    // Burn sound
}

void A_HideThing(mobj_t * actor)
{
    //P_UnsetThingPosition(actor);
    actor->flags2 |= MF2_DONTDRAW;
}

void A_UnHideThing(mobj_t * actor)
{
    //P_SetThingPosition(actor);
    actor->flags2 &= ~MF2_DONTDRAW;
}

void Heretic_A_Scream(mobj_t * actor)
{
    switch (actor->type)
    {
        case HERETIC_MT_CHICPLAYER:
        case HERETIC_MT_SORCERER1:
        case HERETIC_MT_MINOTAUR:
            // Make boss death sounds full volume
            S_StartSound(NULL, actor->info->deathsound);
            break;
        case HERETIC_MT_PLAYER:
            // Handle the different player death screams
            if (actor->special1.i < 10)
            {                   // Wimpy death sound
                S_StartSound(actor, heretic_sfx_plrwdth);
            }
            else if (actor->health > -50)
            {                   // Normal death sound
                S_StartSound(actor, actor->info->deathsound);
            }
            else if (actor->health > -100)
            {                   // Crazy death sound
                S_StartSound(actor, heretic_sfx_plrcdth);
            }
            else
            {                   // Extreme death sound
                S_StartSound(actor, heretic_sfx_gibdth);
            }
            break;
        default:
            S_StartSound(actor, actor->info->deathsound);
            break;
    }
}

void Heretic_A_BossDeath(mobj_t * actor)
{
    mobj_t *mo;
    thinker_t *think;
    line_t dummyLine;
    static mobjtype_t bossType[6] = {
        HERETIC_MT_HEAD,
        HERETIC_MT_MINOTAUR,
        HERETIC_MT_SORCERER2,
        HERETIC_MT_HEAD,
        HERETIC_MT_MINOTAUR,
        -1
    };

    if (gamemap != 8)
    {                           // Not a boss level
        return;
    }
    if (actor->type != bossType[gameepisode - 1])
    {                           // Not considered a boss in this episode
        return;
    }
    // Make sure all bosses are dead
    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != P_MobjThinker)
        {                       // Not a mobj thinker
            continue;
        }
        mo = (mobj_t *) think;
        if ((mo != actor) && (mo->type == actor->type) && (mo->health > 0))
        {                       // Found a living boss
            return;
        }
    }
    if (gameepisode > 1)
    {                           // Kill any remaining monsters
        P_Massacre();
    }
    dummyLine.tag = 666;
    EV_DoFloor(&dummyLine, lowerFloor);
}

#define MONS_LOOK_RANGE (20*64*FRACUNIT)
#define MONS_LOOK_LIMIT 64

// Not proxied by P_LookForMonsters - this is post-death brawling
dboolean Heretic_P_LookForMonsters(mobj_t * actor)
{
    int count;
    mobj_t *mo;
    thinker_t *think;

    if (!P_CheckSight(players[0].mo, actor))
    {                           // Player can't see monster
        return (false);
    }
    count = 0;
    for (think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if (think->function != P_MobjThinker)
        {                       // Not a mobj thinker
            continue;
        }
        mo = (mobj_t *) think;
        if (!(mo->flags & MF_COUNTKILL) || (mo == actor) || (mo->health <= 0))
        {                       // Not a valid monster
            continue;
        }
        if (P_AproxDistance(actor->x - mo->x, actor->y - mo->y)
            > MONS_LOOK_RANGE)
        {                       // Out of range
            continue;
        }
        if (P_Random(pr_heretic) < 16)
        {                       // Skip
            continue;
        }
        if (count++ > MONS_LOOK_LIMIT)
        {                       // Stop searching
            return (false);
        }
        if (!P_CheckSight(actor, mo))
        {                       // Out of sight
            continue;
        }
        // Found a target monster
        P_SetTarget(&actor->target, mo);
        return (true);
    }
    return (false);
}

dboolean Heretic_P_LookForPlayers(mobj_t * actor, dboolean allaround)
{
    int c;
    int stop;
    player_t *player;
    angle_t an;
    fixed_t dist;

    if (!netgame && players[0].health <= 0)
    {                           // Single player game and player is dead, look for monsters
        return (Heretic_P_LookForMonsters(actor));
    }
    c = 0;
    stop = (actor->lastlook - 1) & 3;
    for (;; actor->lastlook = (actor->lastlook + 1) & 3)
    {
        if (!playeringame[actor->lastlook])
            continue;

        if (c++ == 2 || actor->lastlook == stop)
            return false;       // done looking

        player = &players[actor->lastlook];
        if (player->health <= 0)
            continue;           // dead
        if (!P_CheckSight(actor, player->mo))
            continue;           // out of sight

        if (!allaround)
        {
            an = R_PointToAngle2(actor->x, actor->y,
                                 player->mo->x, player->mo->y) - actor->angle;
            if (an > ANG90 && an < ANG270)
            {
                dist = P_AproxDistance(player->mo->x - actor->x,
                                       player->mo->y - actor->y);
                // if real close, react anyway
                if (dist > WAKEUPRANGE)
                    continue;   // behind back
            }
        }
        if (player->mo->flags & MF_SHADOW)
        {                       // Player is invisible
            if ((P_AproxDistance(player->mo->x - actor->x,
                                 player->mo->y - actor->y) > SNEAKRANGE)
                && P_AproxDistance(player->mo->momx, player->mo->momy)
                < 5 * FRACUNIT)
            {                   // Player is sneaking - can't detect
                return (false);
            }
            if (P_Random(pr_heretic) < 225)
            {                   // Player isn't sneaking, but still didn't detect
                return (false);
            }
        }
        P_SetTarget(&actor->target, player->mo);
        return (true);
    }
    return (false);
}
