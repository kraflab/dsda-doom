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

#include "st_stuff.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "doomstat.h"

#include "dsda.h"
#include "dsda/global.h"
#include "dsda/settings.h"
#include "dsda/exhud.h"
#include "dsda/command_display.h"
#include "dsda/coordinate_display.h"
#include "dsda/intermission_display.h"
#include "hud.h"

#define DSDA_TEXT_X 2
#define DSDA_SPLIT_Y 12
#define DSDA_SPLIT_LIFETIME 105
#define DSDA_SPLIT_SIZE 80

// hook into screen settings
extern int SCREENHEIGHT;
extern int viewheight;

typedef struct {
  hu_textline_t text;
  char msg[DSDA_SPLIT_SIZE];
  int ticks;
} dsda_split_text_t;

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

dsda_split_text_t dsda_split;

void dsda_RefreshHudText(dsda_text_t* hud_text) {
  const char* s;

  HUlib_clearTextLine(&hud_text->text);

  s = hud_text->msg;
  while (*s) HUlib_addCharToTextLine(&hud_text->text, *(s++));
}

void dsda_InitHud(patchnum_t* font) {
  HUlib_initTextLine(
    &dsda_split.text,
    DSDA_TEXT_X,
    DSDA_SPLIT_Y,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT
  );

  dsda_InitIntermissionDisplay(font);
  dsda_InitExHud(font);
  dsda_InitCommandDisplay(font);
  dsda_InitCoordinateDisplay(font);
}

static dboolean dsda_ExHudVisible(void) {
  return dsda_ExHud() && // extended hud turned on
         viewheight != SCREENHEIGHT && // not zoomed in
         (!(automapmode & am_active) || (automapmode & am_overlay)); // automap inactive
}

static dboolean dsda_CommandDisplayVisible(void) {
  return dsda_CommandDisplay() && // command display turned on
         viewheight != SCREENHEIGHT && // not zoomed in
         (!(automapmode & am_active) || (automapmode & am_overlay)); // automap inactive
}

static dboolean dsda_CoordinateDisplayVisible(void) {
  return dsda_CoordinateDisplay() && // command display turned on
         viewheight != SCREENHEIGHT && // not zoomed in
         (!(automapmode & am_active) || (automapmode & am_overlay)); // automap inactive
}

void dsda_UpdateHud(void) {
  int i;

  if (dsda_split.ticks > 0) --dsda_split.ticks;

  for (i = 0; i < DSDA_SPLIT_CLASS_COUNT; ++i)
    if (dsda_split_state[i].delay > 0)
      --dsda_split_state[i].delay;

  if (dsda_ExHudVisible()) dsda_UpdateExHud();
  if (dsda_CoordinateDisplayVisible()) dsda_UpdateCoordinateDisplay();
}

void dsda_DrawHud(void) {
  if (dsda_split.ticks > 0) HUlib_drawTextLine(&dsda_split.text, false);

  if (dsda_ExHudVisible()) dsda_DrawExHud();
  if (dsda_CommandDisplayVisible()) dsda_DrawCommandDisplay();
  if (dsda_CoordinateDisplayVisible()) dsda_DrawCoordinateDisplay();
}

void dsda_EraseHud(void) {
  if (dsda_split.ticks > 0) HUlib_eraseTextLine(&dsda_split.text);

  dsda_EraseExHud();
  dsda_EraseCommandDisplay();
  dsda_EraseCoordinateDisplay();
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

  // to match the timer, we use the leveltime value at the end of the frame
  minutes = (leveltime + 1) / 35 / 60;
  seconds = (float)((leveltime + 1) % (60 * 35)) / 35;
  snprintf(
    dsda_split.msg, DSDA_SPLIT_SIZE, "%d:%05.2f - %s",
    minutes, seconds, split_state->msg
  );

  HUlib_clearTextLine(&dsda_split.text);

  s = dsda_split.msg;
  while (*s) HUlib_addCharToTextLine(&dsda_split.text, *(s++));
}
