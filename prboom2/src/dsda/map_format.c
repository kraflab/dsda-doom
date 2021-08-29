//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Map Format
//

#include "lprintf.h"
#include "p_spec.h"
#include "r_state.h"
#include "w_wad.h"

#include "map_format.h"

map_format_t map_format;

typedef enum {
  door_type_none = -1,
  door_type_red,
  door_type_blue,
  door_type_yellow,
  door_type_unknown = door_type_yellow,
  door_type_multiple
} door_type_t;

int dsda_DoorType(int index) {
  int special = lines[index].special;

  if (map_format.hexen) {
    if (special == 13 || special == 83)
      return door_type_unknown;

    return door_type_none;
  }

  if (heretic && special > 34)
    return door_type_none;

  if (GenLockedBase <= special && special < GenDoorBase) {
    special -= GenLockedBase;
    special = (special & LockedKey) >> LockedKeyShift;
    if (!special || special == 7)
      return door_type_multiple;
    else
      return (special - 1) % 3;
  }

  switch (special) {
    case 26:
    case 32:
    case 99:
    case 133:
      return door_type_blue;
    case 27:
    case 34:
    case 136:
    case 137:
      return door_type_yellow;
    case 28:
    case 33:
    case 134:
    case 135:
      return door_type_red;
    default:
      return door_type_none;
  }
}

dboolean dsda_IsExitLine(int index) {
  int special = lines[index].special;

  if (map_format.hexen)
    return special == 74 ||
           special == 75;

  return special == 11  ||
         special == 52  ||
         special == 197 ||
         special == 51  ||
         special == 124 ||
         special == 198;
}

dboolean dsda_IsTeleportLine(int index) {
  int special = lines[index].special;

  if (map_format.hexen)
  {
    return special == 70 ||
           special == 71;
  }

  if (heretic)
  {
    return special == 39;
  }

  return special == 39  ||
         special == 97  ||
         special == 125 ||
         special == 126;
}

void dsda_DetectMapFormat(void) {
  // if (W_CheckNumForName("BEHAVIOR") >= 0) {
  //   if (!hexen)
  //     I_Error("Hexen map format is only supported in Hexen!");

  // Can't just look for BEHAVIOR lumps, because some wads have vanilla and non-vanilla maps
  // Need proper per-map format swapping
  if (hexen) {
    map_format.hexen = true;
    map_format.polyobjs = true;
    map_format.acs = true;
    map_format.mapinfo = true;
    map_format.sndseq = true;
    map_format.sndinfo = true;
    map_format.mapthing_size = sizeof(mapthing_t);
    map_format.maplinedef_size = sizeof(hexen_maplinedef_t);
  }
  else {
    map_format.hexen = false;
    map_format.polyobjs = false;
    map_format.acs = false;
    map_format.mapinfo = false;
    map_format.sndseq = false;
    map_format.sndinfo = false;
    map_format.mapthing_size = sizeof(doom_mapthing_t);
    map_format.maplinedef_size = sizeof(doom_maplinedef_t);
  }
}
