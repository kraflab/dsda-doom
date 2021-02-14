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
} dsda_input_state_t;

static int dsda_input_counter;
static dboolean gamekeydown[NUMKEYS];
static dsda_input_state_t mousearray[MAX_MOUSE_BUTTONS + 1];
static dsda_input_state_t *mousebuttons = &mousearray[1]; // allow [-1]
extern dboolean* joybuttons;

static void dsda_InputTrackMouse(event_t* ev) {
  int i;

  for (i = 0; i < MAX_MOUSE_BUTTONS; ++i) {
    unsigned int button_on = (ev->data1 & (1 << i)) != 0;

    if (!mousebuttons[i].on && button_on)
      mousebuttons[i].activated_at = dsda_input_counter;

    if (mousebuttons[i].on && !button_on)
      mousebuttons[i].deactivated_at = dsda_input_counter;

    mousebuttons[i].on = button_on;
  }
}

void dsda_InputTrackEvent(event_t* ev) {
  ++dsda_input_counter;

  switch (ev->type)
  {
    case ev_mouse:
      dsda_InputTrackMouse(ev);
      break;
  }
}

dboolean dsda_InputMouseBActivated(int identifier) {
  return mousebuttons[
    dsda_input[dsda_input_index][identifier].mouseb
  ].activated_at == dsda_input_counter;
}

dboolean dsda_InputMouseBDeactivated(int identifier) {
  return mousebuttons[
    dsda_input[dsda_input_index][identifier].mouseb
  ].deactivated_at == dsda_input_counter;
}

void dsda_InputFlush(void) {
  memset(gamekeydown, 0, sizeof(gamekeydown));
  memset(mousearray, 0, sizeof(mousearray));
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

  return (input->key         && gamekeydown[input->key]) ||
         (input->mouseb >= 0 && mousebuttons[input->mouseb].on) ||
         (input->joyb >= 0   && joybuttons[input->joyb]);
}

dboolean dsda_InputKeyActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return input->key && gamekeydown[input->key];
}

dboolean dsda_InputMouseBActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return input->mouseb >= 0 && mousebuttons[input->mouseb].on;
}

dboolean dsda_InputJoyBActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return input->joyb >= 0 && joybuttons[input->joyb];
}

void dsda_InputActivateKey(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  if (!input->key) return;

  gamekeydown[input->key] = true;
}

void dsda_InputDeactivateKey(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  if (!input->key) return;

  gamekeydown[input->key] = false;
}

void dsda_InputActivateKeyValue(int key) {
  if (key >= NUMKEYS) return;

  gamekeydown[key] = true;
}

void dsda_InputDeactivateKeyValue(int key) {
  if (key >= NUMKEYS) return;

  gamekeydown[key] = false;
}
