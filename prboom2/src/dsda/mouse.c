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

#include "SDL.h"

#include "dsda/configuration.h"
#include "dsda/features.h"

#include "i_video.h"
#include "mouse.h"

static int quickstart_cache_tics;
static int quickstart_queued;
static signed short angleturn_cache[35];
static unsigned int angleturn_cache_index;

void dsda_InitQuickstartCache(void) {
  quickstart_cache_tics = dsda_IntConfig(dsda_config_quickstart_cache_tics);
}

void dsda_ApplyQuickstartMouseCache(int* mousex) {
  int i;

  if (!quickstart_cache_tics) return;

  if (quickstart_queued) {
    signed short result = 0;

    quickstart_queued = false;

    dsda_TrackFeature(uf_quickstartcache);

    for (i = 0; i < quickstart_cache_tics; ++i)
      result += angleturn_cache[i];

    *mousex = result;
  }
  else {
    angleturn_cache[angleturn_cache_index] = *mousex;
    ++angleturn_cache_index;
    if (angleturn_cache_index >= quickstart_cache_tics)
      angleturn_cache_index = 0;
  }
}

void dsda_QueueQuickstart(void) {
  quickstart_queued = true;
}

void dsda_GetMousePosition(int *x, int *y)
{
  SDL_GetMouseState(x, y);

  // Lets account for HiDPI displays since SDL_GetMouseState doesnt
  *x = (int)(*x * (float)renderer_rect.w / (float)window_rect.w);
  *y = (int)(*y * (float)renderer_rect.h / (float)window_rect.h);

  // x and y currently have the mouse position in the renderer_rect
  // but we want the mouse position in the viewport_rect
  if (viewport_rect.x < *x && *x < (viewport_rect.w + viewport_rect.x) &&
      viewport_rect.y < *y && *y < (viewport_rect.h + viewport_rect.y))
  {
    *x -= viewport_rect.x;
    *y -= viewport_rect.y;
  }
  else
  {
    *x = 0;
    *y = 0;
  }
}
