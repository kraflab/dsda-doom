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

#include <time.h>

#include "doomstat.h"
#include "s_advsound.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "p_saveg.h"
#include "p_map.h"
#include "r_draw.h"
#include "r_fps.h"
#include "r_main.h"
#include "g_game.h"
#include "m_argv.h"
#include "m_misc.h"
#include "i_system.h"
#include "lprintf.h"
#include "e6y.h"

#include "heretic/sb_bar.h"

#include "dsda/demo.h"
#include "dsda/options.h"
#include "dsda/save.h"
#include "dsda/settings.h"
#include "key_frame.h"

// Hook into the save & demo ecosystem
extern const byte* demo_p;
extern byte* savebuffer;
extern size_t savegamesize;
extern dboolean setsizeneeded;
extern dboolean BorderNeedRefresh;
struct MapEntry *G_LookupMapinfo(int gameepisode, int gamemap);
void RecalculateDrawnSubsectors(void);

static byte* dsda_quick_key_frame_buffer;
static int dsda_key_frame_restored;

typedef struct {
  byte* buffer;
  int index;
} dsda_key_frame_t;

static dsda_key_frame_t* dsda_auto_key_frames;
static int dsda_last_auto_key_frame;
static int dsda_auto_key_frames_size;
static int restore_key_frame_index = -1;

void dsda_InitKeyFrame(void) {
  dsda_auto_key_frames_size = dsda_AutoKeyFrameDepth();

  if (dsda_auto_key_frames_size == 0) return;

  if (dsda_auto_key_frames != NULL) free(dsda_auto_key_frames);

  dsda_auto_key_frames =
    calloc(dsda_auto_key_frames_size, sizeof(dsda_key_frame_t));
  dsda_last_auto_key_frame = -1;
}

void dsda_ExportKeyFrame(byte* buffer, int length) {
  char name[40];
  FILE* fp = NULL;
  int timestamp;

  timestamp = totalleveltimes + leveltime;

  snprintf(name, 40, "backup-%010d.kf", timestamp);

  if ((fp = fopen(name, "rb")) != NULL) {
    fclose(fp);
    snprintf(name, 40, "backup-%010d-%lld.kf", timestamp, time(NULL));
  }

  if (!M_WriteFile(name, buffer, length))
    I_Error("dsda_ExportKeyFrame: Failed to write key frame.");
}

// Stripped down version of G_DoSaveGame
void dsda_StoreKeyFrame(byte** buffer, byte complete) {
  int demo_write_buffer_offset, i, length;
  demo_write_buffer_offset = dsda_DemoBufferOffset();

  save_p = savebuffer = malloc(savegamesize);

  CheckSaveGame(5 + FUTURE_MAXPLAYERS);
  *save_p++ = compatibility_level;
  *save_p++ = gameskill;
  *save_p++ = gameepisode;
  *save_p++ = gamemap;

  for (i = 0; i < g_maxplayers; i++)
    *save_p++ = playeringame[i];

  for (; i < FUTURE_MAXPLAYERS; i++)
    *save_p++ = 0;

  *save_p++ = idmusnum;

  CheckSaveGame(dsda_GameOptionSize());
  save_p = G_WriteOptions(save_p);

  // Store progress bar for demo playback
  CheckSaveGame(sizeof(demo_curr_tic));
  memcpy(save_p, &demo_curr_tic, sizeof(demo_curr_tic));
  save_p += sizeof(demo_curr_tic);

  // Store location in demo playback buffer
  CheckSaveGame(sizeof(demo_p));
  memcpy(save_p, &demo_p, sizeof(demo_p));
  save_p += sizeof(demo_p);

  // Store location in demo recording buffer
  CheckSaveGame(sizeof(demo_write_buffer_offset));
  memcpy(save_p, &demo_write_buffer_offset, sizeof(demo_write_buffer_offset));
  save_p += sizeof(demo_write_buffer_offset);

  if (complete && demo_write_buffer_offset) {
    CheckSaveGame(demo_write_buffer_offset);
    save_p += dsda_CopyDemoBuffer(save_p);
  }

  CheckSaveGame(sizeof(leveltime));
  memcpy(save_p, &leveltime, sizeof(leveltime));
  save_p += sizeof(leveltime);

  CheckSaveGame(sizeof(totalleveltimes));
  memcpy(save_p, &totalleveltimes, sizeof(totalleveltimes));
  save_p += sizeof(totalleveltimes);

  CheckSaveGame(1);
  *save_p++ = (gametic - basetic) & 255;

  dsda_ArchiveAll();

  if (*buffer != NULL) free(*buffer);

  length = save_p - savebuffer;

  *buffer = savebuffer;
  savebuffer = save_p = NULL;

  if (complete) {
    if (demo_write_buffer_offset)
      dsda_ExportKeyFrame(*buffer, length);

    doom_printf("Stored key frame");
  }
}

// Stripped down version of G_DoLoadGame
// save_p is coopted to use the save logic
void dsda_RestoreKeyFrame(byte* buffer, byte complete) {
  int demo_write_buffer_offset, i;

  if (buffer == NULL) {
    doom_printf("No key frame found");
    return;
  }

  save_p = buffer;

  compatibility_level = *save_p++;
  gameskill = *save_p++;
  gameepisode = *save_p++;
  gamemap = *save_p++;
  gamemapinfo = G_LookupMapinfo(gameepisode, gamemap);

  for (i = 0; i < g_maxplayers; i++)
    playeringame[i] = *save_p++;
  save_p += FUTURE_MAXPLAYERS - g_maxplayers;

  idmusnum = *save_p++;
  if (idmusnum == 255) idmusnum = -1;

  save_p += (G_ReadOptions(save_p) - save_p);

  // Restore progress bar for demo playback
  memcpy(&demo_curr_tic, save_p, sizeof(demo_curr_tic));
  save_p += sizeof(demo_curr_tic);

  // Restore location in demo playback buffer
  memcpy(&demo_p, save_p, sizeof(demo_p));
  save_p += sizeof(demo_p);

  // Restore location in demo recording buffer
  memcpy(&demo_write_buffer_offset, save_p, sizeof(demo_write_buffer_offset));
  save_p += sizeof(demo_write_buffer_offset);

  if (complete && demo_write_buffer_offset) {
    dsda_SetDemoBufferOffset(0);
    dsda_WriteToDemo(save_p, demo_write_buffer_offset);
    save_p += demo_write_buffer_offset;
  }
  else {
    dsda_SetDemoBufferOffset(demo_write_buffer_offset);
  }

  G_InitNew(gameskill, gameepisode, gamemap);

  memcpy(&leveltime, save_p, sizeof(leveltime));
  save_p += sizeof(leveltime);

  memcpy(&totalleveltimes, save_p, sizeof(totalleveltimes));
  save_p += sizeof(totalleveltimes);

  restore_key_frame_index = (totalleveltimes + leveltime) / (35 * dsda_AutoKeyFrameInterval());

  basetic = gametic - *save_p++;

  dsda_UnArchiveAll();

  R_ActivateSectorInterpolations();
  R_SmoothPlaying_Reset(NULL);

  if (musinfo.current_item != -1)
    S_ChangeMusInfoMusic(musinfo.current_item, true);

  RecalculateDrawnSubsectors();

  if (hexen)
  {
    SB_SetClassData();
  }

  if (setsizeneeded) R_ExecuteSetViewSize();

  R_FillBackScreen();

  BorderNeedRefresh = true;
  ST_Start();

  dsda_key_frame_restored = 1;

  doom_printf("Restored key frame");
}

int dsda_KeyFrameRestored(void) {
  if (!dsda_key_frame_restored) return 0;

  dsda_key_frame_restored = 0;
  return 1;
}

void dsda_StoreQuickKeyFrame(void) {
  dsda_StoreKeyFrame(&dsda_quick_key_frame_buffer, true);
}

void dsda_RestoreQuickKeyFrame(void) {
  dsda_RestoreKeyFrame(dsda_quick_key_frame_buffer, true);
}

void dsda_RestoreKeyFrameFile(const char* name) {
  char *filename;
  byte* buffer;

  filename = I_FindFile(name, ".kf");
  if (filename)
  {
    M_ReadFile(filename, &buffer);
    free(filename);

    dsda_RestoreKeyFrame(buffer, true);
    free(buffer);
  }
  else
    I_Error("dsda_RestoreKeyFrameFile: cannot find %s", name);
}

void dsda_ContinueKeyFrame(void) {
  int p;

  p = M_CheckParm("-from_key_frame");
  if (p && (p + 1 < myargc)) {
    dsda_RestoreKeyFrameFile(myargv[p + 1]);
  }
}

void dsda_RewindAutoKeyFrame(void) {
  int current_time;
  int interval_tics;
  int key_frame_index;
  int history_index;

  if (dsda_auto_key_frames_size == 0) {
    doom_printf("No key frame found");
    return;
  }

  current_time = totalleveltimes + leveltime;
  interval_tics = 35 * dsda_AutoKeyFrameInterval();

  key_frame_index = current_time / interval_tics - 1;

  history_index = dsda_last_auto_key_frame - 1;
  if (history_index < 0) history_index = dsda_auto_key_frames_size - 1;

  if (dsda_auto_key_frames[history_index].index <= key_frame_index) {
    dsda_last_auto_key_frame = history_index;
    dsda_SkipNextWipe();
    dsda_RestoreKeyFrame(dsda_auto_key_frames[history_index].buffer, false);
  }
  else doom_printf("No key frame found"); // rewind past the depth limit
}

void dsda_UpdateAutoKeyFrames(void) {
  int key_frame_index;
  int current_time;
  int interval_tics;
  dsda_key_frame_t* current_key_frame;

  if (
    dsda_auto_key_frames_size == 0 ||
    gamestate != GS_LEVEL ||
    gameaction != ga_nothing
  ) return;

  current_time = totalleveltimes + leveltime;
  interval_tics = 35 * dsda_AutoKeyFrameInterval();

  // Automatically save a key frame each interval
  if (current_time % interval_tics == 0) {
    key_frame_index = current_time / interval_tics;

    // Don't duplicate on rewind
    if (key_frame_index == restore_key_frame_index)
      return;

    dsda_last_auto_key_frame += 1;
    if (dsda_last_auto_key_frame >= dsda_auto_key_frames_size)
      dsda_last_auto_key_frame = 0;

    current_key_frame = &dsda_auto_key_frames[dsda_last_auto_key_frame];
    current_key_frame->index = key_frame_index;

    dsda_StoreKeyFrame(&current_key_frame->buffer, false);
  }
}
