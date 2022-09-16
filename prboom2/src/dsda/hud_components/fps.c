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
//	DSDA FPS HUD Component
//

#include "base.h"

#include "fps.h"

static dsda_text_t component;

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  extern int dsda_render_stats_fps;

  snprintf(
    str,
    max_size,
    "\x1b%c%4d",
    dsda_render_stats_fps < 35 ? HUlib_Color(CR_RED) : HUlib_Color(CR_GRAY),
    dsda_render_stats_fps
  );
}

void dsda_InitFPSHC(int x_offset, int y_offset, int vpt) {
  dsda_InitTextHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateFPSHC(void) {
  dsda_UpdateComponentText(component.msg, sizeof(component.msg));
  dsda_RefreshHudText(&component);
}

void dsda_DrawFPSHC(void) {
  HUlib_drawTextLine(&component.text, false);
}

void dsda_EraseFPSHC(void) {
  HUlib_eraseTextLine(&component.text);
}
