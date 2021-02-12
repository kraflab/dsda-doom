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

#include "input.h"

#define DSDA_SEPARATE_CONFIG_COUNT 1

int dsda_input_index = 0;
dsda_input_t dsda_input[DSDA_SEPARATE_CONFIG_COUNT][DSDA_INPUT_IDENTIFIER_COUNT];

extern dboolean gamekeydown[];
extern int* mousebuttons;
extern dboolean* joybuttons;

dsda_input_t dsda_Input(int identifier) {
  return dsda_input[dsda_input_index][identifier];
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
         (input->mouseb >= 0 && mousebuttons[input->mouseb]) ||
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

  return input->mouseb >= 0 && mousebuttons[input->mouseb];
}

dboolean dsda_InputJoyBActive(int identifier) {
  dsda_input_t* input;
  input = &dsda_input[dsda_input_index][identifier];

  return input->joyb >= 0 && joybuttons[input->joyb];
}
