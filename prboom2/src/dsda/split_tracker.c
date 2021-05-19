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
//	DSDA Split Tracker
//

#include <stdlib.h>

#include "dsda.h"
#include "dsda/settings.h"
#include "dsda/data_organizer.h"

#include "split_tracker.h"

static dsda_split_t* dsda_splits;
static size_t dsda_splits_count;
static int run_counter;
static int current_split;
static char* dsda_split_tracker_dir;
static char* dsda_split_tracker_path;

extern int gameskill, gamemap, gameepisode, leveltime, totalleveltimes;

static char* dsda_SplitTrackerDir(void) {
  if (!dsda_split_tracker_dir)
    dsda_split_tracker_dir = dsda_DataDir();

  return dsda_split_tracker_dir;
}

static char* dsda_SplitTrackerPath(void) {
  static char* dsda_split_tracker_path = NULL;

  if (!dsda_split_tracker_path) {
    int length;
    char* name_base;
    char* dir;

    name_base = dsda_DemoNameBase();
    if (!name_base)
      return NULL;

    dir = dsda_SplitTrackerDir();

    length = strlen(dir) + strlen(name_base) + 24;
    dsda_split_tracker_path = malloc(length);

    snprintf(
      dsda_split_tracker_path, "%s/%s_%i_%i_%i_splits.txt",
      length, dir, name_base, gameskill, gamemap, gameepisode
    );
  }

  return dsda_split_tracker_path;
}

static void dsda_LoadSplits(void) {
  char* path;
  byte* buffer;
  int read_result;

  path = dsda_SplitTrackerPath();

  if (!path)
    return;

  if (M_ReadFile(path, &buffer) != -1) {
    // Parse

    free(buffer);
  }
}

static void dsda_InitSplitTime(dsda_split_time_t* split_time) {
  split_time->current = 0;
  split_time->best = 0;
  split_time->best_delta = 0;
  split_time->session_best = 0;
  split_time->session_best_delta = 0;
}

static void dsda_TrackSplitTime(dsda_split_time_t* split_time, int current) {
  split_time->current = current;
  split_time->best_delta = current - split_time->best;
  split_time->session_best_delta = current - split_time->session_best;

  if (current < split_time->best || !split_time->best)
    split_time->best = current;

  if (current < split_time->session_best || !split_time->session_best)
    split_time->session_best = current;
}

void dsda_RecordSplit(void) {
  int i;

  if (!dsda_UseSplitTracker()) return;

  dsda_LoadSplits();

  for (i = 0; i < dsda_splits_count; ++i)
    if (dsda_splits[i].episode == gameepisode && dsda_splits[i].map == gamemap) {
      dsda_splits[i].first_time = 0;
      break;
    }

  if (i == dsda_splits_count) {
    dsda_splits = realloc(dsda_splits, (++dsda_splits_count) * sizeof(dsda_split_t));
    dsda_splits[i].first_time = 1;
    dsda_InitSplitTime(&dsda_splits[i].leveltime);
    dsda_InitSplitTime(&dsda_splits[i].totalleveltimes);
    dsda_splits[i].episode = gameepisode;
    dsda_splits[i].map = gamemap;
  }

  current_split = i;
  dsda_splits[i].run_counter = run_counter;
  dsda_TrackSplitTime(&dsda_splits[i].leveltime, leveltime);
  dsda_TrackSplitTime(&dsda_splits[i].totalleveltimes, totalleveltimes);
}

dsda_split_t* dsda_CurrentSplit(void) {
  if (!dsda_UseSplitTracker()) return NULL;

  return &dsda_splits[current_split];
}

void dsda_ResetSplits(void) {
  if (!dsda_UseSplitTracker()) return;

  ++run_counter;
}
