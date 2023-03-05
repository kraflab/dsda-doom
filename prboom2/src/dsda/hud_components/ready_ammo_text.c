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
//	DSDA Ready Ammo Text HUD Component
//

#include "base.h"

#include "ready_ammo_text.h"

static dsda_text_t component;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  player_t* player;

  player = &players[displayplayer];

  if (hexen) {
    snprintf(str, max_size, "AMM \x1b%c%3d \x1b%c%3d",
            HUlib_Color(CR_LIGHTBLUE), player->ammo[0],
            HUlib_Color(CR_GREEN), player->ammo[1]);
  }
  else {
    ammotype_t ammo_type;

    ammo_type = weaponinfo[player->readyweapon].ammo;

    if (ammo_type == am_noammo || !player->maxammo[ammo_type])
      snprintf(str, max_size, "AMM N/A");
    else
      snprintf(str, max_size, "AMM %3d", player->ammo[ammo_type]);
  }
}

void dsda_InitReadyAmmoTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count) {
  dsda_InitTextHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateReadyAmmoTextHC(void) {
  dsda_UpdateComponentText(component.msg, sizeof(component.msg));
  dsda_RefreshHudText(&component);
}

void dsda_DrawReadyAmmoTextHC(void) {
  dsda_DrawBasicText(&component);
}
