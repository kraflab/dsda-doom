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
#include "m_argv.h"
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
#ifdef USE_WINDOWS_LAUNCHER
#include "e6y_launcher.h"
#endif

#include "dsda/settings.h"

// NSM
#include "i_capture.h"

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
      *buffer = Z_Malloc(length, PU_STATIC, 0);
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

//
// DEFAULTS
//

int usemouse;
dboolean    precache = true; /* if true, load all graphics at start */

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
#ifdef GL_DOOM
extern int gl_nearclip;
extern int gl_colorbuffer_bits;
extern int gl_depthbuffer_bits;
extern int gl_texture_filter;
extern int gl_sprite_filter;
extern int gl_patch_filter;
extern int gl_texture_filter_anisotropic;
extern const char *gl_tex_format_string;
extern int gl_sky_detail;
extern int gl_use_paletted_texture;
extern int gl_use_shared_texture_palette;

//cfg values
extern int gl_ext_texture_filter_anisotropic_default;
extern int gl_arb_texture_non_power_of_two_default;
extern int gl_arb_multitexture_default;
extern int gl_arb_texture_compression_default;
extern int gl_ext_framebuffer_object_default;
extern int gl_ext_packed_depth_stencil_default;
extern int gl_ext_blend_color_default;
extern int gl_use_stencil_default;
extern int gl_ext_arb_vertex_buffer_object_default;
extern int gl_arb_pixel_buffer_object_default;
extern int gl_arb_shader_objects_default;

//e6y: motion bloor
extern int gl_motionblur;

//e6y: fog
extern int gl_fog;
extern int gl_fog_color;

extern int gl_finish;
extern int gl_clear;
extern int gl_ztrick;
#else
// dummy variables for !GL_DOOM
static int gl_nearclip;
extern int gl_colorbuffer_bits;
extern int gl_depthbuffer_bits;
static int gl_texture_filter;
static int gl_sprite_filter;
static int gl_patch_filter;
static int gl_texture_filter_anisotropic;
static const char *gl_tex_format_string;
static int gl_sky_detail;
static int gl_use_paletted_texture;
static int gl_use_shared_texture_palette;
static int gl_ext_texture_filter_anisotropic_default;
static int gl_arb_texture_non_power_of_two_default;
static int gl_arb_multitexture_default;
static int gl_arb_texture_compression_default;
static int gl_ext_framebuffer_object_default;
static int gl_ext_packed_depth_stencil_default;
static int gl_ext_blend_color_default;
static int gl_use_stencil_default;
static int gl_ext_arb_vertex_buffer_object_default;
static int gl_arb_pixel_buffer_object_default;
static int gl_arb_shader_objects_default;
static int gl_motionblur;
static int gl_fog;
static int gl_fog_color;
static int gl_finish;
static int gl_clear;
static int gl_ztrick;

// dummy variables for !GL_DOOM declared in gl_struct.h
int gl_use_display_lists;
int gl_sprite_offset_default;
int gl_sprite_blend;
int gl_mask_sprite_threshold;
int gl_skymode;
int gl_allow_detail_textures;
int gl_detail_maxdist;
spriteclipmode_t gl_spriteclip;
int gl_spriteclip_threshold;
int gl_sprites_frustum_culling;
int gl_boom_colormaps_default;
int gl_hires_24bit_colormap;
int gl_texture_internal_hires;
int gl_texture_external_hires;
int gl_hires_override_pwads;
const char *gl_texture_hires_dir;
int gl_texture_hqresize;
int gl_texture_hqresize_textures;
int gl_texture_hqresize_sprites;
int gl_texture_hqresize_patches;
motion_blur_params_t motion_blur;
gl_lightmode_t gl_lightmode_default;
int gl_light_ambient;
int useglgamma;
int gl_color_mip_levels;
simple_shadow_params_t simple_shadows;
int gl_shadows_maxdist;
int gl_shadows_factor;
int gl_blend_animations;

#endif

extern int realtic_clock_rate;         // killough 4/13/98: adjustable timer
extern int tran_filter_pct;            // killough 2/21/98

extern int screenblocks;

#ifndef DJGPP
int         mus_pause_opt; // 0 = kill music, 1 = pause, 2 = continue
#endif

extern const char* chat_macros[];

extern int endoom_mode;

/* cph - Some MBF stuff parked here for now
 * killough 10/98
 */
int map_point_coordinates;
int map_level_stat;

default_t defaults[] =
{
  //e6y
  {"System settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"process_priority", {&process_priority},{0},0,2,def_int,ss_none},

  {"Misc settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"default_compatibility_level",{(int*)&default_compatibility_level},
   {-1},-1,MAX_COMPATIBILITY_LEVEL-1,
   def_int,ss_none}, // compatibility level" - CPhipps
  {"vanilla_keymap",{&vanilla_keymap},{0},0,1,
   def_bool,ss_none}, // Use vanilla keyboard mapping
  {"realtic_clock_rate",{&realtic_clock_rate},{100},0,UL,
   def_int,ss_none}, // percentage of normal speed (35 fps) realtic clock runs at
  {"menu_background", {(int*)&menu_background}, {1}, 0, 1,
   def_bool,ss_none}, // do Boom fullscreen menus have backgrounds?
  {"max_player_corpse", {&bodyquesize}, {32},-1,UL,   // killough 2/8/98
   def_int,ss_none}, // number of dead bodies in view supported (-1 = no limit)
  {"flashing_hom",{&flashing_hom},{0},0,1,
   def_bool,ss_none}, // killough 10/98 - enable flashing HOM indicator
  {"endoom_mode", {&endoom_mode},{5},0,7, // CPhipps - endoom flags
   def_hex, ss_none}, // 0, +1 for colours, +2 for non-ascii chars, +4 for skip-last-line
  {"level_precache",{(int*)&precache},{1},0,1,
   def_bool,ss_none}, // precache level data?
  {"demo_smoothturns", {&demo_smoothturns},  {0},0,1,
   def_bool,ss_stat},
  {"demo_smoothturnsfactor", {&demo_smoothturnsfactor},  {6},1,SMOOTH_PLAYING_MAXFACTOR,
   def_int,ss_stat},

  {"Files",{NULL},{0},UL,UL,def_none,ss_none},
  /* cph - MBF-like wad/deh/bex autoload code */
  {"wadfile_1",{NULL,&wad_files[1]},{0,""},UL,UL,def_str,ss_none},
  {"wadfile_2",{NULL,&wad_files[2]},{0,""},UL,UL,def_str,ss_none},
  {"dehfile_1",{NULL,&deh_files[0]},{0,""},UL,UL,def_str,ss_none},
  {"dehfile_2",{NULL,&deh_files[1]},{0,""},UL,UL,def_str,ss_none},

  {"Game settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"default_skill",{&defaultskill},{4},1,5, // jff 3/24/98 allow default skill setting
   def_int,ss_none}, // selects default skill 1=TYTD 2=NTR 3=HMP 4=UV 5=NM
  {"weapon_attack_alignment",{&weapon_attack_alignment},{0},0,3,         // phares 2/25/98
   def_int,ss_weap, &weapon_attack_alignment},

  {"sts_always_red",{&sts_always_red},{1},0,1, // no color changes on status bar
   def_bool,ss_stat},
  {"sts_pct_always_gray",{&sts_pct_always_gray},{0},0,1, // 2/23/98 chg default
   def_bool,ss_stat}, // makes percent signs on status bar always gray
  {"sts_traditional_keys",{&sts_traditional_keys},{0},0,1,  // killough 2/28/98
   def_bool,ss_stat}, // disables doubled card and skull key display on status bar
  {"sts_armorcolor_type",{&sts_armorcolor_type},{1},0,1, //  armor color depends on type
   def_bool,ss_stat},
  {"show_messages",{(int *)&dsda_setting[dsda_show_messages]},{1},0,1,
   def_bool,ss_none}, // enables message display
  {"autorun",{(int *)&dsda_setting[dsda_autorun]},{1},0,1,  // killough 3/6/98: preserve autorun across games
   def_bool,ss_none},

  {"Dehacked settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"deh_apply_cheats",{&deh_apply_cheats},{1},0,1,
   def_bool,ss_stat}, // if 0, dehacked cheat replacements are ignored.

  {"Sound settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"snd_pcspeaker",{&snd_pcspeaker},{0}, 0, 1, def_bool,ss_none},
  {"sound_card",{&snd_card},{-1},-1,7,       // jff 1/18/98 allow Allegro drivers
   def_int,ss_none}, // select sounds driver (DOS), -1 is autodetect, 0 is none; in Linux, non-zero enables sound
  {"music_card",{&mus_card},{-1},-1,9,       //  to be set,  -1 = autodetect
   def_int,ss_none}, // select music driver (DOS), -1 is autodetect, 0 is none"; in Linux, non-zero enables music
  {"pitched_sounds",{&pitched_sounds},{0},0,1, // killough 2/21/98
   def_bool,ss_none}, // enables variable pitch in sound effects (from id's original code)
  {"samplerate",{&snd_samplerate},{44100},11025,48000, def_int,ss_none},
  {"slice_samplecount",{&snd_samplecount},{512},32,8192, def_int,ss_none},
  {"sfx_volume",{&snd_SfxVolume},{8},0,15, def_int,ss_none},
  {"music_volume",{&snd_MusicVolume},{8},0,15, def_int,ss_none},
  {"mus_pause_opt",{&mus_pause_opt},{1},0,2, // CPhipps - music pausing
   def_int, ss_none}, // 0 = kill music when paused, 1 = pause music, 2 = let music continue
  {"snd_channels",{&default_numChannels},{32},1,32,
   def_int,ss_none}, // number of audio events simultaneously // killough
  {"snd_midiplayer",{NULL, &snd_midiplayer},{0,"fluidsynth"},UL,UL,def_str,ss_none},
  {"snd_soundfont",{NULL, &snd_soundfont},{0,"soundfonts/dsda-doom.sf2"},UL,UL,def_str,ss_none},
  {"snd_mididev",{NULL, &snd_mididev},{0,""},UL,UL,def_str,ss_none}, // midi device to use for portmidiplayer
  {"full_sounds",{&full_sounds},{0},0,1,def_bool,ss_none}, // disable sound cutoffs

#ifdef _WIN32
  {"mus_extend_volume",{&mus_extend_volume},{0},0,1,
   def_bool,ss_none}, // e6y: apply midi volume to all midi devices
#endif
  {"mus_fluidsynth_chorus",{&mus_fluidsynth_chorus},{0},0,1,def_bool,ss_none},
  {"mus_fluidsynth_reverb",{&mus_fluidsynth_reverb},{0},0,1,def_bool,ss_none},
  {"mus_fluidsynth_gain",{&mus_fluidsynth_gain},{50},0,1000,def_int,ss_none}, // NSM  fine tune fluidsynth output level
  {"mus_opl_gain",{&mus_opl_gain},{50},0,1000,def_int,ss_none}, // NSM  fine tune opl output level

  {"Video settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"videomode",{NULL, &default_videomode},{0,"Software"},UL,UL,def_str,ss_none},
  /* 640x480 default resolution */
  {"screen_resolution",{NULL, &screen_resolution},{0,"640x480"},UL,UL,def_str,ss_none},
  {"custom_resolution",{0,&custom_resolution},{0,""},UL,UL,def_str,ss_chat},
  {"use_fullscreen",{&use_fullscreen},{0},0,1, /* proff 21/05/2000 */
   def_bool,ss_none},
  {"exclusive_fullscreen",{&exclusive_fullscreen},{0},0,1, // [FG] mode-changing fullscreen
  def_bool,ss_none},
  {"gl_exclusive_fullscreen",{&gl_exclusive_fullscreen},{1},0,1,def_bool,ss_none},
  {"render_vsync",{&render_vsync},{1},0,1,
   def_bool,ss_none},
  {"tran_filter_pct",{&tran_filter_pct},{66},0,100,         // killough 2/21/98
   def_int,ss_none}, // set percentage of foreground/background translucency mix
  {"screenblocks",{&screenblocks},{10},3,11,  // killough 2/21/98: default to 10
   def_int,ss_none},
  {"usegamma",{&usegamma},{0},0,4, //jff 3/6/98 fix erroneous upper limit in range
   def_int,ss_none}, // gamma correction level // killough 1/18/98
  {"uncapped_framerate", {&movement_smooth_default},  {1},0,1,
   def_bool,ss_stat},
  {"dsda_fps_limit", {&dsda_fps_limit}, {0}, 0, 1000, def_int, ss_stat},
  {"filter_wall",{(int*)&drawvars.filterwall},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int,ss_none},
  {"filter_floor",{(int*)&drawvars.filterfloor},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int,ss_none},
  {"filter_sprite",{(int*)&drawvars.filtersprite},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int,ss_none},
  {"filter_z",{(int*)&drawvars.filterz},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_LINEAR, def_int,ss_none},
  {"filter_patch",{(int*)&drawvars.filterpatch},{RDRAW_FILTER_POINT},
   RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int,ss_none},
  {"filter_threshold",{(int*)&drawvars.mag_threshold},{49152},
   0, UL, def_int,ss_none},
  {"sprite_edges",{(int*)&drawvars.sprite_edges},{RDRAW_MASKEDCOLUMNEDGE_SQUARE},
   RDRAW_MASKEDCOLUMNEDGE_SQUARE, RDRAW_MASKEDCOLUMNEDGE_SLOPED, def_int,ss_none},
  {"patch_edges",{(int*)&drawvars.patch_edges},{RDRAW_MASKEDCOLUMNEDGE_SQUARE},
   RDRAW_MASKEDCOLUMNEDGE_SQUARE, RDRAW_MASKEDCOLUMNEDGE_SLOPED, def_int,ss_none},

  {"OpenGL settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"gl_arb_multitexture", {&gl_arb_multitexture_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_arb_texture_compression", {&gl_arb_texture_compression_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_arb_texture_non_power_of_two", {&gl_arb_texture_non_power_of_two_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_ext_arb_vertex_buffer_object", {&gl_ext_arb_vertex_buffer_object_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_arb_pixel_buffer_object", {&gl_arb_pixel_buffer_object_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_arb_shader_objects", {&gl_arb_shader_objects_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_ext_blend_color", {&gl_ext_blend_color_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_ext_framebuffer_object", {&gl_ext_framebuffer_object_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_ext_packed_depth_stencil", {&gl_ext_packed_depth_stencil_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_ext_texture_filter_anisotropic", {&gl_ext_texture_filter_anisotropic_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_use_stencil", {&gl_use_stencil_default}, {1},0,1,
   def_bool,ss_stat},
  {"gl_use_display_lists",{&gl_use_display_lists},{0},0,1,
   def_bool,ss_none},

  {"gl_finish",{&gl_finish},{1},0,1,
   def_bool,ss_none},
  {"gl_clear",{&gl_clear},{0},0,1,
   def_bool,ss_none},
  {"gl_ztrick",{&gl_ztrick},{0},0,1,
   def_bool,ss_none},
  {"gl_nearclip",{&gl_nearclip},{5},0,UL,
   def_int,ss_none}, /* near clipping plane pos */
  {"gl_colorbuffer_bits",{&gl_colorbuffer_bits},{32},16,32,
   def_int,ss_none},
  {"gl_depthbuffer_bits",{&gl_depthbuffer_bits},{24},16,32,
   def_int,ss_none},
  {"gl_texture_filter",{(int*)&gl_texture_filter},
   {filter_nearest_mipmap_linear}, filter_nearest, filter_count - 1, def_int,ss_none},
  {"gl_sprite_filter",{(int*)&gl_sprite_filter},
   {filter_nearest}, filter_nearest, filter_linear_mipmap_nearest, def_int,ss_none},
  {"gl_patch_filter",{(int*)&gl_patch_filter},
   {filter_nearest}, filter_nearest, filter_linear, def_int,ss_none},
  {"gl_texture_filter_anisotropic",{(int*)&gl_texture_filter_anisotropic},
   {gl_anisotropic_8x}, gl_anisotropic_off, gl_anisotropic_16x, def_int,ss_none},
  {"gl_tex_format_string", {NULL,&gl_tex_format_string}, {0,"GL_RGBA"},UL,UL,
   def_str,ss_none},
  {"gl_sprite_offset",{&gl_sprite_offset_default},{0}, 0, 5,
   def_int,ss_none}, // amount to bring items out of floor (GL) Mead 8/13/03
  {"gl_sprite_blend",{&gl_sprite_blend},{0},0,1,
   def_bool,ss_none},
  {"gl_mask_sprite_threshold",{&gl_mask_sprite_threshold},{50},0,100,
   def_int,ss_none},
  {"gl_skymode",{(int*)&gl_skymode},
  {skytype_auto}, skytype_auto, skytype_count - 1, def_int,ss_none},
  {"gl_sky_detail",{&gl_sky_detail},{16},1,32,
   def_int,ss_none},
  {"gl_use_paletted_texture",{&gl_use_paletted_texture},{0},0,1,
   def_bool,ss_none},
  {"gl_use_shared_texture_palette",{&gl_use_shared_texture_palette},{0},0,1,
   def_bool,ss_none},

  // defaults { key, mouseb, joyb }
  { "Input settings", { NULL }, { 0 }, UL, UL, def_none, ss_none },

  { "input_profile", { &dsda_input_profile }, { 0 }, 0, DSDA_INPUT_PROFILE_COUNT - 1, def_int, ss_none },

  { "input_forward", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_forward, { 'w', 2, -1 } },
  { "input_backward", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_backward, { 's', -1, -1 } },
  { "input_turnleft", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_turnleft, { 'e', -1, -1 } },
  { "input_turnright", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_turnright, { 'q', -1, -1 } },
  { "input_speed", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_speed, { KEYD_RSHIFT, -1, 2 } },
  { "input_strafeleft", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_strafeleft, { 'a', -1, 4 } },
  { "input_straferight", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_straferight, { 'd', -1, 5 } },
  { "input_strafe", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_strafe, { KEYD_RALT, 1, 1 } },
  { "input_autorun", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_autorun, { KEYD_CAPSLOCK, -1, -1 } },
  { "input_reverse", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_reverse, { '/', -1, -1 } },
  { "input_use", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_use, { ' ', -1, 3 } },
  { "input_flyup", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_flyup, { '.', -1, -1 } },
  { "input_flydown", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_flydown, { ',', -1, -1 } },
  { "input_flycenter", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_flycenter, { 0, -1, -1 } },
  { "input_mlook", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_mlook, { '\\', -1, -1 } },
  { "input_novert", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_novert, { 0, -1, -1 } },

  { "input_weapon1", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_weapon1, { '1', -1, -1 } },
  { "input_weapon2", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_weapon2, { '2', -1, -1 } },
  { "input_weapon3", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_weapon3, { '3', -1, -1 } },
  { "input_weapon4", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_weapon4, { '4', -1, -1 } },
  { "input_weapon5", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_weapon5, { '5', -1, -1 } },
  { "input_weapon6", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_weapon6, { '6', -1, -1 } },
  { "input_weapon7", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_weapon7, { '7', -1, -1 } },
  { "input_weapon8", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_weapon8, { '8', -1, -1 } },
  { "input_weapon9", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_weapon9, { '9', -1, -1 } },
  { "input_nextweapon", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_nextweapon, { KEYD_MWHEELUP, -1, -1 } },
  { "input_prevweapon", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_prevweapon, { KEYD_MWHEELDOWN, -1, -1 } },
  { "input_toggleweapon", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_toggleweapon, { '0', -1, -1 } },
  { "input_fire", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_fire, { KEYD_RCTRL, 0, 0 } },

  { "input_setup", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_setup, { 0, -1, -1 } },
  { "input_pause", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_pause, { KEYD_PAUSE, -1, -1 } },
  { "input_map", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map, { KEYD_TAB, -1, -1 } },
  { "input_soundvolume", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_soundvolume, { KEYD_F4, -1, -1 } },
  { "input_hud", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_hud, { KEYD_F5, -1, -1 } },
  { "input_messages", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_messages, { KEYD_F8, -1, -1 } },
  { "input_gamma", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_gamma, { KEYD_F11, -1, -1 } },
  { "input_spy", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_spy, { KEYD_F12, -1, -1 } },
  { "input_zoomin", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_zoomin, { '=', -1, -1 } },
  { "input_zoomout", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_zoomout, { '-', -1, -1 } },
  { "input_screenshot", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_screenshot, { '*', -1, -1 } },
  { "input_savegame", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_savegame, { KEYD_F2, -1, -1 } },
  { "input_loadgame", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_loadgame, { KEYD_F3, -1, -1 } },
  { "input_quicksave", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_quicksave, { KEYD_F6, -1, -1 } },
  { "input_quickload", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_quickload, { KEYD_F9, -1, -1 } },
  { "input_endgame", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_endgame, { KEYD_F7, -1, -1 } },
  { "input_quit", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_quit, { KEYD_F10, -1, -1 } },

  { "input_map_follow", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_follow, { 'f', -1, -1 } },
  { "input_map_zoomin", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_zoomin, { '=', -1, -1 } },
  { "input_map_zoomout", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_zoomout, { '-', -1, -1 } },
  { "input_map_up", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_up, { KEYD_UPARROW, -1, -1 } },
  { "input_map_down", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_down, { KEYD_DOWNARROW, -1, -1 } },
  { "input_map_left", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_left, { KEYD_LEFTARROW, -1, -1 } },
  { "input_map_right", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_right, { KEYD_RIGHTARROW, -1, -1 } },
  { "input_map_mark", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_mark, { 'm', -1, -1 } },
  { "input_map_clear", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_clear, { 'c', -1, -1 } },
  { "input_map_gobig", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_gobig, { '0', -1, -1 } },
  { "input_map_grid", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_grid, { 'g', -1, -1 } },
  { "input_map_rotate", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_rotate, { 'r', -1, -1 } },
  { "input_map_overlay", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_overlay, { 'o', -1, -1 } },
  { "input_map_textured", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_map_textured, { 0, -1, -1 } },

  { "input_chat", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_chat, { 't', -1, -1 } },
  { "input_chat_dest0", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_chat_dest0, { 'g', -1, -1 } },
  { "input_chat_dest1", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_chat_dest1, { 'i', -1, -1 } },
  { "input_chat_dest2", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_chat_dest2, { 'b', -1, -1 } },
  { "input_chat_dest3", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_chat_dest3, { 'r', -1, -1 } },
  { "input_chat_backspace", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_chat_backspace, { KEYD_BACKSPACE, -1, -1 } },
  { "input_chat_enter", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_chat_enter, { KEYD_ENTER, -1, -1 } },

  { "input_speed_up", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_speed_up, { 0, -1, -1 } },
  { "input_speed_down", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_speed_down, { 0, -1, -1 } },
  { "input_speed_default", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_speed_default, { 0, -1, -1 } },
  { "input_demo_skip", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_demo_skip, { KEYD_INSERT, -1, -1 } },
  { "input_demo_endlevel", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_demo_endlevel, { KEYD_END, -1, -1 } },
  { "input_walkcamera", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_walkcamera, { KEYD_KEYPAD0, -1, -1 } },
  { "input_join_demo", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_join_demo, { 0, -1, -1 } },
  { "input_restart", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_restart, { KEYD_HOME, -1, -1 } },
  { "input_nextlevel", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_nextlevel, { KEYD_PAGEDOWN, -1, -1 } },
  { "input_showalive", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_showalive, { KEYD_KEYPADDIVIDE, -1, -1 } },

  { "input_menu_down", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_menu_down, { KEYD_DOWNARROW, -1, -1 } },
  { "input_menu_up", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_menu_up, { KEYD_UPARROW, -1, -1 } },
  { "input_menu_left", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_menu_left, { KEYD_LEFTARROW, -1, -1 } },
  { "input_menu_right", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_menu_right, { KEYD_RIGHTARROW, -1, -1 } },
  { "input_menu_backspace", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_menu_backspace, { KEYD_BACKSPACE, -1, -1 } },
  { "input_menu_enter", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_menu_enter, { KEYD_ENTER, -1, -1 } },
  { "input_menu_escape", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_menu_escape, { KEYD_ESCAPE, -1, -1 } },
  { "input_menu_clear", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_menu_clear, { KEYD_DEL, -1, -1 } },

  { "input_iddqd", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_iddqd, { 0, -1, -1 } },
  { "input_idkfa", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idkfa, { 0, -1, -1 } },
  { "input_idfa", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idfa, { 0, -1, -1 } },
  { "input_idclip", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idclip, { 0, -1, -1 } },
  { "input_idbeholdh", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idbeholdh, { 0, -1, -1 } },
  { "input_idbeholdm", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idbeholdm, { 0, -1, -1 } },
  { "input_idbeholdv", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idbeholdv, { 0, -1, -1 } },
  { "input_idbeholds", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idbeholds, { 0, -1, -1 } },
  { "input_idbeholdi", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idbeholdi, { 0, -1, -1 } },
  { "input_idbeholdr", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idbeholdr, { 0, -1, -1 } },
  { "input_idbeholda", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idbeholda, { 0, -1, -1 } },
  { "input_idbeholdl", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idbeholdl, { 0, -1, -1 } },
  { "input_idmypos", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idmypos, { 0, -1, -1 } },
  { "input_idrate", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_idrate, { 0, -1, -1 } },
  { "input_iddt", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_iddt, { 0, -1, -1 } },
  { "input_ponce", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_ponce, { 0, -1, -1 } },
  { "input_shazam", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_shazam, { 0, -1, -1 } },
  { "input_chicken", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_chicken, { 0, -1, -1 } },

  { "input_lookup", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_lookup, { 0, -1, -1 } },
  { "input_lookdown", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_lookdown, { 0, -1, -1 } },
  { "input_lookcenter", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_lookcenter, { 0, -1, -1 } },
  { "input_use_artifact", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_use_artifact, { 0, -1, -1 } },
  { "input_arti_tome", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_tome, { 0, -1, -1 } },
  { "input_arti_quartz", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_quartz, { 0, -1, -1 } },
  { "input_arti_urn", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_urn, { 0, -1, -1 } },
  { "input_arti_bomb", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_bomb, { 0, -1, -1 } },
  { "input_arti_ring", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_ring, { 0, -1, -1 } },
  { "input_arti_chaosdevice", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_chaosdevice, { 0, -1, -1 } },
  { "input_arti_shadowsphere", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_shadowsphere, { 0, -1, -1 } },
  { "input_arti_wings", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_wings, { 0, -1, -1 } },
  { "input_arti_torch", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_torch, { 0, -1, -1 } },
  { "input_arti_morph", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_arti_morph, { 0, -1, -1 } },
  { "input_invleft", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_invleft, { 0, -1, -1 } },
  { "input_invright", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_invright, { 0, -1, -1 } },
  { "input_store_quick_key_frame", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_store_quick_key_frame, { 0, -1, -1 } },
  { "input_restore_quick_key_frame", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_restore_quick_key_frame, { 0, -1, -1 } },
  { "input_rewind", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_rewind, { 0, -1, -1 } },
  { "input_cycle_profile", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_cycle_profile, { 0, -1, -1 } },
  { "input_cycle_palette", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_cycle_palette, { 0, -1, -1 } },
  { "input_command_display", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_command_display, { 0, -1, -1 } },
  { "input_strict_mode", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_strict_mode, { 0, -1, -1 } },
  { "input_console", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_console, { 0, -1, -1 } },
  { "input_coordinate_display", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_coordinate_display, { 0, -1, -1 } },
  { "input_avj", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_avj, { 0, -1, -1 } },
  { "input_exhud", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_exhud, { 0, -1, -1 } },
  { "input_mute_sfx", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_mute_sfx, { 0, -1, -1 } },
  { "input_mute_music", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_mute_music, { 0, -1, -1 } },
  { "input_cheat_codes", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_cheat_codes, { 0, -1, -1 } },
  { "input_notarget", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_notarget, { 0, -1, -1 } },

  { "input_jump", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_jump, { 0, -1, -1 } },
  { "input_hexen_arti_incant", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_hexen_arti_incant, { 0, -1, -1 } },
  { "input_hexen_arti_summon", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_hexen_arti_summon, { 0, -1, -1 } },
  { "input_hexen_arti_disk", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_hexen_arti_disk, { 0, -1, -1 } },
  { "input_hexen_arti_flechette", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_hexen_arti_flechette, { 0, -1, -1 } },
  { "input_hexen_arti_banishment", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_hexen_arti_banishment, { 0, -1, -1 } },
  { "input_hexen_arti_boots", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_hexen_arti_boots, { 0, -1, -1 } },
  { "input_hexen_arti_krater", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_hexen_arti_krater, { 0, -1, -1 } },
  { "input_hexen_arti_bracers", { NULL }, { 0 }, UL, UL, def_input, ss_keys, NULL, NULL,
    dsda_input_hexen_arti_bracers, { 0, -1, -1 } },

  {"Mouse settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"use_mouse",{&usemouse},{1},0,1,
   def_bool,ss_none}, // enables use of mouse with DOOM
  //jff 4/3/98 allow unlimited sensitivity
  {"mouse_sensitivity_horiz",{&mouseSensitivity_horiz},{10},0,UL,
   def_int,ss_none}, /* adjust horizontal (x) mouse sensitivity killough/mead */
  //jff 4/3/98 allow unlimited sensitivity
  {"mouse_sensitivity_vert",{&mouseSensitivity_vert},{1},0,UL,
   def_int,ss_none}, /* adjust vertical (y) mouse sensitivity killough/mead */

  {"Joystick settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"use_joystick",{&usejoystick},{0},0,2,
   def_int,ss_none}, // number of joystick to use (0 for none)

  {"Chat macros",{NULL},{0},UL,UL,def_none,ss_none},
  {"chatmacro0", {0,&chat_macros[0]}, {0,HUSTR_CHATMACRO0},UL,UL,
   def_str,ss_chat}, // chat string associated with 0 key
  {"chatmacro1", {0,&chat_macros[1]}, {0,HUSTR_CHATMACRO1},UL,UL,
   def_str,ss_chat}, // chat string associated with 1 key
  {"chatmacro2", {0,&chat_macros[2]}, {0,HUSTR_CHATMACRO2},UL,UL,
   def_str,ss_chat}, // chat string associated with 2 key
  {"chatmacro3", {0,&chat_macros[3]}, {0,HUSTR_CHATMACRO3},UL,UL,
   def_str,ss_chat}, // chat string associated with 3 key
  {"chatmacro4", {0,&chat_macros[4]}, {0,HUSTR_CHATMACRO4},UL,UL,
   def_str,ss_chat}, // chat string associated with 4 key
  {"chatmacro5", {0,&chat_macros[5]}, {0,HUSTR_CHATMACRO5},UL,UL,
   def_str,ss_chat}, // chat string associated with 5 key
  {"chatmacro6", {0,&chat_macros[6]}, {0,HUSTR_CHATMACRO6},UL,UL,
   def_str,ss_chat}, // chat string associated with 6 key
  {"chatmacro7", {0,&chat_macros[7]}, {0,HUSTR_CHATMACRO7},UL,UL,
   def_str,ss_chat}, // chat string associated with 7 key
  {"chatmacro8", {0,&chat_macros[8]}, {0,HUSTR_CHATMACRO8},UL,UL,
   def_str,ss_chat}, // chat string associated with 8 key
  {"chatmacro9", {0,&chat_macros[9]}, {0,HUSTR_CHATMACRO9},UL,UL,
   def_str,ss_chat}, // chat string associated with 9 key

  {"Automap settings",{NULL},{0},UL,UL,def_none,ss_none},
  //jff 1/7/98 defaults for automap colors
  //jff 4/3/98 remove -1 in lower range, 0 now disables new map features
  {"mapcolor_back", {&mapcolor_back}, {247},0,255,  // black //jff 4/6/98 new black
   def_colour,ss_auto}, // color used as background for automap
  {"mapcolor_grid", {&mapcolor_grid}, {104},0,255,  // dk gray
   def_colour,ss_auto}, // color used for automap grid lines
  {"mapcolor_wall", {&mapcolor_wall}, {23},0,255,   // red-brown
   def_colour,ss_auto}, // color used for one side walls on automap
  {"mapcolor_fchg", {&mapcolor_fchg}, {55},0,255,   // lt brown
   def_colour,ss_auto}, // color used for lines floor height changes across
  {"mapcolor_cchg", {&mapcolor_cchg}, {215},0,255,  // orange
   def_colour,ss_auto}, // color used for lines ceiling height changes across
  {"mapcolor_clsd", {&mapcolor_clsd}, {208},0,255,  // white
   def_colour,ss_auto}, // color used for lines denoting closed doors, objects
  {"mapcolor_rkey", {&mapcolor_rkey}, {175},0,255,  // red
   def_colour,ss_auto}, // color used for red key sprites
  {"mapcolor_bkey", {&mapcolor_bkey}, {204},0,255,  // blue
   def_colour,ss_auto}, // color used for blue key sprites
  {"mapcolor_ykey", {&mapcolor_ykey}, {231},0,255,  // yellow
   def_colour,ss_auto}, // color used for yellow key sprites
  {"mapcolor_rdor", {&mapcolor_rdor}, {175},0,255,  // red
   def_colour,ss_auto}, // color used for closed red doors
  {"mapcolor_bdor", {&mapcolor_bdor}, {204},0,255,  // blue
   def_colour,ss_auto}, // color used for closed blue doors
  {"mapcolor_ydor", {&mapcolor_ydor}, {231},0,255,  // yellow
   def_colour,ss_auto}, // color used for closed yellow doors
  {"mapcolor_tele", {&mapcolor_tele}, {119},0,255,  // dk green
   def_colour,ss_auto}, // color used for teleporter lines
  {"mapcolor_secr", {&mapcolor_secr}, {252},0,255,  // purple
   def_colour,ss_auto}, // color used for lines around secret sectors
  {"mapcolor_revsecr", {&mapcolor_revsecr}, {112},0,255,  // green
   def_colour,ss_auto}, // color used for lines around revealed secrets
  {"mapcolor_exit", {&mapcolor_exit}, {0},0,255,    // none
   def_colour,ss_auto}, // color used for exit lines
  {"mapcolor_unsn", {&mapcolor_unsn}, {104},0,255,  // dk gray
   def_colour,ss_auto}, // color used for lines not seen without computer map
  {"mapcolor_flat", {&mapcolor_flat}, {88},0,255,   // lt gray
   def_colour,ss_auto}, // color used for lines with no height changes
  {"mapcolor_sprt", {&mapcolor_sprt}, {112},0,255,  // green
   def_colour,ss_auto}, // color used as things
  {"mapcolor_item", {&mapcolor_item}, {231},0,255,  // yellow
   def_colour,ss_auto}, // color used for counted items
  {"mapcolor_hair", {&mapcolor_hair}, {208},0,255,  // white
   def_colour,ss_auto}, // color used for dot crosshair denoting center of map
  {"mapcolor_sngl", {&mapcolor_sngl}, {208},0,255,  // white
   def_colour,ss_auto}, // color used for the single player arrow
  {"mapcolor_me",   {&mapcolor_me}, {112},0,255, // green
   def_colour,ss_auto}, // your (player) colour
  {"mapcolor_enemy",   {&mapcolor_enemy}, {177},0,255,
   def_colour,ss_auto},
  {"mapcolor_frnd",   {&mapcolor_frnd}, {112},0,255,
   def_colour,ss_auto},
  //jff 3/9/98 add option to not show secrets til after found
  {"map_secret_after", {&map_secret_after}, {0},0,1, // show secret after gotten
   def_bool,ss_auto}, // prevents showing secret sectors till after entered
  {"map_point_coord", {&map_point_coordinates}, {0},0,1,
   def_bool,ss_auto},
  {"map_level_stat", {&map_level_stat}, {1},0,1,
   def_bool,ss_auto},
  //jff 1/7/98 end additions for automap
  {"automapmode", {(int*)&automapmode}, {am_follow}, 0, 31, // CPhipps - remember automap mode
   def_hex,ss_none}, // automap mode
  {"map_always_updates", {&map_always_updates}, {1},0,1,
   def_bool,ss_auto},
  {"map_grid_size", {&map_grid_size}, {128},8,256,
   def_int,ss_auto},
  {"map_scroll_speed", {&map_scroll_speed}, {8},1,32,
   def_int,ss_auto},
  {"map_wheel_zoom", {&map_wheel_zoom}, {1},0,1,
   def_bool,ss_auto},
  {"map_use_multisamling", {&map_use_multisamling}, {0},0,1,
   def_bool,ss_auto},
  {"map_textured", {&map_textured}, {1},0,1,
   def_bool,ss_auto},
  {"map_textured_trans", {&map_textured_trans}, {100},0,100,
   def_int,ss_auto},
  {"map_textured_overlay_trans", {&map_textured_overlay_trans}, {66},0,100,
   def_int,ss_auto},
  {"map_lines_overlay_trans", {&map_lines_overlay_trans}, {100},0,100,
   def_int,ss_auto},
  {"map_overlay_pos_x", {&map_overlay_pos_x}, {0},0,319,
   def_int,ss_auto},
  {"map_overlay_pos_y", {&map_overlay_pos_y}, {0},0,199,
   def_int,ss_auto},
  {"map_overlay_pos_width", {&map_overlay_pos_width}, {320},0,320,
   def_int,ss_auto},
  {"map_overlay_pos_height", {&map_overlay_pos_height}, {200},0,200,
   def_int,ss_auto},
  {"map_things_appearance", {(int*)&map_things_appearance}, {map_things_appearance_max-1},0,map_things_appearance_max-1,
   def_int,ss_auto},

  {"Heads-up display settings",{NULL},{0},UL,UL,def_none,ss_none},
  //jff 2/16/98 defaults for color ranges in hud and status
  {"hudcolor_titl", {&hudcolor_titl}, {5},0,9,  // gold range
   def_int,ss_auto}, // color range used for automap level title
  {"hudcolor_xyco", {&hudcolor_xyco}, {3},0,9,  // green range
   def_int,ss_auto}, // color range used for automap coordinates
   {"hudcolor_mapstat_title", {&hudcolor_mapstat_title}, {6},0,9, // red range
   def_int,ss_auto}, // color range used for automap statistics for titles
  {"hudcolor_mapstat_value", {&hudcolor_mapstat_value}, {2},0,9,    // gray range
   def_int,ss_auto}, // color range used for automap statistics for data
  {"hudcolor_mapstat_time", {&hudcolor_mapstat_time}, {2},0,9,    // gray range
   def_int,ss_auto}, // color range used for automap statistics for level time and total time
  {"hudcolor_mesg", {&hudcolor_mesg}, {6},0,9,  // red range
   def_int,ss_mess}, // color range used for messages during play
  {"hudcolor_chat", {&hudcolor_chat}, {5},0,9,  // gold range
   def_int,ss_mess}, // color range used for chat messages and entry
  {"hudcolor_list", {&hudcolor_list}, {5},0,9,  // gold range  //jff 2/26/98
   def_int,ss_mess}, // color range used for message review
  {"hud_msg_lines", {&hud_msg_lines}, {1},1,16,  // 1 line scrolling window
   def_int,ss_mess}, // number of messages in review display (1=disable)
  {"hud_list_bgon", {&hud_list_bgon}, {0},0,1,  // solid window bg ena //jff 2/26/98
   def_bool,ss_mess}, // enables background window behind message review

  {"health_red",    {&health_red}   , {25},0,200, // below is red
   def_int,ss_stat}, // amount of health for red to yellow transition
  {"health_yellow", {&health_yellow}, {50},0,200, // below is yellow
   def_int,ss_stat}, // amount of health for yellow to green transition
  {"health_green",  {&health_green} , {100},0,200,// below is green, above blue
   def_int,ss_stat}, // amount of health for green to blue transition
  {"armor_red",     {&armor_red}    , {25},0,200, // below is red
   def_int,ss_stat}, // amount of armor for red to yellow transition
  {"armor_yellow",  {&armor_yellow} , {50},0,200, // below is yellow
   def_int,ss_stat}, // amount of armor for yellow to green transition
  {"armor_green",   {&armor_green}  , {100},0,200,// below is green, above blue
   def_int,ss_stat}, // amount of armor for green to blue transition
  {"ammo_red",      {&ammo_red}     , {25},0,100, // below 25% is red
   def_int,ss_stat}, // percent of ammo for red to yellow transition
  {"ammo_yellow",   {&ammo_yellow}  , {50},0,100, // below 50% is yellow, above green
   def_int,ss_stat}, // percent of ammo for yellow to green transition
  {"ammo_colour_behaviour",{(int*)&ammo_colour_behaviour},
   {ammo_colour_behaviour_max-1}, // whether backpack changes thresholds above
   0,ammo_colour_behaviour_max-1,def_int,ss_stat},

  //jff 2/16/98 HUD and status feature controls
  {"hud_num",    {&hud_num}, {6},0,100,
   def_int,ss_none},
  //jff 2/23/98
  {"hud_displayed", {&hud_displayed},  {0},0,1, // whether hud is displayed
   def_bool,ss_none}, // enables display of HUD

  { "Prboom-plus heads-up display settings", { NULL }, { 0 }, UL, UL, def_none, ss_none },
    { "hudadd_gamespeed", { &hudadd_gamespeed }, { 0 }, 0, 1, def_bool, ss_stat },
  { "hudadd_leveltime", { &hudadd_leveltime }, { 0 }, 0, 1, def_bool, ss_stat },
  { "hudadd_demotime", { &hudadd_demotime }, { 0 }, 0, 1, def_bool, ss_stat },
  { "hudadd_secretarea", { &hudadd_secretarea }, { 0 }, 0, 1, def_bool, ss_stat },
  { "hudadd_maxtotals", { &hudadd_maxtotals }, { 0 }, 0, 1, def_bool, ss_stat },
  { "hudadd_demoprogressbar", { &hudadd_demoprogressbar }, { 1 }, 0, 1, def_bool, ss_stat },
  { "hudadd_crosshair", { &hudadd_crosshair }, { 0 }, 0, HU_CROSSHAIRS - 1, def_bool, ss_stat },
  { "hudadd_crosshair_scale", { &hudadd_crosshair_scale }, { 0 }, 0, 1, def_bool, ss_stat },
  { "hudadd_crosshair_color", { &hudadd_crosshair_color }, { 3 }, 0, 9, def_int, ss_stat },
  { "hudadd_crosshair_health", { &hudadd_crosshair_health }, { 0 }, 0, 1, def_bool, ss_stat },
  { "hudadd_crosshair_target", { &hudadd_crosshair_target }, { 0 }, 0, 1, def_bool, ss_stat },
  { "hudadd_crosshair_target_color", { &hudadd_crosshair_target_color }, { 9 }, 0, 9, def_int, ss_stat },
  { "hudadd_crosshair_lock_target", { &hudadd_crosshair_lock_target }, { 0 }, 0, 1, def_bool, ss_stat },

  //e6y
  { "Prboom-plus mouse settings", { NULL }, { 0 }, UL, UL, def_none, ss_none },
  { "mouse_acceleration", { &mouse_acceleration }, { 0 }, 0, UL, def_int, ss_none },
  { "mouse_sensitivity_mlook", { &mouseSensitivity_mlook }, { 10 }, 0, UL, def_int, ss_none },
  { "mouse_doubleclick_as_use", { &mouse_doubleclick_as_use }, { 1 }, 0, 1, def_bool, ss_stat },
  { "mouse_carrytics", { &mouse_carrytics }, { 1 }, 0, 1, def_bool, ss_stat },

  { "Prboom-plus demos settings", { NULL }, { 0 }, UL, UL, def_none, ss_none },
  { "demo_demoex_filename", { NULL, &demo_demoex_filename }, { 0, "" }, UL, UL, def_str, ss_none },
  { "getwad_cmdline", { NULL, &getwad_cmdline }, { 0, "" }, UL, UL, def_str, ss_none },
  { "demo_overwriteexisting", { &demo_overwriteexisting }, { 0 }, 0, 1, def_bool, ss_stat },
  { "quickstart_window_ms", { &quickstart_window_ms }, { 0 }, 0, 1000, def_int, ss_stat },

  { "Prboom-plus game settings", { NULL }, { 0 }, UL, UL, def_none, ss_none },
  { "movement_strafe50", { &movement_strafe50 }, { 0 }, 0, 1, def_bool, ss_stat },
  { "movement_strafe50onturns", { &movement_strafe50onturns }, { 0 }, 0, 1, def_bool, ss_stat },
  { "movement_shorttics", { &movement_shorttics }, { 0 }, 0, 1, def_bool, ss_stat },
  { "interpolation_maxobjects", { &interpolation_maxobjects }, { 0 }, 0, UL, def_int, ss_stat },
  { "speed_step", { &speed_step }, { 0 }, 0, 1000, def_int, ss_none },

  { "Prboom-plus misc settings", { NULL }, { 0 }, UL, UL, def_none, ss_none },
  { "showendoom", { &showendoom }, { 0 }, 0, 1, def_bool, ss_stat },
  { "screenshot_dir", { NULL, &screenshot_dir }, { 0, "" }, UL, UL, def_str, ss_none },
  { "health_bar", { &health_bar }, { 0 }, 0, 1, def_bool, ss_stat },
  { "health_bar_full_length", { &health_bar_full_length }, { 1 }, 0, 1, def_bool, ss_stat },
  { "health_bar_red", { &health_bar_red }, { 50 }, 0, 100, def_int, ss_stat },
  { "health_bar_yellow", { &health_bar_yellow }, { 99 }, 0, 100, def_int, ss_stat },
  { "health_bar_green", { &health_bar_green }, { 0 }, 0, 100, def_int, ss_stat },

  { "DSDA-Doom settings", { NULL }, { 0 }, UL, UL, def_none, ss_none },
  { "dsda_strict_mode", { (int *) &dsda_setting[dsda_strict_mode] }, { 1 }, 0, 1, def_bool, ss_stat },
  { "dsda_cycle_ghost_colors", { &dsda_cycle_ghost_colors }, { 0 }, 0, 1, def_bool, ss_stat },
  { "dsda_auto_key_frame_interval", { &dsda_auto_key_frame_interval }, { 1 }, 1, 600, def_int, ss_stat },
  { "dsda_auto_key_frame_depth", { &dsda_auto_key_frame_depth }, { 60 }, 0, 600, def_int, ss_stat },
  { "dsda_auto_key_frame_timeout", { &dsda_auto_key_frame_timeout }, { 10 }, 0, 25, def_int, ss_stat },
  { "dsda_exhud", { (int *) &dsda_setting[dsda_exhud] }, { 0 }, 0, 1, def_bool, ss_stat },
  { "dsda_wipe_at_full_speed", { &dsda_wipe_at_full_speed }, { 1 }, 0, 1, def_bool, ss_stat },
  { "dsda_show_demo_attempts", { &dsda_show_demo_attempts }, { 1 }, 0, 1, def_bool, ss_stat },
  { "dsda_fine_sensitivity", { &dsda_fine_sensitivity }, { 0 }, 0, 99, def_int, ss_stat },
  { "dsda_hide_horns", { &dsda_hide_horns }, { 0 }, 0, 1, def_bool, ss_stat },
  { "dsda_organized_saves", { &dsda_organized_saves }, { 1 }, 0, 1, def_bool, ss_stat },
  { "dsda_command_display", { (int *) &dsda_setting[dsda_command_display] }, { 0 }, 0, 1, def_bool, ss_stat },
  { "dsda_command_history_size", { &dsda_command_history_size }, { 10 }, 1, 20, def_int, ss_stat },
  { "dsda_hide_empty_commands", { &dsda_hide_empty_commands }, { 1 }, 0, 1, def_bool, ss_stat },
  { "dsda_coordinate_display", { (int *) &dsda_setting[dsda_coordinate_display] }, { 0 }, 0, 1, def_bool, ss_stat },
  { "dsda_skip_quit_prompt", { &dsda_skip_quit_prompt }, { 0 }, 0, 1, def_bool, ss_stat },
  { "dsda_show_split_data", { &dsda_show_split_data }, { 1 }, 0, 1, def_bool, ss_stat },
  { "dsda_player_name", { 0, &dsda_player_name }, { 0, "Anonymous" }, UL, UL, def_str, ss_chat },
  { "dsda_quickstart_cache_tics", { &dsda_quickstart_cache_tics }, { 0 }, 0, 35, def_int, ss_stat },
  { "dsda_death_use_action", { &dsda_death_use_action }, { 0 }, 0, 2, def_int, ss_none },
  { "dsda_mute_sfx", { (int *) &dsda_setting[dsda_mute_sfx] }, { 0 }, 0, 1, def_bool, ss_stat },
  { "dsda_mute_music", { (int *) &dsda_setting[dsda_mute_music] }, { 0 }, 0, 1, def_bool, ss_stat },
  { "dsda_cheat_codes", { (int *) &dsda_setting[dsda_cheat_codes] }, { 1 }, 0, 1, def_bool, ss_stat },
  { "dsda_allow_jumping", { &dsda_allow_jumping }, { 0 }, 0, 1, def_bool, ss_stat },
  { "dsda_parallel_sfx_limit", { &dsda_parallel_sfx_limit }, { 0 }, 0, 32, def_int, ss_stat },
  { "dsda_parallel_sfx_window", { &dsda_parallel_sfx_window }, { 1 }, 1, 32, def_int, ss_stat },
  { "dsda_switch_when_ammo_runs_out", { &dsda_switch_when_ammo_runs_out }, { 1 }, 0, 1, def_bool, ss_stat },
  { "dsda_viewbob", { &dsda_viewbob }, { 1 }, 0, 1, def_bool, ss_stat },
  { "dsda_weaponbob", { &dsda_weaponbob }, { 1 }, 0, 1, def_bool, ss_stat },

  // NSM
  {"Video capture encoding settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"cap_soundcommand",{NULL, &cap_soundcommand},{0,"ffmpeg -f s16le -ar %s -ac 2 -i - -c:a libopus -y temp_a.nut"},UL,UL,def_str,ss_none},
  {"cap_videocommand",{NULL, &cap_videocommand},{0,"ffmpeg -f rawvideo -pix_fmt rgb24 -r %r -s %wx%h -i - -c:v libx264 -y temp_v.nut"},UL,UL,def_str,ss_none},
  {"cap_muxcommand",{NULL, &cap_muxcommand},{0,"ffmpeg -i temp_v.nut -i temp_a.nut -c copy -y %f"},UL,UL,def_str,ss_none},
  {"cap_tempfile1",{NULL, &cap_tempfile1},{0,"temp_a.nut"},UL,UL,def_str,ss_none},
  {"cap_tempfile2",{NULL, &cap_tempfile2},{0,"temp_v.nut"},UL,UL,def_str,ss_none},
  {"cap_remove_tempfiles", {&cap_remove_tempfiles},{1},0,1,def_bool,ss_none},
  {"cap_fps", {&cap_fps},{60},16,300,def_int,ss_none},

  {"Prboom-plus video settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"sdl_video_window_pos", {NULL,&sdl_video_window_pos}, {0,"center"},UL,UL,
   def_str,ss_none},
  {"palette_ondamage", {&palette_ondamage},  {1},0,1,
   def_bool,ss_stat},
  {"palette_onbonus", {&palette_onbonus},  {1},0,1,
   def_bool,ss_stat},
  {"palette_onpowers", {&palette_onpowers},  {1},0,1,
   def_bool,ss_stat},
  {"render_wipescreen", {&render_wipescreen},  {1},0,1,
   def_bool,ss_stat},
  {"render_screen_multiply", {&render_screen_multiply},  {1},1,5,
   def_int,ss_stat},
  {"integer_scaling", {&integer_scaling},  {0},0,1,
   def_bool,ss_stat},
  {"render_aspect", {&render_aspect},  {0},0,4,
   def_int,ss_stat},
  {"render_doom_lightmaps", {&render_doom_lightmaps},  {0},0,1,
   def_bool,ss_stat},
  {"fake_contrast", {&fake_contrast},  {1},0,1,
   def_bool,ss_stat}, /* cph - allow crappy fake contrast to be disabled */
  {"render_stretch_hud", {&render_stretch_hud_default},{patch_stretch_16x10},0,patch_stretch_max - 1,
  def_int,ss_stat},
  {"render_patches_scalex", {&render_patches_scalex},{0},0,16,
  def_int,ss_stat},
  {"render_patches_scaley", {&render_patches_scaley},{0},0,16,
  def_int,ss_stat},
  {"render_stretchsky",{&r_stretchsky},{1},0,1,
   def_bool,ss_none},
  {"sprites_doom_order", {&sprites_doom_order}, {DOOM_ORDER_STATIC},0,DOOM_ORDER_LAST - 1,
   def_int,ss_stat},

  {"movement_mouselook", {(int *)&dsda_setting[dsda_mouselook]},  {0},0,1,
   def_bool,ss_stat},
  {"movement_mousenovert", {(int *)&dsda_setting[dsda_novert]},  {0},0,1,
   def_bool,ss_stat},
  {"movement_maxviewpitch", {&movement_maxviewpitch},  {90},0,90,
   def_int,ss_stat},
   {"movement_mousestrafedivisor", {&movement_mousestrafedivisor},  {4},1,512,
    def_int,ss_stat},
  {"movement_mouseinvert", {&movement_mouseinvert},  {0},0,1,
   def_bool,ss_stat},

  {"Prboom-plus OpenGL settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"gl_allow_detail_textures", {&gl_allow_detail_textures},  {1},0,1,
   def_bool,ss_stat},
  {"gl_detail_maxdist", {&gl_detail_maxdist},  {0},0,65535,
   def_int,ss_stat},
  {"render_multisampling", {&render_multisampling},  {0},0,8,
   def_int,ss_stat},
  {"render_fov", {&render_fov},  {90},20,160,
   def_int,ss_stat},
  {"gl_spriteclip",{(int*)&gl_spriteclip},{spriteclip_smart}, spriteclip_const, spriteclip_smart, def_int,ss_none},
   {"gl_spriteclip_threshold", {&gl_spriteclip_threshold},  {10},0,100,
   def_int,ss_stat},
   {"gl_sprites_frustum_culling", {&gl_sprites_frustum_culling},  {1},0,1,
   def_bool,ss_stat},
  {"render_paperitems", {&render_paperitems},  {0},0,1,
   def_bool,ss_stat},
  {"gl_boom_colormaps", {&gl_boom_colormaps_default},  {1},0,1,
   def_bool,ss_stat},
  {"gl_hires_24bit_colormap", {&gl_hires_24bit_colormap},  {0},0,1,
   def_bool,ss_stat},
  {"gl_texture_internal_hires", {&gl_texture_internal_hires},  {1},0,1,
   def_bool,ss_stat},
  {"gl_texture_external_hires", {&gl_texture_external_hires},  {0},0,1,
   def_bool,ss_stat},
  {"gl_hires_override_pwads", {&gl_hires_override_pwads},  {0},0,1,
   def_bool,ss_stat},
  {"gl_texture_hires_dir", {NULL,&gl_texture_hires_dir}, {0,""},UL,UL,
   def_str,ss_none},
  {"gl_texture_hqresize", {&gl_texture_hqresize},  {0},0,1,
   def_bool,ss_stat},
  {"gl_texture_hqresize_textures", {&gl_texture_hqresize_textures},
   {hq_scale_2x},hq_scale_none,hq_scale_max-1, def_int,ss_stat},
  {"gl_texture_hqresize_sprites", {&gl_texture_hqresize_sprites},
   {hq_scale_none},hq_scale_none,hq_scale_max-1, def_int,ss_stat},
  {"gl_texture_hqresize_patches", {&gl_texture_hqresize_patches},
   {hq_scale_2x},hq_scale_none,hq_scale_max-1,def_int,ss_stat},
  {"gl_motionblur", {&gl_motionblur},  {0},0,1,
   def_bool,ss_stat},
  {"gl_motionblur_min_speed", {NULL,&motion_blur.str_min_speed}, {0,"21.36"},UL,UL,
   def_str,ss_none},
  {"gl_motionblur_min_angle", {NULL,&motion_blur.str_min_angle}, {0,"20.0"},UL,UL,
   def_str,ss_none},
  {"gl_motionblur_att_a", {NULL,&motion_blur.str_att_a}, {0,"55.0"},UL,UL,
   def_str,ss_none},
  {"gl_motionblur_att_b", {NULL,&motion_blur.str_att_b}, {0,"1.8"},UL,UL,
   def_str,ss_none},
  {"gl_motionblur_att_c", {NULL,&motion_blur.str_att_c}, {0,"0.9"},UL,UL,
   def_str,ss_none},
  {"gl_lightmode",{(int*)&gl_lightmode_default},{gl_lightmode_shaders},
   gl_lightmode_glboom, gl_lightmode_last-1, def_int,ss_none},
  {"gl_light_ambient", {&gl_light_ambient},  {20},1,255,
   def_int,ss_stat},
  {"gl_fog", {&gl_fog},  {1},0,1,
   def_bool,ss_stat},
  {"gl_fog_color", {&gl_fog_color},  {0},0,0xffffff,
   def_hex,ss_stat},
  {"useglgamma",{&useglgamma},{6},0,MAX_GLGAMMA,
   def_int,ss_none},
  {"gl_color_mip_levels", {&gl_color_mip_levels},  {0},0,1,
   def_bool,ss_stat},
  {"gl_shadows", {&simple_shadows.enable},  {0},0,1,
   def_bool,ss_stat},
  {"gl_shadows_maxdist",{&gl_shadows_maxdist},{1000},0,32767,
   def_int,ss_none},
  {"gl_shadows_factor",{&gl_shadows_factor},{128},0,255,
   def_int,ss_none},
  {"gl_blend_animations",{&gl_blend_animations},{0},0,1,
   def_bool,ss_none},

  {"Prboom-plus emulation settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"overrun_spechit_warn", {&overflows[OVERFLOW_SPECHIT].warn},  {0},0,1,
   def_bool,ss_stat},
  {"overrun_spechit_emulate", {&overflows[OVERFLOW_SPECHIT].emulate},  {1},0,1,
   def_bool,ss_stat},
  {"overrun_reject_warn", {&overflows[OVERFLOW_REJECT].warn},  {0},0,1,
   def_bool,ss_stat},
  {"overrun_reject_emulate", {&overflows[OVERFLOW_REJECT].emulate},  {1},0,1,
   def_bool,ss_stat},
  {"overrun_intercept_warn", {&overflows[OVERFLOW_INTERCEPT].warn},  {0},0,1,
   def_bool,ss_stat},
  {"overrun_intercept_emulate", {&overflows[OVERFLOW_INTERCEPT].emulate},  {1},0,1,
   def_bool,ss_stat},
  {"overrun_playeringame_warn", {&overflows[OVERFLOW_PLYERINGAME].warn},  {0},0,1,
   def_bool,ss_stat},
  {"overrun_playeringame_emulate", {&overflows[OVERFLOW_PLYERINGAME].emulate},  {1},0,1,
   def_bool,ss_stat},
  {"overrun_donut_warn", {&overflows[OVERFLOW_DONUT].warn},  {0},0,1,
   def_bool,ss_stat},
  {"overrun_donut_emulate", {&overflows[OVERFLOW_DONUT].emulate},  {0},0,1,
   def_bool,ss_stat},
  {"overrun_missedbackside_warn", {&overflows[OVERFLOW_MISSEDBACKSIDE].warn},  {0},0,1,
   def_bool,ss_stat},
  {"overrun_missedbackside_emulate", {&overflows[OVERFLOW_MISSEDBACKSIDE].emulate},  {0},0,1,
   def_bool,ss_stat},

  {"Prboom-plus 'bad' compatibility settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"comperr_zerotag", {&default_comperr[comperr_zerotag]},  {0},0,1,
   def_bool,ss_stat},
  {"comperr_passuse", {&default_comperr[comperr_passuse]},  {0},0,1,
   def_bool,ss_stat},
  {"comperr_hangsolid", {&default_comperr[comperr_hangsolid]},  {0},0,1,
   def_bool,ss_stat},
  {"comperr_blockmap", {&default_comperr[comperr_blockmap]},  {0},0,1,
   def_bool,ss_stat},
  {"comperr_freeaim", {&default_comperr[comperr_freeaim]},  {0},0,1,
   def_bool,ss_stat},

#ifdef USE_WINDOWS_LAUNCHER
  {"Prboom-plus launcher settings",{NULL},{0},UL,UL,def_none,ss_none},
  {"launcher_enable",{(int*)&launcher_enable},{launcher_enable_never},
   launcher_enable_never, launcher_enable_count - 1, def_int,ss_none},
  {"launcher_history0", {NULL,&launcher_history[0]}, {0,""},UL,UL,def_str,ss_none},
  {"launcher_history1", {NULL,&launcher_history[1]}, {0,""},UL,UL,def_str,ss_none},
  {"launcher_history2", {NULL,&launcher_history[2]}, {0,""},UL,UL,def_str,ss_none},
  {"launcher_history3", {NULL,&launcher_history[3]}, {0,""},UL,UL,def_str,ss_none},
  {"launcher_history4", {NULL,&launcher_history[4]}, {0,""},UL,UL,def_str,ss_none},
  {"launcher_history5", {NULL,&launcher_history[5]}, {0,""},UL,UL,def_str,ss_none},
  {"launcher_history6", {NULL,&launcher_history[6]}, {0,""},UL,UL,def_str,ss_none},
  {"launcher_history7", {NULL,&launcher_history[7]}, {0,""},UL,UL,def_str,ss_none},
  {"launcher_history8", {NULL,&launcher_history[8]}, {0,""},UL,UL,def_str,ss_none},
  {"launcher_history9", {NULL,&launcher_history[9]}, {0,""},UL,UL,def_str,ss_none},
#endif
  {"Prboom-plus demo patterns list. Put your patterns here",{NULL},{0},UL,UL,def_none,ss_none},
  {"demo_patterns_mask", {NULL, &demo_patterns_mask, &demo_patterns_count, &demo_patterns_list}, {0,"demo_pattern",9, &demo_patterns_list_def[0]},UL,UL,def_arr,ss_none},
  {"demo_pattern0", {NULL,&demo_patterns_list_def[0]},
   {0,"DOOM 2: Hell on Earth/((lv)|(nm)|(pa)|(ty))\\d\\d.\\d\\d\\d\\.lmp/doom2.wad"},UL,UL,def_str,ss_none},
  {"demo_pattern1", {NULL,&demo_patterns_list_def[1]},
   {0,"DOOM 2: Plutonia Experiment/p(c|f|l|n|p|r|s|t)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|plutonia.wad"},UL,UL,def_str,ss_none},
  {"demo_pattern2", {NULL,&demo_patterns_list_def[2]},
   {0,"DOOM 2: TNT - Evilution/((e(c|f|v|p|r|s|t))|(tn))\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|tnt.wad"},UL,UL,def_str,ss_none},
  {"demo_pattern3", {NULL,&demo_patterns_list_def[3]},
   {0,"The Ultimate DOOM/(((e|f|n|p|r|t|u)\\dm\\d)|(n\\ds\\d)).\\d\\d\\d\\.lmp/doom.wad"},UL,UL,def_str,ss_none},
  {"demo_pattern4", {NULL,&demo_patterns_list_def[4]},
   {0,"Alien Vendetta/a(c|f|n|p|r|s|t|v)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|av.wad|av.deh"},UL,UL,def_str,ss_none},
  {"demo_pattern5", {NULL,&demo_patterns_list_def[5]},
   {0,"Requiem/r(c|f|n|p|q|r|s|t)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|requiem.wad|req21fix.wad|reqmus.wad"},UL,UL,def_str,ss_none},
  {"demo_pattern6", {NULL,&demo_patterns_list_def[6]},
   {0,"Hell Revealed/h(c|e|f|n|p|r|s|t)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|hr.wad|hrmus.wad"},UL,UL,def_str,ss_none},
  {"demo_pattern7", {NULL,&demo_patterns_list_def[7]},
   {0,"Memento Mori/mm\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|mm.wad|mmmus.wad"},UL,UL,def_str,ss_none},
  {"demo_pattern8", {NULL,&demo_patterns_list_def[8]},
   {0,"Memento Mori 2/m2\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|mm2.wad|mm2mus.wad"},UL,UL,def_str,ss_none},

  {"Weapon preferences",{NULL},{0},UL,UL,def_none,ss_none},
  // killough 2/8/98: weapon preferences set by user:
  {"weapon_choice_1", {&weapon_preferences[0][0]}, {6}, 0,9,
   def_int,ss_weap}, // first choice for weapon (best)
  {"weapon_choice_2", {&weapon_preferences[0][1]}, {9}, 0,9,
   def_int,ss_weap}, // second choice for weapon
  {"weapon_choice_3", {&weapon_preferences[0][2]}, {4}, 0,9,
   def_int,ss_weap}, // third choice for weapon
  {"weapon_choice_4", {&weapon_preferences[0][3]}, {3}, 0,9,
   def_int,ss_weap}, // fourth choice for weapon
  {"weapon_choice_5", {&weapon_preferences[0][4]}, {2}, 0,9,
   def_int,ss_weap}, // fifth choice for weapon
  {"weapon_choice_6", {&weapon_preferences[0][5]}, {8}, 0,9,
   def_int,ss_weap}, // sixth choice for weapon
  {"weapon_choice_7", {&weapon_preferences[0][6]}, {5}, 0,9,
   def_int,ss_weap}, // seventh choice for weapon
  {"weapon_choice_8", {&weapon_preferences[0][7]}, {7}, 0,9,
   def_int,ss_weap}, // eighth choice for weapon
  {"weapon_choice_9", {&weapon_preferences[0][8]}, {1}, 0,9,
   def_int,ss_weap}, // ninth choice for weapon (worst)
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
    if (defaults[i].type == def_none) {
      // CPhipps - pure headers
      fprintf(f, "\n# %s\n", defaults[i].name);
    } else
      // e6y: arrays
      if (defaults[i].type == def_arr)
      {
        int k;
        fprintf (f,"%-*s \"%s\"\n",maxlen,defaults[i].name,*(defaults[i].location.ppsz));
        for (k = 0; k < *(defaults[i].location.array_size); k++)
        {
          char ***arr = defaults[i].location.array_data;
          if ((*arr)[k])
          {
            char def[80];
            sprintf(def, "%s%d", *(defaults[i].location.ppsz), k);
            fprintf (f,"%-*s \"%s\"\n",maxlen,def, (*arr)[k]);
          }
        }
        i += defaults[i].defaultvalue.array_size;
      }
      else

    // CPhipps - modified for new default_t form
    if (!IS_STRING(defaults[i])) //jff 4/10/98 kill super-hack on pointer value
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

#define NUMCHATSTRINGS 10 // phares 4/13/98

#define CFG_BUFFERMAX 32000

void M_LoadDefaults (void)
{
  int   i;
  int   len;
  FILE* f;
  char  def[80];
  char* strparm = malloc(CFG_BUFFERMAX);
  char* cfgline = malloc(CFG_BUFFERMAX);
  char* newstring = NULL;   // killough
  int   parm;
  dboolean isstring;
  // e6y: arrays
  default_t *item = NULL;

  // set everything to base values

  numdefaults = sizeof(defaults)/sizeof(defaults[0]);
  for (i = 0 ; i < numdefaults ; i++) {
    if (defaults[i].type == def_input)
    {
      int c;
      for (c = 0; c < DSDA_INPUT_PROFILE_COUNT; ++c)
        dsda_InputSetSpecific(c, defaults[i].identifier, defaults[i].input);
    }
    else
    {
      if (defaults[i].location.ppsz)
        *defaults[i].location.ppsz = strdup(defaults[i].defaultvalue.psz);
      if (defaults[i].location.pi)
        *defaults[i].location.pi = defaults[i].defaultvalue.i;
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

  //e6y: arrays
  for (i = 0 ; i < numdefaults ; i++) {
    if (defaults[i].type == def_arr)
    {
      int k;
      default_t *item = &defaults[i];
      char ***arr = (char***)(item->location.array_data);
      // free memory
      for (k = 0; k < *(item->location.array_size); k++)
      {
        if ((*arr)[k])
        {
          free((*arr)[k]);
          (*arr)[k] = NULL;
        }
      }
      free(*arr);
      *arr = NULL;
      *(item->location.array_size) = 0;
      // load predefined data
      *arr = realloc(*arr, sizeof(char*) * item->defaultvalue.array_size);
      *(item->location.array_size) = item->defaultvalue.array_size;
      item->location.array_index = 0;
      for (k = 0; k < item->defaultvalue.array_size; k++)
      {
        if (item->defaultvalue.array_data[k])
          (*arr)[k] = strdup(item->defaultvalue.array_data[k]);
        else
          (*arr)[k] = strdup("");
      }
    }
  }

  // check for a custom default file

#define BOOM_CFG "dsda-doom.cfg"

  i = M_CheckParm ("-config");
  if (i && i < myargc-1)
  {
    defaultfile = strdup(myargv[i+1]);
  }
  else
  {
    const char* exedir = I_DoomExeDir();
    /* get config file from same directory as executable */
    int len = doom_snprintf(NULL, 0, "%s/" BOOM_CFG, exedir);
    defaultfile = malloc(len+1);
    doom_snprintf(defaultfile, len+1, "%s/" BOOM_CFG, exedir);
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

        //jff 3/3/98 skip lines not starting with an alphanum

        if (!isalnum(def[0]))
          continue;

        if (strparm[0] == '"') {
          // get a string default

          isstring = true;
          len = strlen(strparm);
          newstring = malloc(len);
          strparm[len-1] = 0; // clears trailing double-quote mark
          strcpy(newstring, strparm+1); // clears leading double-quote mark
  } else if ((strparm[0] == '0') && (strparm[1] == 'x')) {
    // CPhipps - allow ints to be specified in hex
    sscanf(strparm+2, "%x", &parm);
  } else {
          sscanf(strparm, "%i", &parm);
    // Keycode hack removed
  }

        // e6y: arrays
        if (item)
        {
          int *pcount = item->location.array_size;
          int *index = &item->location.array_index;
          char ***arr = (char***)(item->location.array_data);
          if (!strncmp(def, *(item->location.ppsz), strlen(*(item->location.ppsz)))
              && ((item->maxvalue == UL) || *(item->location.array_size) < item->maxvalue) )
          {
            if ((*index) + 1 > *pcount)
            {
              *arr = realloc(*arr, sizeof(char*) * ((*index) + 1));
              (*pcount)++;
            }
            else
            {
              if ((*arr)[(*index)])
              {
                free((*arr)[(*index)]);
                (*arr)[(*index)] = NULL;
              }
            }
            (*arr)[(*index)] = newstring;
            (*index)++;
            continue;
          }
          else
          {
            item = NULL;
          }
        }

        for (i = 0 ; i < numdefaults ; i++)
          if ((defaults[i].type != def_none) && !strcmp(def, defaults[i].name))
            {
              // e6y: arrays
              if (defaults[i].type == def_arr)
              {
                union { const char **c; char **s; } u; // type punning via unions

                u.c = defaults[i].location.ppsz;
                free(*(u.s));
                *(u.s) = newstring;

                item = &defaults[i];
                continue;
              }

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
                free(*(u.s));
                *(u.s) = newstring;
              }
            break;
            }
        }
      }

    fclose (f);
    }

  free(strparm);
  free(cfgline);

  dsda_InitSettings();

  //jff 3/4/98 redundant range checks for hud deleted here
  /* proff 2001/7/1 - added prboom.wad as last entry so it's always loaded and
     doesn't overlap with the cfg settings */
  //e6y: Check on existence of prboom.wad
  if (!(wad_files[0] = I_FindFile(WAD_DATA, "")))
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
    base = malloc(len + 1);
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
  int p;
  int        success = 0;

  if ((p = M_CheckParm("-shotdir")) && (p < myargc - 1))
    shot_dir = M_CheckWritableDir(myargv[p + 1]);
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
      int size = doom_snprintf(NULL, 0, "%s/doom%02d" SCREENSHOT_EXT, shot_dir, shot);
      lbmname = realloc(lbmname, size+1);
      doom_snprintf(lbmname, size+1, "%s/doom%02d" SCREENSHOT_EXT, shot_dir, shot);
      shot++;
    } while (!access(lbmname,0) && (shot != startshot) && (shot < 10000));

    if (access(lbmname,0))
    {
      S_StartSound(NULL,gamemode==commercial ? sfx_radio : sfx_tink);
      M_DoScreenShot(lbmname); // cph
      success = 1;
    }
    free(lbmname);
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

void M_ArrayFree(array_t *data)
{
  if (data->data)
  {
    free(data->data);
    data->data = NULL;
  }

  data->capacity = 0;
  data->count = 0;
}

void M_ArrayAddItem(array_t *data, void *item, int itemsize)
{
  if (data->count + 1 >= data->capacity)
  {
    data->capacity = (data->capacity ? data->capacity * 2 : 128);
    data->data = realloc(data->data, data->capacity * itemsize);
  }

  memcpy((unsigned char*)data->data + data->count * itemsize, item, itemsize);
  data->count++;
}

void* M_ArrayGetNewItem(array_t *data, int itemsize)
{
  if (data->count + 1 >= data->capacity)
  {
    data->capacity = (data->capacity ? data->capacity * 2 : 128);
    data->data = realloc(data->data, data->capacity * itemsize);
  }

  data->count++;

  return (unsigned char*)data->data + (data->count - 1) * itemsize;
}
