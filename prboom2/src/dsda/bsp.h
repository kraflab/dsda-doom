//
// Copyright(C) 2023 Brian Koropoff
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
//	BSP Analysis
//

#ifndef __DSDA_BSP__
#define __DSDA_BSP__

#include "r_defs.h"

extern iseg_t* isegs;
extern unsigned int numisegs;
extern int* adjacency;
extern unsigned int numadjacency;
extern fixed_t mapbox[4];

static inline iseg_t* dsda_ISeg(int id)
{
  return id == NO_ISEG ? NULL : &isegs[id];
}

void dsda_AnnotateBSP(void);
void dsda_ClearBSP(void);

#endif
