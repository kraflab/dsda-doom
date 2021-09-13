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

#include "mouse.h"

int dsda_quickstart_cache_tics;
static int quickstart_queued;

void dsda_ApplyQuickstartMouseCache(ticcmd_t* cmd) {
  static signed short angleturn_cache[35];
  static unsigned int angleturn_cache_index;
  int i;

  if (!dsda_quickstart_cache_tics) return;

  if (quickstart_queued) {
    signed short result = 0;

    quickstart_queued = false;

    for (i = 0; i < dsda_quickstart_cache_tics; ++i)
      result += angleturn_cache[i];

    cmd->angleturn = result;
  }
  else {
    angleturn_cache[angleturn_cache_index] = cmd->angleturn;
    ++angleturn_cache_index;
    if (angleturn_cache_index >= dsda_quickstart_cache_tics)
      angleturn_cache_index = 0;
  }
}

void dsda_QueueQuickstart(void) {
  quickstart_queued = true;
}
