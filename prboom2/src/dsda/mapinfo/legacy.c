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
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"

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

void dsda_LegacyUpdateMapInfo(void) {
  // nothing to do right now
}

void dsda_LegacyUpdateLastMapInfo(void) {
  // nothing to do right now
}

void dsda_LegacyUpdateNextMapInfo(void) {
  // nothing to do right now
}

static int dsda_CannotCLEV(int episode, int map) {
  char* next;

  if (
    episode < 1 ||
    map < 0 ||
    ((gamemode == retail || gamemode == registered) && (episode > 9 || map > 9)) ||
    (gamemode == shareware && (episode > 1 || map > 9)) ||
    (gamemode == commercial && (episode > 1 || map > 99)) ||
    (gamemission == pack_nerve && map > 9)
  ) return true;

  if (map_format.mapinfo)
    map = P_TranslateMap(map);

  // Catch invalid maps
  next = MAPNAME(episode, map);
  if (W_CheckNumForName(next) == -1) {
    doom_printf("IDCLEV target not found: %s", next);
    return true;
  }

  return false;
}

int dsda_LegacyResolveCLEV(int* clev, int* episode, int* map) {
  if (dsda_CannotCLEV(*episode, *map))
    *clev = false;
  else {
    if (gamemission == chex)
      *episode = 1;

    *clev = true;
  }

  return true;
}

static inline int WRAP(int i, int w)
{
  while (i < 0)
    i += w;

  return i % w;
}

int dsda_LegacyMapMusic(int* music_index, int* music_lump) {
  *music_lump = -1;

  // hexen mapinfo
  if (map_format.mapinfo) {
    *music_index = gamemap;

    return true;
  }

  if (idmusnum != -1)
    *music_index = idmusnum; //jff 3/17/98 reload IDMUS music if not -1
  else {
    if (gamemode == commercial)
      *music_index = mus_runnin + WRAP(gamemap - 1, DOOM_MUSINFO - mus_runnin);
    else {
      static const int spmus[] = {
        mus_e3m4,
        mus_e3m2,
        mus_e3m3,
        mus_e1m5,
        mus_e2m7,
        mus_e2m4,
        mus_e2m6,
        mus_e2m5,
        mus_e1m9
      };

      if (heretic)
        *music_index = heretic_mus_e1m1 +
                       WRAP((gameepisode - 1) * 9 + gamemap - 1,
                            HERETIC_NUMMUSIC - heretic_mus_e1m1);
      else if (gameepisode < 4)
        *music_index = mus_e1m1 +
                       WRAP((gameepisode - 1) * 9 + gamemap - 1, mus_runnin - mus_e1m1);
      else
        *music_index = spmus[WRAP(gamemap - 1, 9)];
    }
  }

  return true;
}

int dsda_LegacyInterMusic(int* music_index, int* music_lump) {
  *music_lump = -1;

  switch (gamemode) {
    case shareware:
    case registered:
    case retail:
      *music_index = mus_victor;
      break;
    default:
      *music_index = mus_read_m;
      break;
  }

  return true;
}

int dsda_LegacyStartFinale(void) {
  return true;
}

int dsda_LegacyFTicker(void) {
  return true;
}

void dsda_LegacyFDrawer(void) {
  return;
}

int dsda_LegacyBossAction(mobj_t* mo) {
  return false;
}
