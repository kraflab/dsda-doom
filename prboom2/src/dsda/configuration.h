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
//	DSDA Config
//

#ifndef __DSDA_CONFIG__
#define __DSDA_CONFIG__

#include <stdio.h>

#include "doomtype.h"

typedef enum {
  dsda_config_none,
  dsda_config_realtic_clock_rate,
  dsda_config_default_complevel,
  dsda_config_default_skill,
  dsda_config_vanilla_keymap,
  dsda_config_menu_background,
  dsda_config_process_priority,
  dsda_config_max_player_corpse,
  dsda_config_input_profile,
  dsda_config_weapon_choice_1,
  dsda_config_weapon_choice_2,
  dsda_config_weapon_choice_3,
  dsda_config_weapon_choice_4,
  dsda_config_weapon_choice_5,
  dsda_config_weapon_choice_6,
  dsda_config_weapon_choice_7,
  dsda_config_weapon_choice_8,
  dsda_config_weapon_choice_9,
  dsda_config_flashing_hom,
  dsda_config_demo_smoothturns,
  dsda_config_demo_smoothturnsfactor,
  dsda_config_weapon_attack_alignment,
  dsda_config_count,
} dsda_config_identifier_t;

void dsda_InitConfig(void);
dboolean dsda_ReadConfig(const char* name, const char* string_param, int int_param);
void dsda_WriteConfig(dsda_config_identifier_t id, int key_length, FILE* file);
int dsda_ToggleConfig(dsda_config_identifier_t id, dboolean persist);
int dsda_CycleConfig(dsda_config_identifier_t id, dboolean persist);
int dsda_UpdateIntConfig(dsda_config_identifier_t id, int value, dboolean persist);
const char* dsda_UpdateStringConfig(dsda_config_identifier_t id, const char* value, dboolean persist);
int dsda_IntConfig(dsda_config_identifier_t id);
int dsda_PersistentIntConfig(dsda_config_identifier_t id);
const char* dsda_StringConfig(dsda_config_identifier_t id);
const char* dsda_PersistentStringConfig(dsda_config_identifier_t id);
char* dsda_ConfigSummary(const char* name);
int dsda_ConfigIDByName(const char* name);

#endif
