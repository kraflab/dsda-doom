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

#include "settings.h"

#include "split_tracker.h"

static dsda_split_t* dsda_splits;
static size_t dsda_splits_count;
static int run_counter;
static int current_split;

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

extern int gamemap, gameepisode, leveltime, totalleveltimes;

void dsda_RecordSplit(void) {
  int i;

  if (!dsda_UseSplitTracker()) return;

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
