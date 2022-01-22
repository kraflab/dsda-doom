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

#include "dsda/mapinfo.h"

#include "u.h"

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
