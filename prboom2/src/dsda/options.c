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
#include "w_wad.h"
#include "lprintf.h"

#include "options.h"

static const dsda_options_t default_vanilla_options = {
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

static const dsda_options_t default_boom_options = {
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

static const dsda_options_t default_mbf_options = {
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
  .dog_jumping = 1,

  .comp_telefrag = 0,
  .comp_dropoff = 0,
  .comp_vile = 0,
  .comp_pain = 0,
  .comp_skull = 0,
  .comp_blazing = 0,
  .comp_doorlight = 0,
  .comp_model = 0,
  .comp_god = 0,
  .comp_falloff = 0,
  .comp_floors = 0,
  .comp_skymap = 0,
  .comp_pursuit = 0,
  .comp_doorstuck = 0,
  .comp_staylift = 0,
  .comp_zombie = 1,
  .comp_stairs = 0,
  .comp_infcheat = 0,
  .comp_zerotags = 0

  // These are not configurable:
  // .comp_moveblock = 0,
  // .comp_respawn = 1,
  // .comp_sound = 0,
  // .comp_666 = 0,
  // .comp_soul = 1,
  // .comp_maskedanim = 0,
  // .comp_ouchface = 1,
  // .comp_maxhealth = 0,
  // .comp_translucency = 0
};

static const dsda_options_t default_latest_options = {
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
  .dog_jumping = 1,

  .comp_telefrag = 0,
  .comp_dropoff = 0,
  .comp_vile = 0,
  .comp_pain = 0,
  .comp_skull = 0,
  .comp_blazing = 0,
  .comp_doorlight = 0,
  .comp_model = 0,
  .comp_god = 0,
  .comp_falloff = 0,
  .comp_floors = 0,
  .comp_skymap = 0,
  .comp_pursuit = 0,
  .comp_doorstuck = 0,
  .comp_staylift = 0,
  .comp_zombie = 1,
  .comp_stairs = 0,
  .comp_infcheat = 0,
  .comp_zerotags = 0,

  .comp_moveblock = 0,
  .comp_respawn = 0,
  .comp_sound = 0,
  .comp_666 = 0,
  .comp_soul = 0,
  .comp_maskedanim = 0,
  .comp_ouchface = 0,
  .comp_maxhealth = 0,
  .comp_translucency = 0
};

static dsda_options_t mbf_options;

typedef struct {
  const char* key;
  int* value;
  int min;
  int max;
} dsda_option_t;

static dsda_option_t option_list[] = {
  { "weapon_recoil", &mbf_options.weapon_recoil, 0, 1 },
  { "monsters_remember", &mbf_options.monsters_remember, 0, 1 },
  { "monster_infighting", &mbf_options.monster_infighting, 0, 1 },
  { "monster_backing", &mbf_options.monster_backing, 0, 1 },
  { "monster_avoid_hazards", &mbf_options.monster_avoid_hazards, 0, 1 },
  { "monkeys", &mbf_options.monkeys, 0, 1 },
  { "monster_friction", &mbf_options.monster_friction, 0, 1 },
  { "help_friends", &mbf_options.help_friends, 0, 1 },
  { "player_helpers", &mbf_options.player_helpers, 0, 3 },
  { "friend_distance", &mbf_options.friend_distance, 0, 999 },
  { "dog_jumping", &mbf_options.dog_jumping, 0, 1 },
  { "comp_telefrag", &mbf_options.comp_telefrag, 0, 1 },
  { "comp_dropoff", &mbf_options.comp_dropoff, 0, 1 },
  { "comp_vile", &mbf_options.comp_vile, 0, 1 },
  { "comp_pain", &mbf_options.comp_pain, 0, 1 },
  { "comp_skull", &mbf_options.comp_skull, 0, 1 },
  { "comp_blazing", &mbf_options.comp_blazing, 0, 1 },
  { "comp_doorlight", &mbf_options.comp_doorlight, 0, 1 },
  { "comp_model", &mbf_options.comp_model, 0, 1 },
  { "comp_god", &mbf_options.comp_god, 0, 1 },
  { "comp_falloff", &mbf_options.comp_falloff, 0, 1 },
  { "comp_floors", &mbf_options.comp_floors, 0, 1 },
  { "comp_skymap", &mbf_options.comp_skymap, 0, 1 },
  { "comp_pursuit", &mbf_options.comp_pursuit, 0, 1 },
  { "comp_doorstuck", &mbf_options.comp_doorstuck, 0, 1 },
  { "comp_staylift", &mbf_options.comp_staylift, 0, 1 },
  { "comp_zombie", &mbf_options.comp_zombie, 0, 1 },
  { "comp_stairs", &mbf_options.comp_stairs, 0, 1 },
  { "comp_infcheat", &mbf_options.comp_infcheat, 0, 1 },
  { "comp_zerotags", &mbf_options.comp_zerotags, 0, 1 },
  { "comp_moveblock", &mbf_options.comp_moveblock, 0, 1 },
  { "comp_respawn", &mbf_options.comp_respawn, 0, 1 },
  { "comp_respawnfix", &mbf_options.comp_respawn, 0, 1 },
  { "comp_sound", &mbf_options.comp_sound, 0, 1 },
  { "comp_666", &mbf_options.comp_666, 0, 1 },
  { "comp_soul", &mbf_options.comp_soul, 0, 1 },
  { "comp_maskedanim", &mbf_options.comp_maskedanim, 0, 1 },
  { "comp_ouchface", &mbf_options.comp_ouchface, 0, 1 },
  { "comp_maxhealth", &mbf_options.comp_maxhealth, 0, 1 },
  { "comp_translucency", &mbf_options.comp_translucency, 0, 1 },
  // { "comp_29", &mbf_options.comp_29, 0, 1 },
  // { "comp_30", &mbf_options.comp_30, 0, 1 },
  // { "comp_31", &mbf_options.comp_31, 0, 1 },
  // { "comp_32", &mbf_options.comp_32, 0, 1 },
  { 0 }
};

#define OPTIONS_LINE_LENGTH 80

typedef struct {
  const char* data;
  int length;
} options_lump_t;

static const char* dsda_ReadOption(char* buf, size_t size, options_lump_t* lump) {
  if (lump->length <= 0)
    return NULL;

  while (size > 1 && *lump->data && lump->length) {
    size--;
    lump->length--;
    if ((*buf++ = *lump->data++) == '\n')
      break;
  }

  *buf = '\0';

  return lump->data;
}

static const dsda_options_t* dsda_LumpOptions(int lumpnum) {
  options_lump_t lump;
  char buf[OPTIONS_LINE_LENGTH];
  char key[OPTIONS_LINE_LENGTH];
  char* scan;
  int value, count;
  dsda_option_t* option;

  lump.length = W_LumpLength(lumpnum);
  lump.data = W_CacheLumpNum(lumpnum);

  while (dsda_ReadOption(buf, OPTIONS_LINE_LENGTH, &lump)) {
    if (buf[0] == '#')
      continue;

    scan = buf;
    count = sscanf(scan, "%79s %d", key, &value);

    if (count != 2)
      continue;

    lprintf(LO_INFO, "dsda_LumpOptions: %s = %d\n", key, value);

    for (option = option_list; option->value; option++) {
      if (!strncmp(key, option->key, OPTIONS_LINE_LENGTH)) {
        *option->value = BETWEEN(option->min, option->max, value);

        break;
      }
    }
  }

  W_UnlockLumpNum(lumpnum);

  return &mbf_options;
}

static const dsda_options_t* dsda_MBFOptions(void) {
  int lumpnum;

  if (compatibility_level == mbf_compatibility)
    mbf_options = default_mbf_options;
  else
    mbf_options = default_latest_options;

  lumpnum = W_CheckNumForName("OPTIONS");

  if (lumpnum == -1)
    return &mbf_options;

  return dsda_LumpOptions(lumpnum);
}

const dsda_options_t* dsda_Options(void) {
  if (demo_compatibility)
    return &default_vanilla_options;

  if (!mbf_features)
    return &default_boom_options;

  return dsda_MBFOptions();
}
