//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA MAPINFO Doom Parser
//

#ifndef __DSDA_MAPINFO_DOOM_PARSER__
#define __DSDA_MAPINFO_DOOM_PARSER__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

typedef enum {
  dmi_end_null,
  dmi_end_game_scroll,
  dmi_end_game_cast,
} dmi_end_t;

typedef struct {
  char* map;
  char* endpic;
  dmi_end_t end;
} doom_mapinfo_map_next_t;

typedef struct {
  char* lump;
  float scrollspeed;
} doom_mapinfo_sky_t;

typedef enum {
  dmi_lighting_normal,
  dmi_lighting_even,
  dmi_lighting_smooth,
} dmi_lighting_t;

typedef struct {
  int monster_type;
  int action_special;
  int special_args[5];
} doom_mapinfo_special_action_t;

#define DMI_INTERMISSION                   0x00000001ul
#define DMI_ALLOW_MONSTER_TELEFRAGS        0x00000002ul
#define DMI_ACTIVATE_OWN_DEATH_SPECIALS    0x00000004ul
#define DMI_LAX_MONSTER_ACTIVATION         0x00000008ul
#define DMI_MISSILES_ACTIVATE_IMPACT_LINES 0x00000010ul
#define DMI_FILTER_STARTS                  0x00000020ul
#define DMI_ALLOW_RESPAWN                  0x00000040ul
#define DMI_ALLOW_JUMP                     0x00000080ul
#define DMI_ALLOW_FREE_LOOK                0x00000100ul
#define DMI_CHECK_SWITCH_RANGE             0x00000200ul
#define DMI_RESET_HEALTH                   0x00000400ul
#define DMI_RESET_INVENTORY                0x00000800ul
#define DMI_USE_PLAYER_START_Z             0x00001000ul
#define DMI_RANDOM_PLAYER_STARTS           0x00002000ul
#define DMI_REMEMBER_STATE                 0x00004000ul
#define DMI_SHOW_AUTHOR                    0x00008000ul
#define DMI_PASSOVER                       0x00010000ul

typedef uint32_t doom_mapinfo_map_flags_t;

typedef struct {
  char* lump_name;
  char* nice_name;
  char* author;
  int level_num;
  int cluster;
  doom_mapinfo_map_next_t next;
  doom_mapinfo_map_next_t secret_next;
  doom_mapinfo_sky_t sky1;
  char* title_patch;
  char* exit_pic;
  char* enter_pic;
  char* border_texture;
  char* music;
  char* inter_music;
  int par;
  dmi_lighting_t lighting;
  char* gravity;
  char* air_control;
  size_t num_special_actions;
  doom_mapinfo_special_action_t* special_actions;
  doom_mapinfo_map_flags_t flags;
} doom_mapinfo_map_t;

typedef struct {
  size_t num_maps;
  doom_mapinfo_map_t* maps;
  int loaded;
} doom_mapinfo_t;

extern doom_mapinfo_t doom_mapinfo;

typedef void (*doom_mapinfo_errorfunc)(const char *fmt, ...);	// this must not return!

void dsda_ParseDoomMapInfo(const unsigned char* buffer, size_t length, doom_mapinfo_errorfunc err);

#ifdef __cplusplus
}
#endif

#endif
