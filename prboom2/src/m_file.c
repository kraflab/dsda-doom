/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *  External simple file handling.
 *
 *-----------------------------------------------------------------------------*/

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _MSC_VER
#include <io.h>
#endif

#include <sys/stat.h>
#include <errno.h>

#include "lprintf.h"
#include "z_zone.h"

#include "m_file.h"

#define MKDIR_NO_ERROR 0

int M_MakeDir(const char *path, int require) {
  int error = 0;
  struct stat sbuf;

  if (!stat(path, &sbuf) && S_ISDIR(sbuf.st_mode))
    return MKDIR_NO_ERROR;

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

dboolean M_ReadWriteAccess(const char *name)
{
  return !access(name, R_OK | W_OK);
}

dboolean M_ReadAccess(const char *name)
{
  return !access(name, R_OK);
}

dboolean M_WriteAccess(const char *name)
{
  return !access(name, W_OK);
}

FILE* M_OpenFile(const char *name, const char *mode)
{
  return fopen(name, mode);
}

dboolean M_FileExists(const char *name)
{
  FILE* fp;

  fp = M_OpenFile(name, "rb");

  if (fp)
  {
    fclose(fp);

    return true;
  }

  return false;
}

/*
 * M_WriteFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */

dboolean M_WriteFile(char const *name, const void *source, size_t length)
{
  FILE *fp;

  errno = 0;

  if (!(fp = M_OpenFile(name, "wb")))  // Try opening file
    return 0;                          // Could not open file for writing

  length = fwrite(source, 1, length, fp) == (size_t)length;   // Write data
  fclose(fp);

  if (!length)                         // Remove partially written file
    remove(name);

  return length;
}

/*
 * M_ReadFile
 *
 * killough 9/98: rewritten to use stdio and to flash disk icon
 */

int M_ReadFile(char const *name, byte **buffer)
{
  FILE *fp;

  if ((fp = M_OpenFile(name, "rb")))
    {
      size_t length;

      fseek(fp, 0, SEEK_END);
      length = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      *buffer = Z_Malloc(length);
      if (fread(*buffer, 1, length, fp) == length)
        {
          fclose(fp);
          return length;
        }
      fclose(fp);
    }

  /* cph 2002/08/10 - this used to return 0 on error, but that's ambiguous,
   * because we could have a legit 0-length file. So make it -1. */
  return -1;
}

// Same as above, but add null terminator
int M_ReadFileToString(char const *name, char **buffer) {
  FILE *fp;

  if ((fp = M_OpenFile(name, "rb")))
  {
    size_t length;

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    *buffer = Z_Malloc(length + 1);
    if (fread(*buffer, 1, length, fp) == length)
    {
      fclose(fp);
      (*buffer)[length] = '\0';
      return length;
    }
    Z_Free(*buffer);
    *buffer = NULL;
    fclose(fp);
  }

  /* cph 2002/08/10 - this used to return 0 on error, but that's ambiguous,
   * because we could have a legit 0-length file. So make it -1. */
  return -1;
}
