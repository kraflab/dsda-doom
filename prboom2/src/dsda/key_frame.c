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

#include "dsda.h"
#include "dsda/build.h"
#include "dsda/command_display.h"
#include "dsda/demo.h"
#include "dsda/mapinfo.h"
#include "dsda/options.h"
#include "dsda/pause.h"
#include "dsda/playback.h"
#include "dsda/save.h"
#include "dsda/settings.h"
#include "dsda/time.h"

#include "key_frame.h"

// Hook into the save & demo ecosystem
extern byte* savebuffer;
extern size_t savegamesize;
extern dboolean setsizeneeded;
extern dboolean BorderNeedRefresh;
extern int inv_ptr;
void RecalculateDrawnSubsectors(void);

static dboolean auto_kf_timed_out;
static int auto_kf_timeout_count;

#define TIMEOUT_LIMIT 1

static dsda_key_frame_t first_kf;
static dsda_key_frame_t quick_kf;
static auto_kf_t* auto_key_frames;
static auto_kf_t* last_auto_kf;
static int auto_kf_size;
static int restore_key_frame_index = -1;

int dsda_auto_key_frame_depth;
int dsda_auto_key_frame_interval;
int dsda_auto_key_frame_timeout;

static int autoKeyFrameTimeout(void) {
  return dsda_StartInBuildMode() ? 0 : dsda_auto_key_frame_timeout;
}

static int autoKeyFrameDepth(void) {
  if (dsda_StrictMode())
    return 0;

  return dsda_auto_key_frame_depth;
}

static int autoKeyFrameInterval(void) {
  return dsda_auto_key_frame_interval;
}

static dboolean autoKFExists(auto_kf_t* auto_kf) {
  return auto_kf && auto_kf->auto_index && auto_kf->kf.buffer;
}

void dsda_ForgetAutoKeyFrames(void) {
  if (last_auto_kf)
    last_auto_kf->auto_index = 0;
}

static void dsda_ResetParentKF(dsda_key_frame_t* kf) {
  kf->parent.auto_kf = NULL;
  kf->parent.buffer = NULL;
}

static void dsda_AttachAutoKF(dsda_key_frame_t* kf) {
  if (autoKFExists(last_auto_kf)) {
    kf->parent.auto_kf = last_auto_kf;
    kf->parent.buffer = last_auto_kf->kf.buffer;
  }
  else
    dsda_ResetParentKF(kf);
}

static void dsda_ResolveParentKF(dsda_key_frame_t* kf) {
  if (autoKFExists(kf->parent.auto_kf) && kf->parent.auto_kf->kf.buffer == kf->parent.buffer)
    last_auto_kf = kf->parent.auto_kf;
  else {
    dsda_ResetParentKF(kf);
    dsda_ForgetAutoKeyFrames();
  }
}

static void dsda_RewindKF(auto_kf_t** current) {
  auto_kf_t* auto_kf;

  auto_kf = *current;

  if (auto_kf &&
      auto_kf->auto_index && auto_kf->prev->auto_index &&
      auto_kf->auto_index == auto_kf->prev->auto_index + 1)
    *current = auto_kf->prev;
  else
    *current = NULL;
}

static dsda_key_frame_t* dsda_ClosestKeyFrame(int target_tic_count) {
  dsda_key_frame_t* closest = NULL;

  if (last_auto_kf) {
    auto_kf_t* auto_kf;

    auto_kf = last_auto_kf;
    for (auto_kf = last_auto_kf; auto_kf && auto_kf->kf.buffer; dsda_RewindKF(&auto_kf))
      if (auto_kf->kf.game_tic_count <= target_tic_count)
        if (!closest || auto_kf->kf.game_tic_count > closest->game_tic_count) {
          closest = &auto_kf->kf;
          break;
        }
  }

  if (!demorecording && quick_kf.buffer)
    if (quick_kf.game_tic_count <= target_tic_count)
      if (!closest || quick_kf.game_tic_count > closest->game_tic_count)
        closest = &quick_kf;

  if (first_kf.buffer)
    if (first_kf.game_tic_count <= target_tic_count)
      if (!closest || first_kf.game_tic_count > closest->game_tic_count)
        closest = &first_kf;

  return closest;
}

void dsda_CopyKeyFrame(dsda_key_frame_t* dest, dsda_key_frame_t* source) {
  *dest = *source;
  dest->buffer = malloc(dest->buffer_length);
  memcpy(dest->buffer, source->buffer, dest->buffer_length);
}

void dsda_InitKeyFrame(void) {
  int i;

  auto_kf_size = autoKeyFrameDepth();

  if (!auto_kf_size) {
    last_auto_kf = NULL;
    return;
  }

  if (auto_key_frames != NULL)
    free(auto_key_frames);

  ++auto_kf_size; // chain includes a terminator

  auto_key_frames = calloc(auto_kf_size, sizeof(auto_kf_t));

  auto_key_frames[0].prev = &auto_key_frames[auto_kf_size - 1];
  auto_key_frames[auto_kf_size - 1].next = &auto_key_frames[0];

  for (i = 0; i < auto_kf_size - 1; ++i)
    auto_key_frames[i].next = &auto_key_frames[i + 1];

  for (i = 1; i < auto_kf_size; ++i)
    auto_key_frames[i].prev = &auto_key_frames[i - 1];

  last_auto_kf = &auto_key_frames[auto_kf_size - 1];
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
void dsda_StoreKeyFrame(dsda_key_frame_t* key_frame, byte complete) {
  int i;

  save_p = savebuffer = malloc(savegamesize);

  CheckSaveGame(1);
  *save_p++ = complete;

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

  // Store state of demo playback buffer
  CheckSaveGame(dsda_PlaybackPositionSize());
  dsda_StorePlaybackPosition(&save_p);

  // Store state of demo recording buffer
  CheckSaveGame(dsda_DemoDataSize(complete));
  dsda_StoreDemoData(&save_p, complete);

  CheckSaveGame(sizeof(leveltime));
  memcpy(save_p, &leveltime, sizeof(leveltime));
  save_p += sizeof(leveltime);

  CheckSaveGame(sizeof(totalleveltimes));
  memcpy(save_p, &totalleveltimes, sizeof(totalleveltimes));
  save_p += sizeof(totalleveltimes);

  key_frame->game_tic_count = logictic;

  CheckSaveGame(sizeof(key_frame->game_tic_count));
  memcpy(save_p, &key_frame->game_tic_count, sizeof(key_frame->game_tic_count));
  save_p += sizeof(key_frame->game_tic_count);

  dsda_ArchiveAll();

  if (key_frame->buffer != NULL) free(key_frame->buffer);

  key_frame->buffer = savebuffer;
  key_frame->buffer_length = save_p - savebuffer;
  savebuffer = save_p = NULL;

  dsda_AttachAutoKF(key_frame);

  if (complete) {
    if (demorecording)
      dsda_ExportKeyFrame(key_frame->buffer, key_frame->buffer_length);

    doom_printf("Stored key frame");
  }
}

// Stripped down version of G_DoLoadGame
// save_p is coopted to use the save logic
void dsda_RestoreKeyFrame(dsda_key_frame_t* key_frame, dboolean skip_wipe) {
  int demo_write_buffer_offset, i;
  int epi, map;
  byte complete;

  if (key_frame->buffer == NULL) {
    doom_printf("No key frame found");
    return;
  }

  if (skip_wipe || dsda_BuildMode())
    dsda_SkipNextWipe();

  save_p = key_frame->buffer;

  complete = *save_p++;

  compatibility_level = *save_p++;
  gameskill = *save_p++;

  epi = *save_p++;
  map = *save_p++;
  dsda_UpdateGameMap(epi, map);

  for (i = 0; i < g_maxplayers; i++)
    playeringame[i] = *save_p++;
  save_p += FUTURE_MAXPLAYERS - g_maxplayers;

  idmusnum = *save_p++;
  if (idmusnum == 255) idmusnum = -1;

  save_p += (G_ReadOptions(save_p) - save_p);

  // Restore state of demo playback buffer
  dsda_RestorePlaybackPosition(&save_p);

  // Restore state of demo recording buffer
  dsda_RestoreDemoData(&save_p, complete);

  G_InitNew(gameskill, gameepisode, gamemap, false);

  memcpy(&leveltime, save_p, sizeof(leveltime));
  save_p += sizeof(leveltime);

  memcpy(&totalleveltimes, save_p, sizeof(totalleveltimes));
  save_p += sizeof(totalleveltimes);

  restore_key_frame_index = (totalleveltimes + leveltime) / (35 * autoKeyFrameInterval());

  memcpy(&key_frame->game_tic_count, save_p, sizeof(key_frame->game_tic_count));
  save_p += sizeof(key_frame->game_tic_count);
  basetic = gametic - key_frame->game_tic_count;

  dsda_RestoreCommandHistory();

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

  if (raven)
  {
    players[consoleplayer].readyArtifact = players[consoleplayer].inventory[inv_ptr].type;
  }

  if (setsizeneeded) R_ExecuteSetViewSize();

  R_FillBackScreen();

  BorderNeedRefresh = true;
  ST_Start();

  dsda_QueueJoin();

  dsda_ResolveParentKF(key_frame);

  doom_printf("Restored key frame");
}

void dsda_StoreQuickKeyFrame(void) {
  dsda_StoreKeyFrame(&quick_kf, true);
}

void dsda_RestoreQuickKeyFrame(void) {
  dsda_RestoreKeyFrame(&quick_kf, false);
}

dboolean dsda_RestoreClosestKeyFrame(int tic) {
  dsda_key_frame_t* key_frame;

  key_frame = dsda_ClosestKeyFrame(tic);

  if (!key_frame)
    return false;

  dsda_RestoreKeyFrame(key_frame, true);

  return true;
}

void dsda_RestoreKeyFrameFile(const char* name) {
  char *filename;
  dsda_key_frame_t key_frame = { 0 };

  filename = I_FindFile(name, ".kf");
  if (filename)
  {
    M_ReadFile(filename, &key_frame.buffer);
    free(filename);

    dsda_RestoreKeyFrame(&key_frame, false);
    free(key_frame.buffer);
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
  auto_kf_t* load_kf;

  load_kf = last_auto_kf;
  dsda_RewindKF(&load_kf);

  if (load_kf)
    dsda_RestoreKeyFrame(&load_kf->kf, true);
  else
    doom_printf("No key frame found"); // rewind past the depth limit
}

void dsda_ResetAutoKeyFrameTimeout(void) {
  auto_kf_timed_out = false;
  auto_kf_timeout_count = 0;
}

void dsda_UpdateAutoKeyFrames(void) {
  int key_frame_index;
  int current_time;
  int interval_tics;
  dsda_key_frame_t* current_key_frame;

  if (
    auto_kf_timed_out ||
    auto_kf_size == 0 ||
    gamestate != GS_LEVEL ||
    gameaction != ga_nothing
  ) return;

  current_time = totalleveltimes + leveltime;
  interval_tics = 35 * autoKeyFrameInterval();

  // Automatically save a key frame each interval
  if (current_time % interval_tics == 0) {
    key_frame_index = current_time / interval_tics;

    // Don't duplicate on rewind
    if (key_frame_index == restore_key_frame_index) {
      restore_key_frame_index = -1;
      return;
    }

    last_auto_kf = last_auto_kf->next;
    last_auto_kf->next->auto_index = 0;
    last_auto_kf->auto_index = last_auto_kf->prev->auto_index + 1;

    current_key_frame = &last_auto_kf->kf;

    {
      unsigned long long elapsed_time;

      dsda_StartTimer(dsda_timer_key_frame);
      dsda_StoreKeyFrame(current_key_frame, false);
      elapsed_time = dsda_ElapsedTimeMS(dsda_timer_key_frame);

      if (autoKeyFrameTimeout()) {
        if (elapsed_time > autoKeyFrameTimeout()) {
          ++auto_kf_timeout_count;

          if (auto_kf_timeout_count > TIMEOUT_LIMIT) {
            auto_kf_timed_out = true;
            doom_printf("Slow key framing: rewind disabled");
          }
        }
        else
          auto_kf_timeout_count = 0;
      }
    }

    if (!first_kf.buffer)
      dsda_CopyKeyFrame(&first_kf, current_key_frame);
  }
}
