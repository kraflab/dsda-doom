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

#if !defined(__GNUC__) && !defined(__clang__)
#define __attribute__(x)
#endif

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

// Non-allocated, read-only view into a string, like C++ string_view
typedef struct {
  const char* string;
  size_t size;
} dsda_strview_t;

void dsda_InitString(dsda_string_t* dest, const char* value);
void dsda_FreeString(dsda_string_t* dest);
void dsda_StringCat(dsda_string_t* dest, const char* source);
void dsda_StringCatF(dsda_string_t* dest, const char* format, ...) __attribute__((format(printf,2,3)));
void dsda_StringPrintF(dsda_string_t* dest, const char* format, ...) __attribute__((format(printf,2,3)));
void dsda_TranslateCheckSum(dsda_cksum_t* cksum);
dboolean dsda_HasFileExt(const char* file, const char* ext);
char** dsda_SplitString(char* str, const char* delimiter);
void dsda_FixedToString(char* str, fixed_t x);
dsda_fixed_t dsda_SplitFixed(fixed_t x);
dsda_angle_t dsda_SplitAngle(angle_t x);
void dsda_PrintCommandMovement(char* str, ticcmd_t* cmd);
void dsda_CutExtension(char* str);
const char* dsda_BaseName(const char* str);
const char* dsda_FileExtension(const char* str);
double dsda_DistancePointToLine(fixed_t line_x1, fixed_t line_y1,
                                fixed_t line_x2, fixed_t line_y2,
                                fixed_t point_x, fixed_t point_y);
fixed_t dsda_FixedDistancePointToLine(fixed_t line_x1, fixed_t line_y1,
                                      fixed_t line_x2, fixed_t line_y2,
                                      fixed_t point_x, fixed_t point_y,
                                      fixed_t *closest_x, fixed_t *closest_y);
fixed_t dsda_FloatToFixed(float x);
fixed_t dsda_StringToFixed(const char* x);
byte dsda_FloatToPercent(float x);
int dsda_IntToFixed(int x);
angle_t dsda_DegreesToAngle(float x);

static inline void dsda_StrViewInit(dsda_strview_t* sv, const char* string, size_t size)
{
  sv->string = string;
  sv->size = size;
}

static inline dboolean dsda_StrViewIsEmpty(const dsda_strview_t* sv)
{
  return sv->size == 0;
}

static inline void dsda_StrViewOffset(const dsda_strview_t* sv, size_t offset, dsda_strview_t* ofs)
{
  if (offset > sv->size)
  {
    dsda_StrViewInit(ofs, NULL, 0);
    return;
  }

  ofs->string = sv->string + offset;
  ofs->size = sv->size - offset;
}

// Splits `sv` at the first instance of `c`, putting everything up to and including it in `before`
// and everything after in `after`.  `before` or `after` may safely alias `sv`.  If `c` does not
// occur in `sv`, `before` is set to `sv` and `after` is set to empty.  Returns `true` if
// an occurence was found.
dboolean dsda_StrViewSplitAfterChar(const dsda_strview_t* sv, char c,
                                    dsda_strview_t* before,
                                    dsda_strview_t* after);

// Like the above, except `c` is included in `after` if found
dboolean dsda_StrViewSplitBeforeChar(const dsda_strview_t* sv, char c,
                                     dsda_strview_t* before,
                                     dsda_strview_t* after);

// Advances `sv` past the next line and sets `line` to it, including any trailing '\r'
// or '\n'.  If the string does not end with a '\n`, `line` is set to `sv` and `sv`
// is emptied.  Returns `false` if there was no more data, i.e. `sv` was empty.
static inline dboolean dsda_StrViewNextLine(dsda_strview_t* sv, dsda_strview_t* line)
{
  if (dsda_StrViewIsEmpty(sv))
  {
    dsda_StrViewInit(line, NULL, 0);
    return false;
  }

  dsda_StrViewSplitAfterChar(sv, '\n', line, sv);
  return true;
}

static inline dboolean dsda_StrViewStartsWithCStr(const dsda_strview_t* sv, const char* prefix)
{
  size_t len = strlen(prefix);
  return sv->size >= len && !memcmp(sv->string, prefix, len);
}

// Removes any character in the C string `chars` from the start of `sv`, outputting `trim`,
// which may safely alias `sv`.
void dsda_StrViewTrimStartCStr(const dsda_strview_t* sv, const char* chars, dsda_strview_t* trim);

#define DO_ONCE { static int do_once = true; if (do_once) {
#define END_ONCE do_once = false; } }

#endif
