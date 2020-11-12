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

#define TELEFRAG_DAMAGE 10000

int dsda_analysis;
int dsda_track_pacifist;
dboolean dsda_pacifist = true;

void dsda_ReadCommandLine(void) {
  dsda_track_pacifist = M_CheckParm("-track_pacifist");
  dsda_analysis = M_CheckParm("-analysis");
}

void dsda_TrackPacifist(void) {
  if (!dsda_pacifist && dsda_track_pacifist) {
    static dboolean pacifist_note_shown = false;
    
    if (!pacifist_note_shown) {
      pacifist_note_shown = true;
      doom_printf("Not pacifist!");
    }
  }
}

void dsda_WatchDamage(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage) {
  if (((source && source->player) || (inflictor && inflictor->player_damaged_barrel)) && damage != TELEFRAG_DAMAGE) {
    if (target->type == MT_BARREL)
      target->player_damaged_barrel = true;
    else if (!target->player)
      dsda_pacifist = false;
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
  
  fprintf(fstream, "pacifist %d", dsda_pacifist);
  
  fclose(fstream);
  
  return;
}
