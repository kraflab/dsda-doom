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

typedef struct {
  const char* lump_name;
  const char* nice_name;
  int levelnum;
  const char* next;
  const char* secretnext;
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
