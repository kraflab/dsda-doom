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

#include "i_system.h"
#include "lprintf.h"
#include "m_file.h"
#include "z_zone.h"

#include "dsda/utility.h"

static char **temp_dirs;

/* Allow a maximum of 1GB to be uncompressed to prevent zip-bombs */
#define UNZIPPED_BYTES_LIMIT 1000000000ULL

static zip_uint64_t total_bytes_read;

#define CHUNK_SIZE 4 * 1024U

static void dsda_WriteContentToFile(zip_file_t *input_file, FILE *dest_file, zip_uint64_t data_size) {
  byte buffer[CHUNK_SIZE];

  while (data_size != 0) {
    zip_uint64_t chunk_size;
    zip_int64_t bytes_read;

    chunk_size = MIN(data_size, CHUNK_SIZE);
    bytes_read = zip_fread(input_file, buffer, chunk_size);
    if (bytes_read == -1)
      I_Error("dsda_WriteContentToFile: Unable to read data from archive.");

    if (fwrite(buffer, sizeof(char), bytes_read, dest_file) != bytes_read)
      I_Error("dsda_WriteContentToFile: Failed to write data to file.");

    data_size -= bytes_read;
    total_bytes_read += bytes_read;
    if (total_bytes_read >= UNZIPPED_BYTES_LIMIT)
      I_Error("dsda_WriteContentToFile: Too much data to decompress.");
  }
}

static void dsda_WriteZippedFilesToDest(zip_t *archive, const char *destination_directory) {
  zip_int64_t i;

  for (i = 0; i < zip_get_num_entries(archive, ZIP_FL_UNCHANGED); i++) {
    dsda_string_t full_path;
    zip_file_t *zipped_file;
    zip_stat_t stat;
    FILE *dest_file;
    const char *file_name = dsda_BaseName(zip_get_name(archive, i, ZIP_FL_UNCHANGED));

    /* Intermediate directories have a trailing '/', so their base name is empty */
    if (*file_name == '\0') {
      continue;
    }

    dsda_StringPrintF(&full_path, "%s/%s", destination_directory, file_name);

    zip_stat_index(archive, i, ZIP_FL_UNCHANGED, &stat);
    if ((stat.valid & ZIP_STAT_SIZE) == 0)
      I_Error("dsda_WriteZippedFilesToDest: Failed to read size of zipped file %s.", file_name);

    zipped_file = zip_fopen_index(archive, i, ZIP_FL_UNCHANGED);
    if (zipped_file == NULL)
      I_Error("dsda_WriteZippedFilesToDest: Failed to open zipped file %s.", file_name);

    dest_file = M_OpenFile(full_path.string, "wb");
    if (dest_file == NULL)
      I_Error("dsda_WriteZippedFilesToDest: Failed to open destination file %s.", full_path.string);

    dsda_WriteContentToFile(zipped_file, dest_file, stat.size);

    zip_fclose(zipped_file);
    fclose(dest_file);
    dsda_FreeString(&full_path);
  }
}

static void dsda_UnzipFileToDestination(const char *zipped_file_name, const char *destination_directory) {
  int error_code;
  zip_t *archive_handle;

  total_bytes_read = 0;
  archive_handle = zip_open(zipped_file_name, ZIP_RDONLY, &error_code);
  if (archive_handle == NULL) {
    zip_error_t error;
    zip_error_init_with_code(&error, error_code);
    I_Error("dsda_UnzipFileToDestination: Unable to open %s: %s.\n", zipped_file_name, zip_error_strerror(&error));
  }

  dsda_WriteZippedFilesToDest(archive_handle, destination_directory);

  zip_close(archive_handle);
}

const char* dsda_UnzipFile(const char *zipped_file_name) {
  dsda_string_t temporary_directory;
  static unsigned int file_counter = 0;

  dsda_StringPrintF(&temporary_directory, "%s/%u-%s", I_GetTempDir(), file_counter, dsda_BaseName(zipped_file_name));
  if (M_IsDir(temporary_directory.string))
    if (!M_RemoveFilesAtPath(temporary_directory.string))
      I_Error("dsda_UnzipFile: unable to clear tempdir %s\n", temporary_directory.string);
  M_MakeDir(temporary_directory.string, true);

  dsda_UnzipFileToDestination(zipped_file_name, temporary_directory.string);

  temp_dirs = Z_Realloc(temp_dirs, (file_counter + 2) * sizeof(*temp_dirs));
  temp_dirs[file_counter] = temporary_directory.string;
  temp_dirs[file_counter + 1] = NULL;
  file_counter++;

  return temporary_directory.string;
}

void dsda_CleanZipTempDirs(void) {
  int i;

  if(temp_dirs == NULL)
    return;

  for (i = 0; temp_dirs[i] != NULL; i++) {
    M_RemoveFilesAtPath(temp_dirs[i]);
    M_remove(temp_dirs[i]);
    Z_Free(temp_dirs[i]);
  }
  Z_Free(temp_dirs);
}
