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

#include "stdio.h"

#include "doomstat.h"

#include "dsda/global.h"

#include "composite_time.h"

void dsda_CompositeTimeHC(char* str, size_t max_size) {
  int total_time;

  total_time = hexen ?
               players[consoleplayer].worldTimer :
               totalleveltimes + leveltime;

  if (total_time != leveltime)
    snprintf(
      str,
      max_size,
      "\x1b%ctime \x1b%c%d:%02d \x1b%c%d:%05.2f ",
      g_cr_gray + 0x30,
      g_cr_gold + 0x30,
      total_time / 35 / 60,
      (total_time % (60 * 35)) / 35,
      g_cr_green + 0x30,
      leveltime / 35 / 60,
      (float) (leveltime % (60 * 35)) / 35
    );
  else
    snprintf(
      str,
      max_size,
      "\x1b%ctime \x1b%c%d:%05.2f ",
      g_cr_gray + 0x30,
      g_cr_green + 0x30,
      leveltime / 35 / 60,
      (float) (leveltime % (60 * 35)) / 35
    );
}
