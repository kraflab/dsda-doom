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

#include "base.h"

#include "stat_totals.h"

#define STAT_STRING_SIZE 200

static dsda_text_t component;

static dboolean include_kills, include_items, include_secrets;
static const char* kills_format;
static const char* items_format;
static const char* secrets_format;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int i;
  size_t length;
  int fullkillcount, fullitemcount, fullsecretcount;
  int color, killcolor, itemcolor, secretcolor;
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

  if (respawnmonsters) {
    fullkillcount = kill_percent_count;
    max_kill_requirement = totalkills;
  }

  killcolor = (fullkillcount >= max_kill_requirement ? HUlib_Color(CR_BLUE) : HUlib_Color(CR_GOLD));
  secretcolor = (fullsecretcount >= totalsecret ? HUlib_Color(CR_BLUE) : HUlib_Color(CR_GOLD));
  itemcolor = (fullitemcount >= totalitems ? HUlib_Color(CR_BLUE) : HUlib_Color(CR_GOLD));

  if (include_kills) {
    length += snprintf(
      str,
      max_size,
      kills_format,
      HUlib_Color(CR_RED),
      killcolor, fullkillcount, max_kill_requirement
    );
  }

  if (include_items) {
    length += snprintf(
      str + length,
      max_size - length,
      items_format,
      HUlib_Color(CR_RED),
      itemcolor, players[displayplayer].itemcount, totalitems
    );
  }

  if (include_secrets) {
    snprintf(
      str + length,
      max_size - length,
      secrets_format,
      HUlib_Color(CR_RED),
      secretcolor, fullsecretcount, totalsecret
    );
  }
}

void dsda_InitStatTotalsHC(int x_offset, int y_offset, int vpt, int* args) {
  include_kills = args[0];
  include_items = args[1];
  include_secrets = args[2];

  // vertical orientation
  if (args[3]) {
    kills_format = "\x1b%cK \x1b%c%d/%d\n";
    items_format = "\x1b%cI \x1b%c%d/%d\n";
    secrets_format = "\x1b%cS \x1b%c%d/%d";
  }
  else {
    kills_format = "\x1b%cK \x1b%c%d/%d ";
    items_format = "\x1b%cI \x1b%c%d/%d ";
    secrets_format = "\x1b%cS \x1b%c%d/%d";
  }

  if (!include_kills && !include_items && !include_secrets) {
    include_kills = include_items = include_secrets = true;
  }

  dsda_InitTextHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateStatTotalsHC(void) {
  dsda_UpdateComponentText(component.msg, sizeof(component.msg));
  dsda_RefreshHudText(&component);
}

void dsda_DrawStatTotalsHC(void) {
  dsda_DrawBasicText(&component);
}
