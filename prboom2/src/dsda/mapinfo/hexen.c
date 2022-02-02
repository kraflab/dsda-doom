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
//  DSDA MapInfo Hexen
//

#include "doomstat.h"
#include "g_game.h"
#include "lprintf.h"
#include "m_argv.h"
#include "p_setup.h"
#include "r_data.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"

#include "dsda/map_format.h"
#include "dsda/mapinfo.h"

#include "hexen.h"

int dsda_HexenResolveWarp(int arg_p, int* episode, int* map) {
  if (!map_format.mapinfo)
    return false;

  *episode = 1;

  if (arg_p < myargc - 1)
    *map = P_TranslateMap(atoi(myargv[arg_p + 1]));
  else
    *map = P_TranslateMap(1);

  if (*map == -1)
    I_Error("-warp: Invalid map number.\n");

  return true;
}

int dsda_HexenNextMap(int* episode, int* map) {
  if (!map_format.mapinfo)
    return false;

  *episode = 1;
  *map = P_GetMapNextMap(gamemap);

  return true;
}

int dsda_HexenShowNextLocBehaviour(int* behaviour) {
  return false; // TODO
}

int dsda_HexenSkipDrawShowNextLoc(int* skip) {
  return false; // TODO
}

void dsda_HexenUpdateMapInfo(void) {
  // TODO
}

void dsda_HexenUpdateLastMapInfo(void) {
  // TODO
}

void dsda_HexenUpdateNextMapInfo(void) {
  // TODO
}

int dsda_HexenResolveCLEV(int* clev, int* episode, int* map) {
  char* next;

  if (!map_format.mapinfo)
    return false;

  // Catch invalid maps
  next = MAPNAME(*episode, P_TranslateMap(*map));
  if (W_CheckNumForName(next) == -1) {
    doom_printf("IDCLEV target not found: %s", next);
    *clev = false;
  }
  else
    *clev = true;

  return true;
}

int dsda_HexenMapMusic(int* music_index, int* music_lump) {
  if (!map_format.mapinfo)
    return false;

  *music_lump = -1;
  *music_index = gamemap;

  return true;
}

int dsda_HexenInterMusic(int* music_index, int* music_lump) {
  return false; // TODO
}

int dsda_HexenStartFinale(void) {
  return false; // TODO
}

int dsda_HexenFTicker(void) {
  return false; // TODO
}

void dsda_HexenFDrawer(void) {
  return; // TODO
}

int dsda_HexenBossAction(mobj_t* mo) {
  return false; // TODO
}

int dsda_HexenHUTitle(const char** title) {
  if (!map_format.mapinfo)
    return false;

  *title = NULL;

  if (gamestate == GS_LEVEL && gamemap > 0 && gameepisode > 0)
    *title = P_GetMapName(gamemap);

  if (*title == NULL)
    *title = MAPNAME(gameepisode, gamemap);

  return true;
}

int dsda_HexenSkyTexture(int* sky) {
  return false; // TODO
}

int dsda_HexenPrepareIntermission(int* result) {
  return false; // TODO
}

int dsda_HexenPrepareFinale(int* result) {
  return false; // TODO
}

void dsda_HexenLoadMapInfo(void) {
  return; // TODO
}

int dsda_HexenExitPic(const char** exit_pic) {
  return false; // TODO
}

int dsda_HexenEnterPic(const char** enter_pic) {
  return false; // TODO
}

int dsda_HexenPrepareEntering(void) {
  return false; // TODO
}

int dsda_HexenPrepareFinished(void) {
  return false; // TODO
}
