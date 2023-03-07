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
//	DSDA Text Color
//

#include "doomdef.h"
#include "hu_lib.h"
#include "lprintf.h"
#include "w_wad.h"
#include "v_video.h"

#include "dsda/utility.h"

#include "text_color.h"

typedef struct {
  const char* key;
  int color_range;
  char color_str[3];
} dsda_text_color_t;

dsda_text_color_t dsda_text_colors[] = {
  [dsda_tc_exhud_time_label] = { "exhud_time_label", CR_GRAY },
  [dsda_tc_exhud_level_time] = { "exhud_level_time", CR_GREEN },
  [dsda_tc_exhud_total_time] = { "exhud_total_time", CR_GOLD },
  [dsda_tc_exhud_armor_zero] = { "exhud_armor_zero", CR_GRAY },
  [dsda_tc_exhud_armor_one] = { "exhud_armor_one", CR_GREEN },
  [dsda_tc_exhud_armor_two] = { "exhud_armor_two", CR_LIGHTBLUE },
  [dsda_tc_exhud_command_entry] = { "exhud_command_entry", CR_GRAY },
  [dsda_tc_exhud_command_queue] = { "exhud_command_queue", CR_GOLD },
  [dsda_tc_exhud_coords_base] = { "exhud_coords_base", CR_GREEN },
  [dsda_tc_exhud_coords_mf50] = { "exhud_coords_mf50", CR_GRAY },
  [dsda_tc_exhud_coords_sr40] = { "exhud_coords_sr40", CR_GREEN },
  [dsda_tc_exhud_coords_sr50] = { "exhud_coords_sr50", CR_LIGHTBLUE },
  [dsda_tc_exhud_coords_fast] = { "exhud_coords_fast", CR_RED },
  [dsda_tc_exhud_fps_bad] = { "exhud_fps_bad", CR_RED },
  [dsda_tc_exhud_fps_fine] = { "exhud_fps_fine", CR_GRAY },
  [dsda_tc_exhud_health_bad] = { "exhud_health_bad", CR_RED },
  [dsda_tc_exhud_health_warning] = { "exhud_health_warning", CR_GOLD },
  [dsda_tc_exhud_health_ok] = { "exhud_health_ok", CR_GREEN },
  [dsda_tc_exhud_health_super] = { "exhud_health_super", CR_LIGHTBLUE },
  { NULL },
};

const char* dsda_TextColor(dsda_text_color_index_t i) {
  return dsda_text_colors[i].color_str;
}

void dsda_LoadTextColor(void) {
  char* lump;
  char** lines;
  const char* line;
  int line_i;
  int color_range;
  char key[33] = { 0 };
  dsda_text_color_t* p;

  lump = W_ReadLumpToString(W_GetNumForName("DSDATC"));

  lines = dsda_SplitString(lump, "\n\r");

  for (line_i = 0; lines[line_i]; ++line_i) {
    line = lines[line_i];

    if (!line[0] || line[0] == '/')
      continue;

    if (sscanf(line, "%32s %d", key, &color_range) != 2)
      I_Error("DSDATC lump has unknown format! (%s)", line);

    for (p = dsda_text_colors; p->key; p++)
      if (!strcasecmp(key, p->key)) {
        p->color_range = color_range;
        break;
      }

    if (!p->key)
      I_Error("DSDATC lump has unknown key %s!", key);
  }

  for (p = dsda_text_colors; p->key; p++) {
    p->color_str[0] = '\x1b';
    p->color_str[1] = HUlib_Color(p->color_range);
  }

  Z_Free(lines);
  Z_Free(lump);
}
