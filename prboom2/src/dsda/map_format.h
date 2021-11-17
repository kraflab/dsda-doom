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

#ifndef __DSDA_MAP_FORMAT__
#define __DSDA_MAP_FORMAT__

#include "doomtype.h"
#include "r_defs.h"
#include "p_spec.h"

typedef struct {
  dboolean zdoom;
  dboolean hexen;
  dboolean polyobjs;
  dboolean acs;
  dboolean mapinfo;
  dboolean sndseq;
  dboolean sndinfo;
  dboolean animdefs;
  dboolean doublesky;
  dboolean map99;
  dboolean lax_monster_activation;
  short generalized_mask;
  unsigned int switch_activation;
  void (*init_sector_special)(sector_t*, int);
  void (*player_in_special_sector)(player_t*, sector_t*);
  dboolean (*mobj_in_special_sector)(mobj_t*);
  void (*spawn_scroller)(line_t*, int);
  void (*spawn_friction)(line_t*);
  void (*spawn_pusher)(line_t*);
  void (*spawn_extra)(line_t*, int);
  void (*cross_special_line)(line_t *, int, mobj_t *, dboolean);
  void (*shoot_special_line)(mobj_t *, line_t *);
  dboolean (*test_activate_line)(line_t *, mobj_t *, int, unsigned int);
  dboolean (*execute_line_special)(int, byte *, line_t *, int, mobj_t *);
  void (*post_process_line_special)(line_t *);
  void (*post_process_sidedef_special)(side_t *, const mapsidedef_t *, sector_t *, int);
  void (*animate_surfaces)(void);
  void (*check_impact)(mobj_t *);
  void (*translate_line_flags)(unsigned int *);
  void (*apply_sector_movement_special)(mobj_t *, int);
  void (*t_vertical_door)(vldoor_t *);
  void (*t_move_floor)(floormove_t *);
  void (*t_move_ceiling)(ceiling_t *);
  void (*t_build_pillar)(pillar_t *);
  void (*t_plat_raise)(plat_t *);
  int (*ev_teleport)(int, line_t *, int, mobj_t *, int);
  void (*player_thrust)(player_t* player, angle_t angle, fixed_t move);
  size_t mapthing_size;
  size_t maplinedef_size;
  int mt_push;
  int mt_pull;
} map_format_t;

extern map_format_t map_format;

int dsda_DoorType(int index);
dboolean dsda_IsExitLine(int index);
dboolean dsda_IsTeleportLine(int index);
void dsda_ApplyZDoomMapFormat(void);
void dsda_ApplyDefaultMapFormat(void);

#endif
