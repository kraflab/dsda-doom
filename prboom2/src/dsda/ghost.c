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
FILE *dsda_ghost_import;

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

void dsda_InitGhostImport(const char* name) {
  int version;
  char* filename;
  filename = malloc(strlen(name) + 4 + 1);
  AddDefaultExtension(strcpy(filename, name), ".gst");
  
  dsda_ghost_import = fopen(filename, "rb");
  
  if (dsda_ghost_import == NULL)
    I_Error("dsda_InitGhostImport: failed to open %s", name);
  
  fread(&version, sizeof(int), 1, dsda_ghost_import);
  if (version != DSDA_GHOST_VERSION)
    I_Error("dsda_InitGhostImport: ghost version mismatch");
  
  free(filename);
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
  mobj_t* ghost;
  state_t* ghost_state;
  
  if (dsda_ghost_import == NULL) return;

  ghost = Z_Malloc(sizeof(*ghost), PU_LEVEL, NULL);
  memset(ghost, 0, sizeof(*ghost));
  ghost->type = MT_NULL;
  ghost->info = &dsda_ghost_info;
  ghost->flags = dsda_ghost_info.flags;
  
  ghost->x = players[0].mo->x;
  ghost->y = players[0].mo->y;
  ghost->z = players[0].mo->z;
  ghost->angle = players[0].mo->angle;

  ghost_state = &states[dsda_ghost_info.spawnstate];

  ghost->state  = ghost_state;
  ghost->tics   = ghost_state->tics;
  ghost->sprite = ghost_state->sprite;
  ghost->frame  = ghost_state->frame;
  ghost->touching_sectorlist = NULL;

  P_SetThingPosition(ghost);

  ghost->dropoffz =
  ghost->floorz   = ghost->subsector->sector->floorheight;
  ghost->ceilingz = ghost->subsector->sector->ceilingheight;
  
  ghost->PrevX = ghost->x;
  ghost->PrevY = ghost->y;
  ghost->PrevZ = ghost->z;

  ghost->thinker.function = dsda_UpdateGhost;

  ghost->friction = ORIG_FRICTION;
  ghost->index = -1;

  P_AddThinker(&ghost->thinker);
}

void dsda_UpdateGhost(mobj_t* ghost) {
  dsda_ghost_frame_t ghost_frame;
  int c;
  
  if (dsda_ghost_import == NULL) return;
  
  c = fread(&ghost_frame, sizeof(dsda_ghost_frame_t), 1, dsda_ghost_import);
  
  if (c != 1) {
    dsda_ghost_import = NULL;
    return;
  }
    
  P_UnsetThingPosition(ghost);
  
  ghost->PrevX = ghost->x;
  ghost->PrevY = ghost->y;
  ghost->PrevZ = ghost->z;
  
  ghost->x = ghost_frame.x;
  ghost->y = ghost_frame.y;
  ghost->z = ghost_frame.z;
  ghost->angle = ghost_frame.angle;
  ghost->sprite = ghost_frame.sprite;
  ghost->frame = ghost_frame.frame;
  
  P_SetThingPosition(ghost);
}
