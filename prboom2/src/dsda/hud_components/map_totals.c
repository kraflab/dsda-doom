//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA Map Totals HUD Component
//

#include "dsda/skill_info.h"

#include "base.h"
#include "stat_totals.h"

#include "map_totals.h"

typedef struct {
  dsda_text_t component;
  dboolean include_kills, include_items, include_secrets;
  dboolean hide_totals;
  int stats_count;
} local_component_t;

static local_component_t* local;

static const char* dsda_StatSeparator() {
  return local->stats_count > 0 ? "\n" : "";
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  size_t length;
  const char* killcolor;
  const char* itemcolor;
  const char* secretcolor;

  length = 0;
  local->stats_count = 0;

  killcolor   = (dsda_IsAllKills()    ? dsda_TextColor(dsda_tc_map_totals_max) :
                                        dsda_TextColor(dsda_tc_map_totals_value));
  itemcolor   = (dsda_IsAllItems()    ? dsda_TextColor(dsda_tc_map_totals_max) :
                                        dsda_TextColor(dsda_tc_map_totals_value));
  secretcolor = (dsda_IsAllSecrets()  ? dsda_TextColor(dsda_tc_map_totals_max) :
                                        dsda_TextColor(dsda_tc_map_totals_value));


  if (local->include_kills)   local->stats_count++;
  if (local->include_items)   local->stats_count++;
  if (local->include_secrets) local->stats_count++;

  if (local->include_kills)
  {
    local->stats_count--;

    length += snprintf(
      str + length,
      max_size - length,
      "%sMonsters: ",
      dsda_TextColor(dsda_tc_map_totals_label)
    );

    length += dsda_PrintStats(str + length, max_size - length, NULL, killcolor, dsda_GetCurrentKills(), dsda_GetMaxKills(), !local->hide_totals || dsda_IsAllKills(), dsda_StatSeparator());
  }

  if (local->include_items)
  {
    local->stats_count--;

    length += snprintf(
      str + length,
      max_size - length,
      "%sItems: ",
      dsda_TextColor(dsda_tc_map_totals_label)
    );

    length += dsda_PrintStats(str + length, max_size - length, NULL, itemcolor, dsda_GetCurrentItems(), dsda_GetMaxItems(), !local->hide_totals || dsda_IsAllItems(), dsda_StatSeparator());
  }

  if (local->include_secrets)
  {
    local->stats_count--;

    length += snprintf(
      str + length,
      max_size - length,
      "%sSecrets: ",
      dsda_TextColor(dsda_tc_map_totals_label)
    );

    length += dsda_PrintStats(str + length, max_size - length, NULL, secretcolor, dsda_GetCurrentSecrets(), dsda_GetMaxSecrets(), !local->hide_totals || dsda_IsAllSecrets(), dsda_StatSeparator());
  }
}

void dsda_InitMapTotalsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->include_kills = args[0];
  local->include_items = args[1];
  local->include_secrets = args[2];

  local->hide_totals = args[3];

  if (!local->include_kills && !local->include_items && !local->include_secrets)
    local->include_kills = local->include_items = local->include_secrets = true;

  dsda_InitBlockyHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateMapTotalsHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawMapTotalsHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
