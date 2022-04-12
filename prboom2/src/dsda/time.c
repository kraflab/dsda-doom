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

#ifdef _WIN32
#include <windows.h>
#else
#include <time.h>
#endif

#include "i_system.h"

#include "time.h"

// lovey: Win32 interface
#ifdef _WIN32

static unsigned long long dsda_time[DSDA_TIMER_COUNT];

void dsda_StartTimer(int timer) {
  QueryPerformanceCounter((LARGE_INTEGER*)&dsda_time[timer]);
}

unsigned long long dsda_ElapsedTime(int timer) {
  static unsigned long long freq = 0;
  unsigned long long now;

  if (!freq) QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

  QueryPerformanceCounter((LARGE_INTEGER*)&now);
  return (now-dsda_time[timer])*1000000ll/freq;
}

// lovey: POSIX interface
#else //_WIN32

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

#endif //_WIN32

unsigned long long dsda_ElapsedTimeMS(int timer) {
  return dsda_ElapsedTime(timer) / 1000;
}

static void dsda_Throttle(int timer, unsigned long long target_time) {
  unsigned long long elapsed_time;
  unsigned long long remaining_time;

  while (1) {
    elapsed_time = dsda_ElapsedTime(timer);

    if (elapsed_time >= target_time) {
      dsda_StartTimer(timer);
      return;
    }

    // Sleeping doesn't have high accuracy
    remaining_time = target_time - elapsed_time;
    if (remaining_time > 1000)
      I_uSleep(remaining_time - 1000);
  }
}

int dsda_fps_limit;

void dsda_LimitFPS(void) {
  extern int movement_smooth;

  if (movement_smooth && dsda_fps_limit) {
    unsigned long long target_time;

    target_time = 1000000 / dsda_fps_limit;

    dsda_Throttle(dsda_timer_fps, target_time);
  }
}

#define TICRATE 35

int dsda_RealticClockRate(void);

static unsigned long long dsda_RealTime(void) {
  static dboolean started = false;

  if (!started)
  {
    started = true;
    dsda_StartTimer(dsda_timer_realtime);
  }

  return dsda_ElapsedTime(dsda_timer_realtime);
}

static unsigned long long dsda_ScaledTime(void) {
  return dsda_RealTime() * dsda_RealticClockRate() / 100;
}

extern int ms_to_next_tick;

// During a fast demo, each call yields a new tick
static int dsda_GetTickFastDemo(void)
{
  static int tick;
  return tick++;
}

int dsda_GetTickRealTime(void) {
  int i;
  unsigned long long t;

  t = dsda_RealTime();

  i = t * TICRATE / 1000000;
  ms_to_next_tick = (i + 1) * 1000 / TICRATE - t / 1000;
  if (ms_to_next_tick > 1000 / TICRATE) ms_to_next_tick = 1;
  if (ms_to_next_tick < 1) ms_to_next_tick = 0;
  return i;
}

static int dsda_TickMS(int n) {
  return n * 1000 * 100 / dsda_RealticClockRate() / TICRATE;
}

static int dsda_GetTickScaledTime(void) {
  int i;
  unsigned long long t;

  t = dsda_RealTime();

  i = t * TICRATE * dsda_RealticClockRate() / 100 / 1000000;
  ms_to_next_tick = dsda_TickMS(i + 1) - t / 1000;
  if (ms_to_next_tick > dsda_TickMS(1)) ms_to_next_tick = 1;
  if (ms_to_next_tick < 1) ms_to_next_tick = 0;
  return i;
}

// During a fast demo, no time elapses in between ticks
static unsigned long long dsda_TickElapsedTimeFastDemo(void) {
  return 0;
}

static unsigned long long dsda_TickElapsedRealTime(void) {
  int tick = dsda_GetTick();

  return dsda_RealTime() - (unsigned long long) tick * 1000000 / TICRATE;
}

static unsigned long long dsda_TickElapsedScaledTime(void) {
  int tick = dsda_GetTick();

  return dsda_ScaledTime() - (unsigned long long) tick * 1000000 / TICRATE;
}

int (*dsda_GetTick)(void) = dsda_GetTickRealTime;
unsigned long long (*dsda_TickElapsedTime)(void) = dsda_TickElapsedRealTime;

void dsda_ResetTimeFunctions(int fastdemo) {
  if (fastdemo) {
    dsda_GetTick = dsda_GetTickFastDemo;
    dsda_TickElapsedTime = dsda_TickElapsedTimeFastDemo;
  }
  else if (dsda_RealticClockRate() != 100) {
    dsda_GetTick = dsda_GetTickScaledTime;
    dsda_TickElapsedTime = dsda_TickElapsedScaledTime;
  }
  else {
    dsda_GetTick = dsda_GetTickRealTime;
    dsda_TickElapsedTime = dsda_TickElapsedRealTime;
  }
}
