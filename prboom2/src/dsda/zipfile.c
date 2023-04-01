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

/* Allow a maximum of 1GB to be uncompressed to prevent zip-bombs */
#define UNZIPPED_BYTES_LIMIT 1000000000ULL
static zip_uint64_t total_bytes_read = 0;

#define CHUNK_SIZE 4 * 1024U

static void dsda_WriteContentToFile(zip_file_t *input_file, FILE *dest_file, zip_uint64_t data_size) {
  byte buffer[CHUNK_SIZE];

  while (data_size != 0) {
    zip_uint64_t chunk_size;
    zip_int64_t bytes_read;

    chunk_size = MIN(data_size, CHUNK_SIZE);
    bytes_read = zip_fread(input_file, buffer, chunk_size);
    if (bytes_read == -1) {
      I_Error("dsda_WriteContentToFile: Unable to read data from archive.\n");
    }
    if (fwrite(buffer, sizeof(char), bytes_read, dest_file) != bytes_read) {
      I_Error("dsda_WriteContentToFile: Failed to write data to file.\n");
    }
    data_size -= bytes_read;
    total_bytes_read += bytes_read;
    if (total_bytes_read >= UNZIPPED_BYTES_LIMIT) {
      I_Error("dsda_WriteContentToFile: Too much data to decompress.\n");
    }
  }
}

static void dsda_WriteZippedFilesToDest(zip_t *archive, const char *destination_directory) {
  for (zip_int64_t i = 0; i < zip_get_num_entries(archive, ZIP_FL_UNCHANGED); i++) {
    dsda_string_t full_path;
    zip_file_t *zipped_file;
    zip_stat_t stat;
    FILE *dest_file;
    const char *file_name = zip_get_name(archive, i, ZIP_FL_UNCHANGED);

    dsda_StringPrintF(&full_path, "%s%s", destination_directory, file_name);

    /* Intermediate directories have a trailing '/' */
    if (dsda_HasFileExt(full_path.string, "/")) {
      M_MakeDir(full_path.string, true);
      dsda_FreeString(&full_path);
      continue;
    }

    zip_stat_index(archive, i, ZIP_FL_UNCHANGED, &stat);
    if ((stat.valid & ZIP_STAT_SIZE) == 0) {
      I_Error("dsda_WriteZippedFilesToDest: Failed to read size of zipped file %s.\n", file_name);
    }

    zipped_file = zip_fopen_index(archive, i, ZIP_FL_UNCHANGED);
    if (zipped_file == NULL) {
      I_Error("dsda_WriteZippedFilesToDest: Failed to open zipped file %s.\n", file_name);
    }

    dest_file = M_OpenFile(full_path.string, "wb");
    if (dest_file == NULL) {
      I_Error("dsda_WriteZippedFilesToDest: Failed to open destination file %s.\n", full_path.string);
    }

    dsda_WriteContentToFile(zipped_file, dest_file, stat.size);
    zip_fclose(zipped_file);
    fclose(dest_file);
    dsda_FreeString(&full_path);
  }
}

void dsda_UnzipFile(const char *zipped_file_name, const char *destination_directory) {
  int error_code;
  zip_t *archive_handle;

  archive_handle = zip_open(zipped_file_name, ZIP_RDONLY, &error_code);
  if (archive_handle == NULL) {
    zip_error_t error;
    zip_error_init_with_code(&error, error_code);
    I_Error("dsda_UnzipFile: Unable to open %s: %s.\n", zipped_file_name, zip_error_strerror(&error));
  }
  dsda_WriteZippedFilesToDest(archive_handle, destination_directory);
  zip_close(archive_handle);
}
