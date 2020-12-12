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
//	DSDA Key Frame
//

#include "doomstat.h"
#include "s_advsound.h"
#include "s_sound.h"
#include "p_saveg.h"
#include "p_map.h"
#include "r_draw.h"
#include "r_fps.h"
#include "r_main.h"
#include "g_game.h"
#include "lprintf.h"
#include "e6y.h"

#include "key_frame.h"

#define KEY_FRAME_VERSION 1

// Hook into the save & demo ecosystem
extern FILE* demofp;
extern const byte* demo_p;
extern byte* savebuffer;
extern size_t savegamesize;
extern dboolean setsizeneeded;
struct MapEntry *G_LookupMapinfo(int gameepisode, int gamemap);
void RecalculateDrawnSubsectors(void);

static byte* dsda_key_frame_buffer;

// Stripped down version of G_DoSaveGame
void dsda_StoreKeyFrame(void) {
  long int demofp_p = 0;
  if (demofp != NULL) demofp_p = ftell(demofp);
    
  save_p = savebuffer = malloc(savegamesize);
    
  CheckSaveGame(4);
  *save_p++ = gameskill;
  *save_p++ = gameepisode;
  *save_p++ = gamemap;
  *save_p++ = idmusnum;
  
  // Store progress bar for demo playback
  CheckSaveGame(sizeof(demo_curr_tic));
  memcpy(save_p, &demo_curr_tic, sizeof(demo_curr_tic));
  save_p += sizeof(demo_curr_tic);
  
  // Store location in demo playback buffer
  CheckSaveGame(sizeof(demo_p));
  memcpy(save_p, &demo_p, sizeof(demo_p));
  save_p += sizeof(demo_p);
  
  // Store location in demo file stream
  CheckSaveGame(sizeof(demofp_p));
  memcpy(save_p, &demofp_p, sizeof(demofp_p));
  save_p += sizeof(demofp_p);
  
  CheckSaveGame(sizeof(leveltime));
  memcpy(save_p, &leveltime, sizeof(leveltime));
  save_p += sizeof(leveltime);
  
  CheckSaveGame(sizeof(totalleveltimes));
  memcpy(save_p, &totalleveltimes, sizeof(totalleveltimes));
  save_p += sizeof(totalleveltimes);
  
  CheckSaveGame(1);
  *save_p++ = (gametic - basetic) & 255;
    
  Z_CheckHeap();
  P_ArchivePlayers();
  Z_CheckHeap();
  P_ThinkerToIndex();
  P_ArchiveWorld();
  Z_CheckHeap();
  P_ArchiveThinkers();
  P_IndexToThinker();
  Z_CheckHeap();
  P_ArchiveSpecials();
  P_ArchiveRNG();
  Z_CheckHeap();
  P_ArchiveMap();
  Z_CheckHeap();
    
  if (dsda_key_frame_buffer != NULL) free(dsda_key_frame_buffer);
  
  dsda_key_frame_buffer = savebuffer;
  savebuffer = save_p = NULL;
    
  doom_printf("Stored key frame");
}

// Stripped down version of G_DoLoadGame
// save_p is coopted to use the save logic
void dsda_RestoreKeyFrame(void) {
  long int demofp_p = 0;
  
  if (dsda_key_frame_buffer == NULL) {
    doom_printf("No key frame found");
    return;
  }
  
  save_p = dsda_key_frame_buffer;

  gameskill = *save_p++;
  gameepisode = *save_p++;
  gamemap = *save_p++;
  gamemapinfo = G_LookupMapinfo(gameepisode, gamemap);

  idmusnum = *save_p++;
  if (idmusnum==255) idmusnum=-1;
  
  // Restore progress bar for demo playback
  memcpy(&demo_curr_tic, save_p, sizeof(demo_curr_tic));
  save_p += sizeof(demo_curr_tic);
  
  // Restore location in demo playback buffer
  memcpy(&demo_p, save_p, sizeof(demo_p));
  save_p += sizeof(demo_p);
  
  // Restore location in demo file stream
  memcpy(&demofp_p, save_p, sizeof(demofp_p));
  save_p += sizeof(demofp_p);
  
  if (demofp_p > 0) fseek(demofp, demofp_p, SEEK_SET);

  G_InitNew(gameskill, gameepisode, gamemap);

  memcpy(&leveltime, save_p, sizeof(leveltime));
  save_p += sizeof(leveltime);

  memcpy(&totalleveltimes, save_p, sizeof(totalleveltimes));
  save_p += sizeof(totalleveltimes);
  
  basetic = gametic - *save_p++;

  P_MapStart();
  P_UnArchivePlayers();
  P_UnArchiveWorld();
  P_UnArchiveThinkers();
  P_UnArchiveSpecials();
  P_UnArchiveRNG();
  P_UnArchiveMap();
  P_MapEnd();
  R_ActivateSectorInterpolations();
  R_SmoothPlaying_Reset(NULL);

  if (musinfo.current_item != -1)
    S_ChangeMusInfoMusic(musinfo.current_item, true);

  RecalculateDrawnSubsectors();

  if (setsizeneeded) R_ExecuteSetViewSize();

  R_FillBackScreen();
  
  doom_printf("Restored key frame");
}
