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

#include <string.h>

#include "doomdef.h"
#include "z_zone.h"

#include "dsda/input.h"

#include "configuration.h"

typedef enum {
  dsda_config_int,
  dsda_config_string,
} dsda_config_type_t;

typedef union {
  int v_int;
  char* v_string;
} dsda_config_value_t;

typedef struct {
  const char* name;
  dsda_config_identifier_t id;
  dsda_config_type_t type;
  int lower_limit;
  int upper_limit;
  dsda_config_value_t default_value;
  int* int_binding;
  dboolean strict;
  int strict_value;
  void (*onUpdate)(void);
  dsda_config_value_t transient_value;
  dsda_config_value_t persistent_value;
} dsda_config_t;

#define BOOL_DEFAULT_ON dsda_config_int, 0, 1, { 1 }
#define BOOL_DEFAULT_OFF dsda_config_int, 0, 1, { 0 }

extern int dsda_input_profile;
extern int weapon_preferences[2][NUMWEAPONS + 1];

static void UpdateRealticClockRate(void) {
  void I_Init2(void);

  I_Init2();
}

dsda_config_t dsda_config[dsda_config_count] = {
  [dsda_config_realtic_clock_rate] = {
    "realtic_clock_rate", dsda_config_realtic_clock_rate,
    dsda_config_int, 3, 10000, { 100 }, NULL, true, 100, UpdateRealticClockRate
  },
  [dsda_config_default_complevel] = {
    "default_compatibility_level", dsda_config_default_complevel,
    dsda_config_int, 0, mbf21_compatibility, { mbf21_compatibility }
  },
  [dsda_config_vanilla_keymap] = {
    "vanilla_keymap", dsda_config_vanilla_keymap,
    BOOL_DEFAULT_OFF
  },
  [dsda_config_menu_background] = {
    "menu_background", dsda_config_menu_background,
    BOOL_DEFAULT_ON
  },
  [dsda_config_process_priority] = {
    "process_priority", dsda_config_process_priority,
    dsda_config_int, 0, 2, { 0 }
  },
  [dsda_config_max_player_corpse] = {
    "max_player_corpse", dsda_config_max_player_corpse,
    dsda_config_int, -1, INT_MAX, { 32 }, NULL, true, 32
  },
  [dsda_config_input_profile] = {
    "input_profile", dsda_config_input_profile,
    dsda_config_int, 0, DSDA_INPUT_PROFILE_COUNT - 1, { 0 }, &dsda_input_profile
  },
  [dsda_config_weapon_choice_1] = {
    "weapon_choice_1", dsda_config_weapon_choice_1,
    dsda_config_int, 0, 9, { 6 }, &weapon_preferences[0][0]
  },
  [dsda_config_weapon_choice_2] = {
    "weapon_choice_2", dsda_config_weapon_choice_2,
    dsda_config_int, 0, 9, { 9 }, &weapon_preferences[0][1]
  },
  [dsda_config_weapon_choice_3] = {
    "weapon_choice_3", dsda_config_weapon_choice_3,
    dsda_config_int, 0, 9, { 4 }, &weapon_preferences[0][2]
  },
  [dsda_config_weapon_choice_4] = {
    "weapon_choice_4", dsda_config_weapon_choice_4,
    dsda_config_int, 0, 9, { 3 }, &weapon_preferences[0][3]
  },
  [dsda_config_weapon_choice_5] = {
    "weapon_choice_5", dsda_config_weapon_choice_5,
    dsda_config_int, 0, 9, { 2 }, &weapon_preferences[0][4]
  },
  [dsda_config_weapon_choice_6] = {
    "weapon_choice_6", dsda_config_weapon_choice_6,
    dsda_config_int, 0, 9, { 8 }, &weapon_preferences[0][5]
  },
  [dsda_config_weapon_choice_7] = {
    "weapon_choice_7", dsda_config_weapon_choice_7,
    dsda_config_int, 0, 9, { 5 }, &weapon_preferences[0][6]
  },
  [dsda_config_weapon_choice_8] = {
    "weapon_choice_8", dsda_config_weapon_choice_8,
    dsda_config_int, 0, 9, { 7 }, &weapon_preferences[0][7]
  },
  [dsda_config_weapon_choice_9] = {
    "weapon_choice_9", dsda_config_weapon_choice_9,
    dsda_config_int, 0, 9, { 1 }, &weapon_preferences[0][8]
  },
  [dsda_config_flashing_hom] = {
    "flashing_hom", dsda_config_flashing_hom,
    BOOL_DEFAULT_OFF
  },
};

static void dsda_PersistIntConfig(dsda_config_t* conf) {
  conf->persistent_value.v_int = conf->transient_value.v_int;
}

static void dsda_PersistStringConfig(dsda_config_t* conf) {
  if (conf->persistent_value.v_string)
    Z_Free(conf->persistent_value.v_string);

  conf->persistent_value.v_string = Z_Strdup(conf->transient_value.v_string);
}

static void dsda_ConstrainIntConfig(dsda_config_t* conf) {
  if (conf->transient_value.v_int > conf->upper_limit)
    conf->transient_value.v_int = conf->upper_limit;
  else if (conf->transient_value.v_int < conf->lower_limit)
    conf->transient_value.v_int = conf->lower_limit;
}

static void dsda_PropagateIntConfig(dsda_config_t* conf) {
  if (conf->int_binding)
    *conf->int_binding = dsda_IntConfig(conf->id);
}

// No side effects
static void dsda_InitIntConfig(dsda_config_t* conf, int value) {
  conf->transient_value.v_int = value;

  dsda_ConstrainIntConfig(conf);
  dsda_PersistIntConfig(conf);
  dsda_PropagateIntConfig(conf);
}

// No side effects
static void dsda_InitStringConfig(dsda_config_t* conf, const char* value) {
  if (conf->transient_value.v_string)
    Z_Free(conf->transient_value.v_string);

  conf->transient_value.v_string = Z_Strdup(value);
  dsda_PersistStringConfig(conf);
}

void dsda_InitConfig(void) {
  int i;

  for (i = 1; i < dsda_config_count; ++i) {
    dsda_config_t* conf;

    conf = &dsda_config[i];

    if (conf->type == dsda_config_int)
      dsda_InitIntConfig(conf, conf->default_value.v_int);
    else if (conf->type == dsda_config_string)
      dsda_InitStringConfig(conf, conf->default_value.v_string);
  }
}

dboolean dsda_ReadConfig(const char* name, const char* string_param, int int_param) {
  int id;

  id = dsda_ConfigIDByName(name);

  if (id) {
    dsda_config_t* conf;

    conf = &dsda_config[id];

    if (conf->type == dsda_config_int && !string_param)
      dsda_InitIntConfig(conf, int_param);
    else if (conf->type == dsda_config_string && string_param)
      dsda_InitStringConfig(conf, string_param);

    return true;
  }

  return false;
}

void dsda_WriteConfig(dsda_config_identifier_t id, int key_length, FILE* file) {
  dsda_config_t* conf;

  conf = &dsda_config[id];

  if (conf->type == dsda_config_int)
    fprintf(file, "%-*s %i\n", key_length, conf->name, conf->persistent_value.v_int);
}

int dsda_ToggleConfig(dsda_config_identifier_t id, dboolean persist) {
  return dsda_UpdateIntConfig(id, !dsda_config[id].transient_value.v_int, persist);
}

int dsda_CycleConfig(dsda_config_identifier_t id, dboolean persist) {
  int value;

  value = dsda_config[id].transient_value.v_int + 1;

  if (value > dsda_config[id].upper_limit)
    value = dsda_config[id].lower_limit;

  return dsda_UpdateIntConfig(id, value, persist);
}

int dsda_UpdateIntConfig(dsda_config_identifier_t id, int value, dboolean persist) {
  dsda_config[id].transient_value.v_int = value;

  dsda_ConstrainIntConfig(&dsda_config[id]);

  if (persist)
    dsda_PersistIntConfig(&dsda_config[id]);

  dsda_PropagateIntConfig(&dsda_config[id]);

  if (dsda_config[id].onUpdate)
    dsda_config[id].onUpdate();

  return dsda_IntConfig(id);
}

const char* dsda_UpdateStringConfig(dsda_config_identifier_t id, const char* value, dboolean persist) {
  if (dsda_config[id].transient_value.v_string)
    Z_Free(dsda_config[id].transient_value.v_string);

  dsda_config[id].transient_value.v_string = Z_Strdup(value);

  if (persist)
    dsda_PersistStringConfig(&dsda_config[id]);

  return dsda_StringConfig(id);
}

int dsda_IntConfig(dsda_config_identifier_t id) {
  dboolean dsda_StrictMode(void);

  if (dsda_config[id].strict && dsda_StrictMode())
    return dsda_config[id].strict_value;

  return dsda_config[id].transient_value.v_int;
}

int dsda_PersistentIntConfig(dsda_config_identifier_t id) {
  return dsda_config[id].persistent_value.v_int;
}

const char* dsda_StringConfig(dsda_config_identifier_t id) {
  return dsda_config[id].transient_value.v_string;
}

const char* dsda_PersistentStringConfig(dsda_config_identifier_t id) {
  return dsda_config[id].persistent_value.v_string;
}

char* dsda_ConfigSummary(const char* name) {
  int id;
  char* summary = NULL;
  size_t length;

  id = dsda_ConfigIDByName(name);

  if (id) {
    dsda_config_t* conf;

    conf = &dsda_config[id];

    if (conf->type == dsda_config_int) {
      length = snprintf(NULL, 0,
                        "%s: %d (transient), %d (persistent)", conf->name,
                        conf->transient_value.v_int, conf->persistent_value.v_int);
      summary = Z_Malloc(length + 1);
      snprintf(summary, length + 1,
                        "%s: %d (transient), %d (persistent)", conf->name,
                        conf->transient_value.v_int, conf->persistent_value.v_int);
    }
    else if (conf->type == dsda_config_string) {
      length = snprintf(NULL, 0,
                        "%s: %s (transient), %s (persistent)", conf->name,
                        conf->transient_value.v_string, conf->persistent_value.v_string);
      summary = Z_Malloc(length + 1);
      snprintf(summary, length + 1,
                        "%s: %s (transient), %s (persistent)", conf->name,
                        conf->transient_value.v_string, conf->persistent_value.v_string);
    }

    return summary;
  }

  return NULL;
}

int dsda_ConfigIDByName(const char* name) {
  int i;

  for (i = 1; i < dsda_config_count; ++i)
    if (!strcmp(name, dsda_config[i].name))
      return i;

  return 0;
}
