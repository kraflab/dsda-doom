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
#include "e6y.h"

#include "settings.h"

int dsda_key_store_key_frame;
int dsda_key_restore_key_frame;
int dsda_strict_mode;
int dsda_cycle_ghost_colors;
int dsda_tas;

void dsda_InitSettings(void) {
  dsda_ChangeStrictMode();
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

int dsda_RealticClockRate(void) {
  if (dsda_StrictMode()) return 100;
  
  return realtic_clock_rate;
}
