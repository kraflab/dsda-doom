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

static dsda_patch_component_t component;
static int armor_lump_green;
static int armor_lump_blue;
static int patch_delta_x;
static int patch_vertical_spacing;
static int patch_spacing;

static void dsda_DrawComponent(void) {
  player_t* player;
  int x, y;
  int cm;
  int lump;
  int armor;

  player = &players[displayplayer];
  x = component.x;
  y = component.y;

  if (hexen) {
    armor = dsda_HexenArmor(player);
    cm = CR_GRAY;
    lump = armor_lump_green;
  }
  else {
    armor = player->armorpoints[ARMOR_ARMOR];
    if (!hud_armor_color_by_class || armor <= 0) {
      cm = CR_RED;
      lump = armor_lump_green;
    }
    else if (player->armortype < 2) {
      cm = CR_GREEN;
      lump = armor_lump_green;
    }
    else {
      cm = CR_BLUE2;
      lump = armor_lump_blue;
    }
  }

  V_DrawNumPatch(x, y, FG, lump, CR_DEFAULT, component.vpt);

  x += patch_spacing;
  y += patch_vertical_spacing;

  dsda_DrawBigNumber(x, y, patch_delta_x, 0,
                     cm, component.vpt, 3, armor);
}

void dsda_InitBigArmorHC(int x_offset, int y_offset, int vpt) {
  if (heretic) {
    armor_lump_green = R_NumPatchForSpriteIndex(HERETIC_SPR_SHLD);
    armor_lump_blue = R_NumPatchForSpriteIndex(HERETIC_SPR_SHD2);
    patch_delta_x = 10;
    patch_vertical_spacing = 6;
    patch_spacing = 2;
  }
  else if (hexen) {
    armor_lump_green = R_NumPatchForSpriteIndex(HEXEN_SPR_ARM3);
    armor_lump_blue = R_NumPatchForSpriteIndex(HEXEN_SPR_ARM3);
    patch_delta_x = 10;
    patch_vertical_spacing = 4;
    patch_spacing = 2;
  }
  else {
    armor_lump_green = R_NumPatchForSpriteIndex(SPR_ARM1);
    armor_lump_blue = R_NumPatchForSpriteIndex(SPR_ARM2);
    patch_delta_x = 14;
    patch_vertical_spacing = 1;
    patch_spacing = 2;
  }
  patch_spacing += MAX(R_NumPatchWidth(armor_lump_green), R_NumPatchWidth(armor_lump_blue));
  dsda_InitPatchHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateBigArmorHC(void) {
  return;
}

void dsda_DrawBigArmorHC(void) {
  dsda_DrawComponent();
}
