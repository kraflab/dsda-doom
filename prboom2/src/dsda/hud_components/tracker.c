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
//	DSDA Tracker HUD Component
//

#include "dsda/tracker.h"

#include "base.h"

#include "line_distance_tracker.h"
#include "line_tracker.h"
#include "mobj_tracker.h"
#include "null.h"
#include "player_tracker.h"
#include "sector_tracker.h"

#include "tracker.h"

extern dsda_tracker_t dsda_tracker[TRACKER_LIMIT];

dsda_text_t component[TRACKER_LIMIT];

void dsda_InitTrackerHC(int x_offset, int y_offset, int vpt) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    dsda_InitTextHC(&component[i], x_offset, y_offset + i * 8, vpt);
}

void dsda_UpdateTrackerHC(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i) {
    switch (dsda_tracker[i].type) {
      case dsda_tracker_nothing:
        dsda_NullHC(component[i].msg, sizeof(component[i].msg));
        break;
      case dsda_tracker_line:
        dsda_LineTrackerHC(component[i].msg, sizeof(component[i].msg), dsda_tracker[i].id);
        break;
      case dsda_tracker_line_distance:
        dsda_LineDistanceTrackerHC(component[i].msg, sizeof(component[i].msg), dsda_tracker[i].id);
        break;
      case dsda_tracker_sector:
        dsda_SectorTrackerHC(component[i].msg, sizeof(component[i].msg), dsda_tracker[i].id);
        break;
      case dsda_tracker_mobj:
        dsda_MobjTrackerHC(component[i].msg, sizeof(component[i].msg), dsda_tracker[i].id, dsda_tracker[i].mobj);
        break;
      case dsda_tracker_player:
        dsda_PlayerTrackerHC(component[i].msg, sizeof(component[i].msg));
        break;
    }

    dsda_RefreshHudText(&component[i]);
  }
}

void dsda_DrawTrackerHC(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    HUlib_drawTextLine(&component[i].text, false);
}

void dsda_EraseTrackerHC(void) {
  int i;

  for (i = 0; i < TRACKER_LIMIT; ++i)
    HUlib_eraseTextLine(&component[i].text);
}
