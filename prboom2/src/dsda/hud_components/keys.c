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
//	DSDA Keys HUD Component
//

#include "base.h"

#include "keys.h"

#define PATCH_DELTA_Y 10

static dsda_patch_component_t component;

static void dsda_DrawComponent(void) {
  player_t* player;
  int x, y;
  const char* name;

  player = &players[displayplayer];

  x = component.x;
  y = component.y;

  if (player->cards[0] && player->cards[3])
    name = "STKEYS6";
  else if (player->cards[0])
    name = "STKEYS0";
  else if (player->cards[3])
    name = "STKEYS3";
  else
    name = NULL;

  if (name)
    V_DrawNamePatch(x, y, FG, name, CR_DEFAULT, component.vpt);

  y += PATCH_DELTA_Y;

  if (player->cards[1] && player->cards[4])
    name = "STKEYS7";
  else if (player->cards[1])
    name = "STKEYS1";
  else if (player->cards[4])
    name = "STKEYS4";
  else
    name = NULL;

  if (name)
    V_DrawNamePatch(x, y, FG, name, CR_DEFAULT, component.vpt);

  y += PATCH_DELTA_Y;

  if (player->cards[2] && player->cards[5])
    name = "STKEYS8";
  else if (player->cards[2])
    name = "STKEYS2";
  else if (player->cards[5])
    name = "STKEYS5";
  else
    name = NULL;

  if (name)
    V_DrawNamePatch(x, y, FG, name, CR_DEFAULT, component.vpt);
}

void dsda_InitKeysHC(int x_offset, int y_offset, int vpt) {
  dsda_InitPatchHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateKeysHC(void) {
  return;
}

void dsda_DrawKeysHC(void) {
  dsda_DrawComponent();
}

void dsda_EraseKeysHC(void) {
  return;
}
