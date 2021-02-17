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
//	DSDA Input
//

#include <string.h>

#include "input.h"

static int dsda_input_index = 0;
static dsda_input_t dsda_input[DSDA_SEPARATE_CONFIG_COUNT][DSDA_INPUT_IDENTIFIER_COUNT];

typedef struct
{
  dboolean on;
  int activated_at;
  int deactivated_at;
  int game_activated_at;
  int game_deactivated_at;
} dsda_input_state_t;

static int dsda_input_counter; // +1 for each event
static int dsda_input_tick_counter; // +1 for each game tick
static dsda_input_state_t gamekeys[NUMKEYS];
static dsda_input_state_t mousearray[MAX_MOUSE_BUTTONS + 1];
static dsda_input_state_t *mousebuttons = &mousearray[1]; // allow [-1]
static dsda_input_state_t joyarray[MAX_JOY_BUTTONS + 1];
static dsda_input_state_t *joybuttons = &joyarray[1];    // allow [-1]

static void dsda_InputTrackButtons(dsda_input_state_t* buttons, int max, event_t* ev) {
  int i;

  for (i = 0; i < max; ++i) {
    unsigned int button_on = (ev->data1 & (1 << i)) != 0;

    if (!buttons[i].on && button_on)
      buttons[i].activated_at = dsda_input_counter;

    if (buttons[i].on && !button_on)
      buttons[i].deactivated_at = dsda_input_counter;
  }
}

static void dsda_InputTrackKeyDown(event_t* ev) {
  int key = ev->data1;

  if (key >= NUMKEYS || gamekeys[key].on) return;

  gamekeys[key].activated_at = dsda_input_counter;
}

static void dsda_InputTrackKeyUp(event_t* ev) {
  int key = ev->data1;

  if (key >= NUMKEYS || !gamekeys[key].on) return;

  gamekeys[key].deactivated_at = dsda_input_counter;
}

void dsda_InputFlushTick(void) {
  dsda_input_tick_counter = dsda_input_counter;
}

void dsda_InputTrackEvent(event_t* ev) {
  ++dsda_input_counter;

  switch (ev->type)
  {
    case ev_keydown:
      dsda_InputTrackKeyDown(ev);
      break;
    case ev_keyup:
      dsda_InputTrackKeyUp(ev);
      break;
    case ev_mouse:
      dsda_InputTrackButtons(mousebuttons, MAX_MOUSE_BUTTONS, ev);
      break;
    case ev_joystick:
      dsda_InputTrackButtons(joybuttons, MAX_JOY_BUTTONS, ev);
      break;
  }
}

static void dsda_InputTrackGameButtons(dsda_input_state_t* buttons, int max, event_t* ev) {
  int i;

  for (i = 0; i < max; ++i) {
    unsigned int button_on = (ev->data1 & (1 << i)) != 0;

    if (!buttons[i].on && button_on)
      buttons[i].game_activated_at = dsda_input_counter;

    if (buttons[i].on && !button_on)
      buttons[i].game_deactivated_at = dsda_input_counter;

    buttons[i].on = button_on;
  }
}

static void dsda_InputTrackGameKeyDown(event_t* ev) {
  int key = ev->data1;

  if (key >= NUMKEYS || gamekeys[key].on) return;

  gamekeys[key].game_activated_at = dsda_input_counter;
  gamekeys[key].on = true;
}

static void dsda_InputTrackGameKeyUp(event_t* ev) {
  int key = ev->data1;

  if (key >= NUMKEYS || !gamekeys[key].on) return;

  gamekeys[key].game_deactivated_at = dsda_input_counter;
  gamekeys[key].on = false;
}

void dsda_InputTrackGameEvent(event_t* ev) {
  switch (ev->type)
  {
    case ev_keydown:
      dsda_InputTrackGameKeyDown(ev);
      break;
    case ev_keyup:
      dsda_InputTrackGameKeyUp(ev);
      break;
    case ev_mouse:
      dsda_InputTrackGameButtons(mousebuttons, MAX_MOUSE_BUTTONS, ev);
      break;
    case ev_joystick:
      dsda_InputTrackGameButtons(joybuttons, MAX_JOY_BUTTONS, ev);
      break;
  }
}

dboolean dsda_InputActivated(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return
    gamekeys[input->key].activated_at == dsda_input_counter ||
    mousebuttons[input->mouseb].activated_at == dsda_input_counter ||
    joybuttons[input->joyb].activated_at == dsda_input_counter;
}

dboolean dsda_InputTickActivated(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return
    gamekeys[input->key].game_activated_at > dsda_input_tick_counter ||
    mousebuttons[input->mouseb].game_activated_at > dsda_input_tick_counter ||
    joybuttons[input->joyb].game_activated_at > dsda_input_tick_counter;
}

dboolean dsda_InputDeactivated(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return
    !gamekeys[input->key].on &&
    !gamekeys[input->mouseb].on &&
    !gamekeys[input->joyb].on &&
    (
      gamekeys[input->key].deactivated_at == dsda_input_counter ||
      mousebuttons[input->mouseb].deactivated_at == dsda_input_counter ||
      joybuttons[input->joyb].deactivated_at == dsda_input_counter
    );
}

void dsda_InputFlush(void) {
  memset(gamekeys, 0, sizeof(gamekeys));
  memset(mousearray, 0, sizeof(mousearray));
  memset(joyarray, 0, sizeof(joyarray));
  dsda_input_tick_counter = 0;
  dsda_input_counter = 0;
}

dsda_input_t dsda_Input(int identifier) {
  return dsda_input[dsda_input_index][identifier];
}

void dsda_InputCopy(int identifier, dsda_input_t* input) {
  int i;

  for (i = 0; i < DSDA_SEPARATE_CONFIG_COUNT; ++i) {
    input[i] = dsda_input[i][identifier];
  }
}

int dsda_InputKey(int identifier) {
  return dsda_input[dsda_input_index][identifier].key;
}

int dsda_InputMouseB(int identifier) {
  return dsda_input[dsda_input_index][identifier].mouseb;
}

int dsda_InputJoyB(int identifier) {
  return dsda_input[dsda_input_index][identifier].joyb;
}

void dsda_InputReset(int identifier) {
  dsda_input[dsda_input_index][identifier] = (dsda_input_t){ 0, -1, -1 };
}

void dsda_InputSet(int identifier, dsda_input_t input) {
  dsda_input[dsda_input_index][identifier] = input;
}

void dsda_InputSetSpecific(int config_index, int identifier, dsda_input_t input) {
  dsda_input[config_index][identifier] = input;
}

void dsda_InputSetKey(int identifier, int value) {
  dsda_input[dsda_input_index][identifier].key = value;
}

void dsda_InputSetMouseB(int identifier, int value) {
  dsda_input[dsda_input_index][identifier].mouseb = value;
}

void dsda_InputSetJoyB(int identifier, int value) {
  dsda_input[dsda_input_index][identifier].joyb = value;
}

dboolean dsda_InputActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return (input->key         && gamekeys[input->key].on) ||
         (input->mouseb >= 0 && mousebuttons[input->mouseb].on) ||
         (input->joyb >= 0   && joybuttons[input->joyb].on);
}

dboolean dsda_InputKeyActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return input->key && gamekeys[input->key].on;
}

dboolean dsda_InputMouseBActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return input->mouseb >= 0 && mousebuttons[input->mouseb].on;
}

dboolean dsda_InputJoyBActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return input->joyb >= 0 && joybuttons[input->joyb].on;
}
