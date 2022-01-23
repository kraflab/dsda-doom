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
//	DSDA MapInfo
//

#include <stdio.h>
#include <string.h>

#include "doomstat.h"
#include "m_misc.h"

#include "dsda/mapinfo/u.h"
#include "dsda/mapinfo/legacy.h"

#include "mapinfo.h"

int dsda_NameToMap(const char* name, int* episode, int* map) {
  char name_upper[9];
  int episode_from_name = -1;
  int map_from_name = -1;

  if (strlen(name) > 8)
    return false;

  strncpy(name_upper, name, 8);
  name_upper[8] = 0;
  M_Strupr(name_upper);

  if (gamemode != commercial) {
    if (sscanf(name_upper, "E%dM%d", &episode_from_name, &map_from_name) != 2)
      return false;
  }
  else {
    if (sscanf(name_upper, "MAP%d", &map_from_name) != 1)
      return false;

    episode_from_name = 1;
  }

  *episode = episode_from_name;
  *map = map_from_name;

  return true;
}

void dsda_NextMap(int* episode, int* map) {
  if (dsda_UNextMap(episode, map))
    return;

  dsda_LegacyNextMap(episode, map);
}

void dsda_ShowNextLocBehaviour(int* behaviour) {
  if (dsda_UShowNextLocBehaviour(behaviour))
    return;

  dsda_LegacyShowNextLocBehaviour(behaviour);
}

int dsda_SkipDrawShowNextLoc(void) {
  int skip;

  if (dsda_USkipDrawShowNextLoc(&skip))
    return skip;

  dsda_LegacySkipDrawShowNextLoc(&skip);

  return skip;
}

void dsda_UpdateMapInfo(void) {
  dsda_UUpdateMapInfo();
  dsda_LegacyUpdateMapInfo();
}

void dsda_UpdateLastMapInfo(void) {
  dsda_UUpdateLastMapInfo();
  dsda_LegacyUpdateLastMapInfo();
}

void dsda_UpdateNextMapInfo(void) {
  dsda_UUpdateNextMapInfo();
  dsda_LegacyUpdateNextMapInfo();
}

int dsda_ResolveCLEV(int* episode, int* map) {
  int clev;

  if (dsda_UResolveCLEV(&clev, episode, map))
    return clev;

  dsda_LegacyResolveCLEV(&clev, episode, map);

  return clev;
}

void dsda_MapMusic(int* music_index, int* music_lump) {
  if (dsda_UMapMusic(music_index, music_lump))
    return;

  dsda_LegacyMapMusic(music_index, music_lump);
}

void dsda_InterMusic(int* music_index, int* music_lump) {
  if (dsda_UInterMusic(music_index, music_lump))
    return;

  dsda_LegacyInterMusic(music_index, music_lump);
}

typedef enum {
  finale_owner_legacy,
  finale_owner_u,
} finale_owner_t;

static finale_owner_t finale_owner = finale_owner_legacy;

void dsda_StartFinale(void) {
  if (dsda_UStartFinale()) {
    finale_owner = finale_owner_u;
    return;
  }

  dsda_LegacyStartFinale();
  finale_owner = finale_owner_legacy;
}

int dsda_FTicker(void) {
  if (finale_owner == finale_owner_u) {
    if (!dsda_UFTicker())
      finale_owner = finale_owner_legacy;

    return true;
  }

  dsda_LegacyFTicker();
  return false;
}

int dsda_FDrawer(void) {
  if (finale_owner == finale_owner_u) {
    dsda_UFDrawer();

    return true;
  }

  dsda_LegacyFDrawer();
  return false;
}

int dsda_BossAction(mobj_t* mo) {
  if (dsda_UBossAction(mo))
    return true;

  dsda_LegacyBossAction(mo);
  return false;
}

void dsda_HUTitle(const char** title) {
  if (dsda_UHUTitle(title))
    return;

  dsda_LegacyHUTitle(title);
}
