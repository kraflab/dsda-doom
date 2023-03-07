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

#ifndef __DSDA_TEXT_COLOR__
#define __DSDA_TEXT_COLOR__

typedef enum {
  dsda_tc_exhud_time_label,
  dsda_tc_exhud_level_time,
  dsda_tc_exhud_total_time,
  dsda_tc_exhud_armor_zero,
  dsda_tc_exhud_armor_one,
  dsda_tc_exhud_armor_two,
  dsda_tc_exhud_command_entry,
  dsda_tc_exhud_command_queue,
  dsda_tc_exhud_coords_base,
  dsda_tc_exhud_coords_mf50,
  dsda_tc_exhud_coords_sr40,
  dsda_tc_exhud_coords_sr50,
  dsda_tc_exhud_coords_fast,
} dsda_text_color_index_t;

void dsda_LoadTextColor(void);
const char* dsda_TextColor(dsda_text_color_index_t i);

#endif
