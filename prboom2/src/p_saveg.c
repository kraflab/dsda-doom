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
 *      Archiving: SaveGame I/O.
 *
 *-----------------------------------------------------------------------------*/

#include <stdint.h>

#include "doomstat.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_saveg.h"
#include "m_random.h"
#include "am_map.h"
#include "p_enemy.h"
#include "lprintf.h"
#include "s_advsound.h"
#include "hu_tracers.h"
#include "e6y.h"//e6y

#include "hexen/a_action.h"
#include "hexen/p_acs.h"
#include "hexen/po_man.h"
#include "hexen/sn_sonix.h"
#include "hexen/sv_save.h"

#include "dsda/msecnode.h"

#define MARKED_FOR_DELETION -2

byte *save_p;

//
// P_ArchivePlayers
//
void P_ArchivePlayers (void)
{
  int i;

  CheckSaveGame(sizeof(player_t) * g_maxplayers); // killough
  for (i = 0; i < g_maxplayers; i++)
    if (playeringame[i])
      {
        int      j;
        player_t *dest;

        dest = (player_t *) save_p;
        memcpy(dest, &players[i], sizeof(player_t));
        save_p += sizeof(player_t);
        for (j=0; j<NUMPSPRITES; j++)
          if (dest->psprites[j].state)
            dest->psprites[j].state =
              (state_t *)(dest->psprites[j].state-states);
      }
}

//
// P_UnArchivePlayers
//
void P_UnArchivePlayers (void)
{
  int i;

  for (i = 0; i < g_maxplayers; i++)
    if (playeringame[i])
      {
        int j;

        memcpy(&players[i], save_p, sizeof(player_t));
        save_p += sizeof(player_t);

        // will be set when unarc thinker
        players[i].mo = NULL;
        players[i].message = NULL;
        players[i].attacker = NULL;
        // HERETIC_TODO: does the rain need to be remembered?
        players[i].rain1 = NULL;
        players[i].rain2 = NULL;

        // hexen_note: poisoner not reloaded
        players[i].poisoner = NULL;

        for (j=0 ; j<NUMPSPRITES ; j++)
          if (players[i]. psprites[j].state)
            players[i]. psprites[j].state =
              &states[ (size_t)players[i].psprites[j].state ];
      }
}


//
// P_ArchiveWorld
//
void P_ArchiveWorld (void)
{
  int            i;
  const sector_t *sec;
  const line_t   *li;
  const side_t   *si;
  short          *put;

  // killough 3/22/98: fix bug caused by hoisting save_p too early
  // killough 10/98: adjust size for changes below
  size_t size =
    (sizeof(short)*5 + sizeof sec->floorheight + sizeof sec->ceilingheight)
    * numsectors + sizeof(short)*3*numlines + 4 + 2;

  for (i=0; i<numlines; i++)
    {
      if (lines[i].sidenum[0] != NO_INDEX)
        size +=
    sizeof(short)*3 + sizeof si->textureoffset + sizeof si->rowoffset;
      if (lines[i].sidenum[1] != NO_INDEX)
  size +=
    sizeof(short)*3 + sizeof si->textureoffset + sizeof si->rowoffset;
    }

  CheckSaveGame(size); // killough

  put = (short *)save_p;

  // do sectors
  for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
    {
      // killough 10/98: save full floor & ceiling heights, including fraction
      memcpy(put, &sec->floorheight, sizeof sec->floorheight);
      put = (void *)((char *) put + sizeof sec->floorheight);
      memcpy(put, &sec->ceilingheight, sizeof sec->ceilingheight);
      put = (void *)((char *) put + sizeof sec->ceilingheight);

      *put++ = sec->floorpic;
      *put++ = sec->ceilingpic;
      *put++ = sec->lightlevel;
      *put++ = sec->special;            // needed?   yes -- transfer types
      *put++ = sec->tag;                // needed?   need them -- killough
    }

  // do lines
  for (i=0, li = lines ; i<numlines ; i++,li++)
    {
      int j;

      *put++ = li->flags;
      *put++ = li->special;
      *put++ = li->tag;

      for (j=0; j<2; j++)
        if (li->sidenum[j] != NO_INDEX)
          {
      si = &sides[li->sidenum[j]];

      // killough 10/98: save full sidedef offsets,
      // preserving fractional scroll offsets

      memcpy(put, &si->textureoffset, sizeof si->textureoffset);
      put = (void *)((char *) put + sizeof si->textureoffset);
      memcpy(put, &si->rowoffset, sizeof si->rowoffset);
      put = (void *)((char *) put + sizeof si->rowoffset);

            *put++ = si->toptexture;
            *put++ = si->bottomtexture;
            *put++ = si->midtexture;
          }
    }

  *put++ = musinfo.current_item;

  save_p = (byte *) put;
}



//
// P_UnArchiveWorld
//
void P_UnArchiveWorld (void)
{
  int          i;
  sector_t     *sec;
  line_t       *li;
  short        *get;

  get = (short *) save_p;

  // do sectors
  for (i=0, sec = sectors ; i<numsectors ; i++,sec++)
    {
      // killough 10/98: load full floor & ceiling heights, including fractions

      memcpy(&sec->floorheight, get, sizeof sec->floorheight);
      get = (void *)((char *) get + sizeof sec->floorheight);
      memcpy(&sec->ceilingheight, get, sizeof sec->ceilingheight);
      get = (void *)((char *) get + sizeof sec->ceilingheight);

      sec->floorpic = *get++;
      sec->ceilingpic = *get++;
      sec->lightlevel = *get++;
      sec->special = *get++;
      sec->tag = *get++;
      sec->ceilingdata = 0; //jff 2/22/98 now three thinker fields, not two
      sec->floordata = 0;
      sec->soundtarget = 0;
    }

  // do lines
  for (i=0, li = lines ; i<numlines ; i++,li++)
    {
      int j;

      li->flags = *get++;
      li->special = *get++;
      li->tag = *get++;
      for (j=0 ; j<2 ; j++)
        if (li->sidenum[j] != NO_INDEX)
          {
            side_t *si = &sides[li->sidenum[j]];

      // killough 10/98: load full sidedef offsets, including fractions

      memcpy(&si->textureoffset, get, sizeof si->textureoffset);
      get = (void *)((char *) get + sizeof si->textureoffset);
      memcpy(&si->rowoffset, get, sizeof si->rowoffset);
      get = (void *)((char *) get + sizeof si->rowoffset);

            si->toptexture = *get++;
            si->bottomtexture = *get++;
            si->midtexture = *get++;
          }
    }

  musinfo.current_item = *get++;

  save_p = (byte *) get;
}

//
// Thinkers
//

// phares 9/13/98: Moved this code outside of P_ArchiveThinkers so the
// thinker indices could be used by the code that saves sector info.

static int number_of_thinkers;

static dboolean P_IsMobjThinker(thinker_t* thinker)
{
  return thinker->function == P_MobjThinker ||
         thinker->function == P_BlasterMobjThinker ||
         (thinker->function == P_RemoveThinkerDelayed && thinker->references);
}

static void P_ReplaceMobjWithIndex(mobj_t **mobj)
{
  if (*mobj)
  {
    *mobj = P_IsMobjThinker(&(*mobj)->thinker) ?
            (mobj_t *) (*mobj)->thinker.prev   :
            NULL;
  }
}

/*
 * killough 11/98
 *
 * Same as P_SetTarget() in p_tick.c, except that the target is nullified
 * first, so that no old target's reference count is decreased (when loading
 * savegames, old targets are indices, not really pointers to targets).
 */

static void P_SetNewTarget(mobj_t **mop, mobj_t *targ)
{
  *mop = NULL;
  P_SetTarget(mop, targ);
}

// savegame file stores ints in the corresponding * field; this function
// safely casts them back to int.
int P_GetMobj(mobj_t* mi, size_t s)
{
  size_t i = (size_t)mi;
  if (i >= s)
    I_Error("Corrupt savegame");
  return i;
}

static void P_ReplaceIndexWithMobj(mobj_t **mobj, mobj_t **mobj_p, int mobj_count)
{
  P_SetNewTarget(
    mobj,
    mobj_p[
      P_GetMobj(*mobj, mobj_count + 1)
    ]
  );
}

void P_ThinkerToIndex(void)
{
  thinker_t *th;

  // killough 2/14/98:
  // count the number of thinkers, and mark each one with its index, using
  // the prev field as a placeholder, since it can be restored later.

  number_of_thinkers = 0;
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (P_IsMobjThinker(th))
      th->prev = (thinker_t *)(intptr_t) ++number_of_thinkers;
}

// phares 9/13/98: Moved this code outside of P_ArchiveThinkers so the
// thinker indices could be used by the code that saves sector info.

void P_IndexToThinker(void)
{
  // killough 2/14/98: restore prev pointers
  thinker_t *th;
  thinker_t *prev = &thinkercap;

  for (th = thinkercap.next ; th != &thinkercap ; prev=th, th=th->next)
    th->prev = prev;
}

// killough 2/16/98: save/restore random number generator state information

void P_ArchiveRNG(void)
{
  CheckSaveGame(sizeof rng);
  memcpy(save_p, &rng, sizeof rng);
  save_p += sizeof rng;
}

void P_UnArchiveRNG(void)
{
  memcpy(&rng, save_p, sizeof rng);
  save_p += sizeof rng;
}

// killough 2/22/98: Save/restore automap state
// killough 2/22/98: Save/restore automap state
void P_ArchiveMap(void)
{
  int i, zero = 0, one = 1;
  CheckSaveGame(2 * sizeof zero + sizeof markpointnum +
                markpointnum * (sizeof(markpoints[0].x) + sizeof(markpoints[0].y)) +
                sizeof automapmode + sizeof one);

  memcpy(save_p, &automapmode, sizeof automapmode);
  save_p += sizeof automapmode;
  memcpy(save_p, &one, sizeof one);   // CPhipps - used to be viewactive, now
  save_p += sizeof one;               // that's worked out locally by D_Display
  memcpy(save_p, &zero, sizeof zero); // CPhipps - used to be followplayer
  save_p += sizeof zero;              //  that is now part of automapmode
  memcpy(save_p, &zero, sizeof zero); // CPhipps - used to be automap_grid, ditto
  save_p += sizeof zero;
  memcpy(save_p, &markpointnum, sizeof markpointnum);
  save_p += sizeof markpointnum;

  for (i = 0; i < markpointnum; i++)
  {
    memcpy(save_p, &markpoints[i].x, sizeof(markpoints[i].x));
    save_p += sizeof(markpoints[i].x);
    memcpy(save_p, &markpoints[i].y, sizeof(markpoints[i].y));
    save_p += sizeof(markpoints[i].y);
  }
}

void P_UnArchiveMap(void)
{
  int unused;
  memcpy(&automapmode, save_p, sizeof automapmode);
  save_p += sizeof automapmode;
  memcpy(&unused, save_p, sizeof unused);
  save_p += sizeof unused;
  memcpy(&unused, save_p, sizeof unused);
  save_p += sizeof unused;
  memcpy(&unused, save_p, sizeof unused);
  save_p += sizeof unused;

  if (automapmode & am_active)
    AM_Start();

  memcpy(&markpointnum, save_p, sizeof markpointnum);
  save_p += sizeof markpointnum;

  if (markpointnum)
    {
      int i;
      while (markpointnum >= markpointnum_max)
        markpoints = realloc(markpoints, sizeof *markpoints *
         (markpointnum_max = markpointnum_max ? markpointnum_max*2 : 16));

      for (i = 0; i < markpointnum; i++)
      {
        memcpy(&markpoints[i].x, save_p, sizeof(markpoints[i].x));
        save_p += sizeof(markpoints[i].x);
        memcpy(&markpoints[i].y, save_p, sizeof(markpoints[i].y));
        save_p += sizeof(markpoints[i].y);

        AM_setMarkParams(i);
      }
    }
}

void P_ArchiveThinkerSubclass(th_class class)
{
  int count;
  thinker_t *cap, *th;

  count = 0;
  cap = &thinkerclasscap[class];
  for (th = cap->cnext; th != cap; th = th->cnext)
    count++;

  CheckSaveGame(count * sizeof(mobj_t*) + sizeof(count));

  memcpy(save_p, &count, sizeof(count));
  save_p += sizeof(count);

  for (th = cap->cnext; th != cap; th = th->cnext)
  {
    memcpy(save_p, &th->prev, sizeof(th->prev));
    save_p += sizeof(th->prev);
  }
}

void P_ArchiveThinkerSubclasses(void)
{
  // Other subclass ordering is not relevant
  P_ArchiveThinkerSubclass(th_friends);
  P_ArchiveThinkerSubclass(th_enemies);
}

void P_UnArchiveThinkerSubclass(th_class class, mobj_t** mobj_p, int mobj_count)
{
  int i;
  int count;

  // Reset thinker subclass list
  thinkerclasscap[class].cprev->cnext = thinkerclasscap[class].cnext;
  thinkerclasscap[class].cnext->cprev = thinkerclasscap[class].cprev;
  thinkerclasscap[class].cprev =
    thinkerclasscap[class].cnext = &thinkerclasscap[class];

  memcpy(&count, save_p, sizeof(count));
  save_p += sizeof(count);

  for (i = 0; i < count; ++i)
  {
    thinker_t* th;
    mobj_t* mobj;

    memcpy(&mobj, save_p, sizeof(mobj));
    save_p += sizeof(mobj);

    mobj = mobj_p[P_GetMobj(mobj, mobj_count + 1)];

    if (mobj)
    {
      // remove mobj from current subclass list
      th = mobj->thinker.cnext;
      if (th != NULL)
      {
        th->cprev = mobj->thinker.cprev;
        th->cprev->cnext = th;
      }

      th = &thinkerclasscap[class];
      th->cprev->cnext = &mobj->thinker;
      mobj->thinker.cnext = th;
      mobj->thinker.cprev = th->cprev;
      th->cprev = &mobj->thinker;
    }
    else
    {
      I_Error("P_UnArchiveThinkerSubclass: mobj does not exist!\n");
    }
  }
}

void P_UnArchiveThinkerSubclasses(mobj_t** mobj_p, int mobj_count)
{
  P_UnArchiveThinkerSubclass(th_friends, mobj_p, mobj_count);
  P_UnArchiveThinkerSubclass(th_enemies, mobj_p, mobj_count);
}

extern mobj_t** blocklinks;
extern int      bmapwidth;
extern int      bmapheight;

void P_ArchiveBlockLinks(void)
{
  int i;
  int size;

  size = bmapwidth * bmapheight;

  for (i = 0; i < size; ++i)
  {
    int count = 0;
    mobj_t*  mobj;

    mobj = blocklinks[i];
    while (mobj)
    {
      ++count;
      mobj = mobj->bnext;
    }

    CheckSaveGame(count * sizeof(mobj_t*) + sizeof(count));

    memcpy(save_p, &count, sizeof(count));
    save_p += sizeof(count);

    mobj = blocklinks[i];
    while (mobj)
    {
      memcpy(save_p, &mobj->thinker.prev, sizeof(mobj->thinker.prev));
      save_p += sizeof(mobj->thinker.prev);
      mobj = mobj->bnext;
    }
  }
}

void P_UnArchiveBlockLinks(mobj_t** mobj_p, int mobj_count)
{
  int i;
  int size;

  size = bmapwidth * bmapheight;

  for (i = 0; i < size; ++i)
  {
    int j;
    int count;
    mobj_t* mobj;
    mobj_t** bprev;

    memcpy(&count, save_p, sizeof(count));
    save_p += sizeof(count);

    bprev = &blocklinks[i];
    for (j = 0; j < count; ++j)
    {
      memcpy(&mobj, save_p, sizeof(mobj));
      save_p += sizeof(mobj);

      mobj = mobj_p[P_GetMobj(mobj, mobj_count + 1)];

      if (mobj)
      {
        *bprev = mobj;
        mobj->bprev = bprev;
        mobj->bnext = NULL;
        bprev = &mobj->bnext;
      }
      else
      {
        I_Error("P_UnArchiveBlockLinks: mobj does not exist!\n");
      }
    }
  }
}

// dsda - fix save / load synchronization
// merges thinkerclass_t and specials_e
typedef enum {
  tc_true_mobj,
  tc_true_ceiling,
  tc_true_door,
  tc_true_floor,
  tc_true_plat,
  tc_true_flash,
  tc_true_strobe,
  tc_true_glow,
  tc_true_elevator,
  tc_true_scroll,
  tc_true_pusher,
  tc_true_flicker,
  tc_true_friction,
  tc_true_light,
  tc_true_phase,
  tc_true_acs,
  tc_true_pillar,
  tc_true_waggle,
  tc_true_poly_rotate,
  tc_true_poly_move,
  tc_true_poly_door,
  tc_true_end
} true_thinkerclass_t;

// dsda - fix save / load synchronization
// merges P_ArchiveThinkers & P_ArchiveSpecials
void P_TrueArchiveThinkers(void) {
  thinker_t *th;
  size_t    size = 0;          // killough

  CheckSaveGame(sizeof brain);      // killough 3/26/98: Save boss brain state
  memcpy(save_p, &brain, sizeof brain);
  save_p += sizeof brain;

  // save off the current thinkers (memory size calculation -- killough)
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (!th->function)
    {
      platlist_t *pl;
      ceilinglist_t *cl;     //jff 2/22/98 need this for ceilings too now
      for (pl=activeplats; pl; pl=pl->next)
        if (pl->plat == (plat_t *) th)   // killough 2/14/98
          {
            size += 4+sizeof(plat_t);
            goto end;
          }
      for (cl=activeceilings; cl; cl=cl->next) // search for activeceiling
        if (cl->ceiling == (ceiling_t *) th)   //jff 2/22/98
          {
            size += 4+sizeof(ceiling_t);
            goto end;
          }
    end:;
    }
    else
      size +=
        th->function==T_MoveCeiling  ? 4+sizeof(ceiling_t)     :
        th->function==T_VerticalDoor ? 4+sizeof(vldoor_t)      :
        th->function==T_MoveFloor    ? 4+sizeof(floormove_t)   :
        th->function==T_PlatRaise    ? 4+sizeof(plat_t)        :
        th->function==T_LightFlash   ? 4+sizeof(lightflash_t)  :
        th->function==T_StrobeFlash  ? 4+sizeof(strobe_t)      :
        th->function==T_Glow         ? 4+sizeof(glow_t)        :
        th->function==T_MoveElevator ? 4+sizeof(elevator_t)    :
        th->function==T_Scroll       ? 4+sizeof(scroll_t)      :
        th->function==T_Pusher       ? 4+sizeof(pusher_t)      :
        th->function==T_FireFlicker  ? 4+sizeof(fireflicker_t) :
        th->function==T_Friction     ? 4+sizeof(friction_t)    :
        th->function==T_Light        ? 4+sizeof(light_t)       :
        th->function==T_Phase        ? 4+sizeof(phase_t)       :
        th->function==T_InterpretACS ? 4+sizeof(acs_t)         :
        th->function==T_BuildPillar  ? 4+sizeof(pillar_t)      :
        th->function==T_FloorWaggle  ? 4+sizeof(floorWaggle_t) :
        th->function==T_RotatePoly   ? 4+sizeof(polyevent_t)   :
        th->function==T_MovePoly     ? 4+sizeof(polyevent_t)   :
        th->function==T_PolyDoor     ? 4+sizeof(polydoor_t)    :
        P_IsMobjThinker(th)          ? 4+sizeof(mobj_t)        :
      0;

  CheckSaveGame(size + 1);    // killough; cph: +1 for the tc_endspecials

  // save off the current thinkers
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next) {
    if (!th->function)
    {
      platlist_t *pl;
      ceilinglist_t *cl;    //jff 2/22/98 add iter variable for ceilings

      // killough 2/8/98: fix plat original height bug.
      // Since acv==NULL, this could be a plat in stasis.
      // so check the active plats list, and save this
      // plat (jff: or ceiling) even if it is in stasis.

      for (pl=activeplats; pl; pl=pl->next)
        if (pl->plat == (plat_t *) th)      // killough 2/14/98
          goto plat;

      for (cl=activeceilings; cl; cl=cl->next)
        if (cl->ceiling == (ceiling_t *) th)      //jff 2/22/98
          goto ceiling;

      continue;
    }

    if (th->function == T_MoveCeiling)
    {
      ceiling_t *ceiling;
    ceiling:                               // killough 2/14/98
      *save_p++ = tc_true_ceiling;
      ceiling = (ceiling_t *)save_p;
      memcpy (ceiling, th, sizeof(*ceiling));
      save_p += sizeof(*ceiling);
      ceiling->sector = (sector_t *)(intptr_t)(ceiling->sector->iSectorID);
      continue;
    }

    if (th->function == T_VerticalDoor)
    {
      vldoor_t *door;
      *save_p++ = tc_true_door;
      door = (vldoor_t *) save_p;
      memcpy (door, th, sizeof *door);
      save_p += sizeof(*door);
      door->sector = (sector_t *)(intptr_t)(door->sector->iSectorID);
      //jff 1/31/98 archive line remembered by door as well
      door->line = (line_t *) (door->line ? door->line-lines : -1);
      continue;
    }

    if (th->function == T_MoveFloor)
    {
      floormove_t *floor;
      *save_p++ = tc_true_floor;
      floor = (floormove_t *)save_p;
      memcpy (floor, th, sizeof(*floor));
      save_p += sizeof(*floor);
      floor->sector = (sector_t *)(intptr_t)(floor->sector->iSectorID);
      continue;
    }

    if (th->function == T_PlatRaise)
    {
      plat_t *plat;
    plat:   // killough 2/14/98: added fix for original plat height above
      *save_p++ = tc_true_plat;
      plat = (plat_t *)save_p;
      memcpy (plat, th, sizeof(*plat));
      save_p += sizeof(*plat);
      plat->sector = (sector_t *)(intptr_t)(plat->sector->iSectorID);
      continue;
    }

    if (th->function == T_LightFlash)
    {
      lightflash_t *flash;
      *save_p++ = tc_true_flash;
      flash = (lightflash_t *)save_p;
      memcpy (flash, th, sizeof(*flash));
      save_p += sizeof(*flash);
      flash->sector = (sector_t *)(intptr_t)(flash->sector->iSectorID);
      continue;
    }

    if (th->function == T_StrobeFlash)
    {
      strobe_t *strobe;
      *save_p++ = tc_true_strobe;
      strobe = (strobe_t *)save_p;
      memcpy (strobe, th, sizeof(*strobe));
      save_p += sizeof(*strobe);
      strobe->sector = (sector_t *)(intptr_t)(strobe->sector->iSectorID);
      continue;
    }

    if (th->function == T_Glow)
    {
      glow_t *glow;
      *save_p++ = tc_true_glow;
      glow = (glow_t *)save_p;
      memcpy (glow, th, sizeof(*glow));
      save_p += sizeof(*glow);
      glow->sector = (sector_t *)(intptr_t)(glow->sector->iSectorID);
      continue;
    }

    // killough 10/4/98: save flickers
    if (th->function == T_FireFlicker)
    {
      fireflicker_t *flicker;
      *save_p++ = tc_true_flicker;
      flicker = (fireflicker_t *)save_p;
      memcpy (flicker, th, sizeof(*flicker));
      save_p += sizeof(*flicker);
      flicker->sector = (sector_t *)(intptr_t)(flicker->sector->iSectorID);
      continue;
    }

    //jff 2/22/98 new case for elevators
    if (th->function == T_MoveElevator)
    {
      elevator_t *elevator;         //jff 2/22/98
      *save_p++ = tc_true_elevator;
      elevator = (elevator_t *)save_p;
      memcpy (elevator, th, sizeof(*elevator));
      save_p += sizeof(*elevator);
      elevator->sector = (sector_t *)(intptr_t)(elevator->sector->iSectorID);
      continue;
    }

    // killough 3/7/98: Scroll effect thinkers
    if (th->function == T_Scroll)
    {
      *save_p++ = tc_true_scroll;
      memcpy (save_p, th, sizeof(scroll_t));
      save_p += sizeof(scroll_t);
      continue;
    }

    // phares 3/22/98: Push/Pull effect thinkers

    if (th->function == T_Pusher)
    {
      *save_p++ = tc_true_pusher;
      memcpy (save_p, th, sizeof(pusher_t));
      save_p += sizeof(pusher_t);
      continue;
    }

    if (th->function == T_Friction)
    {
      *save_p++ = tc_true_friction;
      memcpy (save_p, th, sizeof(friction_t));
      save_p += sizeof(friction_t);
      continue;
    }

    if (th->function == T_Light)
    {
      light_t *light;
      *save_p++ = tc_true_light;
      light = (light_t *)save_p;
      memcpy (save_p, th, sizeof(light_t));
      save_p += sizeof(light_t);
      light->sector = (sector_t *)(intptr_t)(light->sector->iSectorID);
      continue;
    }

    if (th->function == T_Phase)
    {
      phase_t *phase;
      *save_p++ = tc_true_phase;
      phase = (phase_t *)save_p;
      memcpy (save_p, th, sizeof(phase_t));
      save_p += sizeof(phase_t);
      phase->sector = (sector_t *)(intptr_t)(phase->sector->iSectorID);
      continue;
    }

    if (th->function == T_InterpretACS)
    {
      acs_t *acs;
      *save_p++ = tc_true_acs;
      acs = (acs_t *)save_p;
      memcpy (save_p, th, sizeof(acs_t));
      save_p += sizeof(acs_t);

      P_ReplaceMobjWithIndex(&acs->activator);
      acs->line = (line_t *) (acs->line ? acs->line - lines : -1);

      continue;
    }

    if (th->function == T_BuildPillar)
    {
      pillar_t *pillar;
      *save_p++ = tc_true_pillar;
      pillar = (pillar_t *)save_p;
      memcpy (save_p, th, sizeof(pillar_t));
      save_p += sizeof(pillar_t);
      pillar->sector = (sector_t *)(intptr_t)(pillar->sector->iSectorID);
      continue;
    }

    if (th->function == T_FloorWaggle)
    {
      floorWaggle_t *floor_waggle;
      *save_p++ = tc_true_waggle;
      floor_waggle = (floorWaggle_t *)save_p;
      memcpy (save_p, th, sizeof(floorWaggle_t));
      save_p += sizeof(floorWaggle_t);
      floor_waggle->sector = (sector_t *)(intptr_t)(floor_waggle->sector->iSectorID);
      continue;
    }

    if (th->function == T_RotatePoly)
    {
      *save_p++ = tc_true_poly_rotate;
      memcpy (save_p, th, sizeof(polyevent_t));
      save_p += sizeof(polyevent_t);
      continue;
    }

    if (th->function == T_MovePoly)
    {
      *save_p++ = tc_true_poly_move;
      memcpy (save_p, th, sizeof(polyevent_t));
      save_p += sizeof(polyevent_t);
      continue;
    }

    if (th->function == T_PolyDoor)
    {
      *save_p++ = tc_true_poly_door;
      memcpy (save_p, th, sizeof(polydoor_t));
      save_p += sizeof(polydoor_t);
      continue;
    }

    if (P_IsMobjThinker(th))
    {
      mobj_t *mobj;

      *save_p++ = tc_true_mobj;
      mobj = (mobj_t *)save_p;

      //e6y
      memcpy (mobj, th, sizeof(*mobj));
      save_p += sizeof(*mobj);

      mobj->state = (state_t *)(mobj->state - states);

      // Example:
      // - Archvile is attacking a lost soul
      // - The lost soul dies before the attack hits
      // - The lost soul is marked for deletion
      // - The archvile will still attack the spot where the lost soul was
      // - We need to save such objects and remember they are marked for deletion
      if (mobj->thinker.function == P_RemoveThinkerDelayed)
        mobj->index = MARKED_FOR_DELETION;

      // killough 2/14/98: convert pointers into indices.
      // Fixes many savegame problems, by properly saving
      // target and tracer fields. Note: we store NULL if
      // the thinker pointed to by these fields is not a
      // mobj thinker.

      P_ReplaceMobjWithIndex(&mobj->target);
      P_ReplaceMobjWithIndex(&mobj->tracer);

      // killough 2/14/98: new field: save last known enemy. Prevents
      // monsters from going to sleep after killing monsters and not
      // seeing player anymore.

      P_ReplaceMobjWithIndex(&mobj->lastenemy);

      // killough 2/14/98: end changes

      if (raven)
      {
        P_ReplaceMobjWithIndex(&mobj->special1.m);
        P_ReplaceMobjWithIndex(&mobj->special2.m);
      }

      if (mobj->player)
        mobj->player = (player_t *)((mobj->player-players) + 1);
    }
  }

  // add a terminating marker
  *save_p++ = tc_true_end;

  // killough 9/14/98: save soundtargets
  {
    int i;
    CheckSaveGame(numsectors * sizeof(mobj_t *));       // killough 9/14/98
    for (i = 0; i < numsectors; i++)
    {
      mobj_t *target = sectors[i].soundtarget;
      // Fix crash on reload when a soundtarget points to a removed corpse
      // (prboom bug #1590350)
      P_ReplaceMobjWithIndex(&target);
      memcpy(save_p, &target, sizeof target);
      save_p += sizeof target;
    }
  }

  P_ArchiveBlockLinks();
  P_ArchiveThinkerSubclasses();

  dsda_ArchiveMSecNodes();
}

// dsda - fix save / load synchronization
// merges P_UnArchiveThinkers & P_UnArchiveSpecials
void P_TrueUnArchiveThinkers(void) {
  thinker_t *th;
  mobj_t    **mobj_p;    // killough 2/14/98: Translation table
  int    mobj_count;        // killough 2/14/98: size of or index into table
  true_thinkerclass_t tc;

  totallive = 0;
  ClearThingsHealthTracers();

  // killough 3/26/98: Load boss brain state
  memcpy(&brain, save_p, sizeof brain);
  save_p += sizeof brain;

  // remove all the current thinkers
  for (th = thinkercap.next; th != &thinkercap; )
  {
    thinker_t *next = th->next;
    if (P_IsMobjThinker(th))
    {
      P_RemoveMobj ((mobj_t *) th);
      P_RemoveThinkerDelayed(th); // fix mobj leak
    }
    else
      Z_Free (th);
    th = next;
  }
  P_InitThinkers ();

  // killough 2/14/98: count number of thinkers by skipping through them
  {
    byte *sp;     // save pointer and skip header

    sp = save_p;
    mobj_count = 0;

    while ((tc = *save_p++) != tc_true_end)
    {
      if (tc == tc_true_mobj) mobj_count++;
      save_p +=
        tc == tc_true_ceiling     ? sizeof(ceiling_t)     :
        tc == tc_true_door        ? sizeof(vldoor_t)      :
        tc == tc_true_floor       ? sizeof(floormove_t)   :
        tc == tc_true_plat        ? sizeof(plat_t)        :
        tc == tc_true_flash       ? sizeof(lightflash_t)  :
        tc == tc_true_strobe      ? sizeof(strobe_t)      :
        tc == tc_true_glow        ? sizeof(glow_t)        :
        tc == tc_true_elevator    ? sizeof(elevator_t)    :
        tc == tc_true_scroll      ? sizeof(scroll_t)      :
        tc == tc_true_pusher      ? sizeof(pusher_t)      :
        tc == tc_true_flicker     ? sizeof(fireflicker_t) :
        tc == tc_true_friction    ? sizeof(friction_t)    :
        tc == tc_true_light       ? sizeof(light_t)       :
        tc == tc_true_phase       ? sizeof(phase_t)       :
        tc == tc_true_acs         ? sizeof(acs_t)         :
        tc == tc_true_pillar      ? sizeof(pillar_t)      :
        tc == tc_true_waggle      ? sizeof(floorWaggle_t) :
        tc == tc_true_poly_rotate ? sizeof(polyevent_t)   :
        tc == tc_true_poly_move   ? sizeof(polyevent_t)   :
        tc == tc_true_poly_door   ? sizeof(polydoor_t)    :
        tc == tc_true_mobj        ? sizeof(mobj_t)        :
      0;
    }

    if (*--save_p != tc_true_end)
      I_Error ("P_TrueUnArchiveThinkers: Unknown tc %i in size calculation", *save_p);

    // first table entry special: 0 maps to NULL
    *(mobj_p = malloc((mobj_count + 1) * sizeof *mobj_p)) = 0;   // table of pointers
    save_p = sp;           // restore save pointer
  }

  // read in saved thinkers
  mobj_count = 0;
  while ((tc = *save_p++) != tc_true_end)
    switch (tc) {
      case tc_true_ceiling:
        {
          ceiling_t *ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVEL, NULL);
          memcpy (ceiling, save_p, sizeof(*ceiling));
          save_p += sizeof(*ceiling);
          ceiling->sector = &sectors[(size_t)ceiling->sector];
          ceiling->sector->ceilingdata = ceiling; //jff 2/22/98

          if (ceiling->thinker.function)
            ceiling->thinker.function = T_MoveCeiling;

          P_AddThinker (&ceiling->thinker);
          P_AddActiveCeiling(ceiling);
          break;
        }

      case tc_true_door:
        {
          vldoor_t *door = Z_Malloc (sizeof(*door), PU_LEVEL, NULL);
          memcpy (door, save_p, sizeof(*door));
          save_p += sizeof(*door);
          door->sector = &sectors[(size_t)door->sector];

          //jff 1/31/98 unarchive line remembered by door as well
          door->line = (intptr_t)door->line!=-1? &lines[(size_t)door->line] : NULL;

          door->sector->ceilingdata = door;       //jff 2/22/98
          door->thinker.function = T_VerticalDoor;
          P_AddThinker (&door->thinker);
          break;
        }

      case tc_true_floor:
        {
          floormove_t *floor = Z_Malloc (sizeof(*floor), PU_LEVEL, NULL);
          memcpy (floor, save_p, sizeof(*floor));
          save_p += sizeof(*floor);
          floor->sector = &sectors[(size_t)floor->sector];
          floor->sector->floordata = floor; //jff 2/22/98
          floor->thinker.function = T_MoveFloor;
          P_AddThinker (&floor->thinker);
          break;
        }

      case tc_true_plat:
        {
          plat_t *plat = Z_Malloc (sizeof(*plat), PU_LEVEL, NULL);
          memcpy (plat, save_p, sizeof(*plat));
          save_p += sizeof(*plat);
          plat->sector = &sectors[(size_t)plat->sector];
          plat->sector->floordata = plat; //jff 2/22/98

          if (plat->thinker.function)
            plat->thinker.function = T_PlatRaise;

          P_AddThinker (&plat->thinker);
          P_AddActivePlat(plat);
          break;
        }

      case tc_true_flash:
        {
          lightflash_t *flash = Z_Malloc (sizeof(*flash), PU_LEVEL, NULL);
          memcpy (flash, save_p, sizeof(*flash));
          save_p += sizeof(*flash);
          flash->sector = &sectors[(size_t)flash->sector];
          flash->thinker.function = T_LightFlash;
          P_AddThinker (&flash->thinker);
          break;
        }

      case tc_true_strobe:
        {
          strobe_t *strobe = Z_Malloc (sizeof(*strobe), PU_LEVEL, NULL);
          memcpy (strobe, save_p, sizeof(*strobe));
          save_p += sizeof(*strobe);
          strobe->sector = &sectors[(size_t)strobe->sector];
          strobe->thinker.function = T_StrobeFlash;
          P_AddThinker (&strobe->thinker);
          break;
        }

      case tc_true_glow:
        {
          glow_t *glow = Z_Malloc (sizeof(*glow), PU_LEVEL, NULL);
          memcpy (glow, save_p, sizeof(*glow));
          save_p += sizeof(*glow);
          glow->sector = &sectors[(size_t)glow->sector];
          glow->thinker.function = T_Glow;
          P_AddThinker (&glow->thinker);
          break;
        }

      case tc_true_flicker:           // killough 10/4/98
        {
          fireflicker_t *flicker = Z_Malloc (sizeof(*flicker), PU_LEVEL, NULL);
          memcpy (flicker, save_p, sizeof(*flicker));
          save_p += sizeof(*flicker);
          flicker->sector = &sectors[(size_t)flicker->sector];
          flicker->thinker.function = T_FireFlicker;
          P_AddThinker (&flicker->thinker);
          break;
        }

        //jff 2/22/98 new case for elevators
      case tc_true_elevator:
        {
          elevator_t *elevator = Z_Malloc (sizeof(*elevator), PU_LEVEL, NULL);
          memcpy (elevator, save_p, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = &sectors[(size_t)elevator->sector];
          elevator->sector->floordata = elevator; //jff 2/22/98
          elevator->sector->ceilingdata = elevator; //jff 2/22/98
          elevator->thinker.function = T_MoveElevator;
          P_AddThinker (&elevator->thinker);
          break;
        }

      case tc_true_scroll:       // killough 3/7/98: scroll effect thinkers
        {
          scroll_t *scroll = Z_Malloc (sizeof(scroll_t), PU_LEVEL, NULL);
          memcpy (scroll, save_p, sizeof(scroll_t));
          save_p += sizeof(scroll_t);
          scroll->thinker.function = T_Scroll;
          P_AddThinker(&scroll->thinker);
          break;
        }

      case tc_true_pusher:   // phares 3/22/98: new Push/Pull effect thinkers
        {
          pusher_t *pusher = Z_Malloc (sizeof(pusher_t), PU_LEVEL, NULL);
          memcpy (pusher, save_p, sizeof(pusher_t));
          save_p += sizeof(pusher_t);
          pusher->thinker.function = T_Pusher;
          pusher->source = P_GetPushThing(pusher->affectee);
          P_AddThinker(&pusher->thinker);
          break;
        }

      case tc_true_friction:
        {
          friction_t *friction = Z_Malloc (sizeof(friction_t), PU_LEVEL, NULL);
          memcpy (friction, save_p, sizeof(friction_t));
          save_p += sizeof(friction_t);
          friction->thinker.function = T_Friction;
          P_AddThinker(&friction->thinker);
          break;
        }

      case tc_true_light:
        {
          light_t *light = Z_Malloc(sizeof(*light), PU_LEVEL, NULL);
          memcpy(light, save_p, sizeof(*light));
          save_p += sizeof(*light);
          light->sector = &sectors[(size_t)light->sector];
          light->thinker.function = T_Light;
          P_AddThinker(&light->thinker);
          break;
        }

      case tc_true_phase:
        {
          phase_t *phase = Z_Malloc(sizeof(*phase), PU_LEVEL, NULL);
          memcpy(phase, save_p, sizeof(*phase));
          save_p += sizeof(*phase);
          phase->sector = &sectors[(size_t)phase->sector];
          phase->thinker.function = T_Phase;
          P_AddThinker(&phase->thinker);
          break;
        }

      case tc_true_acs:
        {
          acs_t *acs = Z_Malloc(sizeof(*acs), PU_LEVEL, NULL);
          memcpy(acs, save_p, sizeof(*acs));
          save_p += sizeof(*acs);
          acs->line = (intptr_t) acs->line != -1 ? &lines[(size_t) acs->line] : NULL;
          acs->thinker.function = T_InterpretACS;
          P_AddThinker(&acs->thinker);
          break;
        }

      case tc_true_pillar:
        {
          pillar_t *pillar = Z_Malloc(sizeof(*pillar), PU_LEVEL, NULL);
          memcpy(pillar, save_p, sizeof(*pillar));
          save_p += sizeof(*pillar);
          pillar->sector = &sectors[(size_t)pillar->sector];
          pillar->sector->floordata = pillar;
          pillar->thinker.function = T_BuildPillar;
          P_AddThinker(&pillar->thinker);
          break;
        }

      case tc_true_waggle:
        {
          floorWaggle_t *waggle = Z_Malloc(sizeof(*waggle), PU_LEVEL, NULL);
          memcpy(waggle, save_p, sizeof(*waggle));
          save_p += sizeof(*waggle);
          waggle->sector = &sectors[(size_t)waggle->sector];
          waggle->sector->floordata = waggle;
          waggle->thinker.function = T_FloorWaggle;
          P_AddThinker(&waggle->thinker);
          break;
        }

      case tc_true_poly_rotate:
        {
          polyevent_t *poly = Z_Malloc(sizeof(*poly), PU_LEVEL, NULL);
          memcpy(poly, save_p, sizeof(*poly));
          save_p += sizeof(*poly);
          poly->thinker.function = T_RotatePoly;
          P_AddThinker(&poly->thinker);
          break;
        }

      case tc_true_poly_move:
        {
          polyevent_t *poly = Z_Malloc(sizeof(*poly), PU_LEVEL, NULL);
          memcpy(poly, save_p, sizeof(*poly));
          save_p += sizeof(*poly);
          poly->thinker.function = T_MovePoly;
          P_AddThinker(&poly->thinker);
          break;
        }

      case tc_true_poly_door:
        {
          polydoor_t *poly = Z_Malloc(sizeof(*poly), PU_LEVEL, NULL);
          memcpy(poly, save_p, sizeof(*poly));
          save_p += sizeof(*poly);
          poly->thinker.function = T_PolyDoor;
          P_AddThinker(&poly->thinker);
          break;
        }

      case tc_true_mobj:
        {
          mobj_t *mobj = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL);

          // killough 2/14/98 -- insert pointers to thinkers into table, in order:
          mobj_count++;
          mobj_p[mobj_count] = mobj;

          memcpy (mobj, save_p, sizeof(mobj_t));
          save_p += sizeof(mobj_t);

          mobj->state = states + (intptr_t) mobj->state;

          if (mobj->player)
            (mobj->player = &players[(size_t) mobj->player - 1]) -> mo = mobj;

          mobj->info = &mobjinfo[mobj->type];

          // Don't place objects marked for deletion
          if (mobj->index == MARKED_FOR_DELETION)
          {
            mobj->thinker.function = P_RemoveThinkerDelayed;
            P_AddThinker(&mobj->thinker);

            // The references value must be nonzero to reach the target code
            mobj->thinker.references = 1;
            break;
          }
          else
          {
            InitThingsHealthTracer(mobj);
          }

          P_SetThingPosition (mobj);

          // killough 2/28/98:
          // Fix for falling down into a wall after savegame loaded:
          //      mobj->floorz = mobj->subsector->sector->floorheight;
          //      mobj->ceilingz = mobj->subsector->sector->ceilingheight;

          mobj->thinker.function = P_MobjThinker;
          P_AddThinker (&mobj->thinker);

          if (mobj->type == HERETIC_MT_BLASTERFX1)
            mobj->thinker.function = P_BlasterMobjThinker;

          if (!((mobj->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL | MF_CORPSE)))
            totallive++;
          break;
        }

      default:
        I_Error("P_TrueUnarchiveSpecials: Unknown tc %i in extraction", tc);
    }

  // killough 2/14/98: adjust target and tracer fields, plus
  // lastenemy field, to correctly point to mobj thinkers.
  // NULL entries automatically handled by first table entry.
  //
  // killough 11/98: use P_SetNewTarget() to set fields

  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
  {
    if (th->function == T_InterpretACS)
    {
      P_ReplaceIndexWithMobj(&((acs_t *) th)->activator, mobj_p, mobj_count);
    }
    else if (P_IsMobjThinker(th))
    {
      P_ReplaceIndexWithMobj(&((mobj_t *) th)->target, mobj_p, mobj_count);
      P_ReplaceIndexWithMobj(&((mobj_t *) th)->tracer, mobj_p, mobj_count);
      P_ReplaceIndexWithMobj(&((mobj_t *) th)->lastenemy, mobj_p, mobj_count);

      if (raven)
      {
        P_ReplaceIndexWithMobj(&((mobj_t *) th)->special1.m, mobj_p, mobj_count);
        P_ReplaceIndexWithMobj(&((mobj_t *) th)->special2.m, mobj_p, mobj_count);
      }

      // restore references now that targets are set
      if (((mobj_t *) th)->index == MARKED_FOR_DELETION)
      {
        ((mobj_t *) th)->index = -1;
        th->references--;
      }
    }
  }

  {  // killough 9/14/98: restore soundtargets
    int i;
    for (i = 0; i < numsectors; i++)
    {
      memcpy(&sectors[i].soundtarget, save_p, sizeof sectors[i].soundtarget);
      save_p += sizeof sectors[i].soundtarget;
      // Must verify soundtarget. See P_TrueArchiveThinkers.
      P_ReplaceIndexWithMobj(&sectors[i].soundtarget, mobj_p, mobj_count);
    }
  }

  P_UnArchiveBlockLinks(mobj_p, mobj_count);
  P_UnArchiveThinkerSubclasses(mobj_p, mobj_count);

  dsda_UnArchiveMSecNodes(mobj_p, mobj_count);

  free(mobj_p);    // free translation table

  if (hexen)
  {
    P_CreateTIDList();
    P_InitCreatureCorpseQueue(true);    // true = scan for corpses
  }

  // killough 3/26/98: Spawn icon landings:
  if (gamemode == commercial && !hexen)
  {
    // P_SpawnBrainTargets overwrites brain.targeton and brain.easy with zero.
    struct brain_s brain_tmp = brain; // saving

    P_SpawnBrainTargets();

    // old demos with save/load tics should not be affected by this fix
    if (!prboom_comp[PC_RESET_MONSTERSPAWNER_PARAMS_AFTER_LOADING].state)
    {
      brain = brain_tmp; // restoring
    }
  }
}

// hexen

void P_ArchiveACS(void)
{
  size_t size;

  if (!hexen) return;

  size = sizeof(*WorldVars) * MAX_ACS_WORLD_VARS;
  CheckSaveGame(size);
  memcpy(save_p, WorldVars, size);
  save_p += size;

  size = sizeof(*ACSStore) * (MAX_ACS_STORE + 1);
  CheckSaveGame(size);
  memcpy(save_p, ACSStore, size);
  save_p += size;
}

void P_UnArchiveACS(void)
{
  size_t size;

  if (!hexen) return;

  size = sizeof(*WorldVars) * MAX_ACS_WORLD_VARS;
  memcpy(WorldVars, save_p, size);
  save_p += size;

  size = sizeof(*ACSStore) * (MAX_ACS_STORE + 1);
  memcpy(ACSStore, save_p, size);
  save_p += size;
}

void P_ArchivePolyobjs(void)
{
  int i;

  if (!hexen) return;

  CheckSaveGame(po_NumPolyobjs * (sizeof(angle_t) + 2 * sizeof(fixed_t)));

  for (i = 0; i < po_NumPolyobjs; i++)
  {
    memcpy(save_p, &polyobjs[i].angle, sizeof(polyobjs[i].angle));
    save_p += sizeof(polyobjs[i].angle);

    memcpy(save_p, &polyobjs[i].startSpot.x, sizeof(polyobjs[i].startSpot.x));
    save_p += sizeof(polyobjs[i].startSpot.x);

    memcpy(save_p, &polyobjs[i].startSpot.y, sizeof(polyobjs[i].startSpot.y));
    save_p += sizeof(polyobjs[i].startSpot.y);
  }
}

void P_UnArchivePolyobjs(void)
{
  int i;
  angle_t angle;
  fixed_t deltaX;
  fixed_t deltaY;

  if (!hexen) return;

  for (i = 0; i < po_NumPolyobjs; i++)
  {
    memcpy(&angle, save_p, sizeof(angle));
    save_p += sizeof(angle);

    memcpy(&deltaX, save_p, sizeof(deltaX));
    save_p += sizeof(deltaX);

    memcpy(&deltaY, save_p, sizeof(deltaY));
    save_p += sizeof(deltaY);

    PO_RotatePolyobj(polyobjs[i].tag, angle);
    deltaX -= polyobjs[i].startSpot.x;
    deltaY -= polyobjs[i].startSpot.y;
    PO_MovePolyobj(polyobjs[i].tag, deltaX, deltaY);
  }
}

void P_ArchiveScripts(void)
{
  size_t size;

  if (!hexen) return;

  size = sizeof(*ACSInfo) * ACScriptCount;
  CheckSaveGame(size);
  memcpy(save_p, ACSInfo, size);
  save_p += size;

  size = sizeof(*MapVars) * MAX_ACS_MAP_VARS;
  CheckSaveGame(size);
  memcpy(save_p, MapVars, size);
  save_p += size;
}

void P_UnArchiveScripts(void)
{
  size_t size;

  if (!hexen) return;

  size = sizeof(*ACSInfo) * ACScriptCount;
  memcpy(ACSInfo, save_p, size);
  save_p += size;

  size = sizeof(*MapVars) * MAX_ACS_MAP_VARS;
  memcpy(MapVars, save_p, size);
  save_p += size;
}

void P_ArchiveSounds(void)
{
  seqnode_t *node;
  sector_t *sec;
  int difference;
  int i;

  if (!hexen) return;

  CheckSaveGame(sizeof(ActiveSequences) + ActiveSequences * (6 * sizeof(int) + 1));

  memcpy(save_p, &ActiveSequences, sizeof(ActiveSequences));
  save_p += sizeof(ActiveSequences);

  for (node = SequenceListHead; node; node = node->next)
  {
    memcpy(save_p, &node->sequence, sizeof(node->sequence));
    save_p += sizeof(node->sequence);

    memcpy(save_p, &node->delayTics, sizeof(node->delayTics));
    save_p += sizeof(node->delayTics);

    memcpy(save_p, &node->volume, sizeof(node->volume));
    save_p += sizeof(node->volume);

    difference = SN_GetSequenceOffset(node->sequence, node->sequencePtr);
    memcpy(save_p, &difference, sizeof(difference));
    save_p += sizeof(difference);

    memcpy(save_p, &node->currentSoundID, sizeof(node->currentSoundID));
    save_p += sizeof(node->currentSoundID);

    for (i = 0; i < po_NumPolyobjs; i++)
    {
      if (node->mobj == (mobj_t *) &polyobjs[i].startSpot)
      {
        break;
      }
    }

    if (i == po_NumPolyobjs)
    {                       // Sound is attached to a sector, not a polyobj
      sec = R_PointInSubsector(node->mobj->x, node->mobj->y)->sector;
      difference = (int) (sec - sectors);
      *save_p++ = 0;   // 0 -- sector sound origin
    }
    else
    {
      difference = i;
      *save_p++ = 1;   // 1 -- polyobj sound origin
    }

    memcpy(save_p, &difference, sizeof(difference));
    save_p += sizeof(difference);
  }
}

void P_UnArchiveSounds(void)
{
  int i;
  int numSequences;
  int sequence;
  int delayTics;
  int volume;
  int seqOffset;
  int soundID;
  byte polySnd;
  int secNum;
  mobj_t *sndMobj;

  if (!hexen) return;

  memcpy(&numSequences, save_p, sizeof(numSequences));
  save_p += sizeof(numSequences);

  i = 0;
  while (i < numSequences)
  {
    memcpy(&sequence, save_p, sizeof(sequence));
    save_p += sizeof(sequence);

    memcpy(&delayTics, save_p, sizeof(delayTics));
    save_p += sizeof(delayTics);

    memcpy(&volume, save_p, sizeof(volume));
    save_p += sizeof(volume);

    memcpy(&seqOffset, save_p, sizeof(seqOffset));
    save_p += sizeof(seqOffset);

    memcpy(&soundID, save_p, sizeof(soundID));
    save_p += sizeof(soundID);

    polySnd = *save_p++;

    memcpy(&secNum, save_p, sizeof(secNum));
    save_p += sizeof(secNum);

    if (!polySnd)
    {
      sndMobj = (mobj_t *) &sectors[secNum].soundorg;
    }
    else
    {
      sndMobj = (mobj_t *) &polyobjs[secNum].startSpot;
    }
    SN_StartSequence(sndMobj, sequence);
    SN_ChangeNodeData(i, seqOffset, delayTics, volume, soundID);
    i++;
  }
}

void P_ArchiveMisc(void)
{
  size_t size;

  if (!hexen) return;

  size = sizeof(*localQuakeHappening) * MAX_MAXPLAYERS;
  CheckSaveGame(size);
  memcpy(save_p, localQuakeHappening, size);
  save_p += size;
}

void P_UnArchiveMisc(void)
{
  size_t size;

  if (!hexen) return;

  size = sizeof(*localQuakeHappening) * MAX_MAXPLAYERS;
  memcpy(localQuakeHappening, save_p, size);
  save_p += size;
}
