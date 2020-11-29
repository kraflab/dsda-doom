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
//	DSDA Ghost
//

#include <stdio.h>

#include "m_argv.h"
#include "lprintf.h"
#include "doomtype.h"
#include "doomstat.h"
#include "info.h"
#include "p_maputl.h"
#include "p_tick.h"
#include "sounds.h"
#include "z_zone.h"
#include "w_wad.h"

#include "ghost.h"

#define DSDA_GHOST_VERSION 1

typedef struct {
  fixed_t x;
  fixed_t y;
  fixed_t z;
  angle_t angle;
  spritenum_t sprite;
  int frame;
  int map;
  int episode;
  int tic;
} dsda_ghost_frame_t;

typedef struct {
  dsda_ghost_frame_t frame;
  mobj_t* mobj;
  FILE* fstream;
} dsda_ghost_t;

typedef struct {
  thinker_t thinker;
  dsda_ghost_t* ghosts;
  int count;
} dsda_ghost_import_t;

mobjinfo_t dsda_ghost_info = {
  -1,            // doomednum
  S_PLAY,        // spawnstate
  0,             // spawnhealth
  S_PLAY_RUN1,   // seestate
  sfx_None,      // seesound
  0,             // reactiontime
  sfx_None,      // attacksound
  S_PLAY_PAIN,   // painstate
  0,             // painchance
  sfx_None,      // painsound
  S_NULL,        // meleestate
  S_PLAY_ATK1,   // missilestate
  S_PLAY_DIE1,   // deathstate
  S_PLAY_XDIE1,  // xdeathstate
  sfx_None,      // deathsound
  0,             // speed
  0,             // radius
  0,             // height
  0,             // mass
  0,             // damage
  sfx_None,      // activesound
  MF_NOBLOCKMAP | MF_TRANSLUCENT, // flags
  S_NULL         // raisestate
};

FILE *dsda_ghost_export;
dsda_ghost_import_t dsda_ghost_import;

void dsda_InitGhostExport(const char* name) {
  int version;
  char* filename;
  filename = malloc(strlen(name) + 4 + 1);
  AddDefaultExtension(strcpy(filename, name), ".gst");
  
  dsda_ghost_export = fopen(filename, "wb");
  
  if (dsda_ghost_export == NULL)
    I_Error("dsda_InitGhostExport: failed to open %s", name);
  
  version = DSDA_GHOST_VERSION;
  fwrite(&version, sizeof(int), 1, dsda_ghost_export);
  
  free(filename);
}

void dsda_InitGhostImport(int option_i) {
  int version;
  char* filename;
  int arg_i;
  int ghost_i;
  
  ghost_i = 0;
    
  arg_i = option_i;
  while (++arg_i != myargc && *myargv[arg_i] != '-') ++dsda_ghost_import.count;
  
  dsda_ghost_import.ghosts = calloc(dsda_ghost_import.count, sizeof(dsda_ghost_t));
  
  arg_i = option_i;
  while (++arg_i != myargc && *myargv[arg_i] != '-') {
    filename = malloc(strlen(myargv[arg_i]) + 4 + 1);
    AddDefaultExtension(strcpy(filename, myargv[arg_i]), ".gst");
    
    dsda_ghost_import.ghosts[ghost_i].fstream = fopen(filename, "rb");
    
    if (dsda_ghost_import.ghosts[ghost_i].fstream == NULL)
      I_Error("dsda_InitGhostImport: failed to open %s", myargv[arg_i]);
    
    fread(&version, sizeof(int), 1, dsda_ghost_import.ghosts[ghost_i].fstream);
    if (version != DSDA_GHOST_VERSION)
      I_Error("dsda_InitGhostImport: ghost version mismatch %s", myargv[arg_i]);
    
    free(filename);
    
    ++ghost_i;
  }
}

void dsda_ExportGhostFrame(void) {
  dsda_ghost_frame_t ghost_frame;
  mobj_t* player;
  
  if (dsda_ghost_export == NULL) return;
  
  if (gametic == 0) return;
  
  player = players[0].mo;
  
  if (player == NULL) return;
  
  ghost_frame.x = player->x;
  ghost_frame.y = player->y;
  ghost_frame.z = player->z;
  ghost_frame.angle = player->angle;
  ghost_frame.sprite = player->sprite;
  ghost_frame.frame = player->frame;
  ghost_frame.map = gamemap;
  ghost_frame.episode = gameepisode;
  ghost_frame.tic = gametic;
  
  fwrite(&ghost_frame, sizeof(dsda_ghost_frame_t), 1, dsda_ghost_export);
}

// Stripped down version of P_SpawnMobj
void dsda_SpawnGhost(void) {
  mobj_t* mobj;
  state_t* ghost_state;
  int ghost_i;
  dboolean any_ghosts;
  
  for (ghost_i = 0; ghost_i < dsda_ghost_import.count; ++ghost_i) {
    if (dsda_ghost_import.ghosts[ghost_i].fstream == NULL) {
      dsda_ghost_import.ghosts[ghost_i].mobj = NULL;
      continue;
    }
    
    mobj = Z_Malloc(sizeof(*mobj), PU_LEVEL, NULL);
    memset(mobj, 0, sizeof(*mobj));
    mobj->type = MT_NULL;
    mobj->info = &dsda_ghost_info;
    mobj->flags = dsda_ghost_info.flags;
    
    mobj->x = players[0].mo->x;
    mobj->y = players[0].mo->y;
    mobj->z = players[0].mo->z;
    mobj->angle = players[0].mo->angle;
    
    ghost_state = &states[dsda_ghost_info.spawnstate];
    
    mobj->state  = ghost_state;
    mobj->tics   = ghost_state->tics;
    mobj->sprite = ghost_state->sprite;
    mobj->frame  = ghost_state->frame;
    mobj->touching_sectorlist = NULL;
    
    P_SetThingPosition(mobj);
    
    mobj->dropoffz =
    mobj->floorz   = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;
    
    mobj->PrevX = mobj->x;
    mobj->PrevY = mobj->y;
    mobj->PrevZ = mobj->z;
    
    mobj->friction = ORIG_FRICTION;
    mobj->index = -1;
    
    dsda_ghost_import.ghosts[ghost_i].mobj = mobj;
  }
  
  if (dsda_ghost_import.count > 0) {
    dsda_ghost_import.thinker.function = dsda_UpdateGhosts;
    P_AddThinker(&dsda_ghost_import.thinker);
  }
}

void dsda_UpdateGhosts(void* _void) {
  dsda_ghost_t* ghost;
  mobj_t* mobj;
  int ghost_i;
  int read_result;
  
  for (ghost_i = 0; ghost_i < dsda_ghost_import.count; ++ghost_i) {
    ghost = &dsda_ghost_import.ghosts[ghost_i];
    
    if (ghost->fstream == NULL) continue;
    
    read_result = fread(&ghost->frame, sizeof(dsda_ghost_frame_t), 1, ghost->fstream);
    
    if (read_result != 1) {
      fclose(ghost->fstream);
      ghost->fstream = NULL;
      return;
    }
    
    mobj = ghost->mobj;
    
    P_UnsetThingPosition(mobj);
    
    mobj->PrevX = mobj->x;
    mobj->PrevY = mobj->y;
    mobj->PrevZ = mobj->z;
    
    mobj->x = ghost->frame.x;
    mobj->y = ghost->frame.y;
    mobj->z = ghost->frame.z;
    mobj->angle = ghost->frame.angle;
    mobj->sprite = ghost->frame.sprite;
    mobj->frame = ghost->frame.frame;
    
    P_SetThingPosition(mobj);
  }
}
