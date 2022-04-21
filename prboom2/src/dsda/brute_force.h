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

#define MAX_BF_DEPTH 5

typedef enum {
  dsda_bf_x,
  dsda_bf_y,
  dsda_bf_z,
  dsda_bf_momx,
  dsda_bf_momy,
  dsda_bf_speed,
  dsda_bf_damage,
  dsda_bf_rng,
} dsda_bf_attribute_t;

typedef enum {
  dsda_bf_less_than,
  dsda_bf_less_than_or_equal_to,
  dsda_bf_greater_than,
  dsda_bf_greater_than_or_equal_to,
  dsda_bf_equal_to,
  dsda_bf_not_equal_to,
} dsda_bf_operator_t;
