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

#ifndef DSDA_TEMPDIR
#define DSDA_TEMPDIR

/**
 * Get a new empty temporary directory.
 *
 * @return The path to the directory.
 * @note The caller owns the pointer and should call Z_Free on it.
 */
char *dsda_GetTempDir(void);

#endif /* DSDA_TEMPDIR */
