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
//	DSDA MapInfo U
//

#ifndef __DSDA_MAPINFO_U__
#define __DSDA_MAPINFO_U__

#include "p_mobj.h"

int dsda_UNextMap(int* episode, int* map);
int dsda_UShowNextLocBehaviour(int* behaviour);
int dsda_USkipDrawShowNextLoc(int* skip);
void dsda_UUpdateMapInfo(void);
void dsda_UUpdateLastMapInfo(void);
void dsda_UUpdateNextMapInfo(void);
int dsda_UResolveCLEV(int* clev, int* episode, int* map);
int dsda_UMapMusic(int* music_index, int* music_lump);
int dsda_UInterMusic(int* music_index, int* music_lump);
int dsda_UStartFinale(void);
int dsda_UFTicker(void);
void dsda_UFDrawer(void);
int dsda_UBossAction(mobj_t* mo);
int dsda_UHUTitle(const char** title);

#endif
