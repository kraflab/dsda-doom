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

#include "dsda/mapinfo/hexen.h"
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

void dsda_FirstMap(int* episode, int* map) {
  if (dsda_HexenFirstMap(episode, map))
    return;

  if (dsda_UFirstMap(episode, map))
    return;

  dsda_LegacyFirstMap(episode, map);
}

void dsda_NewGameMap(int* episode, int* map) {
  if (dsda_HexenNewGameMap(episode, map))
    return;

  if (dsda_UNewGameMap(episode, map))
    return;

  dsda_LegacyNewGameMap(episode, map);
}

void dsda_ResolveWarp(int arg_p, int* episode, int* map) {
  if (dsda_HexenResolveWarp(arg_p, episode, map))
    return;

  if (dsda_UResolveWarp(arg_p, episode, map))
    return;

  dsda_LegacyResolveWarp(arg_p, episode, map);
}

void dsda_NextMap(int* episode, int* map) {
  if (dsda_HexenNextMap(episode, map))
    return;

  if (dsda_UNextMap(episode, map))
    return;

  dsda_LegacyNextMap(episode, map);
}

void dsda_ShowNextLocBehaviour(int* behaviour) {
  if (dsda_HexenShowNextLocBehaviour(behaviour))
    return;

  if (dsda_UShowNextLocBehaviour(behaviour))
    return;

  dsda_LegacyShowNextLocBehaviour(behaviour);
}

int dsda_SkipDrawShowNextLoc(void) {
  int skip;

  if (dsda_HexenSkipDrawShowNextLoc(&skip))
    return skip;

  if (dsda_USkipDrawShowNextLoc(&skip))
    return skip;

  dsda_LegacySkipDrawShowNextLoc(&skip);

  return skip;
}

void dsda_UpdateMapInfo(void) {
  dsda_HexenUpdateMapInfo();
  dsda_UUpdateMapInfo();
  dsda_LegacyUpdateMapInfo();
}

void dsda_UpdateLastMapInfo(void) {
  dsda_HexenUpdateLastMapInfo();
  dsda_UUpdateLastMapInfo();
  dsda_LegacyUpdateLastMapInfo();
}

void dsda_UpdateNextMapInfo(void) {
  dsda_HexenUpdateNextMapInfo();
  dsda_UUpdateNextMapInfo();
  dsda_LegacyUpdateNextMapInfo();
}

int dsda_ResolveCLEV(int* episode, int* map) {
  int clev;

  if (dsda_HexenResolveCLEV(&clev, episode, map))
    return clev;

  if (dsda_UResolveCLEV(&clev, episode, map))
    return clev;

  dsda_LegacyResolveCLEV(&clev, episode, map);

  return clev;
}

int dsda_ResolveINIT(void) {
  int init;

  if (dsda_HexenResolveINIT(&init))
    return init;

  if (dsda_UResolveINIT(&init))
    return init;

  dsda_LegacyResolveINIT(&init);

  return init;
}

int dsda_MusicIndexToLumpNum(int music_index) {
  int lump;

  if (dsda_HexenMusicIndexToLumpNum(&lump, music_index))
    return lump;

  if (dsda_UMusicIndexToLumpNum(&lump, music_index))
    return lump;

  dsda_LegacyMusicIndexToLumpNum(&lump, music_index);

  return lump;
}

void dsda_MapMusic(int* music_index, int* music_lump) {
  if (dsda_HexenMapMusic(music_index, music_lump))
    return;

  if (dsda_UMapMusic(music_index, music_lump))
    return;

  dsda_LegacyMapMusic(music_index, music_lump);
}

void dsda_InterMusic(int* music_index, int* music_lump) {
  if (dsda_HexenInterMusic(music_index, music_lump))
    return;

  if (dsda_UInterMusic(music_index, music_lump))
    return;

  dsda_LegacyInterMusic(music_index, music_lump);
}

typedef enum {
  finale_owner_legacy,
  finale_owner_u,
  finale_owner_hexen,
} finale_owner_t;

static finale_owner_t finale_owner = finale_owner_legacy;

void dsda_StartFinale(void) {
  if (dsda_HexenStartFinale()) {
    finale_owner = finale_owner_hexen;
    return;
  }

  if (dsda_UStartFinale()) {
    finale_owner = finale_owner_u;
    return;
  }

  dsda_LegacyStartFinale();
  finale_owner = finale_owner_legacy;
}

int dsda_FTicker(void) {
  if (finale_owner == finale_owner_hexen) {
    if (!dsda_HexenFTicker())
      finale_owner = finale_owner_legacy;

    return true;
  }

  if (finale_owner == finale_owner_u) {
    if (!dsda_UFTicker())
      finale_owner = finale_owner_legacy;

    return true;
  }

  dsda_LegacyFTicker();
  return false;
}

int dsda_FDrawer(void) {
  if (finale_owner == finale_owner_hexen) {
    dsda_HexenFDrawer();

    return true;
  }

  if (finale_owner == finale_owner_u) {
    dsda_UFDrawer();

    return true;
  }

  dsda_LegacyFDrawer();
  return false;
}

int dsda_BossAction(mobj_t* mo) {
  if (dsda_HexenBossAction(mo))
    return true;

  if (dsda_UBossAction(mo))
    return true;

  dsda_LegacyBossAction(mo);
  return false;
}

void dsda_HUTitle(const char** title) {
  if (dsda_HexenHUTitle(title))
    return;

  if (dsda_UHUTitle(title))
    return;

  dsda_LegacyHUTitle(title);
}

int dsda_SkyTexture(void) {
  int sky;

  if (dsda_HexenSkyTexture(&sky))
    return sky;

  if (dsda_USkyTexture(&sky))
    return sky;

  dsda_LegacySkyTexture(&sky);

  return sky;
}

void dsda_PrepareInitNew(void) {
  if (dsda_HexenPrepareInitNew())
    return;

  if (dsda_UPrepareInitNew())
    return;

  dsda_LegacyPrepareInitNew();
}

void dsda_PrepareIntermission(int* behaviour) {
  if (dsda_HexenPrepareIntermission(behaviour))
    return;

  if (dsda_UPrepareIntermission(behaviour))
    return;

  dsda_LegacyPrepareIntermission(behaviour);
}

void dsda_PrepareFinale(int* behaviour) {
  if (dsda_HexenPrepareFinale(behaviour))
    return;

  if (dsda_UPrepareFinale(behaviour))
    return;

  dsda_LegacyPrepareFinale(behaviour);
}

void dsda_LoadMapInfo(void) {
  dsda_HexenLoadMapInfo();
  dsda_ULoadMapInfo();
  dsda_LegacyLoadMapInfo();
}

const char* dsda_ExitPic(void) {
  const char* exit_pic;

  if (dsda_HexenExitPic(&exit_pic))
    return exit_pic;

  if (dsda_UExitPic(&exit_pic))
    return exit_pic;

  dsda_LegacyExitPic(&exit_pic);
  return exit_pic;
}

const char* dsda_EnterPic(void) {
  const char* enter_pic;

  if (dsda_HexenEnterPic(&enter_pic))
    return enter_pic;

  if (dsda_UEnterPic(&enter_pic))
    return enter_pic;

  dsda_LegacyEnterPic(&enter_pic);
  return enter_pic;
}

void dsda_PrepareEntering(void) {
  if (dsda_HexenPrepareEntering())
    return;

  if (dsda_UPrepareEntering())
    return;

  dsda_LegacyPrepareEntering();
}

void dsda_PrepareFinished(void) {
  if (dsda_HexenPrepareFinished())
    return;

  if (dsda_UPrepareFinished())
    return;

  dsda_LegacyPrepareFinished();
}

int dsda_MapLightning(int map) {
  int lightning;

  if (dsda_HexenMapLightning(&lightning, map))
    return lightning;

  if (dsda_UMapLightning(&lightning, map))
    return lightning;

  dsda_LegacyMapLightning(&lightning, map);

  return lightning;
}

void dsda_ApplyFadeTable(void) {
  if (dsda_HexenApplyFadeTable())
    return;

  if (dsda_UApplyFadeTable())
    return;

  dsda_LegacyApplyFadeTable();
}
