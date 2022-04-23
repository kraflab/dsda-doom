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

char** dsda_SplitString(char* str, const char* delimiter) {
  char** result;
  int substring_count = 1;
  char* p = str;

  while (*p)
    if (*p++ == *delimiter)
      ++substring_count;

  result = calloc(substring_count, sizeof(*str));

  if (result) {
    char* token;
    int i = 0;

    token = strtok(str, delimiter);
    while (token) {
      ++i;
      result[i] = token;
      token = strtok(NULL, delimiter);
    }
  }

  return result;
}
