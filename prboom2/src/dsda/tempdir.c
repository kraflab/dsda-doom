//
// Copyright(C) 2022 by Pierre Wendling
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
//	DSDA get temporary directories
//

#include "config.h"

#ifdef HAVE_MKDTEMP
#if defined(HAVE_MKDTEMP_STDLIB)
#if defined(HAVE_MKDTEMP_POSIX_SOURCE)
#define _POSIX_C_SOURCE 200809L
#elif defined(HAVE_MKDTEMP_BSD_SOURCE)
#define _BSD_SOURCE
#elif defined(HAVE_MKDTEMP_DEFAULT_SOURCE)
#define _DEFAULT_SOURCE
#endif
#include <stdlib.h>
#elif defined(HAVE_MKDTEMP_UNISTD)
#include <unistd.h>
#endif
#endif

#ifdef HAVE_GET_TEMP_PATH
/* Mingw requires these two headers first */
#include <tchar.h>
#include <windows.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <strsafe.h>
#if defined(HAVE_GET_TEMP_PATH_2)
#define winapi_GetTempDir GetTempPath2
#elif defined(HAVE_GET_TEMP_PATH_1)
#define winapi_GetTempDir GetTempPath
#else
#error "HAVE_GET_TEMP_PATH is defined but GetTempPath(2) is unavailable"
#endif
#endif /* HAVE_GET_TEMP_PATH */

#include <errno.h>
#include <string.h>

#include "../lprintf.h"
#include "../m_file.h"
#include "../z_zone.h"

#include "tempdir.h"

#if defined(HAVE_MKDTEMP)

static const char *env_to_check[] = {
    "TMPDIR", /* Canonical POSIX name */
    "TEMPDIR",
    "TEMP",
    "TMP",
};

#define ENV_COUNT (sizeof env_to_check / sizeof env_to_check[0])

static const char *posix_GetTempDirPrefix(void) {
  const char *prefix = NULL;

  for (int i = 0; i < ENV_COUNT && prefix == NULL; i++) {
    prefix = M_getenv(env_to_check[i]);
  }

  if (prefix == NULL) {
    prefix = "/tmp";
  }

  return prefix;
}

char *posix_GetTempDirTemplate(void) {
  /* While ten X are recommended, some Linux implementations only support six */
  static const char tempdir_suffix[] = "dsda-doom-XXXXXX";
  const char *tempdir_prefix = posix_GetTempDirPrefix();
  /* We make room for a trailing / but don't add it yet */
  int template_size = snprintf(NULL, 0, "%s%s/", tempdir_prefix, tempdir_suffix) + 1;
  char *template = Z_Malloc(template_size);
  if (template == NULL) {
    lprintf(LO_ERROR, "posix_GetTempDirTemplate: Could not allocate template string.\n");
    return NULL;
  }

  snprintf(template, template_size, "%s%s", tempdir_prefix, tempdir_suffix);
  return template;
}

char *posix_GetTempDir(void) {
  char *result;
  char *template = posix_GetTempDirTemplate();
  if (template == NULL) {
    return NULL;
  }

  result = mkdtemp(template);
  if (result == NULL) {
    lprintf(LO_ERROR, "dsda_GetTempDir: Could not get temporary directory: %s\n", strerror(errno));
    return NULL;
  }

  strcat(result, "/");
  lprintf(LO_DEBUG, "dsda_GetTempDir: Got temporary directory: %s\n", result);
  return result;
}

#elif defined(HAVE_GET_TEMP_PATH)

#define TEMPDIR_NAME_SIZE 22

static char *win32_GetTempDirName(void) {
  DWORD ticks = GetTickCount();
  TCHAR *tempdir_name = Z_Calloc(TEMPDIR_NAME_SIZE, sizeof(TCHAR));
  if (tempdir_name == NULL) {
    lprintf(LO_ERROR, "win32_GetTempDirName: Could not allocate tempdir name.\n");
    return NULL;
  }
  StringCchPrintf(tempdir_name, TEMPDIR_NAME_SIZE - 1, "dsda-doom-%u/", (unsigned int)ticks);
  return tempdir_name;
}

char *win32_GetTempDir(void) {
  TCHAR tempdir[MAX_PATH];
  TCHAR *tempdir_name = NULL;
  char *utf8_tempdir;

  DWORD result = winapi_GetTempDir(MAX_PATH, tempdir);
  if (result == 0 || result > MAX_PATH) {
    lprintf(LO_ERROR, "GetTempPath(2) failed with error code: %u.\n", (unsigned int)GetLastError());
    return NULL;
  }
  tempdir_name = win32_GetTempDirName();
  if (tempdir_name == NULL) {
    return NULL;
  }
  StringCchCat(tempdir, MAX_PATH, tempdir_name);
  Z_Free(tempdir_name);
  if (!CreateDirectory(tempdir, NULL)) {
    lprintf(LO_ERROR, "Createdirectory failed with error code: %u.\n", (unsigned int)GetLastError());
    return NULL;
  }

#if defined(UNICODE) && UNICODE
  utf8_tempdir = Z_Calloc(MB_CUR_MAX, sizeof(char));
  wctomb(utf8_tempdir, tempdir);
#else
  utf8_tempdir = Z_Strdup(tempdir);
#endif
  lprintf(LO_DEBUG, "dsda_GetTempDir: Got temporary directory : %s\n", tempdir);
  return utf8_tempdir;
}

#endif

char *generic_GetTempDir(void) {
  lprintf(LO_INFO, "dsda_GetTempDir: Unable to get a temporary directory, using the current directory.\n");
  return Z_Strdup("./");
}

char *dsda_GetTempDir(void) {
  char *result = NULL;
#if defined(HAVE_MKDTEMP)
  result = posix_GetTempDir();
#elif defined(HAVE_GET_TEMP_PATH)
  result = win32_GetTempDir();
#endif
  if (result == NULL) {
    result = generic_GetTempDir();
  }
  return result;
}
