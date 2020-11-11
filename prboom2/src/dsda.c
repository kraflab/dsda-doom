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
//	DSDA Tools
//

#include <stdio.h>

#include "m_argv.h"
#include "p_inter.h"
#include "g_game.h"

#include "dsda.h"

int dsda_analysis;
int dsda_track_pacifist;

void dsda_ReadCommandLine(void) {
  dsda_track_pacifist = M_CheckParm("-track_pacifist");
  dsda_analysis = M_CheckParm("-analysis");
}

void dsda_TrackPacifist(void) {
  if (!pacifist && dsda_track_pacifist) {
    static dboolean pacifist_note_shown = false;
    
    if (!pacifist_note_shown)
    {
      pacifist_note_shown = true;
      doom_printf("Not pacifist!");
    }
  }
}

void dsda_WriteAnalysis(void) {
  FILE *fstream = NULL;
  
  if (!dsda_analysis) return;
  
  fstream = fopen("analysis.txt", "w");
  
  if (fstream == NULL) {
    fprintf(stderr, "Unable to open analysis.txt for writing!\n");
    return;
  }
  
  fprintf(fstream, "pacifist %d", pacifist);
  
  fclose(fstream);
  
  return;
}
