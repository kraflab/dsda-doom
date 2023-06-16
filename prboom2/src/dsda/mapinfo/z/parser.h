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
  const char* lump_name;
  const char* nice_name;
  int levelnum;
  zmapinfo_map_next_t next;
  zmapinfo_map_next_t secretnext;
  zmapinfo_map_redirect_t redirect;
  int cluster;
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
