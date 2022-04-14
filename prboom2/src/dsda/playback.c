//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Playback
//

#include "doomstat.h"
#include "g_game.h"

#include "dsda/demo.h"
#include "dsda/input.h"

#include "playback.h"

static const byte* playback_origin_p;
static const byte* playback_p;
static int playback_length;
static int playback_behaviour;
static int playback_tics;

void dsda_AttachPlaybackStream(const byte* demo_p, int length, int behaviour) {
  playback_origin_p = demo_p;
  playback_p = demo_p;
  playback_length = length;
  playback_behaviour = behaviour;
  playback_tics = 0;
}

dboolean dsda_PlaybackStreamAttached(void) {
  return demoplayback || democontinue;
}

int dsda_PlaybackTics(void) {
  return playback_tics;
}

int dsda_PlaybackPositionSize(void) {
  return sizeof(playback_tics) + sizeof(playback_p);
}

void dsda_StorePlaybackPosition(byte** save_p) {
  memcpy(*save_p, &playback_tics, sizeof(playback_tics));
  *save_p += sizeof(playback_tics);

  memcpy(*save_p, &playback_p, sizeof(playback_p));
  *save_p += sizeof(playback_p);
}

void dsda_RestorePlaybackPosition(byte** save_p) {
  memcpy(&playback_tics, *save_p, sizeof(playback_tics));
  *save_p += sizeof(playback_tics);

  memcpy(&playback_p, *save_p, sizeof(playback_p));
  *save_p += sizeof(playback_p);
}

static void dsda_ClearPlaybackStream(void) {
  playback_origin_p = NULL;
  playback_p = NULL;
  playback_length = 0;
  playback_behaviour = 0;
  playback_tics = 0;

  democontinue = false;
  demoplayback = false;
}

static dboolean dsda_EndOfPlaybackStream(void) {
  return *playback_p == DEMOMARKER ||
         playback_p + bytes_per_tic > playback_origin_p + playback_length;
}

void dsda_TryPlaybackOneTick(ticcmd_t* cmd) {
  dboolean ended = false;

  if (!playback_p)
    return;

  if (dsda_EndOfPlaybackStream())
    ended = true;
  else {
    G_ReadOneTick(cmd, &playback_p);

    ++playback_tics;
  }

  if (ended) {
    if (playback_behaviour & PLAYBACK_JOIN_ON_END) {
      dsda_ClearPlaybackStream();
      dsda_JoinDemoCmd(cmd);
    }
    else
      G_CheckDemoStatus();
  }
  else if (dsda_InputActive(dsda_input_join_demo) || dsda_InputJoyBActive(dsda_input_use)) {
    dsda_ClearPlaybackStream();
    dsda_JoinDemoCmd(cmd);
  }
}
