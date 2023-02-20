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

static dsda_text_t component[6];

typedef struct {
  const char** ammo_name;
  const int* ammo_type;
  const int count;
} ammo_component_config_t;

static const char* doom_ammo_name[4] = {
  "BULL",
  "SHEL",
  "RCKT",
  "CELL",
};

static const char* rekkr_ammo_name[4] = {
  "SOUL",
  "SHOT",
  "RUNE",
  "MANA",
};

static const int doom_ammo_type[4] = { 0, 1, 3, 2 };

static const char* heretic_ammo_name[6] = {
  "CRYS",
  "BOLT",
  "CLAW",
  "RUNE",
  "FLAM",
  "MACE",
};

static const int heretic_ammo_type[6] = { 0, 1, 2, 3, 4, 5 };

static const char* hexen_ammo_name[2] = {
  "BLUE",
  "GREN",
};

static const int hexen_ammo_type[2] = { 0, 1 };

static const ammo_component_config_t doom_ammo = { doom_ammo_name, doom_ammo_type, 4 };
static const ammo_component_config_t rekkr_ammo = { rekkr_ammo_name, doom_ammo_type, 4 };
static const ammo_component_config_t heretic_ammo = { heretic_ammo_name, heretic_ammo_type, 6 };
static const ammo_component_config_t hexen_ammo = { hexen_ammo_name, hexen_ammo_type, 2 };

static const ammo_component_config_t* component_config;

static void dsda_UpdateComponentText(char* str, size_t max_size, int i) {
  player_t* player;
  int current_ammo, max_ammo;
  const char* name;

  player = &players[displayplayer];
  name = component_config->ammo_name[i];
  i = component_config->ammo_type[i];
  current_ammo = player->ammo[i];
  max_ammo = hexen ? MAX_MANA : player->maxammo[i];

  snprintf(
    str,
    max_size,
    "%s %3d\x1b\x01/\x1b\x01%3d",
    name,
    current_ammo,
    max_ammo
  );
}

void dsda_InitAmmoTextHC(int x_offset, int y_offset, int vpt, int* args) {
  int i;

  if (heretic)
    component_config = &heretic_ammo;
  else if (hexen)
    component_config = &hexen_ammo;
  else if (rekkr)
    component_config = &rekkr_ammo;
  else
    component_config = &doom_ammo;

  for (i = 0; i < component_config->count; ++i)
    dsda_InitTextHC(&component[i], x_offset, y_offset - i * 8, vpt);
}

void dsda_UpdateAmmoTextHC(void) {
  int i;

  for (i = 0; i < component_config->count; ++i) {
    dsda_UpdateComponentText(component[i].msg, sizeof(component[i].msg), i);
    dsda_RefreshHudText(&component[i]);
  }
}

void dsda_DrawAmmoTextHC(void) {
  int i;

  for (i = 0; i < component_config->count; ++i)
    dsda_DrawBasicText(&component[i]);
}
