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
//	DSDA Coordinate Display
//

#include "st_stuff.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "doomstat.h"

#include "dsda/hud.h"

#include "coordinate_display.h"

#define COORDINATE_TEXT_X 2
#define COORDINATE_TEXT_Y 8

static dsda_text_t dsda_x_display;
static dsda_text_t dsda_y_display;
static dsda_text_t dsda_z_display;

int dsda_coordinate_display;

static void dsda_WriteCoordinate(dsda_text_t* text, int* x, const char* ch) {
  int base, frac;
  const char* format;

  base = *x >> FRACBITS;
  frac = *x & 0xffff;

  if (base < 0 && frac != 0)
    frac = 0xffff - frac + 1;

  if (frac)
    snprintf(text->msg, sizeof(text->msg), "%s: %i.%i", ch, base, frac);
  else
    snprintf(text->msg, sizeof(text->msg), "%s: %i", ch, base);

  dsda_RefreshHudText(text);
}

void dsda_InitCoordinateDisplay(patchnum_t* font) {
  HUlib_initTextLine(
    &dsda_x_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y,
    font,
    HU_FONTSTART,
    g_cr_green,
    VPT_ALIGN_LEFT_TOP
  );

  HUlib_initTextLine(
    &dsda_y_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + 8,
    font,
    HU_FONTSTART,
    g_cr_green,
    VPT_ALIGN_LEFT_TOP
  );

  HUlib_initTextLine(
    &dsda_z_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + 16,
    font,
    HU_FONTSTART,
    g_cr_green,
    VPT_ALIGN_LEFT_TOP
  );
}

void dsda_UpdateCoordinateDisplay(void) {
  dsda_WriteCoordinate(&dsda_x_display, &players[displayplayer].mo->x, "X");
  dsda_WriteCoordinate(&dsda_y_display, &players[displayplayer].mo->y, "Y");
  dsda_WriteCoordinate(&dsda_z_display, &players[displayplayer].mo->z, "Z");
}

void dsda_DrawCoordinateDisplay(void) {
  HUlib_drawTextLine(&dsda_x_display.text, false);
  HUlib_drawTextLine(&dsda_y_display.text, false);
  HUlib_drawTextLine(&dsda_z_display.text, false);
}

void dsda_EraseCoordinateDisplay(void) {
  HUlib_eraseTextLine(&dsda_x_display.text);
  HUlib_eraseTextLine(&dsda_y_display.text);
  HUlib_eraseTextLine(&dsda_z_display.text);
}
