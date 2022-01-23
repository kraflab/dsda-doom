//
// Copyright(C) 2022 by Ryan Krafnick
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
// DESCRIPTION:
//  DSDA MapInfo U
//

#include "doomstat.h"
#include "g_game.h"
#include "p_spec.h"
#include "p_tick.h"
#include "r_state.h"
#include "w_wad.h"

#include "dsda/global.h"
#include "dsda/map_format.h"
#include "dsda/mapinfo.h"

#include "u.h"

struct MapEntry* gamemapinfo;

static struct MapEntry* dsda_UMapEntry(int gameepisode, int gamemap)
{
  char lumpname[9];
  unsigned i;

  if (gamemode == commercial)
    snprintf(lumpname, 9, "MAP%02d", gamemap);
  else
    snprintf(lumpname, 9, "E%dM%d", gameepisode, gamemap);

  for (i = 0; i < Maps.mapcount; i++)
    if (!stricmp(lumpname, Maps.maps[i].mapname))
      return &Maps.maps[i];

  return NULL;
}

int dsda_UNextMap(int* episode, int* map) {
  const char *name = NULL;

  if (!gamemapinfo)
    return false;

  if (gamemapinfo->nextsecret[0])
    name = gamemapinfo->nextsecret;
  else if (gamemapinfo->nextmap[0])
    name = gamemapinfo->nextmap;
  else if (gamemapinfo->endpic[0] && gamemapinfo->endpic[0] != '-')
  {
    *episode = 1;
    *map = 1;

    return true;
  }

  if (name)
    return dsda_NameToMap(name, episode, map);

  return false;
}

int dsda_UShowNextLocBehaviour(int* behaviour) {
  if (!gamemapinfo)
    return false;

  if (gamemapinfo->endpic[0])
    *behaviour = WI_SHOW_NEXT_DONE;
  else
    *behaviour = WI_SHOW_NEXT_LOC | WI_SHOW_NEXT_EPISODAL;

  return true;
}

int dsda_USkipDrawShowNextLoc(int* skip) {
  if (!gamemapinfo)
    return false;

  *skip = (gamemapinfo->endpic[0] && strcmp(gamemapinfo->endpic, "-") != 0);

  return true;
}

void dsda_UUpdateMapInfo(void) {
  gamemapinfo = dsda_UMapEntry(gameepisode, gamemap);
}

void dsda_UUpdateLastMapInfo(void) {
  wminfo.lastmapinfo = gamemapinfo;
  wminfo.nextmapinfo = NULL;
}

void dsda_UUpdateNextMapInfo(void) {
  wminfo.nextmapinfo = dsda_UMapEntry(wminfo.nextep + 1, wminfo.next + 1);
}

int dsda_UResolveCLEV(int* clev, int* episode, int* map) {
  if (dsda_UMapEntry(*episode, *map)) {
    *clev = true;

    return true;
  }

  return false;
}

int dsda_UMapMusic(int* music_index, int* music_lump) {
  int lump;

  if (!gamemapinfo)
    return false;

  if (!gamemapinfo->music[0])
    return false;

  lump = W_CheckNumForName(gamemapinfo->music);

  if (lump < 0)
    return false;

  *music_index = -1;
  *music_lump = lump;

  return true;
}

int dsda_UInterMusic(int* music_index, int* music_lump) {
  int lump;

  if (!gamemapinfo)
    return false;

  if (!gamemapinfo->intermusic[0])
    return false;

  lump = W_CheckNumForName(gamemapinfo->intermusic);

  if (lump < 0)
    return false;

  *music_index = -1;
  *music_lump = lump;

  return true;
}

int dsda_UStartFinale(void) {
  void FMI_StartFinale(void);

  if (!gamemapinfo)
    return false;

  FMI_StartFinale();

  return true;
}

int dsda_UFTicker(void) {
  int FMI_Ticker(void);

  return FMI_Ticker();
}

void dsda_UFDrawer(void) {
  void FMI_Drawer(void);

  FMI_Drawer();
}

// numbossactions == 0 means to use the defaults.
// numbossactions == -1 means to do nothing.
// positive values mean to check the list of boss actions and run all that apply.
int dsda_UBossAction(mobj_t* mo) {
  int i;
  line_t junk;
  thinker_t* th;

  if (!gamemapinfo || !gamemapinfo->numbossactions)
    return false;

  if (gamemapinfo->numbossactions < 0)
    return true;

  // make sure there is a player alive for victory
  for (i = 0; i < g_maxplayers; i++)
    if (playeringame[i] && players[i].health > 0)
      break;

  if (i == g_maxplayers)
    return true; // no one left alive, so do not end game

  for (i = 0; i < gamemapinfo->numbossactions; i++)
    if (gamemapinfo->bossactions[i].type == mo->type)
      break;

  if (i >= gamemapinfo->numbossactions)
    return true; // no matches found

  // scan the remaining thinkers to see
  // if all bosses are dead
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    if (th->function == P_MobjThinker) {
      mobj_t* mo2 = (mobj_t*) th;

      if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
        return true; // other boss not dead
    }

  for (i = 0; i < gamemapinfo->numbossactions; i++) {
    if (gamemapinfo->bossactions[i].type == mo->type) {
      junk = *lines;
      junk.special = (short) gamemapinfo->bossactions[i].special;
      junk.tag = (short) gamemapinfo->bossactions[i].tag;

      // use special semantics for line activation to block problem types.
      if (!P_UseSpecialLine(mo, &junk, 0, true))
        map_format.cross_special_line(&junk, 0, mo, true);
    }
  }

  return true;
}

int dsda_UHUTitle(const char** title) {
  void HU_AddCharToTitle(char s);

  const char* s;

  if (!gamemapinfo || !gamemapinfo->levelname)
    return false;

  if (gamemapinfo->label)
    s = gamemapinfo->label;
  else
    s = gamemapinfo->mapname;

  if (s == gamemapinfo->mapname || strcmp(s, "-") != 0) {
    while (*s)
      HU_AddCharToTitle(*(s++));

    HU_AddCharToTitle(':');
    HU_AddCharToTitle(' ');
  }

  *title = gamemapinfo->levelname;

  return true;
}
