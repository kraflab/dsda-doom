//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Extended HUD
//

#include <stdio.h>

#include "hu_stuff.h"
#include "lprintf.h"
#include "r_main.h"
#include "v_video.h"
#include "w_wad.h"

#include "dsda/global.h"
#include "dsda/hud_components.h"
#include "dsda/render_stats.h"
#include "dsda/settings.h"
#include "dsda/utility.h"

#include "exhud.h"

typedef struct {
  void (*init)(int x_offset, int y_offset, int vpt_flags);
  void (*update)(void);
  void (*draw)(void);
  void (*erase)(void);
  const char* name;
  int default_vpt;
  dboolean strict;
  dboolean on;
  dboolean initialized;
} exhud_component_t;

typedef enum {
  exhud_ammo_text,
  exhud_armor_text,
  exhud_big_ammo,
  exhud_big_armor,
  exhud_big_health,
  exhud_composite_time,
  exhud_health_text,
  exhud_keys,
  exhud_ready_ammo_text,
  exhud_speed_text,
  exhud_stat_totals,
  exhud_tracker,
  exhud_weapon_text,
  exhud_render_stats,
  exhud_fps,
  exhud_component_count,
} exhud_component_id_t;

exhud_component_t components[exhud_component_count] = {
  [exhud_ammo_text] = {
    dsda_InitAmmoTextHC,
    dsda_UpdateAmmoTextHC,
    dsda_DrawAmmoTextHC,
    dsda_EraseAmmoTextHC,
    "ammo_text"
  },
  [exhud_armor_text] = {
    dsda_InitArmorTextHC,
    dsda_UpdateArmorTextHC,
    dsda_DrawArmorTextHC,
    dsda_EraseArmorTextHC,
    "armor_text"
  },
  [exhud_big_ammo] = {
    dsda_InitBigAmmoHC,
    dsda_UpdateBigAmmoHC,
    dsda_DrawBigAmmoHC,
    dsda_EraseBigAmmoHC,
    "big_ammo"
  },
  [exhud_big_armor] = {
    dsda_InitBigArmorHC,
    dsda_UpdateBigArmorHC,
    dsda_DrawBigArmorHC,
    dsda_EraseBigArmorHC,
    "big_armor",
    VPT_NOOFFSET
  },
  [exhud_big_health] = {
    dsda_InitBigHealthHC,
    dsda_UpdateBigHealthHC,
    dsda_DrawBigHealthHC,
    dsda_EraseBigHealthHC,
    "big_health",
    VPT_NOOFFSET
  },
  [exhud_composite_time] = {
    dsda_InitCompositeTimeHC,
    dsda_UpdateCompositeTimeHC,
    dsda_DrawCompositeTimeHC,
    dsda_EraseCompositeTimeHC,
    "composite_time"
  },
  [exhud_health_text] = {
    dsda_InitHealthTextHC,
    dsda_UpdateHealthTextHC,
    dsda_DrawHealthTextHC,
    dsda_EraseHealthTextHC,
    "health_text"
  },
  [exhud_keys] = {
    dsda_InitKeysHC,
    dsda_UpdateKeysHC,
    dsda_DrawKeysHC,
    dsda_EraseKeysHC,
    "keys",
    VPT_NOOFFSET
  },
  [exhud_ready_ammo_text] = {
    dsda_InitReadyAmmoTextHC,
    dsda_UpdateReadyAmmoTextHC,
    dsda_DrawReadyAmmoTextHC,
    dsda_EraseReadyAmmoTextHC,
    "ready_ammo_text"
  },
  [exhud_speed_text] = {
    dsda_InitSpeedTextHC,
    dsda_UpdateSpeedTextHC,
    dsda_DrawSpeedTextHC,
    dsda_EraseSpeedTextHC,
    "speed_text"
  },
  [exhud_stat_totals] = {
    dsda_InitStatTotalsHC,
    dsda_UpdateStatTotalsHC,
    dsda_DrawStatTotalsHC,
    dsda_EraseStatTotalsHC,
    "stat_totals"
  },
  [exhud_tracker] = {
    dsda_InitTrackerHC,
    dsda_UpdateTrackerHC,
    dsda_DrawTrackerHC,
    dsda_EraseTrackerHC,
    "tracker",
    VPT_NONE,
    true
  },
  [exhud_weapon_text] = {
    dsda_InitWeaponTextHC,
    dsda_UpdateWeaponTextHC,
    dsda_DrawWeaponTextHC,
    dsda_EraseWeaponTextHC,
    "weapon_text"
  },
  [exhud_render_stats] = {
    dsda_InitRenderStatsHC,
    dsda_UpdateRenderStatsHC,
    dsda_DrawRenderStatsHC,
    dsda_EraseRenderStatsHC,
    "render_stats",
    VPT_NONE,
    true
  },
  [exhud_fps] = {
    dsda_InitFPSHC,
    dsda_UpdateFPSHC,
    dsda_DrawFPSHC,
    dsda_EraseFPSHC,
    "fps"
  },
};

#define DSDA_EXHUD_X 2

int exhud_color_default;
int exhud_color_warning;
int exhud_color_alert;

int dsda_show_render_stats;

static void dsda_TurnComponentOn(int id) {
  if (!components[id].initialized)
    return;

  components[id].on = true;
}

static void dsda_TurnComponentOff(int id) {
  components[id].on = false;
}

static void dsda_InitializeComponent(int id, int x, int y, int vpt) {
  components[id].initialized = true;
  components[id].init(x, y, vpt | components[id].default_vpt | VPT_EX_TEXT);
}

void dsda_InitExHud(void) {
  int i;
  static char* lump;
  static char** lines;
  const char* line;
  int line_i;
  char target[16];
  dboolean reading = false;
  char command[64];
  char args[64];
  int count;

  exhud_color_default = CR_GRAY;
  exhud_color_warning = CR_GREEN;
  exhud_color_alert = CR_RED;

  for (i = 0; i < exhud_component_count; ++i) {
    components[i].on = false;
    components[i].initialized = false;
  }

  if (R_FullView() && !hud_displayed)
    return;

  if (!lump)
    lump = W_ReadLumpToString(W_GetNumForName("DSDAHUD"));

  if (!lump)
    return;

  snprintf(target, sizeof(target), "%s %s",
           hexen ? "hexen" : heretic ? "heretic" : "doom",
           R_FullView() ? "full" : "ex");

  if (!lines)
    lines = dsda_SplitString(lump, "\n\r");

  if (!lines)
    return;

  for (line_i = 0; lines[line_i]; ++line_i) {
    line = lines[line_i];

    if (!strncmp(target, line, sizeof(target)))
      reading = true;
    else if (reading) {
      int count;
      dboolean found = false;

      if (line[0] == '#' || line[0] == '/' || line[0] == '!' || !line[0])
        continue;

      count = sscanf(line, "%63s %63[^\n\r]", command, args);
      if (count != 2)
        I_Error("Invalid hud definition \"%s\"", line);

      // The start of another definition
      if (!strncmp(command, "doom", sizeof(command)) ||
          !strncmp(command, "heretic", sizeof(command)) ||
          !strncmp(command, "hexen", sizeof(command)))
        break;

      for (i = 0; i < exhud_component_count; ++i)
        if (!strncmp(command, components[i].name, sizeof(command))) {
          int x, y, vpt, always_on;
          char alignment[16];

          found = true;

          count = sscanf(args, "%d %d %15s %d", &x, &y, alignment, &always_on);
          if (count != 4)
            I_Error("Invalid hud component args \"%s\"", line);

          if (!strncmp(alignment, "bottom_left", sizeof(alignment)))
            vpt = VPT_ALIGN_LEFT_BOTTOM;
          else if (!strncmp(alignment, "bottom_right", sizeof(alignment)))
            vpt = VPT_ALIGN_RIGHT_BOTTOM;
          else if (!strncmp(alignment, "top_left", sizeof(alignment)))
            vpt = VPT_ALIGN_LEFT_TOP;
          else if (!strncmp(alignment, "top_right", sizeof(alignment)))
            vpt = VPT_ALIGN_RIGHT_TOP;
          else
            I_Error("Invalid hud component alignment \"%s\"", line);

          dsda_InitializeComponent(i, x, y, vpt);

          if (always_on)
            dsda_TurnComponentOn(i);
          else
            dsda_TurnComponentOff(i);
        }

      if (!found)
        I_Error("Invalid hud component \"%s\"", line);
    }
  }

  if (dsda_show_render_stats)
    dsda_TurnComponentOn(exhud_render_stats);
}

void dsda_UpdateExHud(void) {
  int i;

  for (i = 0; i < exhud_component_count; ++i)
    if (components[i].on && (!components[i].strict || !dsda_StrictMode()))
      components[i].update();
}

void dsda_DrawExHud(void) {
  int i;

  for (i = 0; i < exhud_component_count; ++i)
    if (components[i].on && (!components[i].strict || !dsda_StrictMode()))
      components[i].draw();
}

void dsda_EraseExHud(void) {
  int i;

  for (i = 0; i < exhud_component_count; ++i)
    if (components[i].on && (!components[i].strict || !dsda_StrictMode()))
      components[i].erase();
}

void dsda_ToggleRenderStats(void) {
  dsda_show_render_stats = !dsda_show_render_stats;

  if (components[exhud_render_stats].on && !dsda_show_render_stats)
    dsda_TurnComponentOff(exhud_render_stats);
  else if (!components[exhud_render_stats].on && dsda_show_render_stats) {
    dsda_BeginRenderStats();
    dsda_TurnComponentOn(exhud_render_stats);
  }
}
