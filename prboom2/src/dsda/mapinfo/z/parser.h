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
//	DSDA ZMAPINFO Parser
//

#ifndef __DSDA_MAPINFO_Z_PARSER__
#define __DSDA_MAPINFO_Z_PARSER__

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

typedef enum {
  zmn_null,
  zmn_endgame1,
  zmn_endgame2,
  zmn_endgamew,
  zmn_endgame4,
  zmn_endgamec,
  zmn_endgame3,
  zmn_enddemon,
  zmn_endgames,
  zmn_endchess,
  zmn_endtitle,
  zmn_end_count,
} zmn_end_t;

typedef struct {
  const char* map;
  const char* endpic
  const char* intermission;
  zmn_end_t end;
} zmapinfo_map_next_t;

// TODO: where is the list of items?
typedef enum {
  zmr_item_null,
  zmr_item_count,
} zmr_item_t;

typedef struct {
  zmr_item_t item;
  const char* map;
} zmapinfo_map_redirect_t;

typedef struct {
  const char* lump;
  float scrollspeed;
} zmapinfo_sky_t;

typedef enum {
  zm_lighting_normal,
  zm_lighting_even,
  zm_lighting_smooth,
} zm_lighting_t;

typedef enum {
  zm_infighting_normal,
  zm_infighting_total,
  zm_infighting_none,
} zm_infighting_t;

#define ZM_DOUBLE_SKY                     0x00000001ul
#define ZM_SKY_STRETCH                    0x00000002ul
#define ZM_INTERMISSION                   0x00000004ul
#define ZM_LIGHTNING                      0x00000008ul
#define ZM_ALLOW_MONSTER_TELEFRAGS        0x00000010ul
#define ZM_ACTIVATE_OWN_DEATH_SPECIALS    0x00000020ul
#define ZM_MAP07_SPECIAL                  0x00000040ul
#define ZM_BARON_SPECIAL                  0x00000080ul
#define ZM_CYBERDEMON_SPECIAL             0x00000100ul
#define ZM_SPIDER_MASTERMIND_SPECIAL      0x00000200ul
#define ZM_SPECIAL_ACTION_EXIT_LEVEL      0x00000400ul
#define ZM_SPECIAL_ACTION_OPEN_DOOR       0x00000800ul
#define ZM_SPECIAL_ACTION_LOWER_FLOOR     0x00001000ul
#define ZM_SPECIAL_ACTION_KILL_MONSTERS   0x00002000ul
#define ZM_CLIP_MID_TEXTURES              0x00004000ul
#define ZM_LAX_MONSTER_ACTIVATION         0x00008000ul
#define ZM_MISSILES_ACTIVATE_IMPACT_LINES 0x00010000ul
#define ZM_AVOID_MELEE                    0x00020000ul
#define ZM_FILTER_STARTS                  0x00040000ul
#define ZM_ALLOW_RESPAWN                  0x00080000ul
#define ZM_ALLOW_JUMP                     0x00100000ul
#define ZM_ALLOW_FREE_LOOK                0x00200000ul
#define ZM_CHECK_SWITCH_RANGE             0x00400000ul
#define ZM_NO_ALLIES                      0x00800000ul
#define ZM_RESET_HEALTH                   0x01000000ul
#define ZM_RESET_INVENTORY                0x02000000ul
#define ZM_GRINDING_POLYOBJ               0x04000000ul
#define ZM_USE_PLAYER_START_Z             0x08000000ul
#define ZM_RANDOM_PLAYER_STARTS           0x10000000ul
#define ZM_REMEMBER_STATE                 0x20000000ul
#define ZM_SPAWN_WITH_WEAPON_RAISED       0x40000000ul
#define ZM_SHOW_AUTHOR                    0x80000000ul

typedef uint32_t zmapinfo_map_flags_t;

typedef struct {
  const char* lump_name;
  const char* nice_name;
  int levelnum;
  zmapinfo_map_next_t next;
  zmapinfo_map_next_t secretnext;
  zmapinfo_map_redirect_t redirect;
  int cluster;
  zmapinfo_sky_t sky1;
  zmapinfo_sky_t sky2;
  const char* fade_table;
  const char* title_patch;
  int par;
  int suck_time;
  const char* music;
  const char* inter_music;
  const char* exit_pic;
  const char* enter_pic;
  const char* border_texture;
  zm_lighting_t lighting;
  int gravity;
  zmapinfo_infighting_t infighting;
  zmapinfo_map_flags_t flags;
} zmapinfo_map_t;

typedef struct {
  size_t num_maps;
  zmapinfo_map_t* maps;
} zmapinfo_t;

extern zmapinfo_t zmapinfo;

typedef void (*zmapinfo_errorfunc)(const char *fmt, ...);	// this must not return!

void dsda_ParseZMapInfo(const unsigned char* buffer, size_t length, zmapinfo_errorfunc err);

#ifdef __cplusplus
}
#endif

#endif
