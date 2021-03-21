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
  .dog_jumping = 1
};

static dsda_options_t lump_options;

typedef struct {
  const char* key;
  int* value;
  int min;
  int max;
} dsda_option_t;

static dsda_option_t option_list[] = {
  { "weapon_recoil", &lump_options.weapon_recoil, 0, 1 },
  { "monsters_remember", &lump_options.monsters_remember, 0, 1 },
  { "monster_infighting", &lump_options.monster_infighting, 0, 1 },
  { "monster_backing", &lump_options.monster_backing, 0, 1 },
  { "monster_avoid_hazards", &lump_options.monster_avoid_hazards, 0, 1 },
  { "monkeys", &lump_options.monkeys, 0, 1 },
  { "monster_friction", &lump_options.monster_friction, 0, 1 },
  { "help_friends", &lump_options.help_friends, 0, 1 },
  { "player_helpers", &lump_options.player_helpers, 0, 3 },
  { "friend_distance", &lump_options.friend_distance, 0, 999 },
  { "dog_jumping", &lump_options.dog_jumping, 0, 1 },
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

  lump_options = default_mbf_options;

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

  return &lump_options;
}

static const dsda_options_t* dsda_MBFOptions(void) {
  int lumpnum;

  lumpnum = W_CheckNumForName("OPTIONS");

  if (lumpnum == -1)
    return &default_mbf_options;

  return dsda_LumpOptions(lumpnum);
}

const dsda_options_t* dsda_Options(void) {
  if (demo_compatibility)
    return &default_vanilla_options;

  if (!mbf_features)
    return &default_boom_options;

  return dsda_MBFOptions();
}
