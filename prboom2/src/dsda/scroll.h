//
// Copyright(C) 2023 by Ryan Krafnick
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
//	DSDA Scroll
//

#ifndef __DSDA_SCROLL__
#define __DSDA_SCROLL__

#include "d_think.h"

#define SCROLL_TOP    0x01
#define SCROLL_MID    0x02
#define SCROLL_BOTTOM 0x04

#define SCROLL_TEXTURE 0x01
#define SCROLL_STATIC  0x02
#define SCROLL_PLAYER  0x04
#define SCROLL_MONSTER 0x08

#define THRUST_STATIC     0x01
#define THRUST_PLAYER     0x02
#define THRUST_MONSTER    0x04
#define THRUST_PROJECTILE 0x08
#define THRUST_WINDTHRUST 0x10

#define THRUST_GROUNDED 0x20
#define THRUST_AIRBORNE 0x40
#define THRUST_CEILING  0x80

#define THRUST_LOCATION_SHIFT 5

typedef struct {
  thinker_t thinker;
  fixed_t dx;
  fixed_t dy;
  int affectee;
  int flags;
} scroll_t;

typedef struct {
  scroll_t scroll;
  int control;
  fixed_t last_height;
  fixed_t vdx;
  fixed_t vdy;
  int accel;
} control_scroll_t;

void dsda_UpdateControlSideScroller(control_scroll_t* s);
void dsda_UpdateSideScroller(scroll_t* s);
void dsda_UpdateControlFloorScroller(control_scroll_t* s);
void dsda_UpdateFloorScroller(scroll_t* s);
void dsda_UpdateControlCeilingScroller(control_scroll_t* s);
void dsda_UpdateCeilingScroller(scroll_t* s);
void dsda_UpdateControlFloorCarryScroller(control_scroll_t* s);
void dsda_UpdateFloorCarryScroller(scroll_t* s);
void dsda_UpdateZDoomFloorScroller(scroll_t* s);
void dsda_UpdateZDoomCeilingScroller(scroll_t* s);
void dsda_UpdateThruster(scroll_t* s);

void dsda_AddSideScroller(fixed_t dx, fixed_t dy, int affectee, int flags);
void dsda_AddControlSideScroller(fixed_t dx, fixed_t dy,
                                 int control, int affectee, int accel, int flags);
void dsda_AddFloorScroller(fixed_t dx, fixed_t dy, int affectee, int flags);
void dsda_AddControlFloorScroller(fixed_t dx, fixed_t dy,
                                  int control, int affectee, int accel, int flags);
void dsda_AddCeilingScroller(fixed_t dx, fixed_t dy, int affectee, int flags);
void dsda_AddControlCeilingScroller(fixed_t dx, fixed_t dy,
                                    int control, int affectee, int accel, int flags);
void dsda_AddFloorCarryScroller(fixed_t dx, fixed_t dy, int affectee, int flags);
void dsda_AddControlFloorCarryScroller(fixed_t dx, fixed_t dy,
                                       int control, int affectee, int accel, int flags);
void dsda_AddZDoomFloorScroller(fixed_t dx, fixed_t dy, int affectee, int flags);
void dsda_AddZDoomCeilingScroller(fixed_t dx, fixed_t dy, int affectee, int flags);
void dsda_AddThruster(fixed_t dx, fixed_t dy, int affectee, int flags);

#endif
