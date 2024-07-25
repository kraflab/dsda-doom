//
// Copyright(C) 2024 by Ryan Krafnick
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
//	DSDA Aim
//

#ifndef __DSDA_AIM__
#define __DSDA_AIM__

#include "d_player.h"

typedef struct {
  angle_t angle;
  fixed_t slope;
  fixed_t z_offset;
} aim_t;

angle_t dsda_PlayerPitch(player_t* player);
fixed_t dsda_PlayerSlope(player_t* player);
int dsda_PitchToLookDir(angle_t pitch);
angle_t dsda_LookDirToPitch(int lookdir);
int dsda_PlayerLookDir(player_t* player);
void dsda_PlayerAim(mobj_t* source, angle_t angle, aim_t* aim, uint64_t target_mask);
void dsda_PlayerAimBad(mobj_t* source, angle_t angle, aim_t* aim, uint64_t target_mask);

#endif
