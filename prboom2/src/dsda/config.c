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

#include "z_zone.h"

#include "config.h"

dsda_config_t dsda_config[dsda_config_count];

static void dsda_PersistIntConfig(dsda_config_identifier_t id) {
  dsda_config[id].persistent_value.v_int = dsda_config[id].transient_value.v_int;
}

static void dsda_PersistStringConfig(dsda_config_identifier_t id) {
  if (dsda_config[id].persistent_value.v_string)
    Z_Free(dsda_config[id].persistent_value.v_string);

  dsda_config[id].persistent_value.v_string = Z_Strdup(dsda_config[id].transient_value.v_string);
}

static void dsda_ConstrainIntConfig(dsda_config_identifier_t id) {
  if (dsda_config[id].transient_value.v_int > dsda_config[id].upper_limit)
    dsda_config[id].transient_value.v_int = dsda_config[id].upper_limit;
  else if (dsda_config[id].transient_value.v_int < dsda_config[id].lower_limit)
    dsda_config[id].transient_value.v_int = dsda_config[id].lower_limit;
}

int dsda_ToggleConfig(dsda_config_identifier_t id, dboolean persist) {
  dsda_config[id].transient_value.v_int = !dsda_config[id].transient_value.v_int;

  if (persist)
    dsda_PersistIntConfig(id);

  return dsda_IntConfig(id);
}

int dsda_CycleConfig(dsda_config_identifier_t id, dboolean persist) {
  ++dsda_config[id].transient_value.v_int;

  if (dsda_config[id].transient_value.v_int > dsda_config[id].upper_limit)
    dsda_config[id].transient_value.v_int = dsda_config[id].lower_limit;

  if (persist)
    dsda_PersistIntConfig(id);

  return dsda_IntConfig(id);
}

int dsda_UpdateIntConfig(dsda_config_identifier_t id, int value, dboolean persist) {
  dsda_config[id].transient_value.v_int = value;

  dsda_ConstrainIntConfig(id);

  if (persist)
    dsda_PersistIntConfig(id);

  return dsda_IntConfig(id);
}

const char* dsda_UpdateStringConfig(dsda_config_identifier_t id, const char* value, dboolean persist) {
  if (dsda_config[id].transient_value.v_string)
    Z_Free(dsda_config[id].transient_value.v_string);

  dsda_config[id].transient_value.v_string = Z_Strdup(value);

  if (persist)
    dsda_PersistStringConfig(id);

  return dsda_StringConfig(id);
}

int dsda_IntConfig(dsda_config_identifier_t id) {
  dboolean dsda_StrictMode(void);

  if (dsda_config[id].strict_value >= 0 && dsda_StrictMode())
    return dsda_config[id].strict_value;

  return dsda_config[id].transient_value.v_int;
}

const char* dsda_StringConfig(dsda_config_identifier_t id) {
  return dsda_config[id].transient_value.v_string;
}
