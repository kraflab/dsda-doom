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
//	DSDA Big Armor HUD Component
//

#include "base.h"

#include "big_armor.h"

#define PATCH_DELTA_X 14
#define PATCH_SPACING 2
#define PATCH_VERTICAL_SPACING 1

static dsda_patch_component_t component;
static int armor_lump_green;
static int armor_lump_blue;

static void dsda_DrawComponent(void) {
  player_t* player;
  int x, y;
  int cm;

  player = &players[displayplayer];
  x = component.x;
  y = component.y;

  if (!player->armorpoints[ARMOR_ARMOR] || player->armortype < 2) {
    cm = g_cr_green;
    V_DrawNumPatch(x, y, FG, armor_lump_green, CR_DEFAULT, component.vpt);
  }
  else {
    cm = CR_BLUE2;
    V_DrawNumPatch(x, y, FG, armor_lump_blue, CR_DEFAULT, component.vpt);
  }

  x += R_NumPatchWidth(armor_lump_green) + PATCH_SPACING;
  y += PATCH_VERTICAL_SPACING;

  dsda_DrawBigNumber(x, y, PATCH_DELTA_X, 0,
                     cm, component.vpt, 3, player->armorpoints[ARMOR_ARMOR]);
}

void dsda_InitBigArmorHC(int x_offset, int y_offset, int vpt) {
  armor_lump_green = R_NumPatchForSpriteIndex(SPR_ARM1);
  armor_lump_blue = R_NumPatchForSpriteIndex(SPR_ARM2);
  dsda_InitPatchHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArmorHC(void) {
  return;
}

void dsda_DrawBigArmorHC(void) {
  dsda_DrawComponent();
}

void dsda_EraseBigArmorHC(void) {
  return;
}
