//
// Copyright(C) 2022 by Ryan Krafnick
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
//	DSDA Game Controller
//

#include "SDL.h"

#include "d_event.h"
#include "d_main.h"
#include "lprintf.h"

#include "dsda/args.h"
#include "dsda/configuration.h"

#include "game_controller.h"

static int use_game_controller;
static SDL_GameController* game_controller;

#define DEADZONE 1024

static const char* button_names[] = {
  [SDL_CONTROLLER_BUTTON_A] = "pad a",
  [SDL_CONTROLLER_BUTTON_B] = "pad b",
  [SDL_CONTROLLER_BUTTON_X] = "pad x",
  [SDL_CONTROLLER_BUTTON_Y] = "pad y",
  [SDL_CONTROLLER_BUTTON_BACK] = "pad back",
  [SDL_CONTROLLER_BUTTON_GUIDE] = "pad guide",
  [SDL_CONTROLLER_BUTTON_START] = "pad start",
  [SDL_CONTROLLER_BUTTON_LEFTSTICK] = "lstick",
  [SDL_CONTROLLER_BUTTON_RIGHTSTICK] = "rstick",
  [SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = "pad l",
  [SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = "pad r",
  [SDL_CONTROLLER_BUTTON_DPAD_UP] = "dpad u",
  [SDL_CONTROLLER_BUTTON_DPAD_DOWN] = "dpad d",
  [SDL_CONTROLLER_BUTTON_DPAD_LEFT] = "dpad l",
  [SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = "dpad r",
};

const char* dsda_GameControllerButtonName(int button) {
  if (button >= sizeof(button_names))
    return "misc";

  return button_names[button];
}

static float dsda_AxisValue(SDL_GameControllerButton button) {
  int value;

  value = SDL_GameControllerGetAxis(game_controller, button);

  // the positive axis max is 1 less
  if (value > (DEADZONE - 1))
    value -= (DEADZONE - 1);
  else if (value < -DEADZONE)
    value += DEADZONE;
  else
    value = 0;

  return (float) value / (32768 - DEADZONE);
}

static void dsda_PollLeftStick(void) {
  event_t ev;

  ev.type = ev_left_analog;
  ev.data1.f = dsda_AxisValue(SDL_CONTROLLER_AXIS_LEFTX);
  ev.data2.f = -dsda_AxisValue(SDL_CONTROLLER_AXIS_LEFTY);

  if (ev.data1.f || ev.data2.f)
    D_PostEvent(&ev);
}

static void dsda_PollRightStick(void) {
  event_t ev;

  ev.type = ev_right_analog;
  ev.data1.f = dsda_AxisValue(SDL_CONTROLLER_AXIS_RIGHTX);
  ev.data2.f = dsda_AxisValue(SDL_CONTROLLER_AXIS_RIGHTY);

  if (ev.data1.f || ev.data2.f)
    D_PostEvent(&ev);
}

static void dsda_PollTrigger(void) {
  event_t ev;

  ev.type = ev_trigger;
  ev.data1.f = dsda_AxisValue(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
  ev.data2.f = dsda_AxisValue(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

  if (ev.data1.f || ev.data2.f)
    D_PostEvent(&ev);
}

static void dsda_PollButtons(void) {
  event_t ev;

  ev.type = ev_joystick;
  ev.data1.i =
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_A) << 0) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_B) << 1) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_X) << 2) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_Y) << 3) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_BACK) << 4) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_GUIDE) << 5) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_START) << 6) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_LEFTSTICK) << 7) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK) << 8) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) << 9) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) << 10) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_DPAD_UP) << 11) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) << 12) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) << 13) |
    (SDL_GameControllerGetButton(game_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) << 14);

  D_PostEvent(&ev);
}

void dsda_PollGameController(void) {
  dsda_PollLeftStick();
  dsda_PollRightStick();
  dsda_PollTrigger();
  dsda_PollButtons();
}

void dsda_InitGameController(void) {
  int num_joysticks;

  use_game_controller =
    dsda_IntConfig(dsda_config_use_game_controller) && !dsda_Flag(dsda_arg_nojoy);

  if (!use_game_controller)
    return;

  SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

  num_joysticks = SDL_NumJoysticks();

  if (use_game_controller > num_joysticks) {
    lprintf(LO_WARN, "dsda_InitGameController: invalid joystick %d\n",
            use_game_controller);
    return;
  }

  if (!SDL_IsGameController(use_game_controller - 1)) {
    lprintf(LO_WARN, "dsda_InitGameController: unsupported joystick %d\n",
            use_game_controller);
    return;
  }

  game_controller = SDL_GameControllerOpen(use_game_controller - 1);

  if (!game_controller) {
    lprintf(LO_ERROR, "dsda_InitGameController: error opening game controller %d\n",
            use_game_controller);
    return;
  }

  lprintf(LO_DEBUG, "Opened game controller %s\n", SDL_GameControllerName(game_controller));
}
