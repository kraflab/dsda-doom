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

#ifndef __DSDA_SETTINGS__
#define __DSDA_SETTINGS__

#define UNSPECIFIED_COMPLEVEL -2

extern int dsda_auto_key_frame_interval;
extern int dsda_auto_key_frame_depth;
extern int dsda_strict_mode;
extern int dsda_cycle_ghost_colors;
extern int dsda_exhud;

void dsda_InitSettings(void);
int dsda_CompatibilityLevel(void);
void dsda_ChangeStrictMode(void);
void dsda_SetTas(void);
dboolean dsda_StrictMode(void);
dboolean dsda_CycleGhostColors(void);
dboolean dsda_AlwaysSR50(void);
dboolean dsda_ExHud(void);
int dsda_RealticClockRate(void);
int dsda_AutoKeyFrameInterval(void);
int dsda_AutoKeyFrameDepth(void);
void dsda_SkipNextWipe(void);
dboolean dsda_SkipWipe(void);

#endif
