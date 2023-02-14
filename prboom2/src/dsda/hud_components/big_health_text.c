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
//	DSDA Big Health Text HUD Component
//

#include "base.h"

#include "big_health_text.h"

static dsda_patch_component_t component;
static int patch_delta_x;

static void dsda_DrawComponent(void) {
  player_t* player;
  int cm;

  player = &players[displayplayer];

  cm = player->health <= hud_health_red ? CR_RED :
       player->health <= hud_health_yellow ? CR_GOLD :
       player->health <= hud_health_green ? CR_GREEN :
       CR_BLUE2;

  dsda_DrawBigNumber(component.x, component.y, patch_delta_x, 0,
                     cm, component.vpt, 3, player->health);
}

void dsda_InitBigHealthTextHC(int x_offset, int y_offset, int vpt) {
  if (heretic)
    patch_delta_x = 10;
  else if (hexen)
    patch_delta_x = 10;
  else
    patch_delta_x = 14;

  dsda_InitPatchHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigHealthTextHC(void) {
  return;
}

void dsda_DrawBigHealthTextHC(void) {
  dsda_DrawComponent();
}
