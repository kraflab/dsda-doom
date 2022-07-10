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
  const char* description;

  arg_type_t type;

  int lower_limit;
  int upper_limit;
  int min_count;
  int max_count;
} arg_config_t;

static arg_config_t arg_config[dsda_arg_count] = {
  [dsda_arg_complevel] = {
    "-complevel", "-cl",
    "sets the compatibility level",
    arg_int, -1, mbf21_compatibility,
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

  switch (config->type) {
    case arg_null:
      if (arg->count)
        I_Error("%s does not take an argument", config->name);

      arg->count = 1; // track that it was found

      break;
    case arg_int:
      if (arg->count == 0)
        I_Error("%s requires an integer argument", config->name);
      if (arg->count > 1)
        I_Error("%s takes only one argument", config->name);

      dsda_ParseIntArg(config, &arg->value.v_int, myargv[myarg_i + 1]);

      break;
    case arg_string:
      if (arg->count == 0)
        I_Error("%s requires a string argument", config->name);
      if (arg->count > 1)
        I_Error("%s takes only one argument", config->name);

      arg->value.v_string = myargv[myarg_i + 1];

      break;
    case arg_int_array:
      if (arg->count < config->min_count)
        I_Error("too few argument for %s (min %i)", config->name, config->min_count);
      else if (arg->count > config->max_count)
        I_Error("too many arguments for %s (max %i)", config->name, config->max_count);

      {
        int i;

        arg->value.v_int_array = Z_Malloc(arg->count * sizeof(int));

        for (i = myarg_i; i < myarg_i + arg->count; ++i)
          dsda_ParseIntArg(config, &arg->value.v_int_array[i - myarg_i], myargv[i + 1]);
      }

      break;

    case arg_string_array:
      if (arg->count < config->min_count)
        I_Error("too few argument for %s (min %i)", config->name, config->min_count);
      else if (arg->count > config->max_count)
        I_Error("too many arguments for %s (max %i)", config->name, config->max_count);

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
  dsda_ParseIntArg(&arg_config[id], &arg_value[id].value.v_int, param);
}

dsda_arg_t* dsda_Arg(dsda_arg_identifier_t id) {
  return &arg_value[id];
}

dboolean dsda_Flag(dsda_arg_identifier_t id) {
  return arg_value[id].count > 0;
}
