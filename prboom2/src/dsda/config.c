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
//	DSDA Config
//

#include <string.h>

#include "z_zone.h"

#include "config.h"

dsda_config_t dsda_config[dsda_config_count];

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

// No side effects
static void dsda_InitIntConfig(dsda_config_t* conf, int value) {
  conf->transient_value.v_int = value;

  dsda_ConstrainIntConfig(conf);
  dsda_PersistIntConfig(conf);
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
  int i;

  for (i = 1; i < dsda_config_count; ++i) {
    dsda_config_t* conf;

    conf = &dsda_config[i];

    if (!strcmp(name, conf->name)) {
      if (conf->type == dsda_config_int && !string_param)
        dsda_InitIntConfig(conf, int_param);
      else if (conf->type == dsda_config_string && string_param)
        dsda_InitStringConfig(conf, string_param);

      return true;
    }
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

  if (dsda_config[id].strict_value >= 0 && dsda_StrictMode())
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
