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
//	DSDA Sector Tracker HUD Component
//

#include "base.h"

#include "sector_tracker.h"

void dsda_SectorTrackerHC(char* str, size_t max_size, int id) {
  dboolean active;
  int special;

  active = P_PlaneActive(&sectors[id]);
  special = sectors[id].special;

  snprintf(
    str,
    max_size,
    "\x1b%cs %d: %d %d %d",
    active ? HUlib_Color(exhud_color_alert)
           : special ? HUlib_Color(exhud_color_warning)
                     : HUlib_Color(exhud_color_default),
    id, special, active,
    sectors[id].floorheight >> FRACBITS
  );
}
