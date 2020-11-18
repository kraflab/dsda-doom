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
//	DSDA Tools
//

#ifndef __DSDA__
#define __DSDA__

void dsda_ReadCommandLine(void);
void dsda_TrackPacifist(void);
void dsda_WatchCrush(mobj_t* thing, int damage);
void dsda_WatchDamage(mobj_t* target, mobj_t* inflictor, mobj_t* source, int damage);
void dsda_WatchIconSpawn(mobj_t* spawned);
void dsda_WatchLevelCompletion(void);
void dsda_WatchWeaponFire(weapontype_t weapon);
void dsda_WriteAnalysis(void);

#endif
