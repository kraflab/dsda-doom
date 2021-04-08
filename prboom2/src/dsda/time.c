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

// [XA] MSVC hack: need to implement a handful of functions here.
// this was yoinked from https://stackoverflow.com/a/31335254 so
// no guarantees on if this stuff is accurate/kosher/whatever,
// so maybe don't cut an official release with MSVC just yet.
// it's enough to hack it into compiling for me, at least. ;)

#ifdef _MSC_VER

#include <windows.h>

#define CLOCK_MONOTONIC -1
#define exp7           10000000i64 //1E+7
#define exp9         1000000000i64 //1E+9
#define w2ux 116444736000000000i64 //1.jan1601 to 1.jan1970

void unix_time(struct timespec *spec) {
  __int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
  wintime -= w2ux;  spec->tv_sec = wintime / exp7;
  spec->tv_nsec = wintime % exp7 * 100;
}

int clock_gettime(int _skip, struct timespec *spec)
{
  static struct timespec startspec;
  static double ticks2nano;
  static __int64 startticks, tps = 0;
  __int64 tmp, curticks;

  QueryPerformanceFrequency((LARGE_INTEGER*)&tmp);

  if (tps != tmp) {
    tps = tmp;
    QueryPerformanceCounter((LARGE_INTEGER*)&startticks);
    unix_time(&startspec); ticks2nano = (double)exp9 / tps;
  }

  QueryPerformanceCounter((LARGE_INTEGER*)&curticks); curticks -= startticks;
  spec->tv_sec = startspec.tv_sec + (curticks / tps);
  spec->tv_nsec = startspec.tv_nsec + (double)(curticks % tps) * ticks2nano;
  if (!(spec->tv_nsec < exp9)) { spec->tv_sec++; spec->tv_nsec -= exp9; }
  return 0;
}

#endif //_MSC_VER

// [XA] END HACK

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
