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
//	DSDA Build Mode
//

#include "doomstat.h"

#include "dsda/input.h"
#include "dsda/settings.h"

#include "build.h"

static dboolean build_mode;
static dboolean advance_frame;
static ticcmd_t build_cmd;

dboolean dsda_AllowBuilding(void) {
  return !dsda_StrictMode() && !demoplayback;
}

dboolean dsda_BuildResponder(event_t* ev) {
  if (!dsda_AllowBuilding())
    return false;

  if (dsda_InputActivated(dsda_input_build)) {
    build_mode = !build_mode;
    paused ^= PAUSE_BUILDMODE;

    return true;
  }

  if (dsda_InputActivated(dsda_input_build_advance_frame)) {
    advance_frame = gametic;

    return true;
  }

  return false;
}

dboolean dsda_AdvanceFrame(void) {
  dboolean result;

  result = advance_frame;
  advance_frame = false;

  return result;
}
