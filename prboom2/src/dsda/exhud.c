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

#include "hu_stuff.h"
#include "r_main.h"
#include "v_video.h"

#include "dsda/global.h"
#include "dsda/hud_components.h"
#include "dsda/settings.h"

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
};

#define DSDA_EXHUD_X 2

int exhud_color_default;
int exhud_color_warning;
int exhud_color_alert;

static void dsda_TurnComponentOn(int id, int x, int y, int vpt) {
  components[id].on = true;
  components[id].init(x, y, vpt | components[id].default_vpt | VPT_EX_TEXT);
}

void dsda_InitExHud(void) {
  int i;

  exhud_color_default = CR_GRAY;
  exhud_color_warning = CR_GREEN;
  exhud_color_alert = CR_RED;

  for (i = 0; i < exhud_component_count; ++i)
    components[i].on = false;

  if (viewheight != SCREENHEIGHT) {
    dsda_TurnComponentOn(exhud_tracker, DSDA_EXHUD_X, 32, VPT_ALIGN_LEFT_BOTTOM);
    dsda_TurnComponentOn(exhud_composite_time, DSDA_EXHUD_X, 16, VPT_ALIGN_LEFT_BOTTOM);
    dsda_TurnComponentOn(exhud_stat_totals, DSDA_EXHUD_X, 8, VPT_ALIGN_LEFT_BOTTOM);
  }
  else if (hud_displayed) {
    int y = 0;

    dsda_TurnComponentOn(exhud_ready_ammo_text, DSDA_EXHUD_X, (y += 8), VPT_ALIGN_LEFT_BOTTOM);
    dsda_TurnComponentOn(exhud_health_text, DSDA_EXHUD_X, (y += 8), VPT_ALIGN_LEFT_BOTTOM);
    dsda_TurnComponentOn(exhud_armor_text, DSDA_EXHUD_X + (10 * 5), y, VPT_ALIGN_LEFT_BOTTOM);
    dsda_TurnComponentOn(exhud_weapon_text, DSDA_EXHUD_X, (y += 8), VPT_ALIGN_LEFT_BOTTOM);
    dsda_TurnComponentOn(exhud_stat_totals, DSDA_EXHUD_X, (y += 8), VPT_ALIGN_LEFT_BOTTOM);
    dsda_TurnComponentOn(exhud_composite_time, DSDA_EXHUD_X, (y += 8), VPT_ALIGN_LEFT_BOTTOM);
    dsda_TurnComponentOn(exhud_tracker, DSDA_EXHUD_X, (y += 16), VPT_ALIGN_LEFT_BOTTOM);

    dsda_TurnComponentOn(exhud_keys, 320 - DSDA_EXHUD_X - (16 * 5), 30, VPT_ALIGN_RIGHT_BOTTOM);

    dsda_TurnComponentOn(exhud_ammo_text, 320 - DSDA_EXHUD_X - (14 * 5), 32, VPT_ALIGN_RIGHT_BOTTOM);
  }
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
