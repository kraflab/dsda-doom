/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  Main loop menu stuff.
 *  Default Config File.
 *  PCX Screenshots.
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _MSC_VER
#include <io.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>

#include "doomstat.h"
#include "g_game.h"
#include "m_menu.h"
#include "am_map.h"
#include "w_wad.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "dstrings.h"
#include "m_misc.h"
#include "s_sound.h"
#include "sounds.h"
#include "i_joy.h"
#include "lprintf.h"
#include "d_main.h"
#include "d_deh.h"
#include "r_draw.h"
#include "r_demo.h"
#include "r_fps.h"
#include "r_main.h"
#include "r_things.h"
#include "r_sky.h"

//e6y
#include "gl_struct.h"
#include "g_overflow.h"
#include "e6y.h"

#include "dsda/args.h"
#include "dsda/console.h"
#include "dsda/settings.h"
#include "dsda/stretch.h"

// NSM
#include "i_capture.h"

#define SETTING_HEADING(str) { str, { NULL }, { 0 }, UL, UL, def_none }
#define INPUT_SETTING(str, id, k, m, j) { str, { NULL }, { 0 }, UL, UL, def_input, 0, id, { k, m, j } }
#define MIGRATED_SETTING(id) { NULL, { NULL }, { 0 }, 0, 0, 0, id }

extern int dsda_auto_key_frame_depth;
extern int dsda_auto_key_frame_interval;
extern int dsda_auto_key_frame_timeout;

/* cph - disk icon not implemented */
static inline void I_BeginRead(void) {}
static inline void I_EndRead(void) {}

/*
 * M_WriteFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */

dboolean M_WriteFile(char const *name, const void *source, size_t length)
{
  FILE *fp;

  errno = 0;

  if (!(fp = fopen(name, "wb")))       // Try opening file
    return 0;                          // Could not open file for writing

  I_BeginRead();                       // Disk icon on
  length = fwrite(source, 1, length, fp) == (size_t)length;   // Write data
  fclose(fp);
  I_EndRead();                         // Disk icon off

  if (!length)                         // Remove partially written file
    remove(name);

  return length;
}

/*
 * M_ReadFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */

int M_ReadFile(char const *name, byte **buffer)
{
  FILE *fp;

  if ((fp = fopen(name, "rb")))
    {
      size_t length;

      I_BeginRead();
      fseek(fp, 0, SEEK_END);
      length = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      *buffer = Z_Malloc(length);
      if (fread(*buffer, 1, length, fp) == length)
        {
          fclose(fp);
          I_EndRead();
          return length;
        }
      fclose(fp);
    }

  /* cph 2002/08/10 - this used to return 0 on error, but that's ambiguous,
   * because we could have a legit 0-length file. So make it -1. */
  return -1;
}

// Same as above, but add null terminator
int M_ReadFileToString(char const *name, char **buffer) {
  FILE *fp;

  if ((fp = fopen(name, "rb")))
  {
    size_t length;

    I_BeginRead();
    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    *buffer = Z_Malloc(length + 1);
    if (fread(*buffer, 1, length, fp) == length)
    {
      fclose(fp);
      I_EndRead();
      (*buffer)[length] = '\0';
      return length;
    }
    Z_Free(*buffer);
    *buffer = NULL;
    fclose(fp);
  }

  /* cph 2002/08/10 - this used to return 0 on error, but that's ambiguous,
   * because we could have a legit 0-length file. So make it -1. */
  return -1;
}

//
// DEFAULTS
//

int usemouse;
int mouse_stutter_correction;

// The available anisotropic
typedef enum {
  gl_anisotropic_off = 0,
  gl_anisotropic_2x  = 1,
  gl_anisotropic_4x  = 2,
  gl_anisotropic_8x  = 3,
  gl_anisotropic_16x = 4,
} gl_anisotropic_mode_t;

extern int viewwidth;
extern int viewheight;
extern int gl_texture_filter;
extern int gl_sprite_filter;
extern int gl_patch_filter;
extern int gl_texture_filter_anisotropic;
extern const char *gl_tex_format_string;

//e6y: fog
extern int gl_fog;

extern int tran_filter_pct;            // killough 2/21/98

extern int screenblocks;

#ifndef DJGPP
int         mus_pause_opt; // 0 = kill music, 1 = pause, 2 = continue
#endif

/* cph - Some MBF stuff parked here for now
 * killough 10/98
 */
int map_point_coordinates;
int map_level_stat;

default_t defaults[] =
{
  //e6y
  SETTING_HEADING("System settings"),
  MIGRATED_SETTING(dsda_config_process_priority),

  SETTING_HEADING("Misc settings"),
  MIGRATED_SETTING(dsda_config_vanilla_keymap),
  MIGRATED_SETTING(dsda_config_realtic_clock_rate),
  MIGRATED_SETTING(dsda_config_menu_background),
  MIGRATED_SETTING(dsda_config_max_player_corpse),
  MIGRATED_SETTING(dsda_config_flashing_hom),
  MIGRATED_SETTING(dsda_config_demo_smoothturns),
  MIGRATED_SETTING(dsda_config_demo_smoothturnsfactor),
  { "screenshot_dir", { NULL, &screenshot_dir }, { 0, "" }, UL, UL, def_str },
  { "health_bar", { &health_bar }, { 0 }, 0, 1, def_bool },
  { "health_bar_full_length", { &health_bar_full_length }, { 1 }, 0, 1, def_bool },
  { "health_bar_red", { &health_bar_red }, { 50 }, 0, 100, def_int },
  { "health_bar_yellow", { &health_bar_yellow }, { 99 }, 0, 100, def_int },
  { "health_bar_green", { &health_bar_green }, { 0 }, 0, 100, def_int },
  { "quickstart_window_ms", { &quickstart_window_ms }, { 0 }, 0, 1000, def_int },

  SETTING_HEADING("Game settings"),
  MIGRATED_SETTING(dsda_config_default_complevel),
  MIGRATED_SETTING(dsda_config_default_skill),
  MIGRATED_SETTING(dsda_config_weapon_attack_alignment),
  MIGRATED_SETTING(dsda_config_sts_always_red),
  MIGRATED_SETTING(dsda_config_sts_pct_always_gray),
  MIGRATED_SETTING(dsda_config_sts_traditional_keys),
  MIGRATED_SETTING(dsda_config_show_messages),
  MIGRATED_SETTING(dsda_config_autorun),
  { "deh_apply_cheats",{&deh_apply_cheats},{1},0,1, def_bool}, // if 0, dehacked cheat replacements are ignored.
  { "movement_strafe50", { &movement_strafe50 }, { 0 }, 0, 1, def_bool },
  { "movement_strafe50onturns", { &movement_strafe50onturns }, { 0 }, 0, 1, def_bool },
  { "movement_shorttics", { &movement_shorttics }, { 0 }, 0, 1, def_bool },

  SETTING_HEADING("Sound settings"),
  { "snd_pcspeaker",{&snd_pcspeaker},{0}, 0, 1, def_bool},
  { "pitched_sounds",{&pitched_sounds},{0},0,1, def_bool}, // enables variable pitch in sound effects (from id's original code)
  { "samplerate",{&snd_samplerate},{44100},11025,48000, def_int},
  { "slice_samplecount",{&snd_samplecount},{512},32,8192, def_int},
  { "sfx_volume",{&snd_SfxVolume},{8},0,15, def_int},
  { "music_volume",{&snd_MusicVolume},{8},0,15, def_int},
  { "mus_pause_opt",{&mus_pause_opt},{1},0,2, def_int}, // 0 = kill music when paused, 1 = pause music, 2 = let music continue
  { "snd_channels",{&default_numChannels},{32},1,32, def_int}, // number of audio events simultaneously // killough
  { "snd_midiplayer",{NULL, &snd_midiplayer},{0,"fluidsynth"},UL,UL,def_str},
  { "snd_soundfont",{NULL, &snd_soundfont},{0,""},UL,UL,def_str},
  { "snd_mididev",{NULL, &snd_mididev},{0,""},UL,UL,def_str}, // midi device to use for portmidiplayer
  { "full_sounds",{&full_sounds},{0},0,1,def_bool}, // disable sound cutoffs

#ifdef _WIN32
  { "mus_extend_volume",{&mus_extend_volume},{0},0,1, def_bool}, // e6y: apply midi volume to all midi devices
#endif
  { "mus_fluidsynth_chorus",{&mus_fluidsynth_chorus},{0},0,1,def_bool},
  { "mus_fluidsynth_reverb",{&mus_fluidsynth_reverb},{0},0,1,def_bool},
  { "mus_fluidsynth_gain",{&mus_fluidsynth_gain},{50},0,1000,def_int}, // NSM  fine tune fluidsynth output level
  { "mus_opl_gain",{&mus_opl_gain},{50},0,1000,def_int}, // NSM  fine tune opl output level

  SETTING_HEADING("Video settings"),
  { "videomode",{NULL, &default_videomode},{0,"Software"},UL,UL,def_str},
  { "screen_resolution",{NULL, &screen_resolution},{0,"640x480"},UL,UL,def_str},
  { "custom_resolution",{0,&custom_resolution},{0,""},UL,UL,def_str},
  { "use_fullscreen",{&use_fullscreen},{0},0,1, def_bool},
  { "exclusive_fullscreen",{&exclusive_fullscreen},{0},0,1, def_bool},
  { "gl_exclusive_fullscreen",{&gl_exclusive_fullscreen},{1},0,1,def_bool},
  { "render_vsync",{&render_vsync},{0},0,1,def_bool},
  { "tran_filter_pct",{&tran_filter_pct},{66},0,100, def_int}, // set percentage of foreground/background translucency mix
  { "screenblocks",{&screenblocks},{10},3,11, def_int},
  { "usegamma",{&usegamma},{0},0,4, def_int}, // gamma correction level // killough 1/18/98
  { "uncapped_framerate", {&movement_smooth_default},  {1},0,1, def_bool},
  { "dsda_fps_limit", {&dsda_fps_limit}, {0}, 0, 1000, def_int},
  { "sdl_video_window_pos", {NULL,&sdl_video_window_pos}, {0,"center"},UL,UL, def_str},
  { "palette_ondamage", {&palette_ondamage},  {1},0,1, def_bool},
  { "palette_onbonus", {&palette_onbonus},  {1},0,1, def_bool},
  { "palette_onpowers", {&palette_onpowers},  {1},0,1, def_bool},
  { "render_wipescreen", {&render_wipescreen},  {1},0,1, def_bool},
  { "render_screen_multiply", {&render_screen_multiply},  {1},1,5, def_int},
  { "integer_scaling", {&integer_scaling},  {0},0,1, def_bool},
  { "render_aspect", {&render_aspect},  {0},0,4, def_int},
  { "render_doom_lightmaps", {&render_doom_lightmaps},  {0},0,1, def_bool},
  { "fake_contrast", {&fake_contrast},  {1},0,1, def_bool}, /* cph - allow crappy fake contrast to be disabled */
  { "render_stretch_hud", {&render_stretch_hud_default},{patch_stretch_not_adjusted},0,patch_stretch_max_config - 1, def_int},
  { "render_patches_scalex", {&render_patches_scalex},{0},0,16, def_int},
  { "render_patches_scaley", {&render_patches_scaley},{0},0,16, def_int},
  { "render_stretchsky",{&r_stretchsky},{1},0,1, def_bool},

  SETTING_HEADING("OpenGL settings"),
  MIGRATED_SETTING(dsda_config_gl_sprite_blend),
  MIGRATED_SETTING(dsda_config_render_paperitems),
  MIGRATED_SETTING(dsda_config_gl_fog),
  MIGRATED_SETTING(dsda_config_gl_blend_animations),

  // TODO: add these back to menu
   { "gl_texture_filter",{(int*)&gl_texture_filter}, {filter_nearest_mipmap_linear}, filter_nearest, filter_count - 1, def_int},
   { "gl_sprite_filter",{(int*)&gl_sprite_filter}, {filter_nearest}, filter_nearest, filter_linear_mipmap_nearest, def_int},
   { "gl_patch_filter",{(int*)&gl_patch_filter}, {filter_nearest}, filter_nearest, filter_linear, def_int},
   { "gl_texture_filter_anisotropic",{(int*)&gl_texture_filter_anisotropic}, {gl_anisotropic_8x}, gl_anisotropic_off, gl_anisotropic_16x, def_int},
  // end TODO
   { "gl_tex_format_string", {NULL,&gl_tex_format_string}, {0,"GL_RGBA"},UL,UL, def_str},
   { "gl_sprite_offset",{&gl_sprite_offset_default},{0}, 0, 5, def_int}, // amount to bring items out of floor (GL) Mead 8/13/03
  { "gl_skymode",{(int*)&gl_skymode}, {skytype_auto}, skytype_auto, skytype_count - 1, def_int},
   { "render_multisampling", {&render_multisampling},  {0},0,8, def_int},
   { "render_fov", {&render_fov},  {90},20,160, def_int},
   { "gl_spriteclip",{(int*)&gl_spriteclip},{spriteclip_smart}, spriteclip_const, spriteclip_smart, def_int},
   { "gl_spriteclip_threshold", {&gl_spriteclip_threshold},  {10},0,100, def_int},
   { "gl_lightmode",{(int*)&gl_lightmode_default},{gl_lightmode_shaders}, gl_lightmode_glboom, gl_lightmode_last-1, def_int},
  MIGRATED_SETTING(dsda_config_useglgamma),
  MIGRATED_SETTING(dsda_config_gl_shadows),
  MIGRATED_SETTING(dsda_config_gl_shadows_maxdist),
  MIGRATED_SETTING(dsda_config_gl_shadows_factor),

  SETTING_HEADING("Mouse settings"),
  { "use_mouse",{&usemouse},{1},0,1, def_bool}, // enables use of mouse with DOOM
  { "mouse_stutter_correction",{&mouse_stutter_correction},{1},0,1, def_bool}, // interpolates mouse input to mitigate stuttering
  { "mouse_sensitivity_horiz",{&mouseSensitivity_horiz},{10},0,UL, def_int}, /* adjust horizontal (x) mouse sensitivity killough/mead */
  { "mouse_sensitivity_vert",{&mouseSensitivity_vert},{1},0,UL, def_int}, /* adjust vertical (y) mouse sensitivity killough/mead */
  { "mouse_acceleration", { &mouse_acceleration }, { 0 }, 0, UL, def_int },
  { "mouse_sensitivity_mlook", { &mouseSensitivity_mlook }, { 10 }, 0, UL, def_int },
  { "mouse_doubleclick_as_use", { &mouse_doubleclick_as_use }, { 0 }, 0, 1, def_bool },
  { "mouse_carrytics", { &mouse_carrytics }, { 1 }, 0, 1, def_bool },
  MIGRATED_SETTING(dsda_config_mouselook),
  MIGRATED_SETTING(dsda_config_vertmouse),
  { "movement_maxviewpitch", {&movement_maxviewpitch},  {90},0,90, def_int},
  { "movement_mousestrafedivisor", {&movement_mousestrafedivisor},  {4},1,512, def_int},
  { "movement_mouseinvert", {&movement_mouseinvert},  {0},0,1, def_bool},

  SETTING_HEADING("Joystick settings"),
  { "use_joystick",{&usejoystick},{0},0,2, def_int}, // number of joystick to use (0 for none)

  SETTING_HEADING("Automap settings"),
  MIGRATED_SETTING(dsda_config_mapcolor_back),
  MIGRATED_SETTING(dsda_config_mapcolor_grid),
  MIGRATED_SETTING(dsda_config_mapcolor_wall),
  MIGRATED_SETTING(dsda_config_mapcolor_fchg),
  MIGRATED_SETTING(dsda_config_mapcolor_cchg),
  MIGRATED_SETTING(dsda_config_mapcolor_clsd),
  MIGRATED_SETTING(dsda_config_mapcolor_rkey),
  MIGRATED_SETTING(dsda_config_mapcolor_bkey),
  MIGRATED_SETTING(dsda_config_mapcolor_ykey),
  MIGRATED_SETTING(dsda_config_mapcolor_rdor),
  MIGRATED_SETTING(dsda_config_mapcolor_bdor),
  MIGRATED_SETTING(dsda_config_mapcolor_ydor),
  MIGRATED_SETTING(dsda_config_mapcolor_tele),
  MIGRATED_SETTING(dsda_config_mapcolor_secr),
  MIGRATED_SETTING(dsda_config_mapcolor_revsecr),
  MIGRATED_SETTING(dsda_config_mapcolor_exit),
  MIGRATED_SETTING(dsda_config_mapcolor_unsn),
  MIGRATED_SETTING(dsda_config_mapcolor_flat),
  MIGRATED_SETTING(dsda_config_mapcolor_sprt),
  MIGRATED_SETTING(dsda_config_mapcolor_item),
  MIGRATED_SETTING(dsda_config_mapcolor_hair),
  MIGRATED_SETTING(dsda_config_mapcolor_sngl),
  MIGRATED_SETTING(dsda_config_mapcolor_me),
  MIGRATED_SETTING(dsda_config_mapcolor_enemy),
  MIGRATED_SETTING(dsda_config_mapcolor_frnd),
  { "map_secret_after", {&map_secret_after}, {0},0,1, def_bool}, // prevents showing secret sectors till after entered
  { "map_point_coord", {&map_point_coordinates}, {0},0,1, def_bool},
  { "map_level_stat", {&map_level_stat}, {1},0,1, def_bool},
  { "automapmode", {(int*)&automapmode}, {am_follow}, 0, 31, def_hex}, // automap mode
  { "map_always_updates", {&map_always_updates}, {1},0,1, def_bool},
  { "map_grid_size", {&map_grid_size}, {128},8,256, def_int},
  { "map_scroll_speed", {&map_scroll_speed}, {8},1,32, def_int},
  { "map_wheel_zoom", {&map_wheel_zoom}, {1},0,1, def_bool},
  { "map_use_multisamling", {&map_use_multisamling}, {0},0,1, def_bool},
  { "map_textured", {&map_textured}, {1},0,1, def_bool},
  { "map_textured_trans", {&map_textured_trans}, {100},0,100, def_int},
  { "map_textured_overlay_trans", {&map_textured_overlay_trans}, {66},0,100, def_int},
  { "map_lines_overlay_trans", {&map_lines_overlay_trans}, {100},0,100, def_int},
  { "map_overlay_pos_x", {&map_overlay_pos_x}, {0},0,319, def_int},
  { "map_overlay_pos_y", {&map_overlay_pos_y}, {0},0,199, def_int},
  { "map_overlay_pos_width", {&map_overlay_pos_width}, {320},0,320, def_int},
  { "map_overlay_pos_height", {&map_overlay_pos_height}, {200},0,200, def_int},
  { "map_things_appearance", {(int*)&map_things_appearance}, {map_things_appearance_max-1},0,map_things_appearance_max-1, def_int},

  SETTING_HEADING("Heads-up display settings"),
  { "hudcolor_titl", {&hudcolor_titl}, {5},0,9, def_int}, // color range used for automap level title
  { "hudcolor_xyco", {&hudcolor_xyco}, {3},0,9, def_int}, // color range used for automap coordinates
  { "hudcolor_mapstat_title", {&hudcolor_mapstat_title}, {6},0,9, def_int}, // color range used for automap statistics for titles
  { "hudcolor_mapstat_value", {&hudcolor_mapstat_value}, {2},0,9, def_int}, // color range used for automap statistics for data
  { "hudcolor_mapstat_time", {&hudcolor_mapstat_time}, {2},0,9, def_int}, // color range used for automap statistics for level time and total time
  { "hudcolor_mesg", {&hudcolor_mesg}, {6},0,9, def_int}, // color range used for messages during play
  { "hudcolor_list", {&hudcolor_list}, {5},0,9, def_int}, // color range used for message review
  { "hud_msg_lines", {&hud_msg_lines}, {1},1,16, def_int}, // number of messages in review display (1=disable)
  { "hud_list_bgon", {&hud_list_bgon}, {0},0,1, def_bool}, // enables background window behind message review
  { "health_red", { &health_red }, { 25 }, 0, 200, def_int }, // amount of health for red to yellow transition
  { "health_yellow", { &health_yellow }, { 50 }, 0, 200, def_int }, // amount of health for yellow to green transition
  { "health_green", { &health_green}, { 100 }, 0, 200, def_int }, // amount of health for green to blue transition
  { "ammo_red", { &ammo_red }, { 25 }, 0, 100, def_int }, // percent of ammo for red to yellow transition
  { "ammo_yellow", { &ammo_yellow }, { 50 }, 0, 100, def_int }, // percent of ammo for yellow to green transition
  { "hud_displayed", { &hud_displayed },  { 0 }, 0, 1, def_bool },
  { "hudadd_secretarea", { &hudadd_secretarea }, { 0 }, 0, 1, def_bool },
  { "hudadd_demoprogressbar", { &hudadd_demoprogressbar }, { 1 }, 0, 1, def_bool },
  { "hudadd_crosshair", { &hudadd_crosshair }, { 0 }, 0, HU_CROSSHAIRS - 1, def_bool },
  { "hudadd_crosshair_scale", { &hudadd_crosshair_scale }, { 0 }, 0, 1, def_bool },
  { "hudadd_crosshair_color", { &hudadd_crosshair_color }, { 3 }, 0, 9, def_int },
  { "hudadd_crosshair_health", { &hudadd_crosshair_health }, { 0 }, 0, 1, def_bool },
  { "hudadd_crosshair_target", { &hudadd_crosshair_target }, { 0 }, 0, 1, def_bool },
  { "hudadd_crosshair_target_color", { &hudadd_crosshair_target_color }, { 9 }, 0, 9, def_int },
  { "hudadd_crosshair_lock_target", { &hudadd_crosshair_lock_target }, { 0 }, 0, 1, def_bool },

  SETTING_HEADING("DSDA-Doom settings"),
  MIGRATED_SETTING(dsda_config_strict_mode),
  { "dsda_cycle_ghost_colors", { &dsda_cycle_ghost_colors }, { 0 }, 0, 1, def_bool },
  { "dsda_auto_key_frame_interval", { &dsda_auto_key_frame_interval }, { 1 }, 1, 600, def_int },
  { "dsda_auto_key_frame_depth", { &dsda_auto_key_frame_depth }, { 60 }, 0, 600, def_int },
  { "dsda_auto_key_frame_timeout", { &dsda_auto_key_frame_timeout }, { 10 }, 0, 25, def_int },
  MIGRATED_SETTING(dsda_config_exhud),
  { "dsda_ex_text_scale", { &dsda_ex_text_scale }, { 0 }, 0, 16, def_int },
  { "dsda_wipe_at_full_speed", { &dsda_wipe_at_full_speed }, { 1 }, 0, 1, def_bool },
  { "dsda_show_demo_attempts", { &dsda_show_demo_attempts }, { 1 }, 0, 1, def_bool },
  { "dsda_fine_sensitivity", { &dsda_fine_sensitivity }, { 0 }, 0, 99, def_int },
  { "dsda_hide_horns", { &dsda_hide_horns }, { 0 }, 0, 1, def_bool },
  { "dsda_organized_saves", { &dsda_organized_saves }, { 1 }, 0, 1, def_bool },
  MIGRATED_SETTING(dsda_config_command_display),
  { "dsda_command_history_size", { &dsda_command_history_size }, { 10 }, 1, 20, def_int },
  { "dsda_hide_empty_commands", { &dsda_hide_empty_commands }, { 1 }, 0, 1, def_bool },
  MIGRATED_SETTING(dsda_config_coordinate_display),
  MIGRATED_SETTING(dsda_config_show_fps),
  { "dsda_skip_quit_prompt", { &dsda_skip_quit_prompt }, { 0 }, 0, 1, def_bool },
  { "dsda_show_split_data", { &dsda_show_split_data }, { 1 }, 0, 1, def_bool },
  { "dsda_player_name", { 0, &dsda_player_name }, { 0, "Anonymous" }, UL, UL, def_str },
  { "dsda_quickstart_cache_tics", { &dsda_quickstart_cache_tics }, { 0 }, 0, 35, def_int },
  { "dsda_death_use_action", { &dsda_death_use_action }, { 0 }, 0, 2, def_int },
  MIGRATED_SETTING(dsda_config_mute_sfx),
  MIGRATED_SETTING(dsda_config_mute_music),
  MIGRATED_SETTING(dsda_config_cheat_codes),
  { "dsda_allow_jumping", { &dsda_allow_jumping }, { 0 }, 0, 1, def_bool },
  { "dsda_parallel_sfx_limit", { &dsda_parallel_sfx_limit }, { 0 }, 0, 32, def_int },
  { "dsda_parallel_sfx_window", { &dsda_parallel_sfx_window }, { 1 }, 1, 32, def_int },
  { "dsda_switch_when_ammo_runs_out", { &dsda_switch_when_ammo_runs_out }, { 1 }, 0, 1, def_bool },
  { "dsda_viewbob", { &dsda_viewbob }, { 1 }, 0, 1, def_bool },
  { "dsda_weaponbob", { &dsda_weaponbob }, { 1 }, 0, 1, def_bool },

  SETTING_HEADING("Scripts"),
  MIGRATED_SETTING(dsda_config_script_0),
  MIGRATED_SETTING(dsda_config_script_1),
  MIGRATED_SETTING(dsda_config_script_2),
  MIGRATED_SETTING(dsda_config_script_3),
  MIGRATED_SETTING(dsda_config_script_4),
  MIGRATED_SETTING(dsda_config_script_5),
  MIGRATED_SETTING(dsda_config_script_6),
  MIGRATED_SETTING(dsda_config_script_7),
  MIGRATED_SETTING(dsda_config_script_8),
  MIGRATED_SETTING(dsda_config_script_9),

  // NSM
  SETTING_HEADING("Video capture encoding settings"),
  { "cap_soundcommand",{NULL, &cap_soundcommand},{0,"ffmpeg -f s16le -ar %s -ac 2 -i - -c:a libopus -y temp_a.nut"},UL,UL,def_str},
  { "cap_videocommand",{NULL, &cap_videocommand},{0,"ffmpeg -f rawvideo -pix_fmt rgb24 -r %r -s %wx%h -i - -c:v libx264 -y temp_v.nut"},UL,UL,def_str},
  { "cap_muxcommand",{NULL, &cap_muxcommand},{0,"ffmpeg -i temp_v.nut -i temp_a.nut -c copy -y %f"},UL,UL,def_str},
  { "cap_tempfile1",{NULL, &cap_tempfile1},{0,"temp_a.nut"},UL,UL,def_str},
  { "cap_tempfile2",{NULL, &cap_tempfile2},{0,"temp_v.nut"},UL,UL,def_str},
  { "cap_remove_tempfiles", {&cap_remove_tempfiles},{1},0,1,def_bool},
  { "cap_fps", {&cap_fps},{60},16,300,def_int},
  { "cap_wipescreen", {&cap_wipescreen},{0},0,1,def_bool},

  SETTING_HEADING("Overrun settings"),
  MIGRATED_SETTING(dsda_config_overrun_spechit_warn),
  MIGRATED_SETTING(dsda_config_overrun_spechit_emulate),
  MIGRATED_SETTING(dsda_config_overrun_reject_warn),
  MIGRATED_SETTING(dsda_config_overrun_reject_emulate),
  MIGRATED_SETTING(dsda_config_overrun_intercept_warn),
  MIGRATED_SETTING(dsda_config_overrun_intercept_emulate),
  MIGRATED_SETTING(dsda_config_overrun_playeringame_warn),
  MIGRATED_SETTING(dsda_config_overrun_playeringame_emulate),
  MIGRATED_SETTING(dsda_config_overrun_donut_warn),
  MIGRATED_SETTING(dsda_config_overrun_donut_emulate),
  MIGRATED_SETTING(dsda_config_overrun_missedbackside_warn),
  MIGRATED_SETTING(dsda_config_overrun_missedbackside_emulate),

  SETTING_HEADING("Mapping error compatibility settings"),
  MIGRATED_SETTING(dsda_config_comperr_zerotag),
  MIGRATED_SETTING(dsda_config_comperr_passuse),
  MIGRATED_SETTING(dsda_config_comperr_hangsolid),
  MIGRATED_SETTING(dsda_config_comperr_blockmap),
  MIGRATED_SETTING(dsda_config_comperr_freeaim),

  SETTING_HEADING("Weapon preferences"),
  MIGRATED_SETTING(dsda_config_weapon_choice_1),
  MIGRATED_SETTING(dsda_config_weapon_choice_2),
  MIGRATED_SETTING(dsda_config_weapon_choice_3),
  MIGRATED_SETTING(dsda_config_weapon_choice_4),
  MIGRATED_SETTING(dsda_config_weapon_choice_5),
  MIGRATED_SETTING(dsda_config_weapon_choice_6),
  MIGRATED_SETTING(dsda_config_weapon_choice_7),
  MIGRATED_SETTING(dsda_config_weapon_choice_8),
  MIGRATED_SETTING(dsda_config_weapon_choice_9),

  SETTING_HEADING("Input settings"),
  MIGRATED_SETTING(dsda_config_input_profile),

  INPUT_SETTING("input_forward", dsda_input_forward, 'w', 2, -1),
  INPUT_SETTING("input_backward", dsda_input_backward, 's', -1, -1),
  INPUT_SETTING("input_turnleft", dsda_input_turnleft, 'e', -1, -1),
  INPUT_SETTING("input_turnright", dsda_input_turnright, 'q', -1, -1),
  INPUT_SETTING("input_speed", dsda_input_speed, KEYD_RSHIFT, -1, 2),
  INPUT_SETTING("input_strafeleft", dsda_input_strafeleft, 'a', -1, 4),
  INPUT_SETTING("input_straferight", dsda_input_straferight, 'd', -1, 5),
  INPUT_SETTING("input_strafe", dsda_input_strafe, KEYD_RALT, 1, 1),
  INPUT_SETTING("input_autorun", dsda_input_autorun, KEYD_CAPSLOCK, -1, -1),
  INPUT_SETTING("input_reverse", dsda_input_reverse, '/', -1, -1),
  INPUT_SETTING("input_use", dsda_input_use, ' ', -1, 3),
  INPUT_SETTING("input_flyup", dsda_input_flyup, '.', -1, -1),
  INPUT_SETTING("input_flydown", dsda_input_flydown, ',', -1, -1),
  INPUT_SETTING("input_flycenter", dsda_input_flycenter, 0, -1, -1),
  INPUT_SETTING("input_mlook", dsda_input_mlook, '\\', -1, -1),
  INPUT_SETTING("input_novert", dsda_input_novert, 0, -1, -1),

  INPUT_SETTING("input_weapon1", dsda_input_weapon1, '1', -1, -1),
  INPUT_SETTING("input_weapon2", dsda_input_weapon2, '2', -1, -1),
  INPUT_SETTING("input_weapon3", dsda_input_weapon3, '3', -1, -1),
  INPUT_SETTING("input_weapon4", dsda_input_weapon4, '4', -1, -1),
  INPUT_SETTING("input_weapon5", dsda_input_weapon5, '5', -1, -1),
  INPUT_SETTING("input_weapon6", dsda_input_weapon6, '6', -1, -1),
  INPUT_SETTING("input_weapon7", dsda_input_weapon7, '7', -1, -1),
  INPUT_SETTING("input_weapon8", dsda_input_weapon8, '8', -1, -1),
  INPUT_SETTING("input_weapon9", dsda_input_weapon9, '9', -1, -1),
  INPUT_SETTING("input_nextweapon", dsda_input_nextweapon, KEYD_MWHEELUP, -1, -1),
  INPUT_SETTING("input_prevweapon", dsda_input_prevweapon, KEYD_MWHEELDOWN, -1, -1),
  INPUT_SETTING("input_toggleweapon", dsda_input_toggleweapon, '0', -1, -1),
  INPUT_SETTING("input_fire", dsda_input_fire, KEYD_RCTRL, 0, 0),

  INPUT_SETTING("input_setup", dsda_input_setup, 0, -1, -1),
  INPUT_SETTING("input_pause", dsda_input_pause, KEYD_PAUSE, -1, -1),
  INPUT_SETTING("input_map", dsda_input_map, KEYD_TAB, -1, -1),
  INPUT_SETTING("input_soundvolume", dsda_input_soundvolume, KEYD_F4, -1, -1),
  INPUT_SETTING("input_hud", dsda_input_hud, KEYD_F5, -1, -1),
  INPUT_SETTING("input_messages", dsda_input_messages, KEYD_F8, -1, -1),
  INPUT_SETTING("input_gamma", dsda_input_gamma, KEYD_F11, -1, -1),
  INPUT_SETTING("input_spy", dsda_input_spy, KEYD_F12, -1, -1),
  INPUT_SETTING("input_zoomin", dsda_input_zoomin, '=', -1, -1),
  INPUT_SETTING("input_zoomout", dsda_input_zoomout, '-', -1, -1),
  INPUT_SETTING("input_screenshot", dsda_input_screenshot, '*', -1, -1),
  INPUT_SETTING("input_savegame", dsda_input_savegame, KEYD_F2, -1, -1),
  INPUT_SETTING("input_loadgame", dsda_input_loadgame, KEYD_F3, -1, -1),
  INPUT_SETTING("input_quicksave", dsda_input_quicksave, KEYD_F6, -1, -1),
  INPUT_SETTING("input_quickload", dsda_input_quickload, KEYD_F9, -1, -1),
  INPUT_SETTING("input_endgame", dsda_input_endgame, KEYD_F7, -1, -1),
  INPUT_SETTING("input_quit", dsda_input_quit, KEYD_F10, -1, -1),

  INPUT_SETTING("input_map_follow", dsda_input_map_follow, 'f', -1, -1),
  INPUT_SETTING("input_map_zoomin", dsda_input_map_zoomin, '=', -1, -1),
  INPUT_SETTING("input_map_zoomout", dsda_input_map_zoomout, '-', -1, -1),
  INPUT_SETTING("input_map_up", dsda_input_map_up, KEYD_UPARROW, -1, -1),
  INPUT_SETTING("input_map_down", dsda_input_map_down, KEYD_DOWNARROW, -1, -1),
  INPUT_SETTING("input_map_left", dsda_input_map_left, KEYD_LEFTARROW, -1, -1),
  INPUT_SETTING("input_map_right", dsda_input_map_right, KEYD_RIGHTARROW, -1, -1),
  INPUT_SETTING("input_map_mark", dsda_input_map_mark, 'm', -1, -1),
  INPUT_SETTING("input_map_clear", dsda_input_map_clear, 'c', -1, -1),
  INPUT_SETTING("input_map_gobig", dsda_input_map_gobig, '0', -1, -1),
  INPUT_SETTING("input_map_grid", dsda_input_map_grid, 'g', -1, -1),
  INPUT_SETTING("input_map_rotate", dsda_input_map_rotate, 'r', -1, -1),
  INPUT_SETTING("input_map_overlay", dsda_input_map_overlay, 'o', -1, -1),
  INPUT_SETTING("input_map_textured", dsda_input_map_textured, 0, -1, -1),

  INPUT_SETTING("input_repeat_message", dsda_input_repeat_message, 0, -1, -1),

  INPUT_SETTING("input_speed_up", dsda_input_speed_up, 0, -1, -1),
  INPUT_SETTING("input_speed_down", dsda_input_speed_down, 0, -1, -1),
  INPUT_SETTING("input_speed_default", dsda_input_speed_default, 0, -1, -1),
  INPUT_SETTING("input_demo_skip", dsda_input_demo_skip, KEYD_INSERT, -1, -1),
  INPUT_SETTING("input_demo_endlevel", dsda_input_demo_endlevel, KEYD_END, -1, -1),
  INPUT_SETTING("input_walkcamera", dsda_input_walkcamera, KEYD_KEYPAD0, -1, -1),
  INPUT_SETTING("input_join_demo", dsda_input_join_demo, 0, -1, -1),
  INPUT_SETTING("input_restart", dsda_input_restart, KEYD_HOME, -1, -1),
  INPUT_SETTING("input_nextlevel", dsda_input_nextlevel, KEYD_PAGEDOWN, -1, -1),
  INPUT_SETTING("input_showalive", dsda_input_showalive, KEYD_KEYPADDIVIDE, -1, -1),

  INPUT_SETTING("input_menu_down", dsda_input_menu_down, KEYD_DOWNARROW, -1, -1),
  INPUT_SETTING("input_menu_up", dsda_input_menu_up, KEYD_UPARROW, -1, -1),
  INPUT_SETTING("input_menu_left", dsda_input_menu_left, KEYD_LEFTARROW, -1, -1),
  INPUT_SETTING("input_menu_right", dsda_input_menu_right, KEYD_RIGHTARROW, -1, -1),
  INPUT_SETTING("input_menu_backspace", dsda_input_menu_backspace, KEYD_BACKSPACE, -1, -1),
  INPUT_SETTING("input_menu_enter", dsda_input_menu_enter, KEYD_ENTER, -1, -1),
  INPUT_SETTING("input_menu_escape", dsda_input_menu_escape, KEYD_ESCAPE, -1, -1),
  INPUT_SETTING("input_menu_clear", dsda_input_menu_clear, KEYD_DEL, -1, -1),

  INPUT_SETTING("input_iddqd", dsda_input_iddqd, 0, -1, -1),
  INPUT_SETTING("input_idkfa", dsda_input_idkfa, 0, -1, -1),
  INPUT_SETTING("input_idfa", dsda_input_idfa, 0, -1, -1),
  INPUT_SETTING("input_idclip", dsda_input_idclip, 0, -1, -1),
  INPUT_SETTING("input_idbeholdh", dsda_input_idbeholdh, 0, -1, -1),
  INPUT_SETTING("input_idbeholdm", dsda_input_idbeholdm, 0, -1, -1),
  INPUT_SETTING("input_idbeholdv", dsda_input_idbeholdv, 0, -1, -1),
  INPUT_SETTING("input_idbeholds", dsda_input_idbeholds, 0, -1, -1),
  INPUT_SETTING("input_idbeholdi", dsda_input_idbeholdi, 0, -1, -1),
  INPUT_SETTING("input_idbeholdr", dsda_input_idbeholdr, 0, -1, -1),
  INPUT_SETTING("input_idbeholda", dsda_input_idbeholda, 0, -1, -1),
  INPUT_SETTING("input_idbeholdl", dsda_input_idbeholdl, 0, -1, -1),
  INPUT_SETTING("input_idmypos", dsda_input_idmypos, 0, -1, -1),
  INPUT_SETTING("input_idrate", dsda_input_idrate, 0, -1, -1),
  INPUT_SETTING("input_iddt", dsda_input_iddt, 0, -1, -1),
  INPUT_SETTING("input_ponce", dsda_input_ponce, 0, -1, -1),
  INPUT_SETTING("input_shazam", dsda_input_shazam, 0, -1, -1),
  INPUT_SETTING("input_chicken", dsda_input_chicken, 0, -1, -1),

  INPUT_SETTING("input_lookup", dsda_input_lookup, 0, -1, -1),
  INPUT_SETTING("input_lookdown", dsda_input_lookdown, 0, -1, -1),
  INPUT_SETTING("input_lookcenter", dsda_input_lookcenter, 0, -1, -1),
  INPUT_SETTING("input_use_artifact", dsda_input_use_artifact, 0, -1, -1),
  INPUT_SETTING("input_arti_tome", dsda_input_arti_tome, 0, -1, -1),
  INPUT_SETTING("input_arti_quartz", dsda_input_arti_quartz, 0, -1, -1),
  INPUT_SETTING("input_arti_urn", dsda_input_arti_urn, 0, -1, -1),
  INPUT_SETTING("input_arti_bomb", dsda_input_arti_bomb, 0, -1, -1),
  INPUT_SETTING("input_arti_ring", dsda_input_arti_ring, 0, -1, -1),
  INPUT_SETTING("input_arti_chaosdevice", dsda_input_arti_chaosdevice, 0, -1, -1),
  INPUT_SETTING("input_arti_shadowsphere", dsda_input_arti_shadowsphere, 0, -1, -1),
  INPUT_SETTING("input_arti_wings", dsda_input_arti_wings, 0, -1, -1),
  INPUT_SETTING("input_arti_torch", dsda_input_arti_torch, 0, -1, -1),
  INPUT_SETTING("input_arti_morph", dsda_input_arti_morph, 0, -1, -1),
  INPUT_SETTING("input_invleft", dsda_input_invleft, 0, -1, -1),
  INPUT_SETTING("input_invright", dsda_input_invright, 0, -1, -1),
  INPUT_SETTING("input_store_quick_key_frame", dsda_input_store_quick_key_frame, 0, -1, -1),
  INPUT_SETTING("input_restore_quick_key_frame", dsda_input_restore_quick_key_frame, 0, -1, -1),
  INPUT_SETTING("input_rewind", dsda_input_rewind, 0, -1, -1),
  INPUT_SETTING("input_cycle_profile", dsda_input_cycle_profile, 0, -1, -1),
  INPUT_SETTING("input_cycle_palette", dsda_input_cycle_palette, 0, -1, -1),
  INPUT_SETTING("input_command_display", dsda_input_command_display, 0, -1, -1),
  INPUT_SETTING("input_strict_mode", dsda_input_strict_mode, 0, -1, -1),
  INPUT_SETTING("input_console", dsda_input_console, 0, -1, -1),
  INPUT_SETTING("input_coordinate_display", dsda_input_coordinate_display, 0, -1, -1),
  INPUT_SETTING("input_fps", dsda_input_fps, 0, -1, -1),
  INPUT_SETTING("input_avj", dsda_input_avj, 0, -1, -1),
  INPUT_SETTING("input_exhud", dsda_input_exhud, 0, -1, -1),
  INPUT_SETTING("input_mute_sfx", dsda_input_mute_sfx, 0, -1, -1),
  INPUT_SETTING("input_mute_music", dsda_input_mute_music, 0, -1, -1),
  INPUT_SETTING("input_cheat_codes", dsda_input_cheat_codes, 0, -1, -1),
  INPUT_SETTING("input_notarget", dsda_input_notarget, 0, -1, -1),
  INPUT_SETTING("input_freeze", dsda_input_freeze, 0, -1, -1),

  INPUT_SETTING("input_build", dsda_input_build, 0, -1, -1),
  INPUT_SETTING("input_build_advance_frame", dsda_input_build_advance_frame, KEYD_RIGHTARROW, -1, -1),
  INPUT_SETTING("input_build_reverse_frame", dsda_input_build_reverse_frame, KEYD_LEFTARROW, -1, -1),
  INPUT_SETTING("input_build_reset_command", dsda_input_build_reset_command, KEYD_DEL, -1, -1),
  INPUT_SETTING("input_build_source", dsda_input_build_source, KEYD_RSHIFT, -1, -1),
  INPUT_SETTING("input_build_forward", dsda_input_build_forward, 'w', -1, -1),
  INPUT_SETTING("input_build_backward", dsda_input_build_backward, 's', -1, -1),
  INPUT_SETTING("input_build_fine_forward", dsda_input_build_fine_forward, 't', -1, -1),
  INPUT_SETTING("input_build_fine_backward", dsda_input_build_fine_backward, 'g', -1, -1),
  INPUT_SETTING("input_build_turn_left", dsda_input_build_turn_left, 'e', -1, -1),
  INPUT_SETTING("input_build_turn_right", dsda_input_build_turn_right, 'q', -1, -1),
  INPUT_SETTING("input_build_strafe_left", dsda_input_build_strafe_left, 'a', -1, -1),
  INPUT_SETTING("input_build_strafe_right", dsda_input_build_strafe_right, 'd', -1, -1),
  INPUT_SETTING("input_build_fine_strafe_left", dsda_input_build_fine_strafe_left, 'f', -1, -1),
  INPUT_SETTING("input_build_fine_strafe_right", dsda_input_build_fine_strafe_right, 'h', -1, -1),
  INPUT_SETTING("input_build_use", dsda_input_build_use, KEYD_SPACEBAR, -1, -1),
  INPUT_SETTING("input_build_fire", dsda_input_build_fire, KEYD_RCTRL, -1, -1),
  INPUT_SETTING("input_build_weapon1", dsda_input_build_weapon1, '1', -1, -1),
  INPUT_SETTING("input_build_weapon2", dsda_input_build_weapon2, '2', -1, -1),
  INPUT_SETTING("input_build_weapon3", dsda_input_build_weapon3, '3', -1, -1),
  INPUT_SETTING("input_build_weapon4", dsda_input_build_weapon4, '4', -1, -1),
  INPUT_SETTING("input_build_weapon5", dsda_input_build_weapon5, '5', -1, -1),
  INPUT_SETTING("input_build_weapon6", dsda_input_build_weapon6, '6', -1, -1),
  INPUT_SETTING("input_build_weapon7", dsda_input_build_weapon7, '7', -1, -1),
  INPUT_SETTING("input_build_weapon8", dsda_input_build_weapon8, '8', -1, -1),
  INPUT_SETTING("input_build_weapon9", dsda_input_build_weapon9, '9', -1, -1),

  INPUT_SETTING("input_jump", dsda_input_jump, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_incant", dsda_input_hexen_arti_incant, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_summon", dsda_input_hexen_arti_summon, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_disk", dsda_input_hexen_arti_disk, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_flechette", dsda_input_hexen_arti_flechette, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_banishment", dsda_input_hexen_arti_banishment, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_boots", dsda_input_hexen_arti_boots, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_krater", dsda_input_hexen_arti_krater, 0, -1, -1),
  INPUT_SETTING("input_hexen_arti_bracers", dsda_input_hexen_arti_bracers, 0, -1, -1),

  INPUT_SETTING("input_script_0", dsda_input_script_0, 0, -1, -1),
  INPUT_SETTING("input_script_1", dsda_input_script_1, 0, -1, -1),
  INPUT_SETTING("input_script_2", dsda_input_script_2, 0, -1, -1),
  INPUT_SETTING("input_script_3", dsda_input_script_3, 0, -1, -1),
  INPUT_SETTING("input_script_4", dsda_input_script_4, 0, -1, -1),
  INPUT_SETTING("input_script_5", dsda_input_script_5, 0, -1, -1),
  INPUT_SETTING("input_script_6", dsda_input_script_6, 0, -1, -1),
  INPUT_SETTING("input_script_7", dsda_input_script_7, 0, -1, -1),
  INPUT_SETTING("input_script_8", dsda_input_script_8, 0, -1, -1),
  INPUT_SETTING("input_script_9", dsda_input_script_9, 0, -1, -1),
};

int numdefaults;
static char* defaultfile; // CPhipps - static, const

//
// M_SaveDefaults
//

void M_SaveDefaults (void)
{
  int   i;
  FILE* f;
  int maxlen = 0;

  f = fopen (defaultfile, "w");
  if (!f)
    return; // can't write the file, but don't complain

  // get maximum config key string length
  for (i = 0 ; i < numdefaults ; i++) {
    int len;
    if (defaults[i].type == def_none) {
      continue;
    }
    len = strlen(defaults[i].name);
    if (len > maxlen && len < 80) {
      maxlen = len;
    }
  }

  // 3/3/98 explain format of file

  fprintf(f,"# Doom config file\n");
  fprintf(f,"# Format:\n");
  fprintf(f,"# variable   value\n");

  for (i = 0 ; i < numdefaults ; i++) {
    if (defaults[i].config_id)
    {
      dsda_WriteConfig(defaults[i].config_id, maxlen, f);
    }
    else
    {
      if (defaults[i].type == def_none) {
        // CPhipps - pure headers
        fprintf(f, "\n# %s\n", defaults[i].name);
      }
      // CPhipps - modified for new default_t form
      else if (!IS_STRING(defaults[i])) //jff 4/10/98 kill super-hack on pointer value
      {
        // CPhipps - remove keycode hack
        // killough 3/6/98: use spaces instead of tabs for uniform justification
        if (defaults[i].type == def_hex)
          fprintf (f,"%-*s 0x%x\n",maxlen,defaults[i].name,*(defaults[i].location.pi));
        else if (defaults[i].type == def_input)
        {
          int a, j;
          dsda_input_t* input[DSDA_INPUT_PROFILE_COUNT];
          dsda_InputCopy(defaults[i].identifier, input);

          fprintf(f, "%-*s", maxlen, defaults[i].name);

          for (a = 0; a < DSDA_INPUT_PROFILE_COUNT; ++a)
          {
            if (input[a]->num_keys)
            {
              fprintf(f, " %i", input[a]->key[0]);
              for (j = 1; j < input[a]->num_keys; ++j)
              {
                fprintf(f, ",%i", input[a]->key[j]);
              }
            }
            else
              fprintf(f, " 0");

            fprintf(f, " %i %i", input[a]->mouseb, input[a]->joyb);

            if (a != DSDA_INPUT_PROFILE_COUNT - 1)
              fprintf(f, " |");
          }

          fprintf(f, "\n");
        }
        else
          fprintf (f,"%-*s %i\n",maxlen,defaults[i].name,*(defaults[i].location.pi));
      }
      else
      {
        fprintf (f,"%-*s \"%s\"\n",maxlen,defaults[i].name,*(defaults[i].location.ppsz));
      }
    }
  }

  fclose (f);
}

/*
 * M_LookupDefault
 *
 * cph - mimic MBF function for now. Yes it's crap.
 */

struct default_s *M_LookupDefault(const char *name)
{
  int i;
  for (i = 0 ; i < numdefaults ; i++)
  {
    if ((defaults[i].type != def_none) && !strcmp(name, defaults[i].name))
      return &defaults[i];
  }

  I_Error("M_LookupDefault: %s not found",name);
  return NULL;
}

//
// M_LoadDefaults
//

#define CFG_BUFFERMAX 32000

void M_LoadDefaults (void)
{
  int   i;
  int   len;
  FILE* f;
  char  def[80];
  char* strparm = Z_Malloc(CFG_BUFFERMAX);
  char* cfgline = Z_Malloc(CFG_BUFFERMAX);
  char* newstring = NULL;   // killough
  int   parm;
  dsda_arg_t *arg;
  dboolean isstring;

  // set everything to base values

  dsda_InitConfig();

  numdefaults = sizeof(defaults)/sizeof(defaults[0]);
  for (i = 0 ; i < numdefaults ; i++) {
    if (!defaults[i].config_id) // not handled in dsda_InitConfig (yet)
    {
      if (defaults[i].type == def_input)
      {
        int c;
        for (c = 0; c < DSDA_INPUT_PROFILE_COUNT; ++c)
          dsda_InputSetSpecific(c, defaults[i].identifier, defaults[i].input);
      }
      else
      {
        if (defaults[i].location.ppsz)
          *defaults[i].location.ppsz = Z_Strdup(defaults[i].defaultvalue.psz);
        if (defaults[i].location.pi)
          *defaults[i].location.pi = defaults[i].defaultvalue.i;
      }
    }
  }

  // special fallback input values
  {
    dsda_input_default_t fallback_help = { KEYD_F1, -1, -1 };
    dsda_input_default_t fallback_escape = { KEYD_ESCAPE, -1, -1 };

    for (i = 0; i < DSDA_INPUT_PROFILE_COUNT; ++i)
    {
      dsda_InputSetSpecific(i, dsda_input_help, fallback_help);
      dsda_InputSetSpecific(i, dsda_input_escape, fallback_escape);
    }
  }

  // check for a custom default file

  arg = dsda_Arg(dsda_arg_config);
  if (arg->found)
  {
    defaultfile = Z_Strdup(arg->value.v_string);
  }
  else
  {
    const char* exedir = I_DoomExeDir();
    int len = snprintf(NULL, 0, "%s/dsda-doom.cfg", exedir);
    defaultfile = Z_Malloc(len + 1);
    snprintf(defaultfile, len + 1, "%s/dsda-doom.cfg", exedir);
  }

  lprintf (LO_INFO, " default file: %s\n",defaultfile);

  // read the file in, overriding any set defaults

  f = fopen (defaultfile, "r");
  if (f)
  {
    while (!feof(f))
    {
      isstring = false;
      parm = 0;
      fgets(cfgline, CFG_BUFFERMAX, f);
      if (sscanf (cfgline, "%79s %[^\n]\n", def, strparm) == 2)
      {
        newstring = NULL;

        //jff 3/3/98 skip lines not starting with an alphanum
        if (!isalnum(def[0]))
          continue;

        if (strparm[0] == '"')
        {
          // get a string default
          isstring = true;
          len = strlen(strparm);
          newstring = Z_Malloc(len);
          strparm[len-1] = 0; // clears trailing double-quote mark
          strcpy(newstring, strparm+1); // clears leading double-quote mark
        }
        else if ((strparm[0] == '0') && (strparm[1] == 'x'))
        {
          // CPhipps - allow ints to be specified in hex
          sscanf(strparm+2, "%x", &parm);
        }
        else
        {
          sscanf(strparm, "%i", &parm);
        }

        if (dsda_ReadConfig(def, newstring, parm))
        {
          Z_Free(newstring);
        }
        else
        {
          for (i = 0 ; i < numdefaults ; i++)
            if ((defaults[i].type != def_none) && !strcmp(def, defaults[i].name))
            {
              // CPhipps - safety check
              if (isstring != IS_STRING(defaults[i])) {
                lprintf(LO_WARN, "M_LoadDefaults: Type mismatch reading %s\n", defaults[i].name);
                continue;
              }

              if (!isstring)
              {
                if (defaults[i].type == def_input)
                {
                  int count;
                  char keys[80];
                  int key, mouseb, joyb;
                  int index = 0;
                  char* key_scan_p;
                  char* config_scan_p;

                  config_scan_p = strparm;
                  do
                  {
                    count = sscanf(config_scan_p, "%79s %d %d", keys, &mouseb, &joyb);

                    if (count != 3)
                      break;

                    dsda_InputResetSpecific(index, defaults[i].identifier);

                    dsda_InputAddSpecificMouseB(index, defaults[i].identifier, mouseb);
                    dsda_InputAddSpecificJoyB(index, defaults[i].identifier, joyb);

                    key_scan_p = strtok(keys, ",");
                    do
                    {
                      count = sscanf(key_scan_p, "%d,", &key);

                      if (count != 1)
                        break;

                      dsda_InputAddSpecificKey(index, defaults[i].identifier, key);

                      key_scan_p = strtok(NULL, ",");
                    } while (key_scan_p);

                    index++;
                    config_scan_p = strchr(config_scan_p, '|');
                    if (config_scan_p)
                      config_scan_p++;
                  } while (config_scan_p && index < DSDA_INPUT_PROFILE_COUNT);
                }

                //jff 3/4/98 range check numeric parameters

                else if ((defaults[i].minvalue==UL || defaults[i].minvalue<=parm) &&
                         (defaults[i].maxvalue==UL || defaults[i].maxvalue>=parm))
                  *(defaults[i].location.pi) = parm;
              }
              else
              {
                union { const char **c; char **s; } u; // type punning via unions

                u.c = defaults[i].location.ppsz;
                Z_Free(*(u.s));
                *(u.s) = newstring;
              }

              break;
            }
        }
      }
    }

    fclose (f);
  }

  Z_Free(strparm);
  Z_Free(cfgline);

  dsda_InitSettings();

  //e6y: Check on existence of dsda-doom.wad
  if (!(port_wad_file = I_FindFile(WAD_DATA, "")))
    I_Error("dsda-doom.wad not found. Can't continue.");
}


//
// SCREEN SHOTS
//

//
// M_ScreenShot
//
// Modified by Lee Killough so that any number of shots can be taken,
// the code is faster, and no annoying "screenshot" message appears.

// CPhipps - modified to use its own buffer for the image
//         - checks for the case where no file can be created (doesn't occur on POSIX systems, would on DOS)
//         - track errors better
//         - split into 2 functions

//
// M_DoScreenShot
// Takes a screenshot into the names file

const char *screenshot_dir;

void M_DoScreenShot (const char* fname)
{
  if (I_ScreenShot(fname) != 0)
    doom_printf("M_ScreenShot: Error writing screenshot\n");
}

#ifndef SCREENSHOT_DIR
#define SCREENSHOT_DIR "."
#endif

#ifdef HAVE_LIBSDL2_IMAGE
#define SCREENSHOT_EXT ".png"
#else
#define SCREENSHOT_EXT ".bmp"
#endif

const char* M_CheckWritableDir(const char *dir)
{
  static char *base = NULL;
  static int base_len = 0;

  const char *result = NULL;
  int len;

  if (!dir || !(len = strlen(dir)))
  {
    return NULL;
  }

  if (len + 1 > base_len)
  {
    base_len = len + 1;
    base = Z_Malloc(len + 1);
  }

  if (base)
  {
    strcpy(base, dir);

    if (base[len - 1] != '\\' && base[len - 1] != '/')
      strcat(base, "/");
    if (!access(base, O_RDWR))
    {
      base[strlen(base) - 1] = 0;
      result = base;
    }
  }

  return result;
}

void M_ScreenShot(void)
{
  static int shot;
  char       *lbmname = NULL;
  int        startshot;
  const char *shot_dir = NULL;
  dsda_arg_t *arg;
  int        success = 0;

  arg = dsda_Arg(dsda_arg_shotdir);
  if (arg->found)
    shot_dir = M_CheckWritableDir(arg->value.v_string);
  if (!shot_dir)
    shot_dir = M_CheckWritableDir(screenshot_dir);
  if (!shot_dir)
#ifdef _WIN32
    shot_dir = M_CheckWritableDir(I_DoomExeDir());
#else
    shot_dir = (!access(SCREENSHOT_DIR, 2) ? SCREENSHOT_DIR : NULL);
#endif

  if (shot_dir)
  {
    startshot = shot; // CPhipps - prevent infinite loop

    do {
      int size = snprintf(NULL, 0, "%s/doom%02d" SCREENSHOT_EXT, shot_dir, shot);
      lbmname = Z_Realloc(lbmname, size+1);
      snprintf(lbmname, size+1, "%s/doom%02d" SCREENSHOT_EXT, shot_dir, shot);
      shot++;
    } while (!access(lbmname,0) && (shot != startshot) && (shot < 10000));

    if (access(lbmname,0))
    {
      S_StartSound(NULL,gamemode==commercial ? sfx_radio : sfx_tink);
      M_DoScreenShot(lbmname); // cph
      success = 1;
    }
    Z_Free(lbmname);
    if (success) return;
  }

  doom_printf ("M_ScreenShot: Couldn't create screenshot");
  return;
}


// Safe string copy function that works like OpenBSD's strlcpy().
// Returns true if the string was not truncated.

dboolean M_StringCopy(char *dest, const char *src, size_t dest_size)
{
    size_t len;

    if (dest_size >= 1)
    {
        dest[dest_size - 1] = '\0';
        strncpy(dest, src, dest_size - 1);
    }
    else
    {
        return false;
    }

    len = strlen(dest);
    return src[len] == '\0';
}

// Safe string concat function that works like OpenBSD's strlcat().
// Returns true if string not truncated.

dboolean M_StringConcat(char *dest, const char *src, size_t dest_size)
{
    size_t offset;

    offset = strlen(dest);
    if (offset > dest_size)
    {
        offset = dest_size;
    }

    return M_StringCopy(dest + offset, src, dest_size - offset);
}


int M_StrToInt(const char *s, int *l)
{
  return (
    (sscanf(s, " 0x%x", l) == 1) ||
    (sscanf(s, " 0X%x", l) == 1) ||
    (sscanf(s, " 0%o", l) == 1) ||
    (sscanf(s, " %d", l) == 1)
  );
}

int M_StrToFloat(const char *s, float *f)
{
  return (
    (sscanf(s, " %f", f) == 1)
  );
}

int M_DoubleToInt(double x)
{
#ifdef __GNUC__
 double tmp = x;
 return (int)tmp;
#else
 return (int)x;
#endif
}

char* M_Strlwr(char* str)
{
  char* p;
  for (p=str; *p; p++) *p = tolower(*p);
  return str;
}

char* M_Strupr(char* str)
{
  char* p;
  for (p=str; *p; p++) *p = toupper(*p);
  return str;
}

char *M_StrRTrim(char *str)
{
  char *end;

  if (str)
  {
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end))
    {
      end--;
    }

    // Write new null terminator
    *(end + 1) = 0;
  }

  return str;
}

void M_ArrayClear(array_t *data)
{
  data->count = 0;
}

void* M_ArrayGetNewItem(array_t *data, int itemsize)
{
  if (data->count + 1 >= data->capacity)
  {
    data->capacity = (data->capacity ? data->capacity * 2 : 128);
    data->data = Z_Realloc(data->data, data->capacity * itemsize);
  }

  data->count++;

  return (unsigned char*)data->data + (data->count - 1) * itemsize;
}
