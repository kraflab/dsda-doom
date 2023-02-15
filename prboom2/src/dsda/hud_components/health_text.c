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
//	DSDA Health Text HUD Component
//

#include "base.h"

#include "health_text.h"

static dsda_text_t component;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  player_t* player;

  player = &players[displayplayer];

  snprintf(
    str,
    max_size,
    "\x1b%cHEL %3d%%",
    player->health <= hud_health_red ? HUlib_Color(CR_RED) :
      player->health <= hud_health_yellow ? HUlib_Color(CR_GOLD) :
      player->health <= hud_health_green ? HUlib_Color(CR_GREEN) :
      HUlib_Color(CR_BLUE),
    player->health
  );
}

void dsda_InitHealthTextHC(int x_offset, int y_offset, int vpt) {
  dsda_InitTextHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateHealthTextHC(void) {
  dsda_UpdateComponentText(component.msg, sizeof(component.msg));
  dsda_RefreshHudText(&component);
}

void dsda_DrawHealthTextHC(void) {
  dsda_DrawBasicText(&component);
}
