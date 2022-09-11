//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Config
//

#include <string.h>

#include "am_map.h"
#include "doomdef.h"
#include "doomstat.h"
#include "g_overflow.h"
#include "gl_struct.h"
#include "r_demo.h"
#include "s_sound.h"
#include "v_video.h"
#include "z_zone.h"

#include "dsda/input.h"

#include "configuration.h"

typedef enum {
  dsda_config_int,
  dsda_config_string,
} dsda_config_type_t;

typedef union {
  int v_int;
  char* v_string;
} dsda_config_value_t;

typedef union {
  int v_int;
  const char* v_string;
} dsda_config_default_t;

typedef struct {
  const char* name;
  dsda_config_identifier_t id;
  dsda_config_type_t type;
  int lower_limit;
  int upper_limit;
  dsda_config_default_t default_value;
  int* int_binding;
  int flags;
  int strict_value;
  void (*onUpdate)(void);
  dsda_config_value_t transient_value;
  dsda_config_value_t persistent_value;
} dsda_config_t;

#define CONF_STRICT 0x01
#define CONF_EVEN   0x02

#define CONF_BOOL(x) dsda_config_int, 0, 1, { x }
#define CONF_COLOR(x) dsda_config_int, 0, 255, { x }
#define CONF_BYTE(x) dsda_config_int, 0, 255, { x }
#define CONF_STRING(x) dsda_config_string, 0, 0, { .v_string = x }
#define CONF_CR(x) dsda_config_int, 0, CR_LIMIT - 1, { x }
#define CONF_WEAPON(x) dsda_config_int, 0, 9, { x }

extern int dsda_input_profile;
extern int weapon_preferences[2][NUMWEAPONS + 1];
extern int demo_smoothturns;
extern int demo_smoothturnsfactor;
extern int sts_always_red;
extern int sts_pct_always_gray;
extern int sts_traditional_keys;
extern int full_sounds;

void I_Init2(void);
void M_ChangeDemoSmoothTurns(void);
void M_ChangeMouseLook(void);
void M_ChangeMessages(void);
void S_ResetSfxVolume(void);
void I_ResetMusicVolume(void);
void dsda_RefreshExHudFPS(void);
void M_ChangeAllowFog(void);
void gld_ResetShadowParameters(void);
void M_ChangeTextureParams(void);
void gld_MultisamplingInit(void);
void M_ChangeFOV(void);
void M_ChangeLightMode(void);
void I_InitMouse(void);
void MouseAccelChanging(void);
void G_UpdateMouseSensitivity(void);
void I_InitJoystick(void);
void M_ChangeSpeed(void);
void M_ChangeShorttics(void);
void I_InitSoundParams(void);
void S_Init(void);
void M_ChangeMIDIPlayer(void);
void HU_init_crosshair(void);

// TODO: migrate all kinds of stuff from M_Init

// TODO: automatically go through strict list
static void UpdateStrictMode(void) {
  void M_ChangeSpeed(void);
  void dsda_InitKeyFrame(void);

  I_Init2(); // side effect of realtic clock rate
  M_ChangeSpeed(); // side effect of always sr50
  dsda_InitKeyFrame();
}

dsda_config_t dsda_config[dsda_config_count] = {
  [dsda_config_realtic_clock_rate] = {
    "realtic_clock_rate", dsda_config_realtic_clock_rate,
    dsda_config_int, 3, 10000, { 100 }, NULL, CONF_STRICT, 100, I_Init2
  },
  [dsda_config_default_complevel] = {
    "default_compatibility_level", dsda_config_default_complevel,
    dsda_config_int, 0, mbf21_compatibility, { mbf21_compatibility }
  },
  [dsda_config_default_skill] = {
    "default_skill", dsda_config_default_skill,
    dsda_config_int, 1, 5, { 4 }
  },
  [dsda_config_vanilla_keymap] = {
    "vanilla_keymap", dsda_config_vanilla_keymap,
    CONF_BOOL(0)
  },
  [dsda_config_menu_background] = {
    "menu_background", dsda_config_menu_background,
    CONF_BOOL(1)
  },
  [dsda_config_process_priority] = {
    "process_priority", dsda_config_process_priority,
    dsda_config_int, 0, 2, { 0 }
  },
  [dsda_config_max_player_corpse] = {
    "max_player_corpse", dsda_config_max_player_corpse,
    dsda_config_int, -1, INT_MAX, { 32 }, NULL, CONF_STRICT, 32
  },
  [dsda_config_input_profile] = {
    "input_profile", dsda_config_input_profile,
    dsda_config_int, 0, DSDA_INPUT_PROFILE_COUNT - 1, { 0 }, &dsda_input_profile
  },
  [dsda_config_weapon_choice_1] = {
    "weapon_choice_1", dsda_config_weapon_choice_1,
    CONF_WEAPON(6), &weapon_preferences[0][0]
  },
  [dsda_config_weapon_choice_2] = {
    "weapon_choice_2", dsda_config_weapon_choice_2,
    CONF_WEAPON(9), &weapon_preferences[0][1]
  },
  [dsda_config_weapon_choice_3] = {
    "weapon_choice_3", dsda_config_weapon_choice_3,
    CONF_WEAPON(4), &weapon_preferences[0][2]
  },
  [dsda_config_weapon_choice_4] = {
    "weapon_choice_4", dsda_config_weapon_choice_4,
    CONF_WEAPON(3), &weapon_preferences[0][3]
  },
  [dsda_config_weapon_choice_5] = {
    "weapon_choice_5", dsda_config_weapon_choice_5,
    CONF_WEAPON(2), &weapon_preferences[0][4]
  },
  [dsda_config_weapon_choice_6] = {
    "weapon_choice_6", dsda_config_weapon_choice_6,
    CONF_WEAPON(8), &weapon_preferences[0][5]
  },
  [dsda_config_weapon_choice_7] = {
    "weapon_choice_7", dsda_config_weapon_choice_7,
    CONF_WEAPON(5), &weapon_preferences[0][6]
  },
  [dsda_config_weapon_choice_8] = {
    "weapon_choice_8", dsda_config_weapon_choice_8,
    CONF_WEAPON(7), &weapon_preferences[0][7]
  },
  [dsda_config_weapon_choice_9] = {
    "weapon_choice_9", dsda_config_weapon_choice_9,
    CONF_WEAPON(1), &weapon_preferences[0][8]
  },
  [dsda_config_flashing_hom] = {
    "flashing_hom", dsda_config_flashing_hom,
    CONF_BOOL(0)
  },
  [dsda_config_demo_smoothturns] = {
    "demo_smoothturns", dsda_config_demo_smoothturns,
    CONF_BOOL(0), &demo_smoothturns,
    0, 0, M_ChangeDemoSmoothTurns
  },
  [dsda_config_demo_smoothturnsfactor] = {
    "demo_smoothturnsfactor", dsda_config_demo_smoothturnsfactor,
    dsda_config_int, 1, SMOOTH_PLAYING_MAXFACTOR, { 6 }, &demo_smoothturnsfactor,
    0, 0, M_ChangeDemoSmoothTurns
  },
  [dsda_config_weapon_attack_alignment] = {
    "weapon_attack_alignment", dsda_config_weapon_attack_alignment,
    dsda_config_int, 0, 3, { 0 }, NULL, CONF_STRICT, 0
  },
  [dsda_config_sts_always_red] = {
    "sts_always_red", dsda_config_sts_always_red,
    CONF_BOOL(1), &sts_always_red
  },
  [dsda_config_sts_pct_always_gray] = {
    "sts_pct_always_gray", dsda_config_sts_pct_always_gray,
    CONF_BOOL(0), &sts_pct_always_gray
  },
  [dsda_config_sts_traditional_keys] = {
    "sts_traditional_keys", dsda_config_sts_traditional_keys,
    CONF_BOOL(0), &sts_traditional_keys
  },
  [dsda_config_strict_mode] = {
    "dsda_strict_mode", dsda_config_strict_mode,
    CONF_BOOL(1), NULL, 0, 0, UpdateStrictMode
  },
  [dsda_config_vertmouse] = {
    "movement_vertmouse", dsda_config_vertmouse,
    CONF_BOOL(0)
  },
  [dsda_config_mouselook] = {
    "movement_mouselook", dsda_config_mouselook,
    CONF_BOOL(0), NULL, CONF_STRICT, 0, M_ChangeMouseLook
  },
  [dsda_config_autorun] = {
    "autorun", dsda_config_autorun,
    CONF_BOOL(1)
  },
  [dsda_config_show_messages] = {
    "show_messages", dsda_config_show_messages,
    CONF_BOOL(1), NULL, 0, 0, M_ChangeMessages
  },
  [dsda_config_command_display] = {
    "dsda_command_display", dsda_config_command_display,
    CONF_BOOL(0), NULL, CONF_STRICT, 0
  },
  [dsda_config_coordinate_display] = {
    "dsda_coordinate_display", dsda_config_coordinate_display,
    CONF_BOOL(0), NULL, CONF_STRICT, 0
  },
  [dsda_config_show_fps] = {
    "dsda_show_fps", dsda_config_show_fps,
    CONF_BOOL(0), NULL, 0, 0, dsda_RefreshExHudFPS
  },
  [dsda_config_exhud] = {
    "dsda_exhud", dsda_config_exhud,
    CONF_BOOL(0)
  },
  [dsda_config_mute_sfx] = {
    "dsda_mute_sfx", dsda_config_mute_sfx,
    CONF_BOOL(0), NULL, 0, 0, S_ResetSfxVolume
  },
  [dsda_config_mute_music] = {
    "dsda_mute_music", dsda_config_mute_music,
    CONF_BOOL(0), NULL, 0, 0, I_ResetMusicVolume
  },
  [dsda_config_cheat_codes] = {
    "dsda_cheat_codes", dsda_config_cheat_codes,
    CONF_BOOL(1)
  },
  [dsda_config_script_0] = {
    "dsda_script_0", dsda_config_script_0,
    CONF_STRING("")
  },
  [dsda_config_script_1] = {
    "dsda_script_1", dsda_config_script_1,
    CONF_STRING("")
  },
  [dsda_config_script_2] = {
    "dsda_script_2", dsda_config_script_2,
    CONF_STRING("")
  },
  [dsda_config_script_3] = {
    "dsda_script_3", dsda_config_script_3,
    CONF_STRING("")
  },
  [dsda_config_script_4] = {
    "dsda_script_4", dsda_config_script_4,
    CONF_STRING("")
  },
  [dsda_config_script_5] = {
    "dsda_script_5", dsda_config_script_5,
    CONF_STRING("")
  },
  [dsda_config_script_6] = {
    "dsda_script_6", dsda_config_script_6,
    CONF_STRING("")
  },
  [dsda_config_script_7] = {
    "dsda_script_7", dsda_config_script_7,
    CONF_STRING("")
  },
  [dsda_config_script_8] = {
    "dsda_script_8", dsda_config_script_8,
    CONF_STRING("")
  },
  [dsda_config_script_9] = {
    "dsda_script_9", dsda_config_script_9,
    CONF_STRING("")
  },
  [dsda_config_overrun_spechit_warn] = {
    "overrun_spechit_warn", dsda_config_overrun_spechit_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_SPECHIT].warn
  },
  [dsda_config_overrun_spechit_emulate] = {
    "overrun_spechit_emulate", dsda_config_overrun_spechit_emulate,
    CONF_BOOL(1), &overflows[OVERFLOW_SPECHIT].emulate
  },
  [dsda_config_overrun_reject_warn] = {
    "overrun_reject_warn", dsda_config_overrun_reject_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_REJECT].warn
  },
  [dsda_config_overrun_reject_emulate] = {
    "overrun_reject_emulate", dsda_config_overrun_reject_emulate,
    CONF_BOOL(1), &overflows[OVERFLOW_REJECT].emulate
  },
  [dsda_config_overrun_intercept_warn] = {
    "overrun_intercept_warn", dsda_config_overrun_intercept_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_INTERCEPT].warn
  },
  [dsda_config_overrun_intercept_emulate] = {
    "overrun_intercept_emulate", dsda_config_overrun_intercept_emulate,
    CONF_BOOL(1), &overflows[OVERFLOW_INTERCEPT].emulate
  },
  [dsda_config_overrun_playeringame_warn] = {
    "overrun_playeringame_warn", dsda_config_overrun_playeringame_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_PLAYERINGAME].warn
  },
  [dsda_config_overrun_playeringame_emulate] = {
    "overrun_playeringame_emulate", dsda_config_overrun_playeringame_emulate,
    CONF_BOOL(1), &overflows[OVERFLOW_PLAYERINGAME].emulate
  },
  [dsda_config_overrun_donut_warn] = {
    "overrun_donut_warn", dsda_config_overrun_donut_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_DONUT].warn
  },
  [dsda_config_overrun_donut_emulate] = {
    "overrun_donut_emulate", dsda_config_overrun_donut_emulate,
    CONF_BOOL(0), &overflows[OVERFLOW_DONUT].emulate
  },
  [dsda_config_overrun_missedbackside_warn] = {
    "overrun_missedbackside_warn", dsda_config_overrun_missedbackside_warn,
    CONF_BOOL(0), &overflows[OVERFLOW_MISSEDBACKSIDE].warn
  },
  [dsda_config_overrun_missedbackside_emulate] = {
    "overrun_missedbackside_emulate", dsda_config_overrun_missedbackside_emulate,
    CONF_BOOL(0), &overflows[OVERFLOW_MISSEDBACKSIDE].emulate
  },
  [dsda_config_comperr_zerotag] = {
    "comperr_zerotag", dsda_config_comperr_zerotag,
    CONF_BOOL(0), &default_comperr[comperr_zerotag]
  },
  [dsda_config_comperr_passuse] = {
    "comperr_passuse", dsda_config_comperr_passuse,
    CONF_BOOL(0), &default_comperr[comperr_passuse]
  },
  [dsda_config_comperr_hangsolid] = {
    "comperr_hangsolid", dsda_config_comperr_hangsolid,
    CONF_BOOL(0), &default_comperr[comperr_hangsolid]
  },
  [dsda_config_comperr_blockmap] = {
    "comperr_blockmap", dsda_config_comperr_blockmap,
    CONF_BOOL(0), &default_comperr[comperr_blockmap]
  },
  [dsda_config_comperr_freeaim] = {
    "comperr_freeaim", dsda_config_comperr_freeaim,
    CONF_BOOL(0), &default_comperr[comperr_freeaim]
  },
  [dsda_config_mapcolor_back] = {
    "mapcolor_back", dsda_config_mapcolor_back,
    CONF_COLOR(247), &mapcolor_back
  },
  [dsda_config_mapcolor_grid] = {
    "mapcolor_grid", dsda_config_mapcolor_grid,
    CONF_COLOR(104), &mapcolor_grid
  },
  [dsda_config_mapcolor_wall] = {
    "mapcolor_wall", dsda_config_mapcolor_wall,
    CONF_COLOR(23), &mapcolor_wall
  },
  [dsda_config_mapcolor_fchg] = {
    "mapcolor_fchg", dsda_config_mapcolor_fchg,
    CONF_COLOR(55), &mapcolor_fchg
  },
  [dsda_config_mapcolor_cchg] = {
    "mapcolor_cchg", dsda_config_mapcolor_cchg,
    CONF_COLOR(215), &mapcolor_cchg
  },
  [dsda_config_mapcolor_clsd] = {
    "mapcolor_clsd", dsda_config_mapcolor_clsd,
    CONF_COLOR(208), &mapcolor_clsd
  },
  [dsda_config_mapcolor_rkey] = {
    "mapcolor_rkey", dsda_config_mapcolor_rkey,
    CONF_COLOR(175), &mapcolor_rkey
  },
  [dsda_config_mapcolor_bkey] = {
    "mapcolor_bkey", dsda_config_mapcolor_bkey,
    CONF_COLOR(204), &mapcolor_bkey
  },
  [dsda_config_mapcolor_ykey] = {
    "mapcolor_ykey", dsda_config_mapcolor_ykey,
    CONF_COLOR(231), &mapcolor_ykey
  },
  [dsda_config_mapcolor_rdor] = {
    "mapcolor_rdor", dsda_config_mapcolor_rdor,
    CONF_COLOR(175), &mapcolor_rdor
  },
  [dsda_config_mapcolor_bdor] = {
    "mapcolor_bdor", dsda_config_mapcolor_bdor,
    CONF_COLOR(204), &mapcolor_bdor
  },
  [dsda_config_mapcolor_ydor] = {
    "mapcolor_ydor", dsda_config_mapcolor_ydor,
    CONF_COLOR(231), &mapcolor_ydor
  },
  [dsda_config_mapcolor_tele] = {
    "mapcolor_tele", dsda_config_mapcolor_tele,
    CONF_COLOR(119), &mapcolor_tele
  },
  [dsda_config_mapcolor_secr] = {
    "mapcolor_secr", dsda_config_mapcolor_secr,
    CONF_COLOR(252), &mapcolor_secr
  },
  [dsda_config_mapcolor_revsecr] = {
    "mapcolor_revsecr", dsda_config_mapcolor_revsecr,
    CONF_COLOR(112), &mapcolor_revsecr
  },
  [dsda_config_mapcolor_exit] = {
    "mapcolor_exit", dsda_config_mapcolor_exit,
    CONF_COLOR(0), &mapcolor_exit
  },
  [dsda_config_mapcolor_unsn] = {
    "mapcolor_unsn", dsda_config_mapcolor_unsn,
    CONF_COLOR(104), &mapcolor_unsn
  },
  [dsda_config_mapcolor_flat] = {
    "mapcolor_flat", dsda_config_mapcolor_flat,
    CONF_COLOR(88), &mapcolor_flat
  },
  [dsda_config_mapcolor_sprt] = {
    "mapcolor_sprt", dsda_config_mapcolor_sprt,
    CONF_COLOR(112), &mapcolor_sprt
  },
  [dsda_config_mapcolor_item] = {
    "mapcolor_item", dsda_config_mapcolor_item,
    CONF_COLOR(231), &mapcolor_item
  },
  [dsda_config_mapcolor_hair] = {
    "mapcolor_hair", dsda_config_mapcolor_hair,
    CONF_COLOR(208), &mapcolor_hair
  },
  [dsda_config_mapcolor_sngl] = {
    "mapcolor_sngl", dsda_config_mapcolor_sngl,
    CONF_COLOR(208), &mapcolor_sngl
  },
  [dsda_config_mapcolor_me] = {
    "mapcolor_me", dsda_config_mapcolor_me,
    CONF_COLOR(112), &mapcolor_me
  },
  [dsda_config_mapcolor_enemy] = {
    "mapcolor_enemy", dsda_config_mapcolor_enemy,
    CONF_COLOR(177), &mapcolor_enemy
  },
  [dsda_config_mapcolor_frnd] = {
    "mapcolor_frnd", dsda_config_mapcolor_frnd,
    CONF_COLOR(112), &mapcolor_frnd
  },
  [dsda_config_gl_sprite_blend] = {
    "gl_sprite_blend", dsda_config_gl_sprite_blend,
    CONF_BOOL(0), &gl_sprite_blend
  },
  [dsda_config_gl_render_paperitems] = {
    "gl_render_paperitems", dsda_config_gl_render_paperitems,
    CONF_BOOL(0), &gl_render_paperitems
  },
  [dsda_config_gl_fog] = {
    "gl_fog", dsda_config_gl_fog,
    CONF_BOOL(1), &gl_fog, 0, 0, M_ChangeAllowFog
  },
  [dsda_config_gl_shadows] = {
    "gl_shadows", dsda_config_gl_shadows,
    CONF_BOOL(0), NULL, CONF_STRICT, false
  },
  [dsda_config_gl_blend_animations] = {
    "gl_blend_animations", dsda_config_gl_blend_animations,
    CONF_BOOL(0), &gl_blend_animations
  },
  [dsda_config_gl_shadows_maxdist] = {
    "gl_shadows_maxdist", dsda_config_gl_shadows_maxdist,
    dsda_config_int, 0, 32767, { 1000 }, NULL, 0, 0, gld_ResetShadowParameters
  },
  [dsda_config_gl_shadows_factor] = {
    "gl_shadows_factor", dsda_config_gl_shadows_factor,
    CONF_BYTE(128), NULL, 0, 0, gld_ResetShadowParameters
  },
  [dsda_config_gl_usegamma] = {
    "gl_usegamma", dsda_config_gl_usegamma,
    dsda_config_int, 0, MAX_GLGAMMA, { 0 }, &gl_usegamma
  },
  [dsda_config_gl_skymode] = {
    "gl_skymode", dsda_config_gl_skymode,
    dsda_config_int, skytype_auto, skytype_count - 1, { skytype_auto }, NULL,
    0, 0, M_ChangeMouseLook
  },
  [dsda_config_gl_texture_filter] = {
    "gl_texture_filter", dsda_config_gl_texture_filter,
    dsda_config_int, filter_nearest, filter_linear_mipmap_linear, { filter_nearest_mipmap_linear },
    NULL, 0, 0, M_ChangeTextureParams
  },
  [dsda_config_gl_sprite_filter] = {
    "gl_sprite_filter", dsda_config_gl_sprite_filter,
    dsda_config_int, filter_nearest, filter_linear_mipmap_nearest, { filter_nearest },
    NULL, 0, 0, M_ChangeTextureParams
  },
  [dsda_config_gl_patch_filter] = {
    "gl_patch_filter", dsda_config_gl_patch_filter,
    dsda_config_int, filter_nearest, filter_linear, { filter_nearest },
    NULL, 0, 0, M_ChangeTextureParams
  },
  [dsda_config_gl_texture_filter_anisotropic] = {
    "gl_texture_filter_anisotropic", dsda_config_gl_texture_filter_anisotropic,
    dsda_config_int, 0, 4, { 3 },
    NULL, 0, 0, M_ChangeTextureParams
  },
  [dsda_config_gl_tex_format_string] = {
    "gl_tex_format_string", dsda_config_gl_tex_format_string,
    CONF_STRING("GL_RGBA"),
    NULL, 0, 0, M_ChangeTextureParams
  },
  [dsda_config_gl_render_multisampling] = {
    "gl_render_multisampling", dsda_config_gl_render_multisampling,
    dsda_config_int, 0, 8, { 0 }, NULL, CONF_EVEN, 0, gld_MultisamplingInit
  },
  [dsda_config_gl_render_fov] = {
    "gl_render_fov", dsda_config_gl_render_fov,
    dsda_config_int, 20, 160, { 90 }, &gl_render_fov, 0, 0, M_ChangeFOV
  },
  [dsda_config_gl_lightmode] = {
    "gl_lightmode", dsda_config_gl_lightmode,
    dsda_config_int, gl_lightmode_glboom, gl_lightmode_last - 1, { gl_lightmode_shaders },
    NULL, 0, 0, M_ChangeLightMode
  },
  [dsda_config_gl_spriteclip] = {
    "gl_spriteclip", dsda_config_gl_spriteclip,
    dsda_config_int, spriteclip_const, spriteclip_smart, { spriteclip_smart }
  },
  [dsda_config_gl_health_bar] = {
    "gl_health_bar", dsda_config_gl_health_bar,
    CONF_BOOL(0), NULL, CONF_STRICT, 0
  },
  [dsda_config_use_mouse] = {
    "use_mouse", dsda_config_use_mouse,
    CONF_BOOL(1), NULL, 0, 0, I_InitMouse
  },
  [dsda_config_mouse_sensitivity_horiz] = {
    "mouse_sensitivity_horiz", dsda_config_mouse_sensitivity_horiz,
    dsda_config_int, 0, INT_MAX, { 10 }, NULL, 0, 0, G_UpdateMouseSensitivity
  },
  [dsda_config_mouse_sensitivity_vert] = {
    "mouse_sensitivity_vert", dsda_config_mouse_sensitivity_vert,
    dsda_config_int, 0, INT_MAX, { 1 }, NULL, 0, 0, G_UpdateMouseSensitivity
  },
  [dsda_config_mouse_acceleration] = {
    "dsda_mouse_acceleration", dsda_config_mouse_acceleration,
    dsda_config_int, 0, INT_MAX, { 0 }, NULL, 0, 0, MouseAccelChanging
  },
  [dsda_config_mouse_sensitivity_mlook] = {
    "mouse_sensitivity_mlook", dsda_config_mouse_sensitivity_mlook,
    dsda_config_int, 0, INT_MAX, { 10 }, NULL, 0, 0, G_UpdateMouseSensitivity
  },
  [dsda_config_mouse_stutter_correction] = {
    "mouse_stutter_correction", dsda_config_mouse_stutter_correction,
    CONF_BOOL(1)
  },
  [dsda_config_mouse_doubleclick_as_use] = {
    "mouse_doubleclick_as_use", dsda_config_mouse_doubleclick_as_use,
    CONF_BOOL(0)
  },
  [dsda_config_mouse_carrytics] = {
    "mouse_carrytics", dsda_config_mouse_carrytics,
    CONF_BOOL(1)
  },
  [dsda_config_movement_mouseinvert] = {
    "movement_mouseinvert", dsda_config_movement_mouseinvert,
    CONF_BOOL(0)
  },
  [dsda_config_movement_mousestrafedivisor] = {
    "movement_mousestrafedivisor", dsda_config_movement_mousestrafedivisor,
    dsda_config_int, 1, INT_MAX, { 4 }, NULL, 0, 0, G_UpdateMouseSensitivity
  },
  [dsda_config_fine_sensitivity] = {
    "dsda_fine_sensitivity", dsda_config_fine_sensitivity,
    dsda_config_int, 0, 99, { 0 }, NULL, 0, 0, G_UpdateMouseSensitivity
  },
  [dsda_config_use_joystick] = {
    "use_joystick", dsda_config_use_joystick,
    dsda_config_int, 0, 2, { 0 }, NULL, 0, 0, I_InitJoystick
  },
  [dsda_config_deh_apply_cheats] = {
    "deh_apply_cheats", dsda_config_deh_apply_cheats,
    CONF_BOOL(1)
  },
  [dsda_config_movement_strafe50] = {
    "movement_strafe50", dsda_config_movement_strafe50,
    CONF_BOOL(0), NULL, CONF_STRICT, 0, M_ChangeSpeed
  },
  [dsda_config_movement_strafe50onturns] = {
    "movement_strafe50onturns", dsda_config_movement_strafe50onturns,
    CONF_BOOL(0), NULL, 0, 0, M_ChangeSpeed
  },
  [dsda_config_movement_shorttics] = {
    "movement_shorttics", dsda_config_movement_shorttics,
    CONF_BOOL(0), NULL, 0, 0, M_ChangeShorttics
  },
  [dsda_config_screenshot_dir] = {
    "screenshot_dir", dsda_config_screenshot_dir,
    CONF_STRING("")
  },
  [dsda_config_startup_delay_ms] = {
    "startup_delay_ms", dsda_config_startup_delay_ms,
    dsda_config_int, 0, 1000, { 0 }
  },
  [dsda_config_snd_pcspeaker] = {
    "snd_pcspeaker", dsda_config_snd_pcspeaker,
    CONF_BOOL(0), NULL, 0, 0, I_InitSoundParams
  },
  [dsda_config_pitched_sounds] = {
    "pitched_sounds", dsda_config_pitched_sounds,
    CONF_BOOL(0), NULL, 0, 0, I_InitSoundParams
  },
  [dsda_config_full_sounds] = {
    "full_sounds", dsda_config_full_sounds,
    CONF_BOOL(0), &full_sounds
  },
  [dsda_config_snd_samplerate] = {
    "samplerate", dsda_config_snd_samplerate,
    dsda_config_int, 11025, 48000, { 44100 }, NULL, 0, 0, I_InitSoundParams
  },
  [dsda_config_snd_samplecount] = {
    "slice_samplecount", dsda_config_snd_samplecount,
    dsda_config_int, 32, 8192, { 512 }, NULL, 0, 0, I_InitSoundParams
  },
  [dsda_config_sfx_volume] = {
    "sfx_volume", dsda_config_sfx_volume,
    dsda_config_int, 0, 15, { 8 }, NULL, 0, 0, S_ResetSfxVolume
  },
  [dsda_config_music_volume] = {
    "music_volume", dsda_config_music_volume,
    dsda_config_int, 0, 15, { 8 }, NULL, 0, 0, I_ResetMusicVolume
  },
  [dsda_config_mus_pause_opt] = {
    "mus_pause_opt", dsda_config_mus_pause_opt,
    dsda_config_int, 0, 2, { 1 }
  },
  [dsda_config_snd_channels] = {
    "snd_channels", dsda_config_snd_channels,
    dsda_config_int, 1, MAX_CHANNELS, { 32 }, NULL, 0, 0, S_Init
  },
  [dsda_config_snd_midiplayer] = {
    "snd_midiplayer", dsda_config_snd_midiplayer,
    CONF_STRING("fluidsynth"), NULL, 0, 0, M_ChangeMIDIPlayer
  },
  [dsda_config_snd_mididev] = {
    "snd_mididev", dsda_config_snd_mididev,
    CONF_STRING("")
  },
  [dsda_config_snd_soundfont] = {
    "snd_soundfont", dsda_config_snd_soundfont,
    CONF_STRING("")
  },
  [dsda_config_mus_fluidsynth_chorus] = {
    "mus_fluidsynth_chorus", dsda_config_mus_fluidsynth_chorus,
    CONF_BOOL(0)
  },
  [dsda_config_mus_fluidsynth_reverb] = {
    "mus_fluidsynth_reverb", dsda_config_mus_fluidsynth_reverb,
    CONF_BOOL(0)
  },
  [dsda_config_mus_fluidsynth_gain] = {
    "mus_fluidsynth_gain", dsda_config_mus_fluidsynth_gain,
    dsda_config_int, 0, 1000, { 50 }
  },
  [dsda_config_mus_opl_gain] = {
    "mus_opl_gain", dsda_config_mus_opl_gain,
    dsda_config_int, 0, 1000, { 50 }
  },
  [dsda_config_mus_portmidi_reset_type] = {
    "mus_portmidi_reset_type", dsda_config_mus_portmidi_reset_type,
    CONF_STRING("gs") // gs, gm, gm2, xg
  },
  [dsda_config_mus_portmidi_reset_delay] = {
    "mus_portmidi_reset_delay", dsda_config_mus_portmidi_reset_delay,
    dsda_config_int, 0, 2000, { 0 }
  },
  [dsda_config_cap_soundcommand] = {
    "cap_soundcommand", dsda_config_cap_soundcommand,
    CONF_STRING("ffmpeg -f s16le -ar %s -ac 2 -i - -c:a libopus -y temp_a.nut")
  },
  [dsda_config_cap_videocommand] = {
    "cap_videocommand", dsda_config_cap_videocommand,
    CONF_STRING("ffmpeg -f rawvideo -pix_fmt rgb24 -r %r -s %wx%h -i - -c:v libx264 -y temp_v.nut")
  },
  [dsda_config_cap_muxcommand] = {
    "cap_muxcommand", dsda_config_cap_muxcommand,
    CONF_STRING("ffmpeg -i temp_v.nut -i temp_a.nut -c copy -y %f")
  },
  [dsda_config_cap_tempfile1] = {
    "cap_tempfile1", dsda_config_cap_tempfile1,
    CONF_STRING("temp_a.nut")
  },
  [dsda_config_cap_tempfile2] = {
    "cap_tempfile2", dsda_config_cap_tempfile2,
    CONF_STRING("temp_v.nut")
  },
  [dsda_config_cap_remove_tempfiles] = {
    "cap_remove_tempfiles", dsda_config_cap_remove_tempfiles,
    CONF_BOOL(1)
  },
  [dsda_config_cap_wipescreen] = {
    "cap_wipescreen", dsda_config_cap_wipescreen,
    CONF_BOOL(0)
  },
  [dsda_config_cap_fps] = {
    "cap_fps", dsda_config_cap_fps,
    dsda_config_int, 16, 300, { 60 }
  },
  [dsda_config_hudadd_crosshair_color] = {
    "hudadd_crosshair_color", dsda_config_hudadd_crosshair_color,
    CONF_CR(3)
  },
  [dsda_config_hudadd_crosshair_target_color] = {
    "hudadd_crosshair_target_color", dsda_config_hudadd_crosshair_target_color,
    CONF_CR(9)
  },
  [dsda_config_hud_displayed] = {
    "hud_displayed", dsda_config_hud_displayed,
    CONF_BOOL(0)
  },
  [dsda_config_hudadd_secretarea] = {
    "hudadd_secretarea", dsda_config_hudadd_secretarea,
    CONF_BOOL(1)
  },
  [dsda_config_hudadd_demoprogressbar] = {
    "hudadd_demoprogressbar", dsda_config_hudadd_demoprogressbar,
    CONF_BOOL(1)
  },
  [dsda_config_hudadd_crosshair_scale] = {
    "hudadd_crosshair_scale", dsda_config_hudadd_crosshair_scale,
    CONF_BOOL(0), NULL, 0, 0, HU_init_crosshair
  },
  [dsda_config_hudadd_crosshair_health] = {
    "hudadd_crosshair_health", dsda_config_hudadd_crosshair_health,
    CONF_BOOL(0), NULL, 0, 0, HU_init_crosshair
  },
  [dsda_config_hudadd_crosshair_target] = {
    "hudadd_crosshair_target", dsda_config_hudadd_crosshair_target,
    CONF_BOOL(0), NULL, CONF_STRICT, 0, HU_init_crosshair
  },
  [dsda_config_hudadd_crosshair_lock_target] = {
    "hudadd_crosshair_lock_target", dsda_config_hudadd_crosshair_lock_target,
    CONF_BOOL(0), NULL, CONF_STRICT, 0, HU_init_crosshair
  },
};

static void dsda_PersistIntConfig(dsda_config_t* conf) {
  conf->persistent_value.v_int = conf->transient_value.v_int;
}

static void dsda_PersistStringConfig(dsda_config_t* conf) {
  if (conf->persistent_value.v_string)
    Z_Free(conf->persistent_value.v_string);

  conf->persistent_value.v_string = Z_Strdup(conf->transient_value.v_string);
}

static void dsda_ConstrainIntConfig(dsda_config_t* conf) {
  if (conf->transient_value.v_int > conf->upper_limit)
    conf->transient_value.v_int = conf->upper_limit;
  else if (conf->transient_value.v_int < conf->lower_limit)
    conf->transient_value.v_int = conf->lower_limit;

  if (conf->flags & CONF_EVEN && (conf->transient_value.v_int % 2))
    conf->transient_value.v_int = conf->default_value.v_int;
}

static void dsda_PropagateIntConfig(dsda_config_t* conf) {
  if (conf->int_binding)
    *conf->int_binding = dsda_IntConfig(conf->id);
}

// No side effects
static void dsda_InitIntConfig(dsda_config_t* conf, int value) {
  conf->transient_value.v_int = value;

  dsda_ConstrainIntConfig(conf);
  dsda_PersistIntConfig(conf);
  dsda_PropagateIntConfig(conf);
}

// No side effects
static void dsda_InitStringConfig(dsda_config_t* conf, const char* value) {
  if (conf->transient_value.v_string)
    Z_Free(conf->transient_value.v_string);

  conf->transient_value.v_string = Z_Strdup(value);
  dsda_PersistStringConfig(conf);
}

void dsda_InitConfig(void) {
  int i;

  for (i = 1; i < dsda_config_count; ++i) {
    dsda_config_t* conf;

    conf = &dsda_config[i];

    if (conf->type == dsda_config_int)
      dsda_InitIntConfig(conf, conf->default_value.v_int);
    else if (conf->type == dsda_config_string)
      dsda_InitStringConfig(conf, conf->default_value.v_string);
  }
}

dboolean dsda_ReadConfig(const char* name, const char* string_param, int int_param) {
  int id;

  id = dsda_ConfigIDByName(name);

  if (id) {
    dsda_config_t* conf;

    conf = &dsda_config[id];

    if (conf->type == dsda_config_int && !string_param)
      dsda_InitIntConfig(conf, int_param);
    else if (conf->type == dsda_config_string && string_param)
      dsda_InitStringConfig(conf, string_param);

    return true;
  }

  return false;
}

void dsda_WriteConfig(dsda_config_identifier_t id, int key_length, FILE* file) {
  dsda_config_t* conf;

  conf = &dsda_config[id];

  if (conf->type == dsda_config_int)
    fprintf(file, "%-*s %i\n", key_length, conf->name, conf->persistent_value.v_int);
  else if (conf->type == dsda_config_string)
    fprintf(file, "%-*s \"%s\"\n", key_length, conf->name, conf->persistent_value.v_string);
}

int dsda_ToggleConfig(dsda_config_identifier_t id, dboolean persist) {
  return dsda_UpdateIntConfig(id, !dsda_config[id].transient_value.v_int, persist);
}

int dsda_IncrementIntConfig(dsda_config_identifier_t id, dboolean persist) {
  return dsda_UpdateIntConfig(id, dsda_config[id].transient_value.v_int + 1, persist);
}

int dsda_DecrementIntConfig(dsda_config_identifier_t id, dboolean persist) {
  return dsda_UpdateIntConfig(id, dsda_config[id].transient_value.v_int - 1, persist);
}

int dsda_CycleConfig(dsda_config_identifier_t id, dboolean persist) {
  int value;

  value = dsda_config[id].transient_value.v_int + 1;

  if (value > dsda_config[id].upper_limit)
    value = dsda_config[id].lower_limit;

  return dsda_UpdateIntConfig(id, value, persist);
}

int dsda_UpdateIntConfig(dsda_config_identifier_t id, int value, dboolean persist) {
  dsda_config[id].transient_value.v_int = value;

  dsda_ConstrainIntConfig(&dsda_config[id]);

  if (persist)
    dsda_PersistIntConfig(&dsda_config[id]);

  dsda_PropagateIntConfig(&dsda_config[id]);

  if (dsda_config[id].onUpdate)
    dsda_config[id].onUpdate();

  return dsda_IntConfig(id);
}

const char* dsda_UpdateStringConfig(dsda_config_identifier_t id, const char* value, dboolean persist) {
  if (dsda_config[id].transient_value.v_string)
    Z_Free(dsda_config[id].transient_value.v_string);

  dsda_config[id].transient_value.v_string = Z_Strdup(value);

  if (persist)
    dsda_PersistStringConfig(&dsda_config[id]);

  return dsda_StringConfig(id);
}

int dsda_IntConfig(dsda_config_identifier_t id) {
  dboolean dsda_StrictMode(void);

  if (dsda_config[id].flags & CONF_STRICT && dsda_StrictMode())
    return dsda_config[id].strict_value;

  return dsda_config[id].transient_value.v_int;
}

int dsda_PersistentIntConfig(dsda_config_identifier_t id) {
  return dsda_config[id].persistent_value.v_int;
}

const char* dsda_StringConfig(dsda_config_identifier_t id) {
  return dsda_config[id].transient_value.v_string;
}

const char* dsda_PersistentStringConfig(dsda_config_identifier_t id) {
  return dsda_config[id].persistent_value.v_string;
}

char* dsda_ConfigSummary(const char* name) {
  int id;
  char* summary = NULL;
  size_t length;

  id = dsda_ConfigIDByName(name);

  if (id) {
    dsda_config_t* conf;

    conf = &dsda_config[id];

    if (conf->type == dsda_config_int) {
      length = snprintf(NULL, 0,
                        "%s: %d (transient), %d (persistent)", conf->name,
                        conf->transient_value.v_int, conf->persistent_value.v_int);
      summary = Z_Malloc(length + 1);
      snprintf(summary, length + 1,
                        "%s: %d (transient), %d (persistent)", conf->name,
                        conf->transient_value.v_int, conf->persistent_value.v_int);
    }
    else if (conf->type == dsda_config_string) {
      length = snprintf(NULL, 0,
                        "%s: %s (transient), %s (persistent)", conf->name,
                        conf->transient_value.v_string, conf->persistent_value.v_string);
      summary = Z_Malloc(length + 1);
      snprintf(summary, length + 1,
                        "%s: %s (transient), %s (persistent)", conf->name,
                        conf->transient_value.v_string, conf->persistent_value.v_string);
    }

    return summary;
  }

  return NULL;
}

int dsda_ConfigIDByName(const char* name) {
  int i;

  for (i = 1; i < dsda_config_count; ++i)
    if (!strcmp(name, dsda_config[i].name))
      return i;

  return 0;
}
