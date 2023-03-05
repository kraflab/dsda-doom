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

static char label[9];

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int speed;

  speed = dsda_RealticClockRate();

  snprintf(
    str,
    max_size,
    "%s\x1b%c%d%%",
    label,
    speed < 100 ? HUlib_Color(CR_GOLD)
                : speed == 100 ? HUlib_Color(CR_GREEN)
                               : HUlib_Color(CR_LIGHTBLUE),
    speed
  );
}

void dsda_InitSpeedTextHC(int x_offset, int y_offset, int vpt, int* args, int arg_count) {
  if (arg_count < 1 || args[0])
    snprintf(label, sizeof(label), "\x1b%cSPEED ", HUlib_Color(CR_GRAY));
  else
    label[0] = '\0';

  dsda_InitTextHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateSpeedTextHC(void) {
  dsda_UpdateComponentText(component.msg, sizeof(component.msg));
  dsda_RefreshHudText(&component);
}

void dsda_DrawSpeedTextHC(void) {
  dsda_DrawBasicText(&component);
}
