//
// Copyright(C) 2020 by Ryan Krafnick
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
//	DSDA Coordinate Display
//

#ifndef __DSDA_COORDINATE_DISPLAY__
#define __DSDA_COORDINATE_DISPLAY__

#include "r_defs.h"

void dsda_InitCoordinateDisplay(patchnum_t* font);
void dsda_UpdateCoordinateDisplay(void);
void dsda_DrawCoordinateDisplay(void);
void dsda_EraseCoordinateDisplay(void);

#endif
