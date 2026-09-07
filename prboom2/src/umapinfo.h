//-----------------------------------------------------------------------------
//
// Copyright 2017 Christoph Oelckers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
//-----------------------------------------------------------------------------

#ifndef __UMAPINFO_H
#define __UMAPINFO_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "r_defs.h"

typedef enum MapinfoFlags
{
	MapInfo_LabelClear = (1u << 0),

	MapInfo_EndGameClear = (1u << 1),
	MapInfo_EndGameArt = (1u << 2),
	MapInfo_EndGameStandard = (1u << 3),
	MapInfo_EndGameCast = (1u << 4),
	MapInfo_EndGameBunny = (1u << 5),

	MapInfo_NoIntermission = (1u << 6),
	MapInfo_InterTextClear = (1u << 7),
	MapInfo_InterTextSecretClear = (1u << 8),

	MapInfo_BossActionClear = (1u << 9),

	MapInfo_Jumping = (1u << 10),
	MapInfo_FreeAim = (1u << 11),
	MapInfo_Crouching = (1u << 12),

	MapInfo_EX_VerticalExplosionThrust = (1u << 13),
	MapInfo_EX_ExplodeIn3D = (1u << 14),

	MapInfo_EndGameAny = (MapInfo_EndGameArt | MapInfo_EndGameStandard |
                        MapInfo_EndGameCast | MapInfo_EndGameBunny),
} UMapinfoFlags;

struct BossAction
{
	dboolean is_param;
	int type;
	int special;
	int args[LINE_ARG_COUNT];
};

typedef enum PlayerMovement
{
  PM_Unset,
  PM_Disallow,
  PM_Allow,
  PM_Require,
} PlayerMovement;

struct MapEntry
{
	char *lumpname;
	char *levelname;
	char *label;
	char *author;
	char *intertext;
	char *intertextsecret;
	char levelpic[9];
	char nextmap[9];
	char nextsecret[9];
	char music[9];
	char skytexture[9];
	char endpic[9];
	char exitpic[9];
	char enterpic[9];
	char interbackdrop[9];
	char intermusic[9];
	int partime;
	int flags;

	int numbossactions;
	struct BossAction *bossactions;
};

struct MapList
{
	unsigned int mapcount;
	struct MapEntry *maps;
};

typedef void (*umapinfo_errorfunc)(const char *fmt, ...);	// this must not return!

extern struct MapList Maps;

int ParseUMapInfo(const unsigned char *buffer, size_t length, umapinfo_errorfunc err);
void FreeMapList();
struct MapProperty *FindProperty(struct MapEntry *map, const char *name);

#ifdef __cplusplus
}
#endif

#endif
