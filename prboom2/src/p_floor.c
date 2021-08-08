/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
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
 *  General plane mover and floor mover action routines
 *  Floor motion, pure changer types, raising stairs. donuts, elevators
 *
 *-----------------------------------------------------------------------------*/

#include "doomstat.h"
#include "r_main.h"
#include "p_map.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "sounds.h"
#include "lprintf.h"
#include "g_overflow.h"
#include "e6y.h"//e6y

//e6y
#define STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE 10

///////////////////////////////////////////////////////////////////////
//
// Plane (floor or ceiling), Floor motion and Elevator action routines
//
///////////////////////////////////////////////////////////////////////

//
// T_MovePlane()
//
// Move a plane (floor or ceiling) and check for crushing. Called
// every tick by all actions that move floors or ceilings.
//
// Passed the sector to move a plane in, the speed to move it at,
// the dest height it is to achieve, whether it crushes obstacles,
// whether it moves a floor or ceiling, and the direction up or down
// to move.
//
// Returns a result_e:
//  ok - plane moved normally, has not achieved destination yet
//  pastdest - plane moved normally and is now at destination height
//  crushed - plane encountered an obstacle, is holding until removed
//
result_e T_MovePlane
( sector_t*     sector,
  fixed_t       speed,
  fixed_t       dest,
  dboolean       crush,
  int           floorOrCeiling,
  int           direction )
{
  dboolean       flag;
  fixed_t       lastpos;
  fixed_t       destheight; //jff 02/04/98 used to keep floors/ceilings
                            // from moving thru each other

#ifdef GL_DOOM
  if (V_IsOpenGLMode())
  {
    gld_UpdateSplitData(sector);
  }
#endif

  switch(floorOrCeiling)
  {
    case 0:
      // Moving a floor
      switch(direction)
      {
        case -1:
          // Moving a floor down
          if (sector->floorheight - speed < dest)
          {
            lastpos = sector->floorheight;
            sector->floorheight = dest;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
            if (flag == true)
            {
              sector->floorheight =lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->floorheight;
            sector->floorheight -= speed;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
            /* cph - make more compatible with original Doom, by
             *  reintroducing this code. This means floors can't lower
             *  if objects are stuck in the ceiling */
            if ((flag == true) && comp[comp_floors]) {
              sector->floorheight = lastpos;
              P_ChangeSector(sector,crush);
              return crushed;
            }
          }
          break;

        case 1:
          // Moving a floor up
          // jff 02/04/98 keep floor from moving thru ceilings
          // jff 2/22/98 weaken check to demo_compatibility
          destheight = (comp[comp_floors] || dest<sector->ceilingheight)?
                          dest : sector->ceilingheight;
          if (sector->floorheight + speed > destheight)
          {
            lastpos = sector->floorheight;
            sector->floorheight = destheight;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
            if (flag == true)
            {
              sector->floorheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->floorheight;
            sector->floorheight += speed;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
            if (flag == true)
            {
        /* jff 1/25/98 fix floor crusher */
              if (!hexen && comp[comp_floors]) {

                //e6y: warning about potential desynch
                if (crush == STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE)
                {
                  lprintf(LO_WARN, "T_MovePlane: Stairs which can potentially crush may lead to desynch in compatibility mode.\n");
                  lprintf(LO_WARN, " gametic: %d, sector: %d, complevel: %d\n", gametic, sector->iSectorID, compatibility_level);
                }

                if (crush == true)
                  return crushed;
              }
              sector->floorheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
              return crushed;
            }
          }
          break;
      }
      break;

    case 1:
      // moving a ceiling
      switch(direction)
      {
        case -1:
          // moving a ceiling down
          // jff 02/04/98 keep ceiling from moving thru floors
          // jff 2/22/98 weaken check to demo_compatibility
          destheight = (comp[comp_floors] || dest>sector->floorheight)?
                          dest : sector->floorheight;
          if (sector->ceilingheight - speed < destheight)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = destheight;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk

            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->ceilingheight;
            sector->ceilingheight -= speed;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk

            if (flag == true)
            {
              if (!hexen && crush == true)
                return crushed;
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
              return crushed;
            }
          }
          break;

        case 1:
          // moving a ceiling up
          if (sector->ceilingheight + speed > dest)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = dest;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);      //jff 3/19/98 use faster chk
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight += speed;
            flag = P_CheckSector(sector,crush); //jff 3/19/98 use faster chk
          }
          break;
      }
      break;
  }
  return ok;
}

//
// T_MoveFloor()
//
// Move a floor to it's destination (up or down).
// Called once per tick for each moving floor.
//
// Passed a floormove_t structure that contains all pertinent info about the
// move. See P_SPEC.H for fields.
// No return.
//
// jff 02/08/98 all cases with labels beginning with gen added to support
// generalized line type behaviors.

static void Hexen_T_MoveFloor(floormove_t * floor);

void T_MoveFloor(floormove_t* floor)
{
  result_e      res;

  if (hexen) return Hexen_T_MoveFloor(floor);

  res = T_MovePlane       // move the floor
  (
    floor->sector,
    floor->speed,
    floor->floordestheight,
    floor->crush,
    0,
    floor->direction
  );

  if (!(leveltime&7))     // make the floormove sound
    S_StartSound((mobj_t *)&floor->sector->soundorg, g_sfx_stnmov);

  if (res == pastdest)    // if destination height is reached
  {
    if (heretic && floor->type == buildStair)
    {
        S_StartSound(&floor->sector->soundorg, heretic_sfx_pstop);
    }
    if (floor->direction == 1)       // going up
    {
      switch(floor->type) // handle texture/type changes
      {
        case donutRaise:
          floor->sector->special = floor->newspecial;
          floor->sector->floorpic = floor->texture;
          break;
        case genFloorChgT:
        case genFloorChg0:
          floor->sector->special = floor->newspecial;
          //jff add to fix bug in special transfers from changes
          floor->sector->oldspecial = floor->oldspecial;
          //fall thru
        case genFloorChg:
          floor->sector->floorpic = floor->texture;
          break;
        default:
          break;
      }
    }
    else if (floor->direction == -1) // going down
    {
      switch(floor->type) // handle texture/type changes
      {
        case lowerAndChange:
          floor->sector->special = floor->newspecial;
          //jff add to fix bug in special transfers from changes
          floor->sector->oldspecial = floor->oldspecial; // heretic_note: here and elsewhere, not in heretic
          floor->sector->floorpic = floor->texture;
          break;
        case genFloorChgT:
        case genFloorChg0:
          floor->sector->special = floor->newspecial;
          //jff add to fix bug in special transfers from changes
          floor->sector->oldspecial = floor->oldspecial;
          //fall thru
        case genFloorChg:
          floor->sector->floorpic = floor->texture;
          break;
        default:
          break;
      }
    }

    floor->sector->floordata = NULL; //jff 2/22/98
    P_RemoveThinker(&floor->thinker);//remove this floor from list of movers

    //jff 2/26/98 implement stair retrigger lockout while still building
    // note this only applies to the retriggerable generalized stairs

    if (floor->sector->stairlock==-2) // if this sector is stairlocked
    {
      sector_t *sec = floor->sector;
      sec->stairlock=-1;              // thinker done, promote lock to -1

      while (sec->prevsec!=-1 && sectors[sec->prevsec].stairlock!=-2)
        sec = &sectors[sec->prevsec]; // search for a non-done thinker
      if (sec->prevsec==-1)           // if all thinkers previous are done
      {
        sec = floor->sector;          // search forward
        while (sec->nextsec!=-1 && sectors[sec->nextsec].stairlock!=-2)
          sec = &sectors[sec->nextsec];
        if (sec->nextsec==-1)         // if all thinkers ahead are done too
        {
          while (sec->prevsec!=-1)    // clear all locks
          {
            sec->stairlock = 0;
            sec = &sectors[sec->prevsec];
          }
          sec->stairlock = 0;
        }
      }
    }

    // Moving floors (but not plats) in versions <= v1.2 did not
    // make floor stop sound
    if (compatibility_level > doom_12_compatibility)
        S_StartSound((mobj_t *)&floor->sector->soundorg, sfx_pstop);
  }
}

//
// T_MoveElevator()
//
// Move an elevator to it's destination (up or down)
// Called once per tick for each moving floor.
//
// Passed an elevator_t structure that contains all pertinent info about the
// move. See P_SPEC.H for fields.
// No return.
//
// jff 02/22/98 added to support parallel floor/ceiling motion
//
void T_MoveElevator(elevator_t* elevator)
{
  result_e      res;

  if (elevator->direction<0)      // moving down
  {
    res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
    (
      elevator->sector,
      elevator->speed,
      elevator->ceilingdestheight,
      0,
      1,                          // move floor
      elevator->direction
    );
    if (res==ok || res==pastdest) // jff 4/7/98 don't move ceil if blocked
      T_MovePlane
      (
        elevator->sector,
        elevator->speed,
        elevator->floordestheight,
        0,
        0,                        // move ceiling
        elevator->direction
      );
  }
  else // up
  {
    res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
    (
      elevator->sector,
      elevator->speed,
      elevator->floordestheight,
      0,
      0,                          // move ceiling
      elevator->direction
    );
    if (res==ok || res==pastdest) // jff 4/7/98 don't move floor if blocked
      T_MovePlane
      (
        elevator->sector,
        elevator->speed,
        elevator->ceilingdestheight,
        0,
        1,                        // move floor
        elevator->direction
      );
  }

  // make floor move sound
  if (!(leveltime&7))
    S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_stnmov);

  if (res == pastdest)            // if destination height acheived
  {
    elevator->sector->floordata = NULL;     //jff 2/22/98
    elevator->sector->ceilingdata = NULL;   //jff 2/22/98
    P_RemoveThinker(&elevator->thinker);    // remove elevator from actives

    // make floor stop sound
    S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_pstop);
  }
}

///////////////////////////////////////////////////////////////////////
//
// Floor motion linedef handlers
//
///////////////////////////////////////////////////////////////////////

//
// EV_DoFloor()
//
// Handle regular and extended floor types
//
// Passed the line that activated the floor and the type of floor motion
// Returns true if a thinker was created.
//
int EV_DoFloor
( line_t*       line,
  floor_e       floortype )
{
  int           secnum;
  int           rtn;
  int           i;
  sector_t*     sec;
  floormove_t*  floor;

  secnum = -1;
  rtn = 0;

  if (ProcessNoTagLines(line, &sec, &secnum)) {if (zerotag_manual) goto manual_floor; else {return rtn;}};//e6y
  // move all floors with the same tag as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];

manual_floor://e6y
    // Don't start a second thinker on the same floor
    if (P_SectorActive(floor_special,sec)) //jff 2/23/98
      { if (!zerotag_manual) continue; else return rtn; }//e6y

    // new floor thinker
    rtn = 1;
    floor = Z_Malloc (sizeof(*floor), PU_LEVEL, 0);
    memset(floor, 0, sizeof(*floor));
    P_AddThinker (&floor->thinker);
    sec->floordata = floor; //jff 2/22/98
    floor->thinker.function = T_MoveFloor;
    floor->type = floortype;
    floor->crush = false;

    // setup the thinker according to the linedef type
    switch(floortype)
    {
      case lowerFloor:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindHighestFloorSurrounding(sec);
        break;

        //jff 02/03/30 support lowering floor by 24 absolute
      case lowerFloor24:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
        break;

        //jff 02/03/30 support lowering floor by 32 absolute (fast)
      case lowerFloor32Turbo:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED*4;
        floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
        break;

      case lowerFloorToLowest:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestFloorSurrounding(sec);
        break;

        //jff 02/03/30 support lowering floor to next lowest floor
      case lowerFloorToNearest:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight =
          P_FindNextLowestFloor(sec,floor->sector->floorheight);
        break;

      case turboLower:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED * 4;
        floor->floordestheight = P_FindHighestFloorSurrounding(sec);
        if (compatibility_level == doom_12_compatibility ||
            floor->floordestheight != sec->floorheight)
          floor->floordestheight += 8*FRACUNIT;
        break;

      case raiseFloorCrush:
        floor->crush = true;
        // fallthrough
      case raiseFloor:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
        if (floor->floordestheight > sec->ceilingheight)
          floor->floordestheight = sec->ceilingheight;
        floor->floordestheight -= (8*FRACUNIT)*(floortype == raiseFloorCrush);
        break;

      case raiseFloorTurbo:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED*4;
        floor->floordestheight = P_FindNextHighestFloor(sec,sec->floorheight);
        break;

      case raiseFloorToNearest:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindNextHighestFloor(sec,sec->floorheight);
        break;

      case raiseFloor24:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
        break;

        // jff 2/03/30 support straight raise by 32 (fast)
      case raiseFloor32Turbo:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED*4;
        floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;
        break;

      case raiseFloor512:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = floor->sector->floorheight + 512 * FRACUNIT;
        break;

      case raiseFloor24AndChange:
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
        sec->floorpic = line->frontsector->floorpic;
        sec->special = line->frontsector->special;
        //jff 3/14/98 transfer both old and new special
        sec->oldspecial = line->frontsector->oldspecial;
        break;

      case raiseToTexture:
        {
          int minsize = INT_MAX;
          side_t*     side;

    /* jff 3/13/98 no ovf */
          if (!comp[comp_model]) minsize = 32000<<FRACBITS;
          floor->direction = 1;
          floor->sector = sec;
          floor->speed = FLOORSPEED;
          for (i = 0; i < sec->linecount; i++)
          {
            if (twoSided (secnum, i) )
            {
              side = getSide(secnum,i,0);
              // jff 8/14/98 don't scan texture 0, its not real
              if (side->bottomtexture > 0 ||
                  (comp[comp_model] && !side->bottomtexture))
                if (textureheight[side->bottomtexture] < minsize)
                  minsize = textureheight[side->bottomtexture];
              side = getSide(secnum,i,1);
              // jff 8/14/98 don't scan texture 0, its not real
              if (side->bottomtexture > 0 ||
                  (comp[comp_model] && !side->bottomtexture))
                if (textureheight[side->bottomtexture] < minsize)
                  minsize = textureheight[side->bottomtexture];
            }
          }
          if (comp[comp_model])
            floor->floordestheight = floor->sector->floorheight + minsize;
          else
          {
            floor->floordestheight =
              (floor->sector->floorheight>>FRACBITS) + (minsize>>FRACBITS);
            if (floor->floordestheight>32000)
              floor->floordestheight = 32000;        //jff 3/13/98 do not
            floor->floordestheight<<=FRACBITS;       // allow height overflow
          }
        }
      break;

      case lowerAndChange:
        floor->direction = -1;
        floor->sector = sec;
        floor->speed = FLOORSPEED;
        floor->floordestheight = P_FindLowestFloorSurrounding(sec);
        floor->texture = sec->floorpic;

        // jff 1/24/98 make sure floor->newspecial gets initialized
        // in case no surrounding sector is at floordestheight
        // --> should not affect compatibility <--
        floor->newspecial = sec->special;
        //jff 3/14/98 transfer both old and new special
        floor->oldspecial = sec->oldspecial;

        //jff 5/23/98 use model subroutine to unify fixes and handling
        sec = P_FindModelFloorSector(floor->floordestheight,sec->iSectorID);
        if (sec)
        {
          floor->texture = sec->floorpic;
          floor->newspecial = sec->special;
          //jff 3/14/98 transfer both old and new special
          floor->oldspecial = sec->oldspecial;
        }
        break;
      default:
        break;
    }
    if (zerotag_manual) return rtn; //e6y
  }
  return rtn;
}

//
// EV_DoChange()
//
// Handle pure change types. These change floor texture and sector type
// by trigger or numeric model without moving the floor.
//
// The linedef causing the change and the type of change is passed
// Returns true if any sector changes
//
// jff 3/15/98 added to better support generalized sector types
//
int EV_DoChange
( line_t*       line,
  change_e      changetype )
{
  int                   secnum;
  int                   rtn;
  sector_t*             sec;
  sector_t*             secm;

  secnum = -1;
  rtn = 0;
  // change all sectors with the same tag as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];

    rtn = 1;

    // handle trigger or numeric change type
    switch(changetype)
    {
      case trigChangeOnly:
        sec->floorpic = line->frontsector->floorpic;
        sec->special = line->frontsector->special;
        sec->oldspecial = line->frontsector->oldspecial;
        break;
      case numChangeOnly:
        secm = P_FindModelFloorSector(sec->floorheight,secnum);
        if (secm) // if no model, no change
        {
          sec->floorpic = secm->floorpic;
          sec->special = secm->special;
          sec->oldspecial = secm->oldspecial;
        }
        break;
      default:
        break;
    }
  }
  return rtn;
}

/*
 * EV_BuildStairs()
 *
 * Handles staircase building. A sequence of sectors chosen by algorithm
 * rise at a speed indicated to a height that increases by the stepsize
 * each step.
 *
 * Passed the linedef triggering the stairs and the type of stair rise
 * Returns true if any thinkers are created
 *
 * cph 2001/09/21 - compatibility nightmares again
 * There are three different ways this function has, during its history, stepped
 * through all the stairs to be triggered by the single switch
 * - original Doom used a linear P_FindSectorFromLineTag, but failed to preserve
 * the index of the previous sector found, so instead it would restart its
 * linear search from the last sector of the previous staircase
 * - MBF/PrBoom with comp_stairs fail to emulate this, because their
 * P_FindSectorFromLineTag is a chained hash table implementation. Instead they
 * start following the hash chain from the last sector of the previous
 * staircase, which will (probably) have the wrong tag, so they miss any further
 * stairs
 * - Boom fixed the bug, and MBF/PrBoom without comp_stairs work right
 */
static inline int P_FindSectorFromLineTagWithLowerBound
(line_t* l, int start, int min)
{
  /* Emulate original Doom's linear lower-bounded P_FindSectorFromLineTag
   * as needed */
  do {
    start = P_FindSectorFromLineTag(l,start);
  } while (start >= 0 && start <= min);
  return start;
}

int EV_BuildStairs
( line_t*       line,
  stair_e       type )
{
  /* cph 2001/09/22 - cleaned up this function to save my sanity. A separate
   * outer loop index makes the logic much cleared, and local variables moved
   * into the inner blocks helps too */
  int                   ssec = -1;
  int                   minssec = -1;
  int                   rtn = 0;

  //e6y
  int           secnum = -1;
  sector_t*     sec;
  if (ProcessNoTagLines(line, &sec, &secnum)) {if (zerotag_manual) goto manual_stair; else {return rtn;}};//e6y

  // start a stair at each sector tagged the same as the linedef
  while ((ssec = P_FindSectorFromLineTagWithLowerBound(line,ssec,minssec)) >= 0)
  {
   //e6y int
   secnum = ssec;
   //e6y sector_t*
   sec = &sectors[secnum];

manual_stair://e6y
   // don't start a stair if the first step's floor is already moving
   if (!P_SectorActive(floor_special,sec)) { //jff 2/22/98
    floormove_t*  floor;
    int           texture, height;
    fixed_t       stairsize;
    fixed_t       speed;
    dboolean      crush;
    int           ok;

    // create new floor thinker for first step
    rtn = 1;
    floor = Z_Malloc (sizeof(*floor), PU_LEVEL, 0);
    memset(floor, 0, sizeof(*floor));
    P_AddThinker (&floor->thinker);
    sec->floordata = floor;
    floor->thinker.function = T_MoveFloor;
    floor->direction = 1;
    floor->sector = sec;
    floor->type = buildStair;   //jff 3/31/98 do not leave uninited
    crush = floor->crush;

    // set up the speed and stepsize according to the stairs type
    switch(type)
    {
      default: // killough -- prevent compiler warning
      case build8:
        speed = FLOORSPEED/4;
        stairsize = 8*FRACUNIT;
        if (!demo_compatibility)
          crush = false; //jff 2/27/98 fix uninitialized crush field
        // e6y
        // Uninitialized crush field will not be equal to 0 or 1 (true)
        // with high probability. So, initialize it with any other value
        // There is no more desync on icarus.wad/ic29uv.lmp
        // http://competn.doom2.net/pub/sda/i-o/icuvlmps.zip
        // http://www.doomworld.com/idgames/index.php?id=5191
        else
        {
          if (!prboom_comp[PC_UNINITIALIZE_CRUSH_FIELD_FOR_STAIRS].state)
            crush = STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE;
        }

        break;
      case turbo16:
        speed = FLOORSPEED*4;
        stairsize = 16*FRACUNIT;
        if (!demo_compatibility)
          crush = true;  //jff 2/27/98 fix uninitialized crush field
        // e6y
        // Uninitialized crush field will not be equal to 0 or 1 (true)
        // with high probability. So, initialize it with any other value
        else
        {
          if (!prboom_comp[PC_UNINITIALIZE_CRUSH_FIELD_FOR_STAIRS].state)
            crush = STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE;
        }

        break;
      case heretic_build8:
        speed = FLOORSPEED;
        stairsize = 8 * FRACUNIT;
        crush = STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE; // heretic_note: I guess

        break;
      case heretic_turbo16:
        speed = FLOORSPEED;
        stairsize = 16 * FRACUNIT;
        crush = STAIRS_UNINITIALIZED_CRUSH_FIELD_VALUE; // heretic_note: I guess

        break;
    }
    floor->speed = speed;
    floor->crush = crush;
    height = sec->floorheight + stairsize;
    floor->floordestheight = height;

    texture = sec->floorpic;

    // Find next sector to raise
    //   1. Find 2-sided line with same sector side[0] (lowest numbered)
    //   2. Other side is the next sector to raise
    //   3. Unless already moving, or different texture, then stop building
    do
    {
      int i;
      ok = 0;

      for (i = 0;i < sec->linecount;i++)
      {
        sector_t* tsec = (sec->lines[i])->frontsector;
        int newsecnum;
        if ( !((sec->lines[i])->flags & ML_TWOSIDED) )
          continue;

        newsecnum = tsec->iSectorID;

        if (secnum != newsecnum)
          continue;

        tsec = (sec->lines[i])->backsector;
        if (!tsec) continue;     //jff 5/7/98 if no backside, continue
        newsecnum = tsec->iSectorID;

        // if sector's floor is different texture, look for another
        if (tsec->floorpic != texture)
          continue;

        /* jff 6/19/98 prevent double stepsize
   * killough 10/98: intentionally left this way [MBF comment]
   * cph 2001/02/06: stair bug fix should be controlled by comp_stairs,
   *  except if we're emulating MBF which perversly reverted the fix
   */
        if (comp[comp_stairs] || (compatibility_level == mbf_compatibility))
          height += stairsize; // jff 6/28/98 change demo compatibility

        // if sector's floor already moving, look for another
        if (P_SectorActive(floor_special,tsec)) //jff 2/22/98
          continue;

  /* cph - see comment above - do this iff we didn't do so above */
        if (!comp[comp_stairs] && (compatibility_level != mbf_compatibility))
          height += stairsize;

        sec = tsec;
        secnum = newsecnum;

        // create and initialize a thinker for the next step
        floor = Z_Malloc (sizeof(*floor), PU_LEVEL, 0);
        memset(floor, 0, sizeof(*floor));
        P_AddThinker (&floor->thinker);

        sec->floordata = floor; //jff 2/22/98
        floor->thinker.function = T_MoveFloor;
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = speed;
        floor->floordestheight = height;
        floor->type = buildStair; //jff 3/31/98 do not leave uninited
        floor->crush = crush; //jff 2/27/98 fix uninitialized crush field

        ok = 1;
        break;
      }
    } while(ok);      // continue until no next step is found

   }
   /* killough 10/98: compatibility option */
   if (comp[comp_stairs]) {
     /* cph 2001/09/22 - emulate buggy MBF comp_stairs for demos, with logic
      * reversed since we now have a separate outer loop index.
      * DEMOSYNC - what about boom_compatibility_compatibility?
      */
     if ((compatibility_level >= mbf_compatibility) && (compatibility_level <
           prboom_3_compatibility)) ssec = secnum; /* Trash outer loop index */
     else {
       /* cph 2001/09/22 - now the correct comp_stairs - Doom used a linear
        * search from the last secnum, so we set that as a minimum value and do
        * a fresh tag search
        */
       ssec = -1; minssec = secnum;
     }
   }
    if (zerotag_manual) return rtn; //e6y
  }
  return rtn;
}

//
// EV_DoDonut()
//
// Handle donut function: lower pillar, raise surrounding pool, both to height,
// texture and type of the sector surrounding the pool.
//
// Passed the linedef that triggered the donut
// Returns whether a thinker was created
//
int EV_DoDonut(line_t*  line)
{
  sector_t* s1;
  sector_t* s2;
  sector_t* s3;
  int       secnum;
  int       rtn;
  int       i;
  floormove_t* floor;

  //e6y
  fixed_t s3_floorheight;
  short s3_floorpic;

  secnum = -1;
  rtn = 0;
  // do function on all sectors with same tag as linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    s1 = &sectors[secnum];                // s1 is pillar's sector

    // do not start the donut if the pillar is already moving
    if (P_SectorActive(floor_special,s1)) //jff 2/22/98
      continue;

    // heretic_note: rtn = 1; // probably doesn't matter?

    s2 = getNextSector(s1->lines[0],s1);  // s2 is pool's sector

    // note lowest numbered line around
    // pillar must be two-sided
    if (!s2)
    {
      if (demo_compatibility)
      {
        lprintf(LO_ERROR,
          "EV_DoDonut: lowest numbered line (linedef: %d) "
          "around pillar (sector: %d) must be two-sided. "
          "Unexpected behavior may occur in Vanilla Doom.\n",
          s1->lines[0]->iLineID, s1->iSectorID);
        continue;
      }
      else
      {
        continue;
      }
    }

    /* do not start the donut if the pool is already moving
     * cph - DEMOSYNC - was !compatibility */
    if (!comp[comp_floors] && P_SectorActive(floor_special,s2))
      continue;                           //jff 5/7/98

    // find a two sided line around the pool whose other side isn't the pillar
    for (i = 0;i < s2->linecount;i++)
    {
      //jff 3/29/98 use true two-sidedness, not the flag
      // killough 4/5/98: changed demo_compatibility to compatibility
      if (comp[comp_model])
      {
        // original code:   !s2->lines[i]->flags & ML_TWOSIDED
        // equivalent to:   (!s2->lines[i]->flags) & ML_TWOSIDED , i.e. 0
        // should be:       !(s2->lines[i]->flags & ML_TWOSIDED)
        if (((!s2->lines[i]->flags) & ML_TWOSIDED) ||
            (s2->lines[i]->backsector == s1))
          continue;
      }
      else if (!s2->lines[i]->backsector || s2->lines[i]->backsector == s1)
        continue;

      rtn = 1; //jff 1/26/98 no donut action - no switch change on return

      s3 = s2->lines[i]->backsector;      // s3 is model sector for changes

      if (!s3)
      {
        // e6y
        // s3->floorheight is an int at 0000:0000
        // s3->floorpic is a short at 0000:0008
        // Trying to emulate
        lprintf(LO_ERROR,
          "EV_DoDonut: Access violation at linedef %d, sector %d. "
          "Unexpected behavior may occur in Vanilla Doom.\n",
          line->iLineID, s1->iSectorID);
        if (DonutOverrun(&s3_floorheight, &s3_floorpic))
        {
          lprintf(LO_WARN, "EV_DoDonut: Emulated with floorheight %d, floor pic %d.\n",
            s3_floorheight >> 16, s3_floorpic);
        }
        else
        {
          lprintf(LO_WARN, "EV_DoDonut: Not emulated.\n");
          break;
        }
      }
      else
      {
        s3_floorheight = s3->floorheight;
        s3_floorpic = s3->floorpic;
      }

      //  Spawn rising slime
      floor = Z_Malloc (sizeof(*floor), PU_LEVEL, 0);
      memset(floor, 0, sizeof(*floor));
      P_AddThinker (&floor->thinker);
      s2->floordata = floor; //jff 2/22/98
      floor->thinker.function = T_MoveFloor;
      floor->type = donutRaise;
      floor->crush = false;
      floor->direction = 1;
      floor->sector = s2;
      floor->speed = FLOORSPEED / 2;
      floor->texture = s3_floorpic;
      floor->newspecial = 0;
      floor->floordestheight = s3_floorheight;

      //  Spawn lowering donut-hole pillar
      floor = Z_Malloc (sizeof(*floor), PU_LEVEL, 0);
      memset(floor, 0, sizeof(*floor));
      P_AddThinker (&floor->thinker);
      s1->floordata = floor; //jff 2/22/98
      floor->thinker.function = T_MoveFloor;
      floor->type = lowerFloor;
      floor->crush = false;
      floor->direction = -1;
      floor->sector = s1;
      floor->speed = FLOORSPEED / 2;
      floor->floordestheight = s3_floorheight;
      break;
    }
  }
  return rtn;
}

//
// EV_DoElevator
//
// Handle elevator linedef types
//
// Passed the linedef that triggered the elevator and the elevator action
//
// jff 2/22/98 new type to move floor and ceiling in parallel
//
int EV_DoElevator
( line_t*       line,
  elevator_e    elevtype )
{
  int                   secnum;
  int                   rtn;
  sector_t*             sec;
  elevator_t*           elevator;

  secnum = -1;
  rtn = 0;
  // act on all sectors with the same tag as the triggering linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];

    // If either floor or ceiling is already activated, skip it
    if (sec->floordata || sec->ceilingdata) //jff 2/22/98
      continue;

    // create and initialize new elevator thinker
    rtn = 1;
    elevator = Z_Malloc (sizeof(*elevator), PU_LEVEL, 0);
    memset(elevator, 0, sizeof(*elevator));
    P_AddThinker (&elevator->thinker);
    sec->floordata = elevator; //jff 2/22/98
    sec->ceilingdata = elevator; //jff 2/22/98
    elevator->thinker.function = T_MoveElevator;
    elevator->type = elevtype;

    // set up the fields according to the type of elevator action
    switch(elevtype)
    {
        // elevator down to next floor
      case elevateDown:
        elevator->direction = -1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight =
          P_FindNextLowestFloor(sec,sec->floorheight);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

        // elevator up to next floor
      case elevateUp:
        elevator->direction = 1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight =
          P_FindNextHighestFloor(sec,sec->floorheight);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

        // elevator to floor height of activating switch's front sector
      case elevateCurrent:
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight = line->frontsector->floorheight;
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        elevator->direction =
          elevator->floordestheight>sec->floorheight?  1 : -1;
        break;

      default:
        break;
    }
  }
  return rtn;
}

// hexen

#include "hexen/p_acs.h"
#include "hexen/sn_sonix.h"

static void Hexen_T_MoveFloor(floormove_t * floor)
{
    result_e res;

    if (floor->resetDelayCount)
    {
        floor->resetDelayCount--;
        if (!floor->resetDelayCount)
        {
            floor->floordestheight = floor->resetHeight;
            floor->direction = -floor->direction;
            floor->resetDelay = 0;
            floor->delayCount = 0;
            floor->delayTotal = 0;
        }
    }
    if (floor->delayCount)
    {
        floor->delayCount--;
        if (!floor->delayCount && floor->textureChange)
        {
            floor->sector->floorpic += floor->textureChange;
        }
        return;
    }

    res = T_MovePlane(floor->sector, floor->speed,
                      floor->floordestheight, floor->crush, 0,
                      floor->direction);

    if (floor->type == FLEV_RAISEBUILDSTEP)
    {
        if ((floor->direction == 1 && floor->sector->floorheight >=
             floor->stairsDelayHeight) || (floor->direction == -1 &&
                                           floor->sector->floorheight <=
                                           floor->stairsDelayHeight))
        {
            floor->delayCount = floor->delayTotal;
            floor->stairsDelayHeight += floor->stairsDelayHeightDelta;
        }
    }
    if (res == pastdest)
    {
        SN_StopSequence((mobj_t *) & floor->sector->soundorg);
        if (floor->delayTotal)
        {
            floor->delayTotal = 0;
        }
        if (floor->resetDelay)
        {
            return;
        }
        floor->sector->floordata = NULL;
        if (floor->textureChange)
        {
            floor->sector->floorpic -= floor->textureChange;
        }
        P_TagFinished(floor->sector->tag);
        P_RemoveThinker(&floor->thinker);
    }
}

int Hexen_EV_DoFloor(line_t * line, byte * args, floor_e floortype)
{
    int secnum;
    int rtn;
    sector_t *sec;
    floormove_t *floor = NULL;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
    {
        sec = &sectors[secnum];

        //      ALREADY MOVING?  IF SO, KEEP GOING...
        if (sec->floordata || sec->ceilingdata)
            continue;

        //
        //      new floor thinker
        //
        rtn = 1;
        floor = Z_Malloc(sizeof(*floor), PU_LEVEL, 0);
        memset(floor, 0, sizeof(*floor));
        P_AddThinker(&floor->thinker);
        sec->floordata = floor;
        floor->thinker.function = T_MoveFloor;
        floor->type = floortype;
        floor->crush = 0;
        floor->speed = args[1] * (FRACUNIT / 8);
        if (floortype == FLEV_LOWERTIMES8INSTANT ||
            floortype == FLEV_RAISETIMES8INSTANT)
        {
            floor->speed = 2000 << FRACBITS;
        }
        switch (floortype)
        {
            case FLEV_LOWERFLOOR:
                floor->direction = -1;
                floor->sector = sec;
                floor->floordestheight = P_FindHighestFloorSurrounding(sec);
                break;
            case FLEV_LOWERFLOORTOLOWEST:
                floor->direction = -1;
                floor->sector = sec;
                floor->floordestheight = P_FindLowestFloorSurrounding(sec);
                break;
            case FLEV_LOWERFLOORBYVALUE:
                floor->direction = -1;
                floor->sector = sec;
                floor->floordestheight = floor->sector->floorheight -
                    args[2] * FRACUNIT;
                break;
            case FLEV_LOWERTIMES8INSTANT:
            case FLEV_LOWERBYVALUETIMES8:
                floor->direction = -1;
                floor->sector = sec;
                floor->floordestheight = floor->sector->floorheight -
                    args[2] * FRACUNIT * 8;
                break;
            case FLEV_RAISEFLOORCRUSH:
                floor->crush = args[2]; // arg[2] = crushing value
                floor->direction = 1;
                floor->sector = sec;
                floor->floordestheight = sec->ceilingheight - 8 * FRACUNIT;
                break;
            case FLEV_RAISEFLOOR:
                floor->direction = 1;
                floor->sector = sec;
                floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
                if (floor->floordestheight > sec->ceilingheight)
                    floor->floordestheight = sec->ceilingheight;
                break;
            case FLEV_RAISEFLOORTONEAREST:
                floor->direction = 1;
                floor->sector = sec;
                floor->floordestheight =
                    P_FindNextHighestFloor(sec, sec->floorheight);
                break;
            case FLEV_RAISEFLOORBYVALUE:
                floor->direction = 1;
                floor->sector = sec;
                floor->floordestheight = floor->sector->floorheight +
                    args[2] * FRACUNIT;
                break;
            case FLEV_RAISETIMES8INSTANT:
            case FLEV_RAISEBYVALUETIMES8:
                floor->direction = 1;
                floor->sector = sec;
                floor->floordestheight = floor->sector->floorheight +
                    args[2] * FRACUNIT * 8;
                break;
            case FLEV_MOVETOVALUETIMES8:
                floor->sector = sec;
                floor->floordestheight = args[2] * FRACUNIT * 8;
                if (args[3])
                {
                    floor->floordestheight = -floor->floordestheight;
                }
                if (floor->floordestheight > floor->sector->floorheight)
                {
                    floor->direction = 1;
                }
                else if (floor->floordestheight < floor->sector->floorheight)
                {
                    floor->direction = -1;
                }
                else
                {               // already at lowest position
                    rtn = 0;
                }
                break;
            default:
                rtn = 0;
                break;
        }
    }
    if (rtn)
    {
        SN_StartSequence((mobj_t *) & floor->sector->soundorg,
                         SEQ_PLATFORM + floor->sector->seqType);
    }
    return rtn;
}

int EV_DoFloorAndCeiling(line_t * line, byte * args, dboolean raise)
{
    dboolean floor, ceiling;
    int secnum;
    sector_t *sec;

    if (raise)
    {
        floor = Hexen_EV_DoFloor(line, args, FLEV_RAISEFLOORBYVALUE);
        secnum = -1;
        while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
        {
            sec = &sectors[secnum];
            sec->floordata = NULL;
        }
        ceiling = Hexen_EV_DoCeiling(line, args, CLEV_RAISEBYVALUE);
    }
    else
    {
        floor = Hexen_EV_DoFloor(line, args, FLEV_LOWERFLOORBYVALUE);
        secnum = -1;
        while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
        {
            sec = &sectors[secnum];
            sec->floordata = NULL;
        }
        ceiling = Hexen_EV_DoCeiling(line, args, CLEV_LOWERBYVALUE);
    }
    return (floor | ceiling);
}

#define STAIR_SECTOR_TYPE       26
#define STAIR_QUEUE_SIZE        32

struct
{
    sector_t *sector;
    int type;
    int height;
} StairQueue[STAIR_QUEUE_SIZE];

static int QueueHead;
static int QueueTail;

static int StepDelta;
static int Direction;
static int Speed;
static int Texture;
static int StartDelay;
static int StartDelayDelta;
static int TextureChange;
static int StartHeight;

static void QueueStairSector(sector_t * sec, int type, int height)
{
    if ((QueueTail + 1) % STAIR_QUEUE_SIZE == QueueHead)
    {
        I_Error("BuildStairs:  Too many branches located.\n");
    }
    StairQueue[QueueTail].sector = sec;
    StairQueue[QueueTail].type = type;
    StairQueue[QueueTail].height = height;

    QueueTail = (QueueTail + 1) % STAIR_QUEUE_SIZE;
}

static sector_t *DequeueStairSector(int *type, int *height)
{
    sector_t *sec;

    if (QueueHead == QueueTail)
    {                           // queue is empty
        return NULL;
    }
    *type = StairQueue[QueueHead].type;
    *height = StairQueue[QueueHead].height;
    sec = StairQueue[QueueHead].sector;
    QueueHead = (QueueHead + 1) % STAIR_QUEUE_SIZE;

    return sec;
}

static void ProcessStairSector(sector_t * sec, int type, int height,
                               stairs_e stairsType, int delay, int resetDelay)
{
    int i;
    sector_t *tsec;
    floormove_t *floor;

    //
    // new floor thinker
    //
    height += StepDelta;
    floor = Z_Malloc(sizeof(*floor), PU_LEVEL, 0);
    memset(floor, 0, sizeof(*floor));
    P_AddThinker(&floor->thinker);
    sec->floordata = floor;
    floor->thinker.function = T_MoveFloor;
    floor->type = FLEV_RAISEBUILDSTEP;
    floor->direction = Direction;
    floor->sector = sec;
    floor->floordestheight = height;
    switch (stairsType)
    {
        case STAIRS_NORMAL:
            floor->speed = Speed;
            if (delay)
            {
                floor->delayTotal = delay;
                floor->stairsDelayHeight = sec->floorheight + StepDelta;
                floor->stairsDelayHeightDelta = StepDelta;
            }
            floor->resetDelay = resetDelay;
            floor->resetDelayCount = resetDelay;
            floor->resetHeight = sec->floorheight;
            break;
        case STAIRS_SYNC:
            floor->speed = FixedMul(Speed, FixedDiv(height - StartHeight,
                                                    StepDelta));
            floor->resetDelay = delay;  //arg4
            floor->resetDelayCount = delay;
            floor->resetHeight = sec->floorheight;
            break;
        default:
            break;
    }
    SN_StartSequence((mobj_t *) & sec->soundorg, SEQ_PLATFORM + sec->seqType);
    //
    // Find next sector to raise
    // Find nearby sector with sector special equal to type
    //
    for (i = 0; i < sec->linecount; i++)
    {
        if (!((sec->lines[i])->flags & ML_TWOSIDED))
        {
            continue;
        }
        tsec = (sec->lines[i])->frontsector;
        if (tsec->special == type + STAIR_SECTOR_TYPE && !tsec->floordata && !tsec->ceilingdata
            && tsec->floorpic == Texture && tsec->validcount != validcount)
        {
            QueueStairSector(tsec, type ^ 1, height);
            tsec->validcount = validcount;
            //tsec->special = 0;
        }
        tsec = (sec->lines[i])->backsector;
        if (tsec->special == type + STAIR_SECTOR_TYPE && !tsec->floordata && !tsec->ceilingdata
            && tsec->floorpic == Texture && tsec->validcount != validcount)
        {
            QueueStairSector(tsec, type ^ 1, height);
            tsec->validcount = validcount;
            //tsec->special = 0;
        }
    }
}

int Hexen_EV_BuildStairs(line_t * line, byte * args, int direction, stairs_e stairsType)
{
    int secnum;
    int height;
    int delay;
    int resetDelay;
    sector_t *sec;
    sector_t *qSec;
    int type;

    // Set global stairs variables
    TextureChange = 0;
    Direction = direction;
    StepDelta = Direction * (args[2] * FRACUNIT);
    Speed = args[1] * (FRACUNIT / 8);
    resetDelay = args[4];
    delay = args[3];
    if (stairsType == STAIRS_PHASED)
    {
        StartDelayDelta = args[3];
        StartDelay = StartDelayDelta;
        resetDelay = StartDelayDelta;
        delay = 0;
        TextureChange = args[4];
    }

    secnum = -1;

    validcount++;
    while ((secnum = P_FindSectorFromTag(args[0], secnum)) >= 0)
    {
        sec = &sectors[secnum];

        Texture = sec->floorpic;
        StartHeight = sec->floorheight;

        // ALREADY MOVING?  IF SO, KEEP GOING...
        if (sec->floordata || sec->ceilingdata)
            continue;

        QueueStairSector(sec, 0, sec->floorheight);
        sec->special = 0;
    }
    while ((qSec = DequeueStairSector(&type, &height)) != NULL)
    {
        ProcessStairSector(qSec, type, height, stairsType, delay, resetDelay);
    }
    return (1);
}

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
        if (sec->floordata || sec->ceilingdata)
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
        memset(pillar, 0, sizeof(*pillar));
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
        if (sec->floordata || sec->ceilingdata)
            continue;           // already moving
        if (sec->floorheight != sec->ceilingheight)
        {                       // pillar isn't closed
            continue;
        }
        rtn = 1;
        pillar = Z_Malloc(sizeof(*pillar), PU_LEVEL, 0);
        memset(pillar, 0, sizeof(*pillar));
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
        if (sector->floordata || sector->ceilingdata)
        {                       // Already busy with another thinker
            continue;
        }
        retCode = true;
        waggle = Z_Malloc(sizeof(*waggle), PU_LEVEL, 0);
        memset(waggle, 0, sizeof(*waggle));
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
