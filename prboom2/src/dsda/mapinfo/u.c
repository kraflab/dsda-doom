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
//	DSDA MapInfo U
//

#include "doomstat.h"
#include "g_game.h"

#include "dsda/mapinfo.h"

#include "u.h"

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
