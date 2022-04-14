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

#include "doomtype.h"

#define PLAYBACK_JOIN_ON_END 1

int dsda_PlaybackArg(void);
void dsda_ExecutePlaybackOptions(void);
int dsda_ParsePlaybackOptions(void);
void dsda_AttachPlaybackStream(const byte* demo_p, int length, int behaviour);
int dsda_PlaybackTics(void);
int dsda_PlaybackPositionSize(void);
void dsda_StorePlaybackPosition(byte** save_p);
void dsda_RestorePlaybackPosition(byte** save_p);
void dsda_TryPlaybackOneTick(ticcmd_t* cmd);
