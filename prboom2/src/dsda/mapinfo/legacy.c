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
//	DSDA MapInfo Legacy
//

#include "doomstat.h"
#include "g_game.h"
#include "p_setup.h"

#include "dsda/map_format.h"
#include "dsda/mapinfo.h"

#include "legacy.h"

int dsda_LegacyNextMap(int* episode, int* map) {
  static byte doom2_next[33] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
    12, 13, 14, 15, 31, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 1,
    32, 16, 3
  };
  static byte doom_next[4][9] = {
    { 12, 13, 19, 15, 16, 17, 18, 21, 14 },
    { 22, 23, 24, 25, 29, 27, 28, 31, 26 },
    { 32, 33, 34, 35, 36, 39, 38, 41, 37 },
    { 42, 49, 44, 45, 46, 47, 48, 11, 43 }
  };
  static byte heretic_next[6][9] = {
    { 12, 13, 14, 15, 16, 19, 18, 21, 17 },
    { 22, 23, 24, 29, 26, 27, 28, 31, 25 },
    { 32, 33, 34, 39, 36, 37, 38, 41, 35 },
    { 42, 43, 44, 49, 46, 47, 48, 51, 45 },
    { 52, 53, 59, 55, 56, 57, 58, 61, 54 },
    { 62, 63, 11, 11, 11, 11, 11, 11, 11 }, // E6M4-E6M9 shouldn't be accessible
  };

  // hexen mapinfo
  if (map_format.mapinfo) {
    *episode = 1;
    *map = P_GetMapNextMap(gamemap);

    return true;
  }

  // next arrays are 0-based, unlike gameepisode and gamemap
  *episode = gameepisode - 1;
  *map = gamemap - 1;

  if (heretic) {
    int next;

    if (gamemode == shareware)
      heretic_next[0][7] = 11;

    if (gamemode == registered)
      heretic_next[2][7] = 11;

    next = heretic_next[BETWEEN(0, 5, *episode)][BETWEEN(0, 8, *map)];
    *episode = next / 10;
    *map = next % 10;
  }
  else if (gamemode == commercial) {
    // secret level
    doom2_next[14] = (haswolflevels ? 31 : 16);

    if (bfgedition && singleplayer) {
      if (gamemission == pack_nerve) {
        doom2_next[3] = 9;
        doom2_next[7] = 1;
        doom2_next[8] = 5;
      }
      else
        doom2_next[1] = 33;
    }

    *episode = 1;
    *map = doom2_next[BETWEEN(0, 32, *map)];
  }
  else {
    int next;

    // shareware doom has only episode 1
    doom_next[0][7] = (gamemode == shareware ? 11 : 21);

    doom_next[2][7] = // the fourth episode for pre-ultimate complevels is not allowed
      ((gamemode == registered) || (compatibility_level < ultdoom_compatibility) ? 11 : 41);

    next = doom_next[BETWEEN(0, 3, *episode)][BETWEEN(0, 9, *map)];
    *episode = next / 10;
    *map = next % 10;
  }

  return true;
}

int dsda_LegacyShowNextLocBehaviour(int* behaviour) {
  if (
    gamemode != commercial &&
    (gamemap == 8 || (gamemission == chex && gamemap == 5))
  )
    *behaviour = WI_SHOW_NEXT_DONE;
  else
    *behaviour = WI_SHOW_NEXT_LOC;

  return true;
}

int dsda_LegacySkipDrawShowNextLoc(int* skip) {
  *skip = false;

  return true;
}
