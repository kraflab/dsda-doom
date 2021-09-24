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
    return special == 70 ||
           special == 71;

  if (heretic)
    return special == 39;

  return special == 39  ||
         special == 97  ||
         special == 125 ||
         special == 126;
}

// Migrate some non-hexen data to hexen format
static void dsda_MigrateMobjInfo(void) {
  int i;

  if (hexen || !map_format.hexen) return;

  for (i = mobj_types_zero; i < num_mobj_types; ++i) {
    if (mobjinfo[i].flags & MF_COUNTKILL)
      mobjinfo[i].flags2 |= MF2_MCROSS;

    if (mobjinfo[i].flags & MF_MISSILE)
      mobjinfo[i].flags2 |= MF2_PCROSS;
  }

  if (!raven) {
    mobjinfo[MT_SKULL].flags2 |= MF2_MCROSS;
    mobjinfo[MT_PLAYER].flags2 |= MF2_WINDTHRUST;
  }
}

extern void P_SpawnCompatibleSectorSpecial(sector_t *sector, int i);
extern void P_SpawnZDoomSectorSpecial(sector_t *sector, int i);

extern void P_PlayerInCompatibleSector(player_t *player, sector_t *sector);
extern void P_PlayerInZDoomSector(player_t *player, sector_t *sector);
extern void P_PlayerInHereticSector(player_t * player, sector_t * sector);
extern void P_PlayerInHexenSector(player_t * player, sector_t * sector);

extern void P_SpawnCompatibleScroller(line_t *l, int i);
extern void P_SpawnZDoomScroller(line_t *l, int i);

extern void P_SpawnCompatibleFriction(line_t *l);
extern void P_SpawnZDoomFriction(line_t *l);

extern void P_SpawnCompatiblePusher(line_t *l);
extern void P_SpawnZDoomPusher(line_t *l);

extern void P_SpawnCompatibleExtra(line_t *l, int i);
extern void P_SpawnZDoomExtra(line_t *l, int i);

extern void P_CrossCompatibleSpecialLine(line_t *line, int side, mobj_t *thing, dboolean bossaction);
extern void P_CrossZDoomSpecialLine(line_t *line, int side, mobj_t *thing, dboolean bossaction);
extern void P_CrossHereticSpecialLine(line_t *line, int side, mobj_t *thing, dboolean bossaction);
extern void P_CrossHexenSpecialLine(line_t *line, int side, mobj_t *thing, dboolean bossaction);

static const map_format_t zdoom_in_hexen_map_format = {
  .zdoom = true,
  .hexen = true,
  .polyobjs = false,
  .acs = false,
  .mapinfo = false,
  .sndseq = false,
  .sndinfo = false,
  .animdefs = false,
  .doublesky = false,
  .map99 = false,
  .friction_mask = ZDOOM_FRICTION_MASK,
  .push_mask = ZDOOM_PUSH_MASK,
  .generalized_mask = ~0xff,
  .init_sector_special = P_SpawnZDoomSectorSpecial,
  .player_in_special_sector = P_PlayerInZDoomSector,
  .spawn_scroller = P_SpawnZDoomScroller,
  .spawn_friction = P_SpawnZDoomFriction,
  .spawn_pusher = P_SpawnZDoomPusher,
  .spawn_extra = P_SpawnZDoomExtra,
  .cross_special_line = P_CrossZDoomSpecialLine,
  .mapthing_size = sizeof(mapthing_t),
  .maplinedef_size = sizeof(hexen_maplinedef_t),
};

static const map_format_t hexen_map_format = {
  .zdoom = false,
  .hexen = true,
  .polyobjs = true,
  .acs = true,
  .mapinfo = true,
  .sndseq = true,
  .sndinfo = true,
  .animdefs = true,
  .doublesky = true,
  .map99 = true,
  .friction_mask = 0, // not used
  .push_mask = 0, // not used
  .init_sector_special = NULL, // not used
  .generalized_mask = 0, // not used
  .player_in_special_sector = P_PlayerInHexenSector,
  .spawn_scroller = NULL, // not used
  .spawn_friction = NULL, // not used
  .spawn_pusher = NULL, // not used
  .spawn_extra = NULL, // not used
  .cross_special_line = P_CrossHexenSpecialLine,
  .mapthing_size = sizeof(mapthing_t),
  .maplinedef_size = sizeof(hexen_maplinedef_t),
};

static const map_format_t heretic_map_format = {
  .zdoom = false,
  .hexen = false,
  .polyobjs = false,
  .acs = false,
  .mapinfo = false,
  .sndseq = false,
  .sndinfo = false,
  .animdefs = false,
  .doublesky = false,
  .map99 = false,
  .friction_mask = FRICTION_MASK,
  .push_mask = PUSH_MASK,
  .generalized_mask = 0,
  .init_sector_special = P_SpawnCompatibleSectorSpecial,
  .player_in_special_sector = P_PlayerInHereticSector,
  .spawn_scroller = P_SpawnCompatibleScroller,
  .spawn_friction = P_SpawnCompatibleFriction,
  .spawn_pusher = P_SpawnCompatiblePusher,
  .spawn_extra = P_SpawnCompatibleExtra,
  .cross_special_line = P_CrossHereticSpecialLine,
  .mapthing_size = sizeof(doom_mapthing_t),
  .maplinedef_size = sizeof(doom_maplinedef_t),
};

static const map_format_t doom_map_format = {
  .zdoom = false,
  .hexen = false,
  .polyobjs = false,
  .acs = false,
  .mapinfo = false,
  .sndseq = false,
  .sndinfo = false,
  .animdefs = false,
  .doublesky = false,
  .map99 = false,
  .friction_mask = FRICTION_MASK,
  .push_mask = PUSH_MASK,
  .generalized_mask = ~31,
  .init_sector_special = P_SpawnCompatibleSectorSpecial,
  .player_in_special_sector = P_PlayerInCompatibleSector,
  .spawn_scroller = P_SpawnCompatibleScroller,
  .spawn_friction = P_SpawnCompatibleFriction,
  .spawn_pusher = P_SpawnCompatiblePusher,
  .spawn_extra = P_SpawnCompatibleExtra,
  .cross_special_line = P_CrossCompatibleSpecialLine,
  .mapthing_size = sizeof(doom_mapthing_t),
  .maplinedef_size = sizeof(doom_maplinedef_t),
};

void dsda_ApplyMapFormat(void) {
  if (false) // in-hexen zdoom format
    map_format = zdoom_in_hexen_map_format;
  else if (hexen)
    map_format = hexen_map_format;
  else if (heretic)
    map_format = heretic_map_format;
  else
    map_format = doom_map_format;

  dsda_MigrateMobjInfo();
}
