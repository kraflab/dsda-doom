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

#include "doomstat.h"

#include "options.h"

static dsda_options_t default_vanilla_options = {
  .weapon_recoil = 0,
  .monsters_remember = 0,
  .monster_infighting = 1,
  .monster_backing = 0,
  .monster_avoid_hazards = 0,
  .monkeys = 0,
  .monster_friction = 0,
  .help_friends = 0,
  .player_helpers = 0,
  .friend_distance = 0,
  .dog_jumping = 0
};

static dsda_options_t default_boom_options = {
  .weapon_recoil = 0,
  .monsters_remember = 1,
  .monster_infighting = 1,
  .monster_backing = 0,
  .monster_avoid_hazards = 0,
  .monkeys = 0,
  .monster_friction = 0,
  .help_friends = 0,
  .player_helpers = 0,
  .friend_distance = 0,
  .dog_jumping = 0
};

static dsda_options_t default_mbf_options = {
  .weapon_recoil = 0,
  .monsters_remember = 1,
  .monster_infighting = 1,
  .monster_backing = 0,
  .monster_avoid_hazards = 1,
  .monkeys = 0,
  .monster_friction = 1,
  .help_friends = 0,
  .player_helpers = 0,
  .friend_distance = 128,
  .dog_jumping = 1
};

dsda_options_t* dsda_Options(void) {
  if (demo_compatibility)
    return &default_vanilla_options;

  if (!mbf_features)
    return &default_boom_options;

  return &default_mbf_options;
}
