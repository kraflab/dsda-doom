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
//	DSDA MapInfo
//

#ifndef __DSDA_MAPINFO__
#define __DSDA_MAPINFO__

#define WI_SHOW_NEXT_LOC      0x01
#define WI_SHOW_NEXT_DONE     0x02
#define WI_SHOW_NEXT_EPISODAL 0x04

int dsda_NameToMap(const char* name, int* episode, int* map);
void dsda_NextMap(int* episode, int* map);
void dsda_ShowNextLocBehaviour(int* behaviour);
int dsda_SkipDrawShowNextLoc(void);
void dsda_UpdateMapInfo(void);
void dsda_UpdateLastMapInfo(void);
void dsda_UpdateNextMapInfo(void);
int dsda_ResolveCLEV(int* episode, int* map);

#endif
