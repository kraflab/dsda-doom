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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "z_zone.h"

#include "utility.h"

void dsda_InitString(dsda_string_t* dest, const char* value) {
  dest->size = 1; // \0
  dest->string = NULL;

  if (value)
    dsda_StringCat(dest, value);
}

void dsda_FreeString(dsda_string_t* dest) {
  Z_Free(dest->string);
  dsda_InitString(dest, NULL);
}

void dsda_StringCat(dsda_string_t* dest, const char* source) {
  if (!source || (!source[0] && dest->string))
    return;

  dest->size += strlen(source);
  if (dest->string)
    dest->string = Z_Realloc(dest->string, dest->size);
  else
    dest->string = Z_Calloc(dest->size, 1);
  strcat(dest->string, source);
}

void dsda_TranslateCheckSum(dsda_cksum_t* cksum) {
  unsigned int i;

  for (i = 0; i < 16; i++)
    sprintf(&cksum->string[i * 2], "%02x", cksum->bytes[i]);
  cksum->string[32] = '\0';
}

dboolean dsda_HasFileExt(const char* file, const char* ext) {
  return strlen(file) > strlen(ext) &&
         !strcasecmp(file + strlen(file) - strlen(ext), ext);
}

char** dsda_SplitString(char* str, const char* delimiter) {
  char** result;
  int substring_count = 2;
  char* p = str;

  while (*p)
    if (*p++ == *delimiter)
      ++substring_count;

  result = Z_Calloc(substring_count, sizeof(*result));

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

  result.base = x >> 24;
  result.frac = (x >> 16) & 0xff;

  return result;
}

void dsda_PrintCommandMovement(char* str, ticcmd_t* cmd) {
  str[0] = '\0';

  if (cmd->forwardmove > 0)
    str += sprintf(str, "MF%2d ", cmd->forwardmove);
  else if (cmd->forwardmove < 0)
    str += sprintf(str, "MB%2d ", -cmd->forwardmove);
  else
    str += sprintf(str, "     ");

  if (cmd->sidemove > 0)
    str += sprintf(str, "SR%2d ", cmd->sidemove);
  else if (cmd->sidemove < 0)
    str += sprintf(str, "SL%2d ", -cmd->sidemove);
  else
    str += sprintf(str, "     ");

  if (cmd->angleturn > 0)
    str += sprintf(str, "TL%2d", cmd->angleturn >> 8);
  else if (cmd->angleturn < 0)
    str += sprintf(str, "TR%2d", -(cmd->angleturn >> 8));
}

void dsda_CutExtension(char* str) {
  char* p;

  p = str + strlen(str);
  while (--p > str && *p != '/' && *p != '\\')
    if (*p == '.') {
      *p = '\0';
      break;
    }
}

static double dsda_DistanceLF(double x1, double y1, double x2, double y2) {
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

double dsda_DistancePointToLine(fixed_t line_x1, fixed_t line_y1,
                                fixed_t line_x2, fixed_t line_y2,
                                fixed_t point_x, fixed_t point_y) {
  double dx, dy;
  double x1, x2, y1, y2, px, py;
  double intersect, intersect_x, intersect_y;

  x1 = (double) line_x1 / FRACUNIT;
  x2 = (double) line_x2 / FRACUNIT;
  y1 = (double) line_y1 / FRACUNIT;
  y2 = (double) line_y2 / FRACUNIT;
  px = (double) point_x / FRACUNIT;
  py = (double) point_y / FRACUNIT;

  if (x1 == x2 && y1 == y2)
    return dsda_DistanceLF(x1, y1, px, py);

  dx = x2 - x1;
  dy = y2 - y1;

  intersect = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);
  intersect = BETWEEN(0, 1, intersect);
  intersect_x = x1 + intersect * dx;
  intersect_y = y1 + intersect * dy;

  return dsda_DistanceLF(intersect_x, intersect_y, px, py);
}
