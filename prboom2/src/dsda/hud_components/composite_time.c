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
//	DSDA Composite Time HUD Component
//

#include "base.h"

#include "composite_time.h"

static dsda_text_t component;

static char label[8];

static void dsda_UpdateComponentText(char* str, size_t max_size) {
  int total_time;

  total_time = hexen ?
               players[consoleplayer].worldTimer :
               totalleveltimes + leveltime;

  if (total_time != leveltime)
    snprintf(
      str,
      max_size,
      "%s%s%d:%02d %s%d:%05.2f ",
      label,
      dsda_TextColor(dsda_tc_exhud_total_time),
      total_time / 35 / 60,
      (total_time % (60 * 35)) / 35,
      dsda_TextColor(dsda_tc_exhud_level_time),
      leveltime / 35 / 60,
      (float) (leveltime % (60 * 35)) / 35
    );
  else
    snprintf(
      str,
      max_size,
      "%s%s%d:%05.2f ",
      label,
      dsda_TextColor(dsda_tc_exhud_level_time),
      leveltime / 35 / 60,
      (float) (leveltime % (60 * 35)) / 35
    );
}

void dsda_InitCompositeTimeHC(int x_offset, int y_offset, int vpt, int* args, int arg_count) {
  if (arg_count < 1 || args[0])
    snprintf(label, sizeof(label), "%stime ", dsda_TextColor(dsda_tc_exhud_time_label));
  else
    label[0] = '\0';

  dsda_InitTextHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateCompositeTimeHC(void) {
  dsda_UpdateComponentText(component.msg, sizeof(component.msg));
  dsda_RefreshHudText(&component);
}

void dsda_DrawCompositeTimeHC(void) {
  dsda_DrawBasicText(&component);
}
