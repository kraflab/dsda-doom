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

typedef enum {
  dsda_strict_mode,
  dsda_novert,
  dsda_mouselook,
  dsda_autorun,
  dsda_show_messages,
  dsda_command_display,
  dsda_coordinate_display,
  dsda_exhud,
  dsda_mute_sfx,
  dsda_mute_music,
  dsda_cheat_codes,
  DSDA_SETTING_IDENTIFIER_COUNT
} dsda_setting_identifier_t;

typedef struct {
  int persistent_value;
  int transient_value;
  const char* name;
  void (*initializer)(void);
  void (*updater)(void);
  int invert_text;
  int persist_changes;
} dsda_setting_t;

extern dsda_setting_t dsda_setting[DSDA_SETTING_IDENTIFIER_COUNT];

extern int dsda_auto_key_frame_interval;
extern int dsda_auto_key_frame_depth;
extern int dsda_auto_key_frame_timeout;
extern int dsda_cycle_ghost_colors;
extern int dsda_command_history_size;
extern int dsda_hide_empty_commands;
extern int dsda_show_demo_attempts;
extern int dsda_wipe_at_full_speed;
extern int dsda_fine_sensitivity;
extern int dsda_hide_horns;
extern int dsda_organized_saves;
extern int dsda_skip_quit_prompt;
extern int dsda_show_split_data;
extern int dsda_fps_limit;
extern int dsda_quickstart_cache_tics;
extern int dsda_death_use_action;
extern const char* dsda_player_name;

void dsda_InitSettings(void);
void dsda_ResetTransient(dsda_setting_t* setting);
void dsda_ToggleSetting(dsda_setting_identifier_t id);
int dsda_CompatibilityLevel(void);
void dsda_ChangeStrictMode(void);
void dsda_SetTas(void);
double dsda_FineSensitivity(int base);
dboolean dsda_ShowMessages(void);
dboolean dsda_AutoRun(void);
dboolean dsda_MouseLook(void);
dboolean dsda_NoVert(void);
dboolean dsda_StrictMode(void);
dboolean dsda_MuteSfx(void);
dboolean dsda_MuteMusic(void);
dboolean dsda_ProcessCheatCodes(void);
dboolean dsda_CycleGhostColors(void);
dboolean dsda_WeaponAttackAlignment(void);
dboolean dsda_AlwaysSR50(void);
dboolean dsda_HideHorns(void);
dboolean dsda_SkipQuitPrompt(void);
dboolean dsda_TrackSplits(void);
dboolean dsda_ShowSplitData(void);
dboolean dsda_ExHud(void);
dboolean dsda_CommandDisplay(void);
dboolean dsda_CoordinateDisplay(void);
dboolean dsda_ShowDemoAttempts(void);
dboolean dsda_ShowHealthBars(void);
dboolean dsda_MapPointCoordinates(void);
dboolean dsda_CrosshairTarget(void);
dboolean dsda_CrosshairLockTarget(void);
dboolean dsda_PainPalette(void);
dboolean dsda_BonusPalette(void);
dboolean dsda_PowerPalette(void);
dboolean dsda_WipeAtFullSpeed(void);
int dsda_RevealAutomap(void);
void dsda_ResetRevealMap(void);
int dsda_RealticClockRate(void);
int dsda_AutoKeyFrameInterval(void);
int dsda_AutoKeyFrameDepth(void);
void dsda_SkipNextWipe(void);
dboolean dsda_PendingSkipWipe(void);
dboolean dsda_SkipWipe(void);

#endif
