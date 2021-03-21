//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Options Lump
//

#ifndef __DSDA_OPTIONS__
#define __DSDA_OPTIONS__

typedef struct dsda_options {
  int weapon_recoil;
  int monsters_remember;
  int monster_infighting;
  int monster_backing;
  int monster_avoid_hazards;
  int monkeys;
  int monster_friction;
  int help_friends;
  int player_helpers;
  int friend_distance;
  int dog_jumping;
  // comp_*
} dsda_options_t;

const dsda_options_t* dsda_Options(void);

#endif
