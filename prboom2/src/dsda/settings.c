//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Settings
//

#include "doomstat.h"
#include "m_argv.h"
#include "e6y.h"

#include "settings.h"

int dsda_auto_key_frame_interval;
int dsda_auto_key_frame_depth;
int dsda_strict_mode;
int dsda_cycle_ghost_colors;
int dsda_exhud;
int dsda_tas;
int dsda_skip_next_wipe;

void dsda_InitSettings(void) {
  dsda_ChangeStrictMode();
}

int dsda_CompatibilityLevel(void) {
  int i, level;

  if (heretic) return doom_12_compatibility;

  i = M_CheckParm("-complevel");

  if (i && (i + 1 < myargc)) {
    level = atoi(myargv[i + 1]);

    if (level >= -1) return level;
  }

  return UNSPECIFIED_COMPLEVEL;
}

void dsda_ChangeStrictMode(void) {
  I_Init2(); // side effect of realtic clock rate
  M_ChangeSpeed(); // side effect of always sr50
}

void dsda_SetTas(void) {
  dsda_tas = true;
}

dboolean dsda_StrictMode(void) {
  return dsda_strict_mode && demorecording && !dsda_tas;
}

dboolean dsda_CycleGhostColors(void) {
  return dsda_cycle_ghost_colors;
}

dboolean dsda_AlwaysSR50(void) {
  return movement_strafe50 && !dsda_StrictMode();
}

dboolean dsda_ExHud(void) {
  return dsda_exhud;
}

int dsda_RealticClockRate(void) {
  if (dsda_StrictMode()) return 100;

  return realtic_clock_rate;
}

int dsda_AutoKeyFrameInterval(void) {
  if (dsda_StrictMode()) return 0;

  return dsda_auto_key_frame_interval;
}

int dsda_AutoKeyFrameDepth(void) {
  if (dsda_StrictMode()) return 0;

  return dsda_auto_key_frame_depth;
}

void dsda_SkipNextWipe(void) {
  dsda_skip_next_wipe = 1;
}

dboolean dsda_SkipWipe(void) {
  if (dsda_skip_next_wipe) {
    dsda_skip_next_wipe = 0;
    return true;
  }

  return !render_wipescreen;
}
