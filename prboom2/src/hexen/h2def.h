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

#ifndef HEXEN_DEF_H
#define HEXEN_DEF_H

#include "doomtype.h"

extern dboolean hexen;

typedef enum
{
  PCLASS_FIGHTER,
  PCLASS_CLERIC,
  PCLASS_MAGE,
  PCLASS_PIG,
  NUMCLASSES
} pclass_t;

typedef enum
{
  MANA_1,
  MANA_2,
  NUMMANA,
  MANA_BOTH,
  MANA_NONE
} manatype_t;

#define MAX_MANA	200

#define WPIECE1		1
#define WPIECE2		2
#define WPIECE3		4

#endif
