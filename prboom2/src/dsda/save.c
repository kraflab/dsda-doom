//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Save
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

#include "save.h"

int dsda_organized_saves;
static char* dsda_base_save_dir;
static char* dsda_wad_save_dir;

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

void dsda_InitSaveDir(void) {
  int         i;    //jff 3/24/98 index of args on commandline
  struct stat sbuf; //jff 3/24/98 used to test save path for existence

  // set save path to -save parm or current dir

  //jff 3/27/98 default to current dir
  //V.Aguilar (5/30/99): In LiNUX, default to $HOME/.lxdoom
  {
    // CPhipps - use DOOMSAVEDIR if defined
    const char *p = getenv("DOOMSAVEDIR");

    if (p == NULL)
      p = I_DoomExeDir();

    free(dsda_base_save_dir);
    dsda_base_save_dir = strdup(p);
  }

  if ((i = M_CheckParm("-save")) && i < myargc - 1) //jff 3/24/98 if -save present
  {
    if (!stat(myargv[i + 1], &sbuf) && S_ISDIR(sbuf.st_mode)) // and is a dir
    {
      free(dsda_base_save_dir);
      dsda_base_save_dir = strdup(myargv[i + 1]);//jff 3/24/98 use that for savegame
    }
    //jff 9/3/98 use logical output routine
    else lprintf(LO_ERROR, "Error: -save path does not exist, using %s\n", dsda_base_save_dir);
  }

  dsda_NormalizeSlashes(dsda_base_save_dir);
}

#define SAVE_DIR_LIMIT 9
static const char* dsda_save_root = "save_files";
static char* dsda_save_dir_strings[SAVE_DIR_LIMIT];

static void dsda_CreateSaveDir(void) {
  int error = 0;

  error =
#if defined(_MSC_VER)
    _mkdir(dsda_wad_save_dir);
#else
  #if defined(_WIN32)
    mkdir(dsda_wad_save_dir);
  #else
    mkdir(dsda_wad_save_dir, 0x733);
  #endif
#endif

  if (error)
    I_Error(
      "dsda_CreateSaveDir: unable to create save file directory %s (%d)",
      dsda_wad_save_dir, error
    );
}

static void dsda_InitWadSaveDir(void) {
  int i;
  int length = 0;
  const int iwad_index = 1;
  int pwad_index = 2;
  struct stat sbuf;

  dsda_save_dir_strings[0] = strdup(dsda_save_root);

  for (i = 0; i < numwadfiles; ++i) {
    char* start;

    start = strrchr(wadfiles[i].name, '/');
    if (start && strlen(start) > 1) {
      start++; // move past '/'
      length = strlen(start) - 4;

      if (length > 0 && !strcasecmp(start + length, ".wad")) {
        int dir_index;

        if (wadfiles[i].src == 0)
          dir_index = iwad_index;
        else if (wadfiles[i].src == 3)
          dir_index = pwad_index;
        else
          dir_index = -1;

        if (dir_index >= 0 && dir_index < SAVE_DIR_LIMIT) {
          dsda_save_dir_strings[dir_index] = malloc(length + 1);
          strncpy(dsda_save_dir_strings[dir_index], start, length);
          dsda_save_dir_strings[dir_index][length] = '\0';

          for (start = dsda_save_dir_strings[dir_index]; *start; ++start)
            *start = tolower(*start);

          if (dir_index == pwad_index)
            pwad_index++;
        }
      }
    }
  }

  length = strlen(dsda_base_save_dir);
  for (i = 0; i < SAVE_DIR_LIMIT; ++i) {
    if (dsda_save_dir_strings[i])
      length += strlen(dsda_save_dir_strings[i]) + 1; // "/"
  }

  dsda_wad_save_dir = calloc(length + 1, 1); // "\0"

  strcat(dsda_wad_save_dir, dsda_base_save_dir);

  for (i = 0; i < SAVE_DIR_LIMIT; ++i) {
    if (dsda_save_dir_strings[i]) {
      strcat(dsda_wad_save_dir, "/");
      strcat(dsda_wad_save_dir, dsda_save_dir_strings[i]);

      if (!stat(dsda_wad_save_dir, &sbuf) && S_ISDIR(sbuf.st_mode))
        continue;
      else
        dsda_CreateSaveDir();
    }
  }

  lprintf(LO_INFO, "Using save file directory: %s\n", dsda_wad_save_dir);
}

static char* dsda_SaveDir(void) {
  if (dsda_organized_saves) {
    if (!dsda_wad_save_dir)
      dsda_InitWadSaveDir();

    return dsda_wad_save_dir;
  }

  return dsda_base_save_dir;
}

extern const char* savegamename;

char* dsda_SaveGameName(int slot, int demo_save) {
  int length;
  char* name;
  const char* save_dir;
  const char* save_type;

  if (slot > 9999 || slot < 0)
    I_Error("dsda_SaveGameName: bad save slot %d", slot);

  save_dir = dsda_SaveDir();

  save_type = demo_save ? "demosav" : savegamename;

  length = strlen(save_type) + strlen(save_dir) + 10; // "/" + "9999.dsg\0"

  name = malloc(length);

  snprintf(name, length, "%s/%s%d.dsg", save_dir, save_type, slot);

  return name;
}
