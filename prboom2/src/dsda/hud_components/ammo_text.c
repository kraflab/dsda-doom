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
//	DSDA Ammo Text HUD Component
//

#include "base.h"

#include "ammo_text.h"

#define AMMO_COMPONENT_COUNT 4

static dsda_text_t component[AMMO_COMPONENT_COUNT];

static const char* ammo_name[AMMO_COMPONENT_COUNT] = {
  "BULL",
  "SHEL",
  "RCKT",
  "CELL",
};

static const int ammo_type[AMMO_COMPONENT_COUNT] = { 0, 1, 3, 2 };

static void dsda_UpdateComponentText(char* str, size_t max_size, int i) {
  player_t* player;
  int current_ammo, max_ammo;
  const char* name;

  player = &players[displayplayer];
  name = ammo_name[i];
  i = ammo_type[i];
  current_ammo = player->ammo[i];
  max_ammo = player->maxammo[i];

  snprintf(
    str,
    max_size,
    "%s %3d / %3d",
    name,
    current_ammo,
    max_ammo
  );
}

void dsda_InitAmmoTextHC(int x_offset, int y_offset, int vpt) {
  int i;

  for (i = 0; i < AMMO_COMPONENT_COUNT; ++i) {
    dsda_InitTextHC(&component[i], x_offset, y_offset - i * 8, vpt);
    component[i].text.space_width = 5;
  }
}

void dsda_UpdateAmmoTextHC(void) {
  int i;

  for (i = 0; i < AMMO_COMPONENT_COUNT; ++i) {
    dsda_UpdateComponentText(component[i].msg, sizeof(component[i].msg), i);
    dsda_RefreshHudText(&component[i]);
  }
}

void dsda_DrawAmmoTextHC(void) {
  int i;

  for (i = 0; i < AMMO_COMPONENT_COUNT; ++i)
    HUlib_drawTextLine(&component[i].text, false);
}

void dsda_EraseAmmoTextHC(void) {
  int i;

  for (i = 0; i < AMMO_COMPONENT_COUNT; ++i)
    HUlib_eraseTextLine(&component[i].text);
}
