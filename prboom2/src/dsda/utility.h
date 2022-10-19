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
//	DSDA Utility
//

#ifndef __DSDA_UTILITY__
#define __DSDA_UTILITY__

#include <string.h>

#include "d_ticcmd.h"
#include "tables.h"

#define FIXED_STRING_LENGTH 16
#define COMMAND_MOVEMENT_STRING_LENGTH 18

#define ZERO_DATA(item) memset(&item, 0, sizeof(item))

typedef struct {
  dboolean negative;
  int base;
  int frac;
} dsda_fixed_t;

typedef struct {
  int base;
  int frac;
} dsda_angle_t;

typedef struct {
  byte bytes[16];
  char string[33];
} dsda_cksum_t;

typedef struct {
  char* string;
  size_t size;
} dsda_string_t;

void dsda_InitString(dsda_string_t* dest, const char* value);
void dsda_FreeString(dsda_string_t* dest);
void dsda_StringCat(dsda_string_t* dest, const char* source);
void dsda_TranslateCheckSum(dsda_cksum_t* cksum);
dboolean dsda_HasFileExt(const char* file, const char* ext);
char** dsda_SplitString(char* str, const char* delimiter);
void dsda_FixedToString(char* str, fixed_t x);
dsda_fixed_t dsda_SplitFixed(fixed_t x);
dsda_angle_t dsda_SplitAngle(angle_t x);
void dsda_PrintCommandMovement(char* str, ticcmd_t* cmd);
void dsda_CutExtension(char* str);
double dsda_DistancePointToLine(fixed_t line_x1, fixed_t line_y1,
                                fixed_t line_x2, fixed_t line_y2,
                                fixed_t point_x, fixed_t point_y);

#endif
