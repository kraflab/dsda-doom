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

#ifndef __DSDA_CONFIG__
#define __DSDA_CONFIG__

#include <stdio.h>

#include "doomtype.h"

typedef enum {
  dsda_config_none,
  dsda_config_realtic_clock_rate,
  dsda_config_default_complevel,
  dsda_config_default_skill,
  dsda_config_vanilla_keymap,
  dsda_config_menu_background,
  dsda_config_process_priority,
  dsda_config_max_player_corpse,
  dsda_config_input_profile,
  dsda_config_weapon_choice_1,
  dsda_config_weapon_choice_2,
  dsda_config_weapon_choice_3,
  dsda_config_weapon_choice_4,
  dsda_config_weapon_choice_5,
  dsda_config_weapon_choice_6,
  dsda_config_weapon_choice_7,
  dsda_config_weapon_choice_8,
  dsda_config_weapon_choice_9,
  dsda_config_flashing_hom,
  dsda_config_demo_smoothturns,
  dsda_config_demo_smoothturnsfactor,
  dsda_config_weapon_attack_alignment,
  dsda_config_sts_always_red,
  dsda_config_sts_pct_always_gray,
  dsda_config_sts_traditional_keys,
  dsda_config_strict_mode,
  dsda_config_vertmouse,
  dsda_config_mouselook,
  dsda_config_autorun,
  dsda_config_show_messages,
  dsda_config_command_display,
  dsda_config_coordinate_display,
  dsda_config_show_fps,
  dsda_config_exhud,
  dsda_config_mute_sfx,
  dsda_config_mute_music,
  dsda_config_cheat_codes,
  dsda_config_script_0,
  dsda_config_script_1,
  dsda_config_script_2,
  dsda_config_script_3,
  dsda_config_script_4,
  dsda_config_script_5,
  dsda_config_script_6,
  dsda_config_script_7,
  dsda_config_script_8,
  dsda_config_script_9,
  dsda_config_overrun_spechit_warn,
  dsda_config_overrun_spechit_emulate,
  dsda_config_overrun_reject_warn,
  dsda_config_overrun_reject_emulate,
  dsda_config_overrun_intercept_warn,
  dsda_config_overrun_intercept_emulate,
  dsda_config_overrun_playeringame_warn,
  dsda_config_overrun_playeringame_emulate,
  dsda_config_overrun_donut_warn,
  dsda_config_overrun_donut_emulate,
  dsda_config_overrun_missedbackside_warn,
  dsda_config_overrun_missedbackside_emulate,
  dsda_config_comperr_zerotag,
  dsda_config_comperr_passuse,
  dsda_config_comperr_hangsolid,
  dsda_config_comperr_blockmap,
  dsda_config_comperr_freeaim,
  dsda_config_mapcolor_back,
  dsda_config_mapcolor_grid,
  dsda_config_mapcolor_wall,
  dsda_config_mapcolor_fchg,
  dsda_config_mapcolor_cchg,
  dsda_config_mapcolor_clsd,
  dsda_config_mapcolor_rkey,
  dsda_config_mapcolor_bkey,
  dsda_config_mapcolor_ykey,
  dsda_config_mapcolor_rdor,
  dsda_config_mapcolor_bdor,
  dsda_config_mapcolor_ydor,
  dsda_config_mapcolor_tele,
  dsda_config_mapcolor_secr,
  dsda_config_mapcolor_revsecr,
  dsda_config_mapcolor_exit,
  dsda_config_mapcolor_unsn,
  dsda_config_mapcolor_flat,
  dsda_config_mapcolor_sprt,
  dsda_config_mapcolor_item,
  dsda_config_mapcolor_hair,
  dsda_config_mapcolor_sngl,
  dsda_config_mapcolor_me,
  dsda_config_mapcolor_enemy,
  dsda_config_mapcolor_frnd,
   dsda_config_gl_arb_multitexture,
   dsda_config_gl_arb_texture_compression,
   dsda_config_gl_arb_texture_non_power_of_two,
   dsda_config_gl_ext_arb_vertex_buffer_object,
   dsda_config_gl_arb_pixel_buffer_object,
   dsda_config_gl_arb_shader_objects,
   dsda_config_gl_ext_blend_color,
   dsda_config_gl_ext_framebuffer_object,
   dsda_config_gl_ext_packed_depth_stencil,
   dsda_config_gl_ext_texture_filter_anisotropic,
   dsda_config_gl_use_stencil,
   dsda_config_gl_use_display_lists,
   dsda_config_gl_finish,
   dsda_config_gl_clear,
   dsda_config_gl_ztrick,
  dsda_config_gl_sprite_blend,
   dsda_config_gl_use_paletted_texture,
   dsda_config_gl_use_shared_texture_palette,
   dsda_config_gl_allow_detail_textures,
   dsda_config_gl_sprites_frustum_culling,
  dsda_config_render_paperitems,
   dsda_config_gl_boom_colormaps,
   dsda_config_gl_hires_24bit_colormap,
   dsda_config_gl_texture_internal_hires,
   dsda_config_gl_texture_external_hires,
   dsda_config_gl_hires_override_pwads,
   dsda_config_gl_texture_hqresize,
  dsda_config_gl_fog,
   dsda_config_gl_color_mip_levels,
  dsda_config_gl_shadows,
   dsda_config_gl_blend_animations,
  dsda_config_count,
} dsda_config_identifier_t;

void dsda_InitConfig(void);
dboolean dsda_ReadConfig(const char* name, const char* string_param, int int_param);
void dsda_WriteConfig(dsda_config_identifier_t id, int key_length, FILE* file);
int dsda_ToggleConfig(dsda_config_identifier_t id, dboolean persist);
int dsda_CycleConfig(dsda_config_identifier_t id, dboolean persist);
int dsda_UpdateIntConfig(dsda_config_identifier_t id, int value, dboolean persist);
const char* dsda_UpdateStringConfig(dsda_config_identifier_t id, const char* value, dboolean persist);
int dsda_IntConfig(dsda_config_identifier_t id);
int dsda_PersistentIntConfig(dsda_config_identifier_t id);
const char* dsda_StringConfig(dsda_config_identifier_t id);
const char* dsda_PersistentStringConfig(dsda_config_identifier_t id);
char* dsda_ConfigSummary(const char* name);
int dsda_ConfigIDByName(const char* name);

#endif
