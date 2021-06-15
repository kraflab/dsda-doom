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

#ifndef __HEXEN_P_ACS__
#define __HEXEN_P_ACS__

#include "r_defs.h"
#include "p_mobj.h"

dboolean P_StartACS(int number, int map, byte * args, mobj_t * activator, line_t * line, int side);
void CheckACSPresent(int number);

#endif
