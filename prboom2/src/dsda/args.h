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
//	DSDA (Command Line) Args
//

#include "doomtype.h"

typedef enum {
  dsda_arg_help,
  dsda_arg_iwad,
  dsda_arg_file,
  dsda_arg_deh,
  dsda_arg_playdemo,
  dsda_arg_timedemo,
  dsda_arg_fastdemo,
  dsda_arg_recordfromto,
  dsda_arg_skill,
  dsda_arg_complevel,
  dsda_arg_fast,
  dsda_arg_respawn,
  dsda_arg_nomonsters,
  dsda_arg_stroller,
  dsda_arg_turbo,
  dsda_arg_tas,
  dsda_arg_build,
  dsda_arg_track_pacifist,
  dsda_arg_track_100k,
  dsda_arg_time_keys,
  dsda_arg_time_use,
  dsda_arg_time_secrets,
  dsda_arg_time_all,
  dsda_arg_analysis,
  dsda_arg_levelstat,
  dsda_arg_movie,
  dsda_arg_verbose,
  dsda_arg_quiet,
  dsda_arg_forceoldbsp,
  dsda_arg_devparm,
  dsda_arg_deathmatch,
  dsda_arg_altdeath,
  dsda_arg_nosound,
  dsda_arg_nomusic,
  dsda_arg_nosfx,
  dsda_arg_nodraw,
  dsda_arg_nodeh,
  dsda_arg_count,
} dsda_arg_identifier_t;

typedef struct {
  int count;
  dboolean found;

  union {
    int v_int;
    int* v_int_array;
    const char* v_string;
    const char** v_string_array;
  } value;
} dsda_arg_t;

void dsda_ParseCommandLineArgs(void);
dsda_arg_t* dsda_Arg(dsda_arg_identifier_t id);
dboolean dsda_Flag(dsda_arg_identifier_t id);
void dsda_UpdateIntArg(dsda_arg_identifier_t id, const char* param);
void dsda_UpdateStringArg(dsda_arg_identifier_t id, const char* param);
void dsda_AppendStringArg(dsda_arg_identifier_t id, const char* param);
void dsda_UpdateFlag(dsda_arg_identifier_t id, dboolean found);
void dsda_PrintArgHelp(void);
