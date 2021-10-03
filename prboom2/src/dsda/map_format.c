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
      mobjinfo[i].flags2 |= MF2_MCROSS | MF2_PUSHWALL;

    if (mobjinfo[i].flags & MF_MISSILE)
      mobjinfo[i].flags2 |= MF2_PCROSS | MF2_IMPACT;
  }

  if (!raven) {
    mobjinfo[MT_SKULL].flags2 |= MF2_MCROSS | MF2_PUSHWALL;
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

extern void P_ShootCompatibleSpecialLine(mobj_t *thing, line_t *line);
extern void P_ShootHexenSpecialLine(mobj_t *thing, line_t *line);

extern dboolean P_TestActivateZDoomLine(line_t *line, mobj_t *mo, int side, unsigned int activationType);
extern dboolean P_TestActivateHexenLine(line_t *line, mobj_t *mo, int side, unsigned int activationType);

extern void P_PostProcessCompatibleLineSpecial(line_t *ld);
extern void P_PostProcessHereticLineSpecial(line_t *ld);
extern void P_PostProcessHexenLineSpecial(line_t *ld);
extern void P_PostProcessZDoomLineSpecial(line_t *ld);

extern void P_PostProcessCompatibleSidedefSpecial(side_t *sd, const mapsidedef_t *msd, sector_t *sec, int i);
extern void P_PostProcessHereticSidedefSpecial(side_t *sd, const mapsidedef_t *msd, sector_t *sec, int i);
extern void P_PostProcessHexenSidedefSpecial(side_t *sd, const mapsidedef_t *msd, sector_t *sec, int i);
extern void P_PostProcessZDoomSidedefSpecial(side_t *sd, const mapsidedef_t *msd, sector_t *sec, int i);

extern void P_AnimateCompatibleSurfaces(void);
extern void P_AnimateHereticSurfaces(void);
extern void P_AnimateHexenSurfaces(void);
extern void P_AnimateZDoomSurfaces(void);

extern void P_CheckCompatibleImpact(mobj_t *);
extern void P_CheckHereticImpact(mobj_t *);
extern void P_CheckZDoomImpact(mobj_t *);

extern void P_TranslateHexenLineFlags(unsigned int *);
extern void P_TranslateZDoomLineFlags(unsigned int *);

extern void P_ApplyCompatibleSectorMovementSpecial(mobj_t *, int);
extern void P_ApplyHereticSectorMovementSpecial(mobj_t *, int);

extern dboolean P_MobjInCompatibleSector(mobj_t *);
extern dboolean P_MobjInHereticSector(mobj_t *);
extern dboolean P_MobjInHexenSector(mobj_t *);
extern dboolean P_MobjInZDoomSector(mobj_t *);

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
  .lax_monster_activation = true,
  .friction_mask = ZDOOM_FRICTION_MASK,
  .push_mask = ZDOOM_PUSH_MASK,
  .generalized_mask = ~0xff,
  .switch_activation = ML_SPAC_USE | ML_SPAC_IMPACT | ML_SPAC_PUSH,
  .init_sector_special = P_SpawnZDoomSectorSpecial,
  .player_in_special_sector = P_PlayerInZDoomSector,
  .mobj_in_special_sector = P_MobjInZDoomSector,
  .spawn_scroller = P_SpawnZDoomScroller,
  .spawn_friction = P_SpawnZDoomFriction,
  .spawn_pusher = P_SpawnZDoomPusher,
  .spawn_extra = P_SpawnZDoomExtra,
  .cross_special_line = P_CrossZDoomSpecialLine,
  .shoot_special_line = P_ShootHexenSpecialLine,
  .test_activate_line = P_TestActivateZDoomLine,
  .post_process_line_special = P_PostProcessZDoomLineSpecial,
  .post_process_sidedef_special = P_PostProcessZDoomSidedefSpecial,
  .animate_surfaces = P_AnimateZDoomSurfaces,
  .check_impact = P_CheckZDoomImpact,
  .translate_line_flags = P_TranslateZDoomLineFlags,
  .apply_sector_movement_special = P_ApplyHereticSectorMovementSpecial,
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
  .lax_monster_activation = false,
  .friction_mask = 0, // not used
  .push_mask = 0, // not used
  .generalized_mask = 0, // not used
  .switch_activation = ML_SPAC_USE | ML_SPAC_IMPACT,
  .init_sector_special = NULL, // not used
  .player_in_special_sector = P_PlayerInHexenSector,
  .mobj_in_special_sector = P_MobjInHexenSector,
  .spawn_scroller = NULL, // not used
  .spawn_friction = NULL, // not used
  .spawn_pusher = NULL, // not used
  .spawn_extra = NULL, // not used
  .cross_special_line = P_CrossHexenSpecialLine,
  .shoot_special_line = P_ShootHexenSpecialLine,
  .test_activate_line = P_TestActivateHexenLine,
  .post_process_line_special = P_PostProcessHexenLineSpecial,
  .post_process_sidedef_special = P_PostProcessHexenSidedefSpecial,
  .animate_surfaces = P_AnimateHexenSurfaces,
  .check_impact = NULL, // not used
  .translate_line_flags = P_TranslateHexenLineFlags,
  .apply_sector_movement_special = P_ApplyHereticSectorMovementSpecial,
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
  .lax_monster_activation = true,
  .friction_mask = FRICTION_MASK,
  .push_mask = PUSH_MASK,
  .generalized_mask = 0,
  .switch_activation = 0, // not used
  .init_sector_special = P_SpawnCompatibleSectorSpecial,
  .player_in_special_sector = P_PlayerInHereticSector,
  .mobj_in_special_sector = P_MobjInHereticSector,
  .spawn_scroller = P_SpawnCompatibleScroller,
  .spawn_friction = P_SpawnCompatibleFriction,
  .spawn_pusher = P_SpawnCompatiblePusher,
  .spawn_extra = P_SpawnCompatibleExtra,
  .cross_special_line = P_CrossHereticSpecialLine,
  .shoot_special_line = P_ShootCompatibleSpecialLine,
  .test_activate_line = NULL, // not used
  .post_process_line_special = P_PostProcessHereticLineSpecial,
  .post_process_sidedef_special = P_PostProcessHereticSidedefSpecial,
  .animate_surfaces = P_AnimateHereticSurfaces,
  .check_impact = P_CheckHereticImpact,
  .apply_sector_movement_special = P_ApplyHereticSectorMovementSpecial,
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
  .lax_monster_activation = true,
  .friction_mask = FRICTION_MASK,
  .push_mask = PUSH_MASK,
  .generalized_mask = ~31,
  .switch_activation = 0, // not used
  .init_sector_special = P_SpawnCompatibleSectorSpecial,
  .player_in_special_sector = P_PlayerInCompatibleSector,
  .mobj_in_special_sector = P_MobjInCompatibleSector,
  .spawn_scroller = P_SpawnCompatibleScroller,
  .spawn_friction = P_SpawnCompatibleFriction,
  .spawn_pusher = P_SpawnCompatiblePusher,
  .spawn_extra = P_SpawnCompatibleExtra,
  .cross_special_line = P_CrossCompatibleSpecialLine,
  .shoot_special_line = P_ShootCompatibleSpecialLine,
  .test_activate_line = NULL, // not used
  .post_process_line_special = P_PostProcessCompatibleLineSpecial,
  .post_process_sidedef_special = P_PostProcessCompatibleSidedefSpecial,
  .animate_surfaces = P_AnimateCompatibleSurfaces,
  .check_impact = P_CheckCompatibleImpact,
  .apply_sector_movement_special = P_ApplyCompatibleSectorMovementSpecial,
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
