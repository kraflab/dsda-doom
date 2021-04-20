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
//	DSDA Time
//

#include <time.h>

#include "time.h"

static struct timespec dsda_time[DSDA_TIMER_COUNT];

void dsda_StartTimer(int timer) {
  clock_gettime(CLOCK_MONOTONIC, &dsda_time[timer]);
}

unsigned long long dsda_ElapsedTime(int timer) {
  struct timespec now;

  clock_gettime(CLOCK_MONOTONIC, &now);

  return (now.tv_nsec - dsda_time[timer].tv_nsec) / 1000 +
         (now.tv_sec - dsda_time[timer].tv_sec) * 1000000;
}
