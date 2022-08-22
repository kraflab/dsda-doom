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
//	DSDA Speed Text HUD Component
//

#include "base.h"

#include "speed_text.h"

static dsda_text_t component;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int speed;

  speed = dsda_RealticClockRate();

  snprintf(
    str,
    max_size,
    "\x1b%cSPEED \x1b%c%d%%",
    0x30 + CR_GRAY,
    speed < 100 ? 0x30 + CR_GOLD :
      speed == 100 ? 0x30 + CR_GREEN :
      0x30 + CR_BLUE,
    speed
  );
}

void dsda_InitSpeedTextHC(int x_offset, int y_offset, int vpt) {
  dsda_InitTextHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateSpeedTextHC(void) {
  dsda_UpdateComponentText(component.msg, sizeof(component.msg));
  dsda_RefreshHudText(&component);
}

void dsda_DrawSpeedTextHC(void) {
  HUlib_drawTextLine(&component.text, false);
}

void dsda_EraseSpeedTextHC(void) {
  HUlib_eraseTextLine(&component.text);
}
