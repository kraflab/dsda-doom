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
#include "m_menu.h"
#include "e6y.h"
#include "r_things.h"
#include "w_wad.h"
#include "g_game.h"
#include "gl_struct.h"
#include "lprintf.h"
#include "i_main.h"

#include "dsda/args.h"
#include "dsda/configuration.h"
#include "dsda/exhud.h"
#include "dsda/key_frame.h"
#include "dsda/map_format.h"

#include "settings.h"

int dsda_cycle_ghost_colors;
int dsda_tas;
int dsda_skip_next_wipe;
int dsda_wipe_at_full_speed;
int dsda_show_demo_attempts;
int dsda_fine_sensitivity;
int dsda_hide_horns;
int dsda_skip_quit_prompt;
int dsda_show_split_data;
int dsda_switch_when_ammo_runs_out;
int dsda_viewbob;
int dsda_weaponbob;

void dsda_InitSettings(void) {
  I_Init2();
  M_ChangeSpeed();
  dsda_InitKeyFrame();
  M_ChangeMouseLook();
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
      data = W_LumpByNum(num);

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
  dsda_arg_t* complevel_arg;

  if (raven) return doom_12_compatibility;

  if (map_format.zdoom) return mbf21_compatibility;

  complevel_arg = dsda_Arg(dsda_arg_complevel);

  if (complevel_arg->count)
    return complevel_arg->value.v_int;

  if (!demoplayback) {
    level = dsda_WadCompatibilityLevel();

    if (level >= 0)
      return level;
  }

  return UNSPECIFIED_COMPLEVEL;
}

void dsda_SetTas(void) {
  dsda_tas = true;
}

double dsda_FineSensitivity(int base) {
  return (double) base + (double) dsda_fine_sensitivity / 100;
}

dboolean dsda_ViewBob(void) {
  return dsda_viewbob;
}

dboolean dsda_WeaponBob(void) {
  return dsda_weaponbob;
}

dboolean dsda_ShowMessages(void) {
  return dsda_IntConfig(dsda_config_show_messages);
}

dboolean dsda_AutoRun(void) {
  return dsda_IntConfig(dsda_config_autorun);
}

dboolean dsda_MouseLook(void) {
  return dsda_IntConfig(dsda_config_mouselook);
}

dboolean dsda_VertMouse(void) {
  return dsda_IntConfig(dsda_config_vertmouse);
}

dboolean dsda_StrictMode(void) {
  return dsda_IntConfig(dsda_config_strict_mode) && demorecording && !dsda_tas;
}

dboolean dsda_MuteSfx(void) {
  return dsda_IntConfig(dsda_config_mute_sfx);
}

dboolean dsda_MuteMusic(void) {
  return dsda_IntConfig(dsda_config_mute_music);
}

dboolean dsda_ProcessCheatCodes(void) {
  return dsda_IntConfig(dsda_config_cheat_codes);
}

dboolean dsda_CycleGhostColors(void) {
  return dsda_cycle_ghost_colors;
}

dboolean dsda_AlwaysSR50(void) {
  return movement_strafe50 && !dsda_StrictMode();
}

dboolean dsda_HideHorns(void) {
  return dsda_hide_horns;
}

dboolean dsda_SwitchWhenAmmoRunsOut(void) {
  return dsda_switch_when_ammo_runs_out;
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
  return dsda_IntConfig(dsda_config_exhud);
}

dboolean dsda_CommandDisplay(void) {
  return dsda_IntConfig(dsda_config_command_display);
}

dboolean dsda_CoordinateDisplay(void) {
  return dsda_IntConfig(dsda_config_coordinate_display);
}

dboolean dsda_ShowFPS(void) {
  return dsda_IntConfig(dsda_config_show_fps);
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

dboolean dsda_SimpleShadows(void) {
  return dsda_IntConfig(dsda_config_gl_shadows);
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
  return gl_health_bar && !dsda_StrictMode();
}

dboolean dsda_WipeAtFullSpeed(void) {
  return dsda_wipe_at_full_speed;
}

int dsda_show_alive_monsters;

int dsda_ShowAliveMonsters(void) {
  if (dsda_StrictMode()) return 0;

  return dsda_show_alive_monsters;
}

int dsda_CycleShowAliveMonsters(void) {
  dsda_show_alive_monsters = (dsda_show_alive_monsters + 1) % 3;

  return dsda_ShowAliveMonsters();
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
  return dsda_IntConfig(dsda_config_realtic_clock_rate);
}

void dsda_UpdateRealticClockRate(int value) {
  dsda_UpdateIntConfig(dsda_config_realtic_clock_rate, value, true);
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
