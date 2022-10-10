/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *   Joystick handling for Linux
 *
 *-----------------------------------------------------------------------------
 */

#ifndef lint
#endif /* lint */

#include <stdlib.h>

#include "SDL.h"
#include "doomdef.h"
#include "doomtype.h"
#include "d_event.h"
#include "d_main.h"
#include "i_joy.h"
#include "lprintf.h"
#include "i_system.h"

#include "dsda/args.h"
#include "dsda/configuration.h"

static int use_joystick;
static SDL_Joystick *joystick;

static void I_EndJoystick(void)
{
  lprintf(LO_DEBUG, "I_EndJoystick : closing joystick\n");
}

void I_PollJoystick(void)
{
  event_t ev;
  Sint16 axis_value;

  if (!use_joystick || !joystick)
    return;

  ev.type = ev_joystick;
  ev.data1 =
    (SDL_JoystickGetButton(joystick, 0) << 0) |
    (SDL_JoystickGetButton(joystick, 1) << 1) |
    (SDL_JoystickGetButton(joystick, 2) << 2) |
    (SDL_JoystickGetButton(joystick, 3) << 3) |
    (SDL_JoystickGetButton(joystick, 4) << 4) |
    (SDL_JoystickGetButton(joystick, 5) << 5) |
    (SDL_JoystickGetButton(joystick, 6) << 6) |
    (SDL_JoystickGetButton(joystick, 7) << 7) |
    (SDL_JoystickGetButton(joystick, 8) << 8) |
    (SDL_JoystickGetButton(joystick, 9) << 9);
  axis_value = SDL_JoystickGetAxis(joystick, 0) / 3000;
  if (abs(axis_value) < 7)
    axis_value = 0;
  ev.data2 = axis_value;
  axis_value = SDL_JoystickGetAxis(joystick, 1) / 3000;
  if (abs(axis_value) < 7)
    axis_value = 0;
  ev.data3 = axis_value;

  D_PostEvent(&ev);
}

void I_InitJoystick(void)
{
  const char* fname = "I_InitJoystick : ";
  int num_joysticks;

  use_joystick = dsda_IntConfig(dsda_config_use_joystick);

  if (!use_joystick)
    return;

  SDL_InitSubSystem(SDL_INIT_JOYSTICK);

  num_joysticks = SDL_NumJoysticks();

  if (dsda_Flag(dsda_arg_nojoy) || use_joystick > num_joysticks || use_joystick < 0) {
    if (use_joystick > num_joysticks || use_joystick < 0)
      lprintf(LO_WARN, "%sinvalid joystick %d\n", fname, use_joystick);
    else
      lprintf(LO_INFO, "%suser disabled\n", fname);
    return;
  }

  joystick = SDL_JoystickOpen(use_joystick - 1);

  if (!joystick)
    lprintf(LO_ERROR, "%serror opening joystick %d\n", fname, use_joystick);
  else {
    I_AtExit(I_EndJoystick, true, "I_EndJoystick", exit_priority_normal);
    lprintf(LO_INFO, "%sopened %s\n", fname, SDL_JoystickName(joystick));
  }
}
