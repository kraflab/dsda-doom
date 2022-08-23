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
//	DSDA Mobj Tracker HUD Component
//

#include "base.h"

#include "mobj_tracker.h"

void dsda_MobjTrackerHC(char* str, size_t max_size, int id, mobj_t* mobj) {
  int health;

  health = mobj->health;

  if (mobj->thinker.function == P_RemoveThinkerDelayed)
    health = 0;

  snprintf(
    str,
    max_size,
    "\x1b%cm %d: %d",
    health > 0 ? HUlib_Color(exhud_color_warning) : HUlib_Color(exhud_color_default),
    id, health
  );
}
