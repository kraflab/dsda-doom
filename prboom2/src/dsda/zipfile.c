//
// Copyright(C) 2023 by Pierre Wendling
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
//	DSDA zipfile support using libzip
//

#include <stdio.h>
#include <zip.h>

#include "lprintf.h"
#include "m_file.h"
#include "z_zone.h"

#include "dsda/utility.h"

static char *dsda_JoinPath(const char *prefix, const char *suffix) {
  int joined_path_length;
  char *joined_path;

  joined_path_length = snprintf(NULL, 0, "%s%s", prefix, suffix) + 1;
  joined_path = (char *)Z_Malloc(joined_path_length);
  if (joined_path == NULL) {
    lprintf(LO_ERROR, "dsda_JoinPath: Failed to allocate joined path.\n");
    return NULL;
  }
  snprintf(joined_path, joined_path_length, "%s%s", prefix, suffix);
  return joined_path;
}

static void dsda_CreateSubDir(const char *directory, const char *subdirectory) {
  char *full_path;
  int result;

  full_path = dsda_JoinPath(directory, subdirectory);
  if (full_path == NULL) {
    return;
  }
  result = M_MakeDir(full_path, false);
  Z_Free(full_path);
  if (result != 0) {
    lprintf(LO_ERROR, "dsda_CreateSubDir: Failed to create subdirectory %s in %s.\n", subdirectory, directory);
  }
}

static FILE *dsda_CreateFile(const char *directory, const char *filename) {
  FILE *file_handle;
  char *full_path;

  full_path = dsda_JoinPath(directory, filename);
  if (full_path == NULL) {
    return NULL;
  }
  file_handle = M_OpenFile(full_path, "wb");
  Z_Free(full_path);
  return file_handle;
}

static void dsda_WriteContentToFile(zip_file_t *input_file, FILE *dest_file, zip_uint64_t data_size) {
  char *buffer;

  buffer = (char *)Z_Malloc(data_size);
  if (buffer == NULL) {
    lprintf(LO_ERROR, "dsda_WriteContentToFile: Could not allocate temporary buffer.\n");
    return;
  }
  zip_fread(input_file, buffer, data_size);
  fwrite(buffer, sizeof(char), data_size, dest_file);
  Z_Free(buffer);
}

static void dsda_WriteZippedFilesToDest(zip_t *archive, const char *destination_directory) {
  for (zip_int64_t i = 0; i < zip_get_num_entries(archive, ZIP_FL_UNCHANGED); i++) {
    zip_file_t *zipped_file;
    zip_stat_t stat;
    FILE *dest_file_handle;
    const char *file_name = zip_get_name(archive, i, ZIP_FL_UNCHANGED);

    /* Intermediate directories show up as file */
    if (dsda_HasFileExt(file_name, "/")) {
      dsda_CreateSubDir(destination_directory, file_name);
      continue;
    }

    zip_stat_index(archive, i, ZIP_FL_UNCHANGED, &stat);
    if ((stat.valid & ZIP_STAT_SIZE) == 0) {
      lprintf(LO_ERROR, "dsda_WriteZippedFilesToDest: Failed to read size of zipped file %s.\n", file_name);
      return;
    }

    zipped_file = zip_fopen_index(archive, i, ZIP_FL_UNCHANGED);
    if (zipped_file == NULL) {
      lprintf(LO_ERROR, "dsda_WriteZippedFilesToDest: Failed to open zipped file %s.\n", file_name);
      return;
    }

    dest_file_handle = dsda_CreateFile(destination_directory, file_name);
    if (dest_file_handle == NULL) {
      lprintf(LO_ERROR, "dsda_WriteZippedFilesToDest: Failed to create destination file %s%s.\n", destination_directory,
              file_name);
      zip_fclose(zipped_file);
      return;
    }
    dsda_WriteContentToFile(zipped_file, dest_file_handle, stat.size);
    zip_fclose(zipped_file);
    fclose(dest_file_handle);
  }
}

void dsda_UnzipFile(const char *zipped_file_name, const char *destination_directory) {
  int error_code;
  zip_t *archive_handle;

  archive_handle = zip_open(zipped_file_name, ZIP_RDONLY, &error_code);
  if (archive_handle == NULL) {
    zip_error_t error;
    zip_error_init_with_code(&error, error_code);
    lprintf(LO_ERROR, "dsda_UnzipFile: Unable to open %s: %s.\n", zipped_file_name, zip_error_strerror(&error));
    zip_error_fini(&error);
    return;
  }
  dsda_WriteZippedFilesToDest(archive_handle, destination_directory);
  zip_close(archive_handle);
}
