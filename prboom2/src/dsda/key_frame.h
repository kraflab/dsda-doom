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

void dsda_InitKeyFrame(void);
void dsda_ContinueKeyFrame(void);
void dsda_StoreKeyFrame(unsigned char** buffer, byte complete);
void dsda_RestoreKeyFrame(unsigned char* buffer, byte complete);
int dsda_KeyFrameRestored(void);
void dsda_StoreQuickKeyFrame(void);
void dsda_RestoreQuickKeyFrame(void);
void dsda_RewindAutoKeyFrame(void);
void dsda_ResetAutoKeyFrameTimeout(void);
void dsda_UpdateAutoKeyFrames(void);

#endif
