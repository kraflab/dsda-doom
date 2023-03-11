//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA Minimap HUD Component
//

#include "base.h"

#include "minimap.h"

static int x, y, width, height, scale;

void dsda_InitMinimapHC(int x_offset, int y_offset, int vpt, int* args, int arg_count, void** data) {
  x = x_offset;
  y = dsda_HudComponentY(y_offset, vpt);
  width = args[0];
  height = args[1];
  scale = args[2];

  if (x < 0)
    x = 0;

  if (width <= 0 || width > 320)
    width = 48;

  if (x + width > 320)
    x = 320 - width;

  if (y < 0)
    y = 0;

  if (height <= 0 || height > 200)
    height = 48;

  if (y + height > 200)
    y = 200 - height;

  if (scale < 64)
    scale = 1024;

  V_GetWideRect(&x, &y, &width, &height, vpt);
}

void dsda_UpdateMinimapHC(void* data) {
  // nothing to do
}

void dsda_DrawMinimapHC(void* data) {
  AM_Drawer(true);
}

void dsda_CopyMinimapCoordinates(int* f_x, int* f_y, int* f_w, int* f_h) {
  *f_x = x;
  *f_y = y;
  *f_w = width;
  *f_h = height;
}

int dsda_MinimapScale(void) {
  return scale;
}
