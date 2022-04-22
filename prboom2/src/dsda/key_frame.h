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

#ifndef __DSDA_KEY_FRAME__
#define __DSDA_KEY_FRAME__

#include "doomtype.h"

typedef struct {
  byte* buffer;
  int index;
  int game_tic_count;
} dsda_key_frame_t;

void dsda_StoreKeyFrame(dsda_key_frame_t* key_frame, byte complete);
void dsda_RestoreKeyFrame(dsda_key_frame_t* key_frame);
void dsda_InitKeyFrame(void);
void dsda_ContinueKeyFrame(void);
int dsda_KeyFrameRestored(void);
void dsda_StoreQuickKeyFrame(void);
void dsda_RestoreQuickKeyFrame(void);
dboolean dsda_RestoreClosestKeyFrame(int tic);
void dsda_RewindAutoKeyFrame(void);
void dsda_ResetAutoKeyFrameTimeout(void);
void dsda_UpdateAutoKeyFrames(void);

#endif
