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
//	DSDA Pause Mode
//

#include "doomstat.h"
#include "e6y.h"

#include "pause.h"

static dboolean paused;
static dboolean old_paused;

dboolean dsda_Paused(void) {
  return paused != 0;
}

dboolean dsda_PausedViaMenu(void) {
  return !demoplayback && menuactive && !netgame;
}

dboolean dsda_PausedOutsideDemo(void) {
  return dsda_PauseMode(PAUSE_PLAYBACK | PAUSE_BUILDMODE) || dsda_PausedViaMenu();
}

dboolean dsda_CameraPaused(void) {
  return paused && !walkcamera.type;
}

dboolean dsda_PauseMode(int mode) {
  return (paused & mode) != 0;
}

void dsda_ApplyPauseMode(int mode) {
  paused |= mode;
}

void dsda_TogglePauseMode(int mode) {
  paused ^= mode;
}

void dsda_ResetPauseMode(void) {
  paused = 0;
  old_paused = 0;
}

void dsda_MaskPause(void) {
  old_paused = paused & ~PAUSE_COMMAND;
  paused &= PAUSE_COMMAND;
}

void dsda_UnmaskPause(void) {
  paused |= old_paused;
  old_paused = 0;
}
