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
//	DSDA Hud
//

#include "hu_lib.h"
#include "hu_stuff.h"
#include "doomstat.h"
#include "hud.h"

#define DSDA_SPLIT_X 2
#define DSDA_SPLIT_Y 12
#define DSDA_SPLIT_LIFETIME 105
#define DSDA_SPLIT_SIZE 80

typedef struct {
  hu_textline_t text;
  char msg[DSDA_SPLIT_SIZE];
  int ticks;
} dsda_text_t;

typedef struct {
  const char* msg;
  int default_delay;
  int delay;
} dsda_split_state_t;

dsda_split_state_t dsda_split_state[] = {
  {"Blue Key", 0, 0},
  {"Yellow Key", 0, 0},
  {"Red Key", 0, 0},
  {"Use", 2, 0},
  {"Secret", 0, 0}
};

dsda_text_t dsda_split;

void dsda_InitHud(patchnum_t* font) {
  HUlib_initTextLine(
    &dsda_split.text,
    DSDA_SPLIT_X,
    DSDA_SPLIT_Y,
    font,
    HU_FONTSTART,
    CR_GRAY,
    VPT_ALIGN_LEFT
  );
}

void dsda_UpdateHud(void) {
  int i;
  
  if (dsda_split.ticks > 0) --dsda_split.ticks;
  
  for (i = 0; i < DSDA_SPLIT_CLASS_COUNT; ++i)
    if (dsda_split_state[i].delay > 0)
      --dsda_split_state[i].delay;
}

void dsda_DrawHud(void) {
  if (dsda_split.ticks > 0) HUlib_drawTextLine(&dsda_split.text, false);
}

void dsda_EraseHud(void) {
  if (dsda_split.ticks > 0) HUlib_eraseTextLine(&dsda_split.text);
}

void dsda_AddSplit(dsda_split_class_t split_class) {
  int minutes;
  float seconds;
  char* s;
  dsda_split_state_t* split_state;
  
  split_state = &dsda_split_state[split_class];
  
  if (split_state->delay > 0) {
    split_state->delay = split_state->default_delay;
    return;
  }
  
  split_state->delay = split_state->default_delay;
  
  dsda_split.ticks = DSDA_SPLIT_LIFETIME;
  
  minutes = leveltime / 35 / 60;
  seconds = (float)(leveltime % (60 * 35)) / 35;
  snprintf(
    dsda_split.msg, DSDA_SPLIT_SIZE, "%d:%05.2f - %s",
    minutes, seconds, split_state->msg
  );
  
  HUlib_clearTextLine(&dsda_split.text);
  
  s = dsda_split.msg;
  while (*s) HUlib_addCharToTextLine(&dsda_split.text, *(s++));
}
