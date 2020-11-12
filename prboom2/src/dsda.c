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
#include "doomstat.h"
#include "p_inter.h"
#include "g_game.h"

#include "dsda.h"

#define TELEFRAG_DAMAGE 10000

int dsda_analysis;
int dsda_track_pacifist;
dboolean dsda_pacifist = true;
dboolean dsda_reality = true;
dboolean dsda_almost_reality = true;

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

  if (target->player) {
    dsda_reality = false;
    
    // "almost reality" means allowing nukage damage
    // we cannot differentiate between crushers and nukage in this scope
    // we account for crushers in dsda_WatchCrush instead
    if (inflictor) dsda_almost_reality = false;
  }
}

void dsda_WatchCrush(mobj_t* thing, int damage) {
  player_t *player;
  
  player = thing->player;
  if (!player) return;
  
  // invincible
  if (
    (damage < 1000 || (!comp[comp_god] && (player->cheats&CF_GODMODE))) \
    && (player->cheats&CF_GODMODE || player->powers[pw_invulnerability])
  ) return;
  
  dsda_almost_reality = false;
}

void dsda_WriteAnalysis(void) {
  FILE *fstream = NULL;
  
  if (!dsda_analysis) return;
  
  fstream = fopen("analysis.txt", "w");
  
  if (fstream == NULL) {
    fprintf(stderr, "Unable to open analysis.txt for writing!\n");
    return;
  }
  
  if (dsda_reality) dsda_almost_reality = false;
  
  fprintf(fstream, "pacifist %d\n", dsda_pacifist);
  fprintf(fstream, "reality %d\n", dsda_reality);
  fprintf(fstream, "almost_reality %d\n", dsda_almost_reality);
  
  fclose(fstream);
  
  return;
}
