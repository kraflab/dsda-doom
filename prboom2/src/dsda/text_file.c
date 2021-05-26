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
//	DSDA Text File
//

#include "m_argv.h"
#include "doomstat.h"
#include "r_demo.h"
#include "lprintf.h"
#include "e6y.h"

#include "dsda.h"

#include "text_file.h"

const char* dsda_player_name;

extern int gamemap, startmap, gameepisode, gameskill,
           totalleveltimes, dsda_last_leveltime, compatibility_level;

static char* dsda_TextFileName(void) {
  int p;
  int name_length;
  char* name;
  const char* playdemo;

  p = IsDemoPlayback();

  if (!p)
    return NULL;

  playdemo = strdup(myargv[p + 1]);
  name_length = strlen(playdemo);

  if (name_length > 4 && !stricmp(playdemo + name_length - 4, ".lmp")) {
    name = strdup(playdemo);
    name[name_length - 4] = '\0';
  }
  else {
    name = calloc(name_length + 4, 1);
    strcat(name, playdemo);
  }

  strcat(name, ".txt");

  return name;
}

static char* dsda_TextFileTime(void) {
  char* text_file_time;

  text_file_time = malloc(16);

  if (startmap == gamemap - 2)
    snprintf(
      text_file_time,
      16,
      "%d:%05.2f",
      dsda_last_leveltime / 35 / 60,
      (float)(dsda_last_leveltime % (60 * 35)) / 35
    );
  else
    snprintf(
      text_file_time,
      16,
      "%d:%02d",
      totalleveltimes / 35 / 60,
      (totalleveltimes / 35) % 60
    );

  return text_file_time;
}

void dsda_ExportTextFile(void) {
  int p;
  char* name;
  const char* iwad = NULL;
  const char* pwad = NULL;
  FILE* file;

  if (!M_CheckParm("-export_text_file"))
    return;

  name = dsda_TextFileName();

  if (!name)
    I_Error("Cannot export text file without demo playback");

  file = fopen(name, "wb");
  free(name);

  if (!file)
    I_Error("Unable to export text file!");

  p = M_CheckParm("-iwad");
  if (p && (++p < myargc))
    iwad = PathFindFileName(myargv[p]);

  p = M_CheckParm("-file");
  if (p && (++p < myargc))
    pwad = PathFindFileName(myargv[p]);

  fprintf(file, "Doom Speed Demo Archive\n");
  fprintf(file, "https://dsdarchive.com/\n");
  fprintf(file, "\n");
  if (iwad)
    fprintf(file, "Iwad:      %s\n", iwad);
  if (pwad)
    fprintf(file, "Pwad:      %s\n", pwad);

  fprintf(file, "Map:       %s\n", MAPNAME(gameepisode, startmap + 1));
  fprintf(file, "Skill:     %i\n", gameskill + 1);
  fprintf(file, "Category:  %s\n", dsda_DetectCategory());
  fprintf(file, "Exe:       %s -complevel %i\n",
          (PACKAGE_NAME" "PACKAGE_VERSION), compatibility_level);
  fprintf(file, "\n");
  fprintf(file, "Time:      %s\n", dsda_TextFileTime());
  fprintf(file, "\n");
  fprintf(file, "Author:    %s\n", dsda_player_name);
  fprintf(file, "\n");
  fprintf(file, "Comments:\n");

  fclose(file);
}
