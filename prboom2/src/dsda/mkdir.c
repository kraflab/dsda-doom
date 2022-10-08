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
//	DSDA mkdir
//

#include <sys/stat.h>

#include "lprintf.h"

#include "mkdir.h"

#define NO_ERROR 0

int dsda_MkDir(const char* path, int require) {
  int error = 0;
  struct stat sbuf;

  if (!stat(path, &sbuf) && S_ISDIR(sbuf.st_mode))
    return NO_ERROR;

  error =
#if defined(_MSC_VER)
    _mkdir(path);
#else
  #if defined(_WIN32)
    mkdir(path);
  #else
    mkdir(path, 0755);
  #endif
#endif

  if (require && error)
    I_Error("Unable to create directory %s (%d)", path, errno);

  return error;
}
