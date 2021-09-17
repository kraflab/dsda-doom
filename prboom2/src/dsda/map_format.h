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
  short friction_mask;
  short push_mask;
  short generalized_mask;
  void (*init_sector_special)(sector_t*, int);
  void (*player_in_special_sector)(player_t*, sector_t*);
  void (*spawn_scroller)(line_t*, int);
  void (*spawn_friction)(line_t*);
  size_t mapthing_size;
  size_t maplinedef_size;
} map_format_t;

extern map_format_t map_format;

int dsda_DoorType(int index);
dboolean dsda_IsExitLine(int index);
dboolean dsda_IsTeleportLine(int index);
void dsda_ApplyMapFormat(void);

#endif
