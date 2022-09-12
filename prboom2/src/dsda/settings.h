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

#include "doomtype.h"

#define UNSPECIFIED_COMPLEVEL -2

extern int dsda_show_alive_monsters;

void dsda_InitSettings(void);
int dsda_CompatibilityLevel(void);
void dsda_SetTas(void);
dboolean dsda_ViewBob(void);
dboolean dsda_WeaponBob(void);
dboolean dsda_ShowMessages(void);
dboolean dsda_AutoRun(void);
dboolean dsda_MouseLook(void);
dboolean dsda_VertMouse(void);
dboolean dsda_StrictMode(void);
dboolean dsda_MuteSfx(void);
dboolean dsda_MuteMusic(void);
dboolean dsda_ProcessCheatCodes(void);
dboolean dsda_CycleGhostColors(void);
dboolean dsda_AlwaysSR50(void);
dboolean dsda_HideHorns(void);
dboolean dsda_SwitchWhenAmmoRunsOut(void);
dboolean dsda_SkipQuitPrompt(void);
dboolean dsda_TrackSplits(void);
dboolean dsda_ShowSplitData(void);
dboolean dsda_ExHud(void);
dboolean dsda_CommandDisplay(void);
dboolean dsda_CoordinateDisplay(void);
dboolean dsda_ShowFPS(void);
dboolean dsda_ShowDemoAttempts(void);
dboolean dsda_ShowHealthBars(void);
dboolean dsda_MapPointCoordinates(void);
dboolean dsda_SimpleShadows(void);
dboolean dsda_PainPalette(void);
dboolean dsda_BonusPalette(void);
dboolean dsda_PowerPalette(void);
dboolean dsda_WipeAtFullSpeed(void);
int dsda_ShowAliveMonsters(void);
int dsda_CycleShowAliveMonsters(void);
int dsda_RevealAutomap(void);
void dsda_ResetRevealMap(void);
int dsda_RealticClockRate(void);
void dsda_UpdateRealticClockRate(int value);
int dsda_AutoKeyFrameInterval(void);
int dsda_AutoKeyFrameDepth(void);
void dsda_SkipNextWipe(void);
dboolean dsda_PendingSkipWipe(void);
dboolean dsda_SkipWipe(void);

#endif
