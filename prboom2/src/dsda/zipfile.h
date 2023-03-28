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
//	DSDA zipfile support using libzip
//

#ifndef DSDA_ZIPFILE
#define DSDA_ZIPFILE

/**
 * Unzips the file into the destination
 *
 * @param zipped_file_name the name of the zip file
 * @param destination_directory the directory to unpack the content to
 */
void dsda_UnzipFile(const char *zipped_file_name, const char *destination_directory);

#endif /* DSDA_ZIPFILE */
