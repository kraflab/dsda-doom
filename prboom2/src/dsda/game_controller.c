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

static int left_analog_deadzone;
static int right_analog_deadzone;
static int left_trigger_deadzone;
static int right_trigger_deadzone;

static const char* button_names[] = {
  [DSDA_CONTROLLER_BUTTON_A] = "pad a",
  [DSDA_CONTROLLER_BUTTON_B] = "pad b",
  [DSDA_CONTROLLER_BUTTON_X] = "pad x",
  [DSDA_CONTROLLER_BUTTON_Y] = "pad y",
  [DSDA_CONTROLLER_BUTTON_BACK] = "pad back",
  [DSDA_CONTROLLER_BUTTON_GUIDE] = "pad guide",
  [DSDA_CONTROLLER_BUTTON_START] = "pad start",
  [DSDA_CONTROLLER_BUTTON_LEFTSTICK] = "lstick",
  [DSDA_CONTROLLER_BUTTON_RIGHTSTICK] = "rstick",
  [DSDA_CONTROLLER_BUTTON_LEFTSHOULDER] = "pad l",
  [DSDA_CONTROLLER_BUTTON_RIGHTSHOULDER] = "pad r",
  [DSDA_CONTROLLER_BUTTON_DPAD_UP] = "dpad u",
  [DSDA_CONTROLLER_BUTTON_DPAD_DOWN] = "dpad d",
  [DSDA_CONTROLLER_BUTTON_DPAD_LEFT] = "dpad l",
  [DSDA_CONTROLLER_BUTTON_DPAD_RIGHT] = "dpad r",
  [DSDA_CONTROLLER_BUTTON_MISC1] = "misc 1",
  [DSDA_CONTROLLER_BUTTON_PADDLE1] = "paddle 1",
  [DSDA_CONTROLLER_BUTTON_PADDLE2] = "paddle 2",
  [DSDA_CONTROLLER_BUTTON_PADDLE3] = "paddle 3",
  [DSDA_CONTROLLER_BUTTON_PADDLE4] = "paddle 4",
  [DSDA_CONTROLLER_BUTTON_TOUCHPAD] = "touchpad",
  [DSDA_CONTROLLER_BUTTON_TRIGGERLEFT] = "pad lt",
  [DSDA_CONTROLLER_BUTTON_TRIGGERRIGHT] = "pad rt",
};

const char* dsda_GameControllerButtonName(int button) {
  if (button >= sizeof(button_names) || !button_names[button])
    return "misc";

  return button_names[button];
}

static float dsda_AxisValue(SDL_GameControllerButton button, int deadzone) {
  int value;

  value = SDL_GameControllerGetAxis(game_controller, button);

  // the positive axis max is 1 less
  if (value > (deadzone - 1))
    value -= (deadzone - 1);
  else if (value < -deadzone)
    value += deadzone;
  else
    value = 0;

  return (float) value / (32768 - deadzone);
}

static void dsda_PollLeftStick(void) {
  event_t ev;

  ev.type = ev_left_analog;
  ev.data1.f = dsda_AxisValue(SDL_CONTROLLER_AXIS_LEFTX, left_analog_deadzone);
  ev.data2.f = -dsda_AxisValue(SDL_CONTROLLER_AXIS_LEFTY, left_analog_deadzone);

  if (ev.data1.f || ev.data2.f)
    D_PostEvent(&ev);
}

static void dsda_PollRightStick(void) {
  event_t ev;

  ev.type = ev_right_analog;
  ev.data1.f = dsda_AxisValue(SDL_CONTROLLER_AXIS_RIGHTX, right_analog_deadzone);
  ev.data2.f = dsda_AxisValue(SDL_CONTROLLER_AXIS_RIGHTY, right_analog_deadzone);

  if (ev.data1.f || ev.data2.f)
    D_PostEvent(&ev);
}

static void dsda_PollButtons(void) {
  event_t ev;
  float trigger;

  ev.type = ev_joystick;
  ev.data1.i =
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_A) << 0) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_B) << 1) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_X) << 2) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_Y) << 3) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_BACK) << 4) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_GUIDE) << 5) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_START) << 6) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_LEFTSTICK) << 7) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_RIGHTSTICK) << 8) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_LEFTSHOULDER) << 9) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_RIGHTSHOULDER) << 10) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_DPAD_UP) << 11) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_DPAD_DOWN) << 12) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_DPAD_LEFT) << 13) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_DPAD_RIGHT) << 14) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_MISC1) << 15) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_PADDLE1) << 16) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_PADDLE2) << 17) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_PADDLE3) << 18) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_PADDLE4) << 19) |
    (SDL_GameControllerGetButton(game_controller, DSDA_CONTROLLER_BUTTON_TOUCHPAD) << 20);

  trigger = dsda_AxisValue(SDL_CONTROLLER_AXIS_TRIGGERLEFT, left_trigger_deadzone);
  if (trigger)
    ev.data1.i |= (1 << DSDA_CONTROLLER_BUTTON_TRIGGERLEFT);

  trigger = dsda_AxisValue(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, right_trigger_deadzone);
  if (trigger)
    ev.data1.i |= (1 << DSDA_CONTROLLER_BUTTON_TRIGGERRIGHT);

  D_PostEvent(&ev);
}

void dsda_PollGameController(void) {
  if (!game_controller)
    return;

  dsda_PollLeftStick();
  dsda_PollRightStick();
  dsda_PollButtons();
}

void dsda_InitGameControllerParameters(void) {
  left_analog_deadzone = dsda_IntConfig(dsda_config_left_analog_deadzone);
  right_analog_deadzone = dsda_IntConfig(dsda_config_right_analog_deadzone);
  left_trigger_deadzone = dsda_IntConfig(dsda_config_left_trigger_deadzone);
  right_trigger_deadzone = dsda_IntConfig(dsda_config_right_trigger_deadzone);
}

void dsda_InitGameController(void) {
  int num_joysticks;

  game_controller = NULL;
  use_game_controller =
    dsda_IntConfig(dsda_config_use_game_controller) && !dsda_Flag(dsda_arg_nojoy);

  if (!use_game_controller)
    return;

  dsda_InitGameControllerParameters();
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
