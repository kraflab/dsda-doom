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
//	DSDA Render Stats HUD Component
//

#include "dsda/render_stats.h"

#include "base.h"

#include "render_stats.h"

static dsda_text_t component[2];

static void dsda_UpdateCurrentComponentText(char* str, size_t max_size) {
  extern dsda_render_stats_t dsda_render_stats;
  extern int dsda_render_stats_fps;

  snprintf(
    str, max_size,
    "\x1b%cFPS \x1b%c%4d \x1b%cSEGS \x1b%c%4d \x1b%cPLANES \x1b%c%4d \x1b%cSPRITES \x1b%c%4d",
    HUlib_Color(CR_GRAY),
    dsda_render_stats_fps < 35 ? HUlib_Color(CR_RED) : HUlib_Color(CR_GOLD),
    dsda_render_stats_fps,
    HUlib_Color(CR_GRAY),
    dsda_render_stats.drawsegs > 256 ? HUlib_Color(CR_RED) : HUlib_Color(CR_GOLD),
    dsda_render_stats.drawsegs,
    HUlib_Color(CR_GRAY),
    dsda_render_stats.visplanes > 128 ? HUlib_Color(CR_RED) : HUlib_Color(CR_GOLD),
    dsda_render_stats.visplanes,
    HUlib_Color(CR_GRAY),
    dsda_render_stats.vissprites > 128 ? HUlib_Color(CR_RED) : HUlib_Color(CR_GOLD),
    dsda_render_stats.vissprites
  );
}

static void dsda_UpdateMaxComponentText(char* str, size_t max_size) {
  extern dsda_render_stats_t dsda_render_stats_max;

  snprintf(
    str, max_size,
    "\x1b%cMAX      SEGS \x1b%c%4d \x1b%cPLANES \x1b%c%4d \x1b%cSPRITES \x1b%c%4d",
    HUlib_Color(CR_GRAY),
    dsda_render_stats_max.drawsegs > 256 ? HUlib_Color(CR_RED) : HUlib_Color(CR_GOLD),
    dsda_render_stats_max.drawsegs,
    HUlib_Color(CR_GRAY),
    dsda_render_stats_max.visplanes > 128 ? HUlib_Color(CR_RED) : HUlib_Color(CR_GOLD),
    dsda_render_stats_max.visplanes,
    HUlib_Color(CR_GRAY),
    dsda_render_stats_max.vissprites > 128 ? HUlib_Color(CR_RED) : HUlib_Color(CR_GOLD),
    dsda_render_stats_max.vissprites
  );
}

void dsda_InitRenderStatsHC(int x_offset, int y_offset, int vpt, int* args) {
  dsda_InitTextHC(&component[0], x_offset, y_offset, vpt);
  dsda_InitTextHC(&component[1], x_offset, y_offset + 8, vpt);
}

void dsda_UpdateRenderStatsHC(void) {
  dsda_UpdateCurrentComponentText(component[0].msg, sizeof(component[0].msg));
  dsda_UpdateMaxComponentText(component[1].msg, sizeof(component[1].msg));
  dsda_RefreshHudText(&component[0]);
  dsda_RefreshHudText(&component[1]);
}

void dsda_DrawRenderStatsHC(void) {
  dsda_DrawBasicText(&component[0]);
  dsda_DrawBasicText(&component[1]);
}
