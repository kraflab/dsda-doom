//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

#ifndef HERETIC_DEF_H
#define HERETIC_DEF_H

#include "doomtype.h"

extern dboolean heretic;

// HERETIC_TODO: actually these are from sb_bar
extern int inv_ptr;
extern int curpos;
extern int ArtifactFlash;

// HERETIC_TODO: actually this is in r_draw
extern dboolean BorderTopRefresh;

#define TELEFOGHEIGHT (32*FRACUNIT)
#define ANG1_X          0x01000000

#define FOOTCLIPSIZE	10*FRACUNIT

#define FLOOR_SOLID 0
#define FLOOR_WATER 1
#define FLOOR_LAVA 2
#define FLOOR_SLUDGE 3

#define USE_GWND_AMMO_1 1
#define USE_GWND_AMMO_2 1
#define USE_CBOW_AMMO_1 1
#define USE_CBOW_AMMO_2 1
#define USE_BLSR_AMMO_1 1
#define USE_BLSR_AMMO_2 5
#define USE_SKRD_AMMO_1 1
#define USE_SKRD_AMMO_2 5
#define USE_PHRD_AMMO_1 1
#define USE_PHRD_AMMO_2 1
#define USE_MACE_AMMO_1 1
#define USE_MACE_AMMO_2 5

#define TOCENTER -8

#define BLINKTHRESHOLD (4*32)

#include "dstrings.h"

#endif
