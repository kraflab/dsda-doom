//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Mouse
//

#ifndef __DSDA_MOUSE__
#define __DSDA_MOUSE__

#include "d_ticcmd.h"

void dsda_ApplyQuickstartMouseCache(int* mousex);
void dsda_QueueQuickstart(void);
void dsda_GetMousePosition(int *x, int *y);

#endif
