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
//	DSDA Demo
//

#ifndef __DSDA_DEMO__
#define __DSDA_DEMO__

#include "d_ticcmd.h"

void dsda_InitDemo(char* name);
void dsda_WriteToDemo(void* buffer, size_t length);
void dsda_WriteDemoToFile(void);
int dsda_DemoBufferOffset(void);
int dsda_CopyDemoBuffer(void* buffer);
void dsda_SetDemoBufferOffset(int offset);
void dsda_JoinDemoCmd(ticcmd_t* cmd);
const byte* dsda_StripDemoVersion255(const byte* demo_p, const byte* header_p, size_t size);
void dsda_WriteDSDADemoHeader(byte** p);

#endif
