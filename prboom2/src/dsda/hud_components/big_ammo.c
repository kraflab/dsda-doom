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
//	DSDA Big Ammo HUD Component
//

#include "base.h"

#include "big_ammo.h"

#define PATCH_DELTA_X 14

static dsda_patch_component_t component;

static void dsda_DrawComponent(void) {
  player_t* player;
  ammotype_t ammo_type;
  int ammo;

  if (hexen)
    return;

  player = &players[displayplayer];
  ammo_type = weaponinfo[player->readyweapon].ammo;

  if (ammo_type == am_noammo || !player->maxammo[ammo_type])
    return;

  ammo = player->ammo[ammo_type];

  dsda_DrawBigNumber(component.x, component.y, PATCH_DELTA_X, 0,
                     CR_DEFAULT, component.vpt, 3, ammo);
}

void dsda_InitBigAmmoHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  dsda_InitPatchHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigAmmoHC(void* data) {
  return;
}

void dsda_DrawBigAmmoHC(void* data) {
  dsda_DrawComponent();
}
