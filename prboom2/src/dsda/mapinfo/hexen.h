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
//  DSDA MapInfo Hexen
//

#ifndef __DSDA_MAPINFO_HEXEN__
#define __DSDA_MAPINFO_HEXEN__

#include "p_mobj.h"

int dsda_HexenFirstMap(int* episode, int* map);
int dsda_HexenNewGameMap(int* episode, int* map);
int dsda_HexenResolveWarp(int arg_p, int* episode, int* map);
int dsda_HexenNextMap(int* episode, int* map);
int dsda_HexenShowNextLocBehaviour(int* behaviour);
int dsda_HexenSkipDrawShowNextLoc(int* skip);
void dsda_HexenUpdateMapInfo(void);
void dsda_HexenUpdateLastMapInfo(void);
void dsda_HexenUpdateNextMapInfo(void);
int dsda_HexenResolveCLEV(int* clev, int* episode, int* map);
int dsda_HexenResolveINIT(int* init);
int dsda_HexenMusicIndexToLumpNum(int* lump, int music_index);
int dsda_HexenMapMusic(int* music_index, int* music_lump);
int dsda_HexenInterMusic(int* music_index, int* music_lump);
int dsda_HexenStartFinale(void);
int dsda_HexenFTicker(void);
void dsda_HexenFDrawer(void);
int dsda_HexenBossAction(mobj_t* mo);
int dsda_HexenHUTitle(const char** title);
int dsda_HexenSkyTexture(int* sky);
int dsda_HexenPrepareInitNew(void);
int dsda_HexenPrepareIntermission(int* result);
int dsda_HexenPrepareFinale(int* result);
void dsda_HexenLoadMapInfo(void);
int dsda_HexenExitPic(const char** exit_pic);
int dsda_HexenEnterPic(const char** enter_pic);
int dsda_HexenPrepareEntering(void);
int dsda_HexenPrepareFinished(void);
int dsda_HexenMapLightning(int* lightning, int map);
int dsda_HexenApplyFadeTable(void);

#endif
