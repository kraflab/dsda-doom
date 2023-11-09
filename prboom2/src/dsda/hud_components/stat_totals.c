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
} local_component_t;

static local_component_t* local;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int i;
  size_t length;
  int fullkillcount, fullitemcount, fullsecretcount;
  const char* killcolor;
  const char* itemcolor;
  const char* secretcolor;
  int kill_percent_count;
  int max_kill_requirement;

  length = 0;
  fullkillcount = 0;
  fullitemcount = 0;
  fullsecretcount = 0;
  kill_percent_count = 0;
  max_kill_requirement = dsda_MaxKillRequirement();

  for (i = 0; i < g_maxplayers; ++i) {
    if (playeringame[i]) {
      fullkillcount += players[i].killcount - players[i].maxkilldiscount;
      fullitemcount += players[i].itemcount;
      fullsecretcount += players[i].secretcount;
      kill_percent_count += players[i].killcount;
    }
  }

  if (skill_info.respawn_time) {
    fullkillcount = kill_percent_count;
    max_kill_requirement = totalkills;
  }

  killcolor = (fullkillcount >= max_kill_requirement ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                                       dsda_TextColor(dsda_tc_exhud_totals_value));
  secretcolor = (fullsecretcount >= totalsecret ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                                  dsda_TextColor(dsda_tc_exhud_totals_value));
  itemcolor = (fullitemcount >= totalitems ? dsda_TextColor(dsda_tc_exhud_totals_max) :
                                             dsda_TextColor(dsda_tc_exhud_totals_value));

  if (local->include_kills) {
    if (!local->hide_totals || fullkillcount >= max_kill_requirement)
      length += snprintf(
        str,
        max_size,
        "%s%s%s%d/%d%s",
        dsda_TextColor(dsda_tc_exhud_totals_label),
        local->label_k,
        killcolor, fullkillcount, max_kill_requirement,
        local->stat_separator
      );
    else
      length += snprintf(
        str,
        max_size,
        "%s%s%s%d%s",
        dsda_TextColor(dsda_tc_exhud_totals_label),
        local->label_k,
        killcolor, fullkillcount,
        local->stat_separator
      );
  }

  if (local->include_items) {
    if (!local->hide_totals || fullitemcount >= totalitems)
      length += snprintf(
        str + length,
        max_size - length,
        "%s%s%s%d/%d%s",
        dsda_TextColor(dsda_tc_exhud_totals_label),
        local->label_i,
        itemcolor, fullitemcount, totalitems,
        local->stat_separator
      );
    else
      length += snprintf(
        str + length,
        max_size - length,
        "%s%s%s%d%s",
        dsda_TextColor(dsda_tc_exhud_totals_label),
        local->label_i,
        itemcolor, fullitemcount,
        local->stat_separator
      );
  }

  if (local->include_secrets) {
    if (!local->hide_totals || fullsecretcount >= totalsecret)
      snprintf(
        str + length,
        max_size - length,
        "%s%s%s%d/%d",
        dsda_TextColor(dsda_tc_exhud_totals_label),
        local->label_s,
        secretcolor, fullsecretcount, totalsecret
      );
    else
      snprintf(
        str + length,
        max_size - length,
        "%s%s%s%d",
        dsda_TextColor(dsda_tc_exhud_totals_label),
        local->label_s,
        secretcolor, fullsecretcount
      );
  }
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
