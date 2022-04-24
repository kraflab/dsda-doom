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
//	DSDA Brute Force
//

#include "doomtype.h"

typedef enum {
  dsda_bf_x,
  dsda_bf_y,
  dsda_bf_z,
  dsda_bf_momx,
  dsda_bf_momy,
  dsda_bf_speed,
  dsda_bf_damage,
  dsda_bf_rng,
  dsda_bf_attribute_max,
} dsda_bf_attribute_t;

typedef enum {
  dsda_bf_less_than,
  dsda_bf_less_than_or_equal_to,
  dsda_bf_greater_than,
  dsda_bf_greater_than_or_equal_to,
  dsda_bf_equal_to,
  dsda_bf_not_equal_to,
  dsda_bf_operator_max,
} dsda_bf_operator_t;

extern const char* dsda_bf_attribute_names[dsda_bf_attribute_max];
extern const char* dsda_bf_operator_names[dsda_bf_operator_max];

dboolean dsda_BruteForce(void);
void dsda_ResetBruteForceConditions(void);
void dsda_AddBruteForceCondition(dsda_bf_attribute_t attribute,
                                 dsda_bf_operator_t operator, fixed_t value);
dboolean dsda_StartBruteForce(int depth,
                              int forwardmove_min, int forwardmove_max,
                              int sidemove_min, int sidemove_max,
                              int angleturn_min, int angleturn_max);
void dsda_UpdateBruteForce(void);
void dsda_PopBruteForceCommand(ticcmd_t* cmd);
