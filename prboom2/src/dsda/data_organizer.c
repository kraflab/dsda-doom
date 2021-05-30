//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Data Organizer
//

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "doomtype.h"
#include "m_argv.h"
#include "lprintf.h"
#include "w_wad.h"
#include "i_system.h"
#include "z_zone.h"
#include "e6y.h"

#include "data_organizer.h"

#define DATA_DIR_LIMIT 9
static const char* dsda_data_root = "dsda_doom_data";
static char* dsda_data_dir_strings[DATA_DIR_LIMIT];
static char* dsda_base_data_dir;
static char* dsda_wad_data_dir;

// Remove trailing slashes, translate backslashes to slashes
// The string to normalize is passed and returned in str
//
// jff 4/19/98 Make killoughs slash fixer a subroutine
//
static void dsda_NormalizeSlashes(char *str)
{
  size_t l;

  // killough 1/18/98: Neater / \ handling.
  // Remove trailing / or \ to prevent // /\ \/ \\, and change \ to /

  if (!str || !(l = strlen(str)))
    return;
  if (str[--l] == '/' || str[l] == '\\')     // killough 1/18/98
    str[l] = 0;
  while (l--)
    if (str[l] == '\\')
      str[l] = '/';
}

char* dsda_DetectDirectory(const char* env_key, const char* param) {
  int i;
  struct stat sbuf;
  char* result = NULL;
  const char* default_directory;

  default_directory = getenv(env_key);

  if (!default_directory)
    default_directory = I_DoomExeDir();

  if ((i = M_CheckParm(param)) && i < myargc - 1) {
    if (!stat(myargv[i + 1], &sbuf) && S_ISDIR(sbuf.st_mode)) {
      if (result) free(result);
      result = strdup(myargv[i + 1]);
    }
    else
      lprintf(LO_ERROR, "Error: %s path does not exist; using %s\n", param, default_directory);
  }

  if (!result)
    result = strdup(default_directory);

  dsda_NormalizeSlashes(result);

  return result;
}

static void dsda_CreateDirectory(const char* path) {
  int error = 0;

  error =
#if defined(_MSC_VER)
    _mkdir(path);
#else
  #if defined(_WIN32)
    mkdir(path);
  #else
    mkdir(path, 0733);
  #endif
#endif

  if (error)
    I_Error("dsda_CreateDirectory: unable to create directory %s (%d)", path, error);
}

void dsda_InitDataDir(void) {
  dsda_base_data_dir = dsda_DetectDirectory("DOOMDATADIR", "-data");
}

static void dsda_InitWadDataDir(void) {
  int i;
  int length = 0;
  const int iwad_index = 1;
  int pwad_index = 2;
  struct stat sbuf;

  dsda_data_dir_strings[0] = strdup(dsda_data_root);

  for (i = 0; i < numwadfiles; ++i) {
    const char* start;
    char* result;

    start = PathFindFileName(wadfiles[i].name);

    length = strlen(start) - 4;

    if (length > 0 && !strcasecmp(start + length, ".wad")) {
      int dir_index;

      if (wadfiles[i].src == 0)
        dir_index = iwad_index;
      else if (wadfiles[i].src == 3)
        dir_index = pwad_index;
      else
        dir_index = -1;

      if (dir_index >= 0 && dir_index < DATA_DIR_LIMIT) {
        dsda_data_dir_strings[dir_index] = malloc(length + 1);
        strncpy(dsda_data_dir_strings[dir_index], start, length);
        dsda_data_dir_strings[dir_index][length] = '\0';

        for (result = dsda_data_dir_strings[dir_index]; *result; ++result)
          *result = tolower(*result);

        if (dir_index == pwad_index)
          pwad_index++;
      }
    }
  }

  length = strlen(dsda_base_data_dir);
  for (i = 0; i < DATA_DIR_LIMIT; ++i) {
    if (dsda_data_dir_strings[i])
      length += strlen(dsda_data_dir_strings[i]) + 1; // "/"
  }

  dsda_wad_data_dir = calloc(length + 1, 1); // "\0"

  strcat(dsda_wad_data_dir, dsda_base_data_dir);

  for (i = 0; i < DATA_DIR_LIMIT; ++i) {
    if (dsda_data_dir_strings[i]) {
      strcat(dsda_wad_data_dir, "/");
      strcat(dsda_wad_data_dir, dsda_data_dir_strings[i]);

      if (!stat(dsda_wad_data_dir, &sbuf) && S_ISDIR(sbuf.st_mode))
        continue;
      else
        dsda_CreateDirectory(dsda_wad_data_dir);
    }
  }

  lprintf(LO_INFO, "Using data file directory: %s\n", dsda_wad_data_dir);
}

char* dsda_DataDir(void) {
  if (!dsda_wad_data_dir)
    dsda_InitWadDataDir();

  return dsda_wad_data_dir;
}
