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
//	DSDA Stat Totals HUD Component
//

#include "dsda/skill_info.h"

#include "base.h"

#include "stat_totals.h"

typedef struct {
  dsda_text_t component;
  dboolean include_kills, include_items, include_secrets;
  dboolean hide_totals;
  const char* label_k;
  const char* label_i;
  const char* label_s;
  const char* stat_separator;
  int stats_count;
} local_component_t;

static local_component_t* local;

int dsda_PrintStats(char *buffer, size_t size, const char* label, const char* cm, const int th_count, const int th_total, dboolean show_totals, const char *separator)
{
    char print_stats[32] = "";
    int has_label = (label != NULL);

    if (show_totals)
      sprintf(print_stats, "%d/%d", th_count, th_total);
    else 
      sprintf(print_stats, "%d", th_count);

    if (has_label)
    {
      return snprintf( buffer, size,
                        "%s%s%s%s%s",
                        dsda_TextColor(dsda_tc_exhud_totals_label),
                        label, cm, print_stats,
                        separator ? separator : "");
    }
    else
    {
      return snprintf( buffer, size,
                        "%s%s%s",
                        cm, print_stats,
                        separator ? separator : "");
    }
}

static const char* dsda_StatSeparator() {
  return local->stats_count > 0 ? local->stat_separator : "";
}

static void dsda_LevelStats(char* str, size_t max_size) {
  size_t length;
  const char* killcolor;
  const char* itemcolor;
  const char* secretcolor;

  length = 0;
  local->stats_count = 0;

  killcolor   = (dsda_IsAllKills()    ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                        dsda_TextColor(dsda_tc_exhud_totals_value));
  itemcolor   = (dsda_IsAllItems()    ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                        dsda_TextColor(dsda_tc_exhud_totals_value));
  secretcolor = (dsda_IsAllSecrets()  ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                        dsda_TextColor(dsda_tc_exhud_totals_value));

  if (local->include_kills)   local->stats_count++;
  if (local->include_items)   local->stats_count++;
  if (local->include_secrets) local->stats_count++;

  if (local->include_kills)
  {
    local->stats_count--;
    length += dsda_PrintStats(str + length, max_size - length, local->label_k, killcolor, dsda_GetCurrentKills(), dsda_GetMaxKills(), !local->hide_totals || dsda_IsAllKills(), dsda_StatSeparator());
  }

  if (local->include_items)
  {
    local->stats_count--;
    length += dsda_PrintStats(str + length, max_size - length, local->label_i, itemcolor, dsda_GetCurrentItems(), dsda_GetMaxItems(), !local->hide_totals || dsda_IsAllItems(), dsda_StatSeparator());
  }

  if (local->include_secrets)
  {
    local->stats_count--;
    length += dsda_PrintStats(str + length, max_size - length, local->label_s, secretcolor, dsda_GetCurrentSecrets(), dsda_GetMaxSecrets(), !local->hide_totals || dsda_IsAllSecrets(), dsda_StatSeparator());
  }
}

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  dsda_LevelStats(str, max_size);
}

void dsda_InitStatTotalsHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  *data = Z_Calloc(1, sizeof(local_component_t));
  local = *data;

  local->include_kills = args[0];
  local->include_items = args[1];
  local->include_secrets = args[2];

  // vertical orientation
  local->stat_separator = args[3] ? "\n" : " ";

  if (arg_count < 5 || args[4]) {
    local->label_k = "K ";
    local->label_i = "I ";
    local->label_s = "S ";
  }
  else {
    local->label_k = "";
    local->label_i = "";
    local->label_s = "";
  }

  local->hide_totals = args[5];

  if (!local->include_kills && !local->include_items && !local->include_secrets) {
    local->include_kills = local->include_items = local->include_secrets = true;
  }

  dsda_InitTextHC(&local->component, x_offset, y_offset, vpt);
}

void dsda_UpdateStatTotalsHC(void* data) {
  local = data;

  dsda_UpdateComponentText(local->component.msg, sizeof(local->component.msg));
  dsda_RefreshHudText(&local->component);
}

void dsda_DrawStatTotalsHC(void* data) {
  local = data;

  dsda_DrawBasicText(&local->component);
}
