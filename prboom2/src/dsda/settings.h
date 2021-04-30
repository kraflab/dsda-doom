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
extern int dsda_command_display;
extern int dsda_command_history_size;
extern int dsda_hide_empty_commands;
extern int dsda_track_attempts;
extern int dsda_wipe_at_full_speed;
extern int dsda_fine_sensitivity;
extern int dsda_hide_horns;
extern int dsda_organized_saves;

void dsda_InitSettings(void);
int dsda_CompatibilityLevel(void);
void dsda_ChangeStrictMode(void);
void dsda_SetTas(void);
double dsda_FineSensitivity(int base);
dboolean dsda_StrictMode(void);
dboolean dsda_CycleGhostColors(void);
dboolean dsda_AlwaysSR50(void);
dboolean dsda_HideHorns(void);
dboolean dsda_ExHud(void);
dboolean dsda_CommandDisplay(void);
dboolean dsda_TrackAttempts(void);
dboolean dsda_ShowHealthBars(void);
dboolean dsda_PainPalette(void);
dboolean dsda_BonusPalette(void);
dboolean dsda_PowerPalette(void);
dboolean dsda_WipeAtFullSpeed(void);
int dsda_RealticClockRate(void);
int dsda_AutoKeyFrameInterval(void);
int dsda_AutoKeyFrameDepth(void);
void dsda_SkipNextWipe(void);
dboolean dsda_SkipWipe(void);

#endif
