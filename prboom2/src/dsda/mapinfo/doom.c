//
// Copyright(C) 2023 by Ryan Krafnick
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
//  DSDA MapInfo Doom
//

#include "doomstat.h"
#include "lprintf.h"
#include "w_wad.h"

#include "dsda/args.h"
#include "dsda/mapinfo/doom/parser.h"

#include "doom.h"

static const doom_mapinfo_map_t* last_map;
static const doom_mapinfo_map_t* next_map;
static const doom_mapinfo_map_t* current_map;

static doom_mapinfo_map_t* dsda_DoomMapEntry(int gamemap) {
  int i;

  for (i = 0; i < doom_mapinfo.num_maps; ++i)
    if (gamemap == doom_mapinfo.maps[i].level_num)
      return &doom_mapinfo.maps[i];

  return NULL;
}

int dsda_DoomFirstMap(int* episode, int* map) {
  return false; // TODO
}

int dsda_DoomNewGameMap(int* episode, int* map) {
  return false; // TODO
}

int dsda_DoomResolveWarp(int* args, int arg_count, int* episode, int* map) {
  const doom_mapinfo_map_t* entry;

  if (!doom_mapinfo.loaded)
    return false;

  entry = dsda_DoomMapEntry(*map);

  if (!entry)
    return false;

  *map = entry->level_num;
  *episode = 1; // TODO: warp episode

  return true;
}

int dsda_DoomNextMap(int* episode, int* map) {
  return false; // TODO
}

int dsda_DoomShowNextLocBehaviour(int* behaviour) {
  return false; // TODO
}

int dsda_DoomSkipDrawShowNextLoc(int* skip) {
  return false; // TODO
}

void dsda_DoomUpdateMapInfo(void) {
  current_map = dsda_DoomMapEntry(gamemap);
}

void dsda_DoomUpdateLastMapInfo(void) {
  last_map = current_map;
  next_map = NULL;
}

void dsda_DoomUpdateNextMapInfo(void) {
  next_map = dsda_DoomMapEntry(wminfo.next + 1);
}

int dsda_DoomResolveCLEV(int* clev, int* episode, int* map) {
  return false; // TODO
}

int dsda_DoomResolveINIT(int* init) {
  return false; // TODO
}

int dsda_DoomMusicIndexToLumpNum(int* lump, int music_index) {
  return false; // TODO
}

int dsda_DoomMapMusic(int* music_index, int* music_lump) {
  int lump;

  if (!current_map || !current_map->music)
    return false;

  lump = W_CheckNumForName(current_map->music);

  if (lump == LUMP_NOT_FOUND)
    return false;

  *music_index = -1;
  *music_lump = lump;

  return true;
}

int dsda_DoomIntermissionMusic(int* music_index, int* music_lump) {
  int lump;

  if (!last_map || !last_map->inter_music)
    return false;

  lump = W_CheckNumForName(last_map->inter_music);

  if (lump == LUMP_NOT_FOUND)
    return false;

  *music_index = -1;
  *music_lump = lump;

  return true;
}

int dsda_DoomInterMusic(int* music_index, int* music_lump) {
  return false; // TODO
}

int dsda_DoomStartFinale(void) {
  return false; // TODO
}

int dsda_DoomFTicker(void) {
  return false; // TODO
}

void dsda_DoomFDrawer(void) {
  return; // TODO
}

int dsda_DoomBossAction(mobj_t* mo) {
  return false; // TODO
}

int dsda_DoomMapLumpName(const char** name, int episode, int map) {
  const doom_mapinfo_map_t* target_map;

  if (!doom_mapinfo.loaded)
    return false;

  target_map = dsda_DoomMapEntry(map);

  if (!target_map)
    return false;

  *name = target_map->lump_name;

  return true;
}

int dsda_DoomHUTitle(dsda_string_t* str) {
  if (!current_map)
    return false;

  dsda_StringPrintF(str, "%s", current_map->nice_name);

  return true;
}

int dsda_DoomSkyTexture(int* sky) {
  return false; // TODO
}

int dsda_DoomPrepareInitNew(void) {
  return false; // TODO
}

int dsda_DoomPrepareIntermission(int* result) {
  return false; // TODO
}

int dsda_DoomPrepareFinale(int* result) {
  return false; // TODO
}

void dsda_DoomLoadMapInfo(void) {
  int p;

  if (!dsda_Flag(dsda_arg_debug_mapinfo))
    return;

  p = -1;
  while ((p = W_ListNumFromName("MAPINFO", p)) >= 0) {
    const unsigned char* lump = (const unsigned char *) W_LumpByNum(p);
    dsda_ParseDoomMapInfo(lump, W_LumpLength(p), I_Error);
  }
}

int dsda_DoomExitPic(const char** exit_pic) {
  return false; // TODO
}

int dsda_DoomEnterPic(const char** enter_pic) {
  return false; // TODO
}

int dsda_DoomPrepareEntering(void) {
  return false; // TODO
}

int dsda_DoomPrepareFinished(void) {
  return false; // TODO
}

int dsda_DoomMapLightning(int* lightning) {
  return false; // TODO
}

int dsda_DoomApplyFadeTable(void) {
  return false; // TODO
}

int dsda_DoomMapCluster(int* cluster, int map) {
  return false; // TODO
}

int dsda_DoomSky1Texture(short* texture) {
  return false; // TODO
}

int dsda_DoomSky2Texture(short* texture) {
  return false; // TODO
}

int dsda_DoomInitSky(void) {
  return false; // TODO
}
