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
//	DSDA Big Health HUD Component
//

#include "base.h"

#include "big_health.h"

#define PATCH_DELTA_X 14
#define PATCH_SPACING 2
#define PATCH_VERTICAL_SPACING 2

static dsda_patch_component_t component;
static int health_lump;
static int patch_delta_x;
static int patch_vertical_spacing;
static int patch_spacing;

static void dsda_DrawComponent(void) {
  player_t* player;
  int health;
  int x, y;
  int cm;

  player = &players[displayplayer];
  x = component.x;
  y = component.y;

  cm = player->health <= hud_health_red ? CR_RED :
       player->health <= hud_health_yellow ? CR_GOLD :
       player->health <= hud_health_green ? CR_GREEN :
       CR_LIGHTBLUE;

  V_DrawNumPatch(x, y, FG, health_lump, CR_DEFAULT, component.vpt);

  x += patch_spacing;
  y += patch_vertical_spacing;

  health = player->health < 0 ? 0 : player->health;

  dsda_DrawBigNumber(x, y, patch_delta_x, 0,
                     cm, component.vpt, 3, player->health);
}

void dsda_InitBigHealthHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  if (heretic) {
    health_lump = R_NumPatchForSpriteIndex(HERETIC_SPR_PTN2);
    patch_delta_x = 10;
    patch_vertical_spacing = 6;
    patch_spacing = 4;
  }
  else if (hexen) {
    health_lump = R_NumPatchForSpriteIndex(HEXEN_SPR_PTN2);
    patch_delta_x = 10;
    patch_vertical_spacing = 6;
    patch_spacing = 4;
  }
  else {
    health_lump = R_NumPatchForSpriteIndex(SPR_MEDI);
    patch_delta_x = 14;
    patch_vertical_spacing = 2;
    patch_spacing = 2;
  }
  patch_spacing += R_NumPatchWidth(health_lump);
  dsda_InitPatchHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigHealthHC(void* data) {
  return;
}

void dsda_DrawBigHealthHC(void* data) {
  dsda_DrawComponent();
}
