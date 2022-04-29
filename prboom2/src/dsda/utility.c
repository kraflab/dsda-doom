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

#include <stdlib.h>
#include <string.h>

#include "z_zone.h"

#include "utility.h"

char** dsda_SplitString(char* str, const char* delimiter) {
  char** result;
  int substring_count = 2;
  char* p = str;

  while (*p)
    if (*p++ == *delimiter)
      ++substring_count;

  result = calloc(substring_count, sizeof(*result));

  if (result) {
    char* token;
    int i = 0;

    token = strtok(str, delimiter);
    while (token) {
      result[i++] = token;
      token = strtok(NULL, delimiter);
    }
  }

  return result;
}

dsda_fixed_t dsda_SplitFixed(fixed_t x) {
  dsda_fixed_t result;

  result.negative = x < 0;
  result.base = x >> FRACBITS;
  result.frac = x & 0xffff;

  if (result.negative)
    if (result.frac) {
      ++result.base;
      result.frac = 0xffff - result.frac + 1;
    }

  return result;
}

void dsda_FixedToString(char* str, fixed_t x) {
  dsda_fixed_t value;

  value = dsda_SplitFixed(x);

  if (value.frac) {
    if (value.negative && !value.base)
      snprintf(str, FIXED_STRING_LENGTH, "-%i.%05i", value.base, value.frac);
    else
      snprintf(str, FIXED_STRING_LENGTH, "%i.%05i", value.base, value.frac);
  }
  else
    snprintf(str, FIXED_STRING_LENGTH, "%i", value.base);
}

dsda_angle_t dsda_SplitAngle(angle_t x) {
  dsda_angle_t result;

  result.negative = x < 0;
  result.base = x >> 24;
  result.frac = (x >> 16) & 0xff;

  if (result.negative)
    if (result.frac) {
      ++result.base;
      result.frac = 0xff - result.frac + 1;
    }

  return result;
}
