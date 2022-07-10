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

#include <stdio.h>
#include <string.h>

#include "lprintf.h"
#include "z_zone.h"

#include "dsda/args.h"

extern int myargc;
extern char** myargv;

typedef enum {
  arg_null,
  arg_int,
  arg_string,
  arg_int_array,
  arg_string_array,
} arg_type_t;

typedef struct {
  const char* name;
  const char* alias;
  const char* default_value;
  const char* description;

  arg_type_t type;

  int lower_limit;
  int upper_limit;
  int min_count;
  int max_count;
} arg_config_t;

#define AT_LEAST_ONE_STRING 0, 0, 1, INT_MAX
#define EXACT_ARRAY_LENGTH(x) 0, 0, x, x

static arg_config_t arg_config[dsda_arg_count] = {
  [dsda_arg_help] = {
    "-help", "--help", NULL,
    "prints out command line argument information",
    arg_null,
  },
  [dsda_arg_iwad] = {
    "-iwad", NULL, NULL,
    "loads the given iwad file",
    arg_string,
  },
  [dsda_arg_file] = {
    "-file", NULL, NULL,
    "loads additional wad files",
    arg_string_array, AT_LEAST_ONE_STRING,
  },
  [dsda_arg_deh] = {
    "-deh", NULL, NULL,
    "loads additional deh files",
    arg_string_array, AT_LEAST_ONE_STRING,
  },
  [dsda_arg_playdemo] = {
    "-playdemo", NULL, NULL,
    "plays the given demo file",
    arg_string,
  },
  [dsda_arg_timedemo] = {
    "-timedemo", NULL, NULL,
    "plays the given demo file as fast as possible, timing the process",
    arg_string,
  },
  [dsda_arg_fastdemo] = {
    "-fastdemo", NULL, NULL,
    "plays the given demo file as fast as possible, skipping some frames",
    arg_string,
  },
  [dsda_arg_recordfromto] = {
    "-recordfromto", NULL, NULL,
    "plays back the first file while writing to the second",
    arg_string_array, EXACT_ARRAY_LENGTH(2),
  },
  [dsda_arg_complevel] = {
    "-complevel", "-cl", NULL,
    "sets the compatibility level",
    arg_int, -1, mbf21_compatibility,
  },
  [dsda_arg_fast] = {
    "-fast", NULL, NULL,
    "turns on fast monsters",
    arg_null,
  },
  [dsda_arg_respawn] = {
    "-respawn", NULL, NULL,
    "turns on monster respawning",
    arg_null,
  },
  [dsda_arg_nomonsters] = {
    "-nomonsters", "-nomo", NULL,
    "turns off monster spawning",
    arg_null,
  },
  [dsda_arg_stroller] = {
    "-stroller", NULL, NULL,
    "applies stroller category limitations",
    arg_null,
  },
  [dsda_arg_turbo] = {
    "-turbo", NULL, "255",
    "sets player speed percent",
    arg_int, 10, 255,
  },
  [dsda_arg_tas] = {
    "-tas", NULL, NULL,
    "lifts strict mode restrictions",
    arg_null,
  },
  [dsda_arg_build] = {
    "-build", NULL, NULL,
    "starts in build mode",
    arg_null,
  },
  [dsda_arg_track_pacifist] = {
    "-track_pacifist", NULL, NULL,
    "tracks pacifist category restrictions",
    arg_null,
  },
  [dsda_arg_track_100k] = {
    "-track_100k", NULL, NULL,
    "tracks when 100% kills is reached",
    arg_null,
  },
  [dsda_arg_time_keys] = {
    "-time_keys", NULL, NULL,
    "announces the time when keys are picked up",
    arg_null,
  },
  [dsda_arg_time_use] = {
    "-time_use", NULL, NULL,
    "announces the time when the use command is activated",
    arg_null,
  },
  [dsda_arg_time_secrets] = {
    "-time_secrets", NULL, NULL,
    "announces the time when a secret is collected",
    arg_null,
  },
  [dsda_arg_time_all] = {
    "-time_all", NULL, NULL,
    "announces the time when any -time_* event happens",
    arg_null,
  },
  [dsda_arg_analysis] = {
    "-analysis", NULL, NULL,
    "writes various data to analysis.txt",
    arg_null,
  },
  [dsda_arg_levelstat] = {
    "-levelstat", NULL, NULL,
    "writes level stats to levelstat.txt",
    arg_null,
  },
  [dsda_arg_movie] = {
    "-movie", NULL, NULL,
    "sets the target final level for movie demos (for automatic exit detection)",
    arg_int, 0, 99,
  },
  [dsda_arg_verbose] = {
    "-verbose", NULL, NULL,
    "enable all logging",
    arg_null,
  },
  [dsda_arg_quiet] = {
    "-quiet", NULL, NULL,
    "disable all logging",
    arg_null,
  },
  [dsda_arg_forceoldbsp] = {
    "-forceoldbsp", NULL, NULL,
    "force classic bsp nodes",
    arg_null,
  },
  [dsda_arg_devparm] = {
    "-devparm", NULL, NULL,
    "turn on old development mode",
    arg_null,
  },
  [dsda_arg_deathmatch] = {
    "-deathmatch", NULL, NULL,
    "turn on deathmatch mode",
    arg_null,
  },
  [dsda_arg_altdeath] = {
    "-altdeath", NULL, NULL,
    "turn on altdeath mode",
    arg_null,
  },
  [dsda_arg_nosound] = {
    "-nosound", NULL, NULL,
    "turn off sound",
    arg_null,
  },
  [dsda_arg_nomusic] = {
    "-nomusic", NULL, NULL,
    "turn off music",
    arg_null,
  },
  [dsda_arg_nosfx] = {
    "-nosfx", NULL, NULL,
    "turn off sfx",
    arg_null,
  },
  [dsda_arg_nodraw] = {
    "-nodraw", NULL, NULL,
    "turn off drawing",
    arg_null,
  },
  [dsda_arg_nodeh] = {
    "-nodeh", NULL, NULL,
    "skip dehacked lumps inside wads",
    arg_null,
  },
};

static dsda_arg_t arg_value[dsda_arg_count];

static void dsda_ParseIntArg(arg_config_t* config, int* value, const char* param) {
  if (sscanf(param, "%i", value) != 1) {
    if (config->type == arg_int)
      I_Error("%s requires an integer argument", config->name);
    else
      I_Error("%s requires integer arguments", config->name);
  }
  if (*value < config->lower_limit)
    I_Error("%s argument too low (min is %i)", config->name, config->lower_limit);
  if (*value > config->upper_limit)
    I_Error("%s argument too high (max is %i)", config->name, config->upper_limit);
}

static void dsda_ValidateArrayArg(arg_config_t* config, dsda_arg_t* arg) {
  if (config->min_count == config->max_count) {
    if (arg->count != config->min_count)
      I_Error("%s requires exactly %i arguments", config->name, config->min_count);
  }
  else {
    if (arg->count < config->min_count || arg->count > config->max_count)
      I_Error("%s requires %i to %i arguments", config->name, config->min_count, config->max_count);
  }
}

static void dsda_ParseArg(arg_config_t* config, dsda_arg_t* arg, int myarg_i) {
  for (arg->count = 0; myarg_i + arg->count < myargc - 1; ++arg->count) {
    if (config->type == arg_int || config->type == arg_int_array) {
      int x;

      // only valid integers should be interpreted as arguments
      if (!sscanf(myargv[myarg_i + arg->count + 1], "%i", &x))
        break;

      continue;
    }

    if (myargv[myarg_i + arg->count + 1][0] == '-')
      break;
  }

  arg->found = true;

  switch (config->type) {
    case arg_null:
      if (arg->count)
        I_Error("%s does not take an argument", config->name);

      break;
    case arg_int:
      if (arg->count == 0) {
        if (config->default_value) {
          dsda_ParseIntArg(config, &arg->value.v_int, config->default_value);

          break;
        }

        I_Error("%s requires an integer argument", config->name);
      }
      if (arg->count > 1)
        I_Error("%s takes only one argument", config->name);

      dsda_ParseIntArg(config, &arg->value.v_int, myargv[myarg_i + 1]);

      break;
    case arg_string:
      if (arg->count == 0) {
        if (config->default_value) {
          arg->value.v_string = config->default_value;

          break;
        }

        I_Error("%s requires a string argument", config->name);
      }
      if (arg->count > 1)
        I_Error("%s takes only one argument", config->name);

      arg->value.v_string = myargv[myarg_i + 1];

      break;
    case arg_int_array:
      dsda_ValidateArrayArg(config, arg);

      {
        int i;

        arg->value.v_int_array = Z_Malloc(arg->count * sizeof(int));

        for (i = myarg_i; i < myarg_i + arg->count; ++i)
          dsda_ParseIntArg(config, &arg->value.v_int_array[i - myarg_i], myargv[i + 1]);
      }

      break;

    case arg_string_array:
      dsda_ValidateArrayArg(config, arg);

      {
        int i;

        arg->value.v_string_array = Z_Malloc(arg->count * sizeof(char *));

        for (i = myarg_i; i < myarg_i + arg->count; ++i)
          arg->value.v_string_array[i - myarg_i] = myargv[i + 1];
      }

      break;
  }
}

void dsda_ParseCommandLineArgs(void) {
  int i;
  int myarg_i;
  arg_config_t* config;

  for (i = 0; i < dsda_arg_count; ++i) {
    config = &arg_config[i];

    for (myarg_i = myargc - 1; myarg_i > 0; --myarg_i)
      if (
        !strcasecmp(config->name, myargv[myarg_i]) ||
        (config->alias && !strcasecmp(config->alias, myargv[myarg_i]))
      ) {
        dsda_ParseArg(config, &arg_value[i], myarg_i);
        break;
      }
  }
}

void dsda_UpdateIntArg(dsda_arg_identifier_t id, const char* param) {
  arg_value[id].count = 1;
  arg_value[id].found = true;
  dsda_ParseIntArg(&arg_config[id], &arg_value[id].value.v_int, param);
}

void dsda_UpdateStringArg(dsda_arg_identifier_t id, const char* param) {
  arg_value[id].count = 1;
  arg_value[id].found = true;
  arg_value[id].value.v_string = param;
}

void dsda_AppendStringArg(dsda_arg_identifier_t id, const char* param) {
  if (arg_config[id].type == arg_string)
    return dsda_UpdateStringArg(id, param);

  ++arg_value[id].count;
  arg_value[id].found = true;
  arg_value[id].value.v_string_array =
    Z_Realloc(arg_value[id].value.v_string_array, arg_value[id].count * sizeof(char *));
  arg_value[id].value.v_string_array[arg_value[id].count - 1] = param;

  dsda_ValidateArrayArg(&arg_config[id], &arg_value[id]);
}

dsda_arg_t* dsda_Arg(dsda_arg_identifier_t id) {
  return &arg_value[id];
}

void dsda_UpdateFlag(dsda_arg_identifier_t id, dboolean found) {
  arg_value[id].found = found;
}

dboolean dsda_Flag(dsda_arg_identifier_t id) {
  return arg_value[id].found;
}

void dsda_PrintArgHelp(void) {
  int i;

  lprintf(LO_INFO, "\nCommand Line Arguments:\n\n");

  for (i = 0; i < dsda_arg_count; ++i) {
    arg_config_t* config;

    config = &arg_config[i];

    lprintf(LO_INFO, "  %s", config->name);
    if (config->alias)
      lprintf(LO_INFO, " / %s", config->alias);
    lprintf(LO_INFO, ":\n");
    lprintf(LO_INFO, "    %s\n", config->description);
    if (config->default_value)
      lprintf(LO_INFO, "    Default: %s\n", config->default_value);
    lprintf(LO_INFO, "\n");
  }
}
