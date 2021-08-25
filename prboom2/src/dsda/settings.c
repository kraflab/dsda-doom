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
#include "m_menu.h"
#include "e6y.h"
#include "r_things.h"
#include "w_wad.h"
#include "g_game.h"
#include "lprintf.h"

#include "dsda/key_frame.h"

#include "settings.h"

dsda_setting_t dsda_setting[DSDA_SETTING_IDENTIFIER_COUNT] = {
  [dsda_strict_mode] = { 0, 0, "Strict Mode", dsda_ChangeStrictMode, dsda_ChangeStrictMode },
  [dsda_novert] = { 0, 0, "Vertical Mouse Movement", NULL, NULL, true },
  [dsda_mouselook] = { 0, 0, "Mouselook", M_ChangeMouseLook, M_ChangeMouseLook, false, true },
  [dsda_autorun] = { 0, 0, "Auto Run", NULL, NULL, false, true },
  [dsda_show_messages] = { 0, 0, NULL, NULL, M_ChangeMessages, false, true },
  [dsda_command_display] = { 0, 0, "Command Display", NULL, NULL, false, true },
  [dsda_coordinate_display] = { 0, 0, "Coordinate Display", NULL, NULL, false, true },
  [dsda_exhud] = { 0, 0, NULL, NULL, NULL, false, true },
};

int dsda_auto_key_frame_interval;
int dsda_auto_key_frame_depth;
int dsda_cycle_ghost_colors;
int dsda_tas;
int dsda_skip_next_wipe;
int dsda_wipe_at_full_speed;
int dsda_show_demo_attempts;
int dsda_fine_sensitivity;
int dsda_hide_horns;
int dsda_skip_quit_prompt;
int dsda_show_split_data;

void dsda_InitSettings(void) {
  int i;

  for (i = 0; i < DSDA_SETTING_IDENTIFIER_COUNT; i++) {
    dsda_ResetTransient(&dsda_setting[i]);
  }
}

void dsda_ResetTransient(dsda_setting_t* setting) {
  setting->transient_value = setting->persistent_value;
  if (setting->initializer)
    setting->initializer();
}

void dsda_ToggleSetting(dsda_setting_identifier_t id) {
  dsda_setting[id].transient_value = !dsda_setting[id].transient_value;

  if (dsda_setting[id].persist_changes)
    dsda_setting[id].persistent_value = dsda_setting[id].transient_value;

  if (dsda_setting[id].updater)
    dsda_setting[id].updater();

  if (dsda_setting[id].name)
    doom_printf(
      "%s %s",
      dsda_setting[id].name,
      dsda_setting[id].transient_value ?
        dsda_setting[id].invert_text ? "off" : "on" :
        dsda_setting[id].invert_text ? "on"  : "off"
    );
}

static int dsda_WadCompatibilityLevel(void) {
  static int complvl = -1;
  static int last_numwadfiles = -1;

  // This might be called before all wads are loaded
  if (numwadfiles != last_numwadfiles) {
    int num;

    last_numwadfiles = numwadfiles;
    num = W_CheckNumForName("COMPLVL");

    if (num >= 0) {
      int length;
      const char* data;

      length = W_LumpLength(num);
      data = W_CacheLumpNum(num);

      if (length == 7 && !strncasecmp("vanilla", data, 7)) {
        if (gamemode == commercial) {
          if (gamemission == pack_plut || gamemission == pack_tnt)
            complvl = 4;
          else
            complvl = 2;
        }
        else
          complvl = 3;
      }
      else if (length == 4 && !strncasecmp("boom", data, 4))
        complvl = 9;
      else if (length == 3 && !strncasecmp("mbf", data, 3))
        complvl = 11;
      else if (length == 5 && !strncasecmp("mbf21", data, 5))
        complvl = 21;

      lprintf(LO_INFO, "Detected COMPLVL lump: %i\n", complvl);
    }
  }

  return complvl;
}

int dsda_CompatibilityLevel(void) {
  int i, level;

  if (raven) return doom_12_compatibility;

  i = M_CheckParm("-complevel");

  if (i && (i + 1 < myargc)) {
    level = atoi(myargv[i + 1]);

    if (level >= -1) return level;
  }

  if (!demoplayback) {
    level = dsda_WadCompatibilityLevel();

    if (level >= 0)
      return level;
  }

  return UNSPECIFIED_COMPLEVEL;
}

void dsda_ChangeStrictMode(void) {
  I_Init2(); // side effect of realtic clock rate
  M_ChangeSpeed(); // side effect of always sr50
  dsda_InitKeyFrame();
}

void dsda_SetTas(void) {
  dsda_tas = true;
}

static int dsda_Transient(dsda_setting_identifier_t id) {
  return dsda_setting[id].transient_value;
}

double dsda_FineSensitivity(int base) {
  return (double) base + (double) dsda_fine_sensitivity / 100;
}

dboolean dsda_ShowMessages(void) {
  return dsda_Transient(dsda_show_messages);
}

dboolean dsda_AutoRun(void) {
  return dsda_Transient(dsda_autorun);
}

dboolean dsda_MouseLook(void) {
  return dsda_Transient(dsda_mouselook);
}

dboolean dsda_NoVert(void) {
  return dsda_Transient(dsda_novert);
}

dboolean dsda_StrictMode(void) {
  return dsda_Transient(dsda_strict_mode) && demorecording && !dsda_tas;
}

dboolean dsda_CycleGhostColors(void) {
  return dsda_cycle_ghost_colors;
}

dboolean dsda_WeaponAttackAlignment(void) {
  return weapon_attack_alignment && !hexen && !dsda_StrictMode();
}

dboolean dsda_AlwaysSR50(void) {
  return movement_strafe50 && !dsda_StrictMode();
}

dboolean dsda_HideHorns(void) {
  return dsda_hide_horns;
}

dboolean dsda_SkipQuitPrompt(void) {
  return dsda_skip_quit_prompt;
}

dboolean dsda_TrackSplits(void) {
  return demorecording;
}

dboolean dsda_ShowSplitData(void) {
  return dsda_show_split_data;
}

dboolean dsda_ExHud(void) {
  return dsda_Transient(dsda_exhud);
}

dboolean dsda_CommandDisplay(void) {
  return dsda_Transient(dsda_command_display) && !dsda_StrictMode();
}

dboolean dsda_CoordinateDisplay(void) {
  return dsda_Transient(dsda_coordinate_display) && !dsda_StrictMode();
}

dboolean dsda_ShowDemoAttempts(void) {
  return dsda_show_demo_attempts && demorecording;
}

dboolean dsda_MapPointCoordinates(void) {
  extern int map_point_coordinates;

  return map_point_coordinates && !dsda_StrictMode();
}

dboolean dsda_CrosshairTarget(void) {
  return hudadd_crosshair_target && !dsda_StrictMode();
}

dboolean dsda_CrosshairLockTarget(void) {
  return hudadd_crosshair_lock_target && !dsda_StrictMode();
}

dboolean dsda_PainPalette(void) {
  return dsda_StrictMode() || palette_ondamage;
}

dboolean dsda_BonusPalette(void) {
  return dsda_StrictMode() || palette_onbonus;
}

dboolean dsda_PowerPalette(void) {
  return dsda_StrictMode() || palette_onpowers;
}

dboolean dsda_ShowHealthBars(void) {
  return health_bar && !dsda_StrictMode();
}

dboolean dsda_WipeAtFullSpeed(void) {
  return dsda_wipe_at_full_speed;
}

int dsda_reveal_map;

int dsda_RevealAutomap(void) {
  if (dsda_StrictMode()) return 0;

  return dsda_reveal_map;
}

void dsda_ResetRevealMap(void) {
  dsda_reveal_map = 0;
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

dboolean dsda_PendingSkipWipe(void) {
  return dsda_skip_next_wipe || !render_wipescreen;
}

dboolean dsda_SkipWipe(void) {
  if (dsda_skip_next_wipe) {
    dsda_skip_next_wipe = 0;
    return true;
  }

  return !render_wipescreen || hexen;
}
