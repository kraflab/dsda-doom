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
//	DSDA MapInfo Legacy
//

#ifndef __DSDA_MAPINFO_LEGACY__
#define __DSDA_MAPINFO_LEGACY__

#include "p_mobj.h"

int dsda_LegacyFirstMap(int* episode, int* map);
int dsda_LegacyNewGameMap(int* episode, int* map);
int dsda_LegacyResolveWarp(int arg_p, int* episode, int* map);
int dsda_LegacyNextMap(int* episode, int* map);
int dsda_LegacyShowNextLocBehaviour(int* behaviour);
int dsda_LegacySkipDrawShowNextLoc(int* skip);
void dsda_LegacyUpdateMapInfo(void);
void dsda_LegacyUpdateLastMapInfo(void);
void dsda_LegacyUpdateNextMapInfo(void);
int dsda_LegacyResolveCLEV(int* clev, int* episode, int* map);
int dsda_LegacyResolveINIT(int* init);
int dsda_LegacyMusicIndexToLumpNum(int* lump, int music_index);
int dsda_LegacyMapMusic(int* music_index, int* music_lump);
int dsda_LegacyInterMusic(int* music_index, int* music_lump);
int dsda_LegacyStartFinale(void);
int dsda_LegacyFTicker(void);
void dsda_LegacyFDrawer(void);
int dsda_LegacyBossAction(mobj_t* mo);
int dsda_LegacyHUTitle(const char** title);
int dsda_LegacySkyTexture(int* sky);
int dsda_LegacyPrepareInitNew(void);
int dsda_LegacyPrepareIntermission(int* result);
int dsda_LegacyPrepareFinale(int* result);
void dsda_LegacyLoadMapInfo(void);
int dsda_LegacyExitPic(const char** exit_pic);
int dsda_LegacyEnterPic(const char** enter_pic);
int dsda_LegacyPrepareEntering(void);
int dsda_LegacyPrepareFinished(void);
int dsda_LegacyMapLightning(int* lightning);
int dsda_LegacyApplyFadeTable(void);
int dsda_LegacyMapCluster(int* cluster, int map);
int dsda_LegacySky1Texture(short* texture);
int dsda_LegacySky2Texture(short* texture);
int dsda_LegacyInitSky(void);

#endif
