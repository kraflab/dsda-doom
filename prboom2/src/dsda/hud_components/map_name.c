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
//	DSDA Map Name HUD Component
//

#include "base.h"
#include "hu_lib.h"

#include "map_name.h"

void dsda_HUTitle(hu_textline_t *line, const char** title);

static dsda_text_t component;

void dsda_InitMapNameHC(int x_offset, int y_offset, int vpt, int* args, int arg_count) {
  dsda_InitTextHC(&component, x_offset, y_offset, vpt);
}

void dsda_UpdateMapNameHC(void) {
  const char *title;

  HUlib_clearTextLine(&component.text);
  dsda_HUTitle(&component.text, &title);

  while (*title) HUlib_addCharToTextLine(&component.text, *(title++));
}

void dsda_DrawMapNameHC(void) {
  dsda_DrawBasicText(&component);
}
