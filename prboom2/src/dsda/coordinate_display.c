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

#include <math.h>

#include "st_stuff.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "doomstat.h"

#include "dsda/hud.h"

#include "coordinate_display.h"

#define COORDINATE_TEXT_X 2
#define COORDINATE_TEXT_Y 8

#define THRESHOLD_1 15.11
#define THRESHOLD_2 19.35
#define THRESHOLD_3 21.37

static dsda_text_t dsda_x_display;
static dsda_text_t dsda_y_display;
static dsda_text_t dsda_z_display;
static dsda_text_t dsda_v_display;
static dsda_text_t dsda_vx_display;
static dsda_text_t dsda_vy_display;
static char dsda_velocity_color;

int dsda_coordinate_display;

static void dsda_BreakDownCoordinate(int* x, int* base, int* frac) {
  *base = *x >> FRACBITS;
  *frac = *x & 0xffff;

  if (*base < 0) {
    if (*frac != 0) {
      *base += 1;
      *frac = 0xffff - *frac + 1;
    }
  }
}

static double dsda_CalculateVelocity(void) {
  double vx, vy;

  vx = (double) players[displayplayer].mo->momx / FRACUNIT;
  vy = (double) players[displayplayer].mo->momy / FRACUNIT;

  return sqrt(vx * vx + vy * vy);
}

static void dsda_WriteCoordinate(dsda_text_t* text, int* x, const char* ch) {
  int base, frac;
  const char* format;

  dsda_BreakDownCoordinate(x, &base, &frac);

  if (frac)
    snprintf(text->msg, sizeof(text->msg), "%s: %i.%05i", ch, base, frac);
  else
    snprintf(text->msg, sizeof(text->msg), "%s: %i", ch, base);

  dsda_RefreshHudText(text);
}

static void dsda_WriteCoordinateSimple(dsda_text_t* text, int* x, const char* ch) {
  int base, frac;
  const char* format;

  dsda_BreakDownCoordinate(x, &base, &frac);

  if (frac)
    snprintf(text->msg, sizeof(text->msg), "\x1b%c%s: %i.%03i",
             dsda_velocity_color, ch, base, 1000 * frac / 0xffff);
  else
    snprintf(text->msg, sizeof(text->msg), "\x1b%c%s: %i", dsda_velocity_color, ch, base);

  dsda_RefreshHudText(text);
}

static void dsda_WriteVelocity(dsda_text_t* text) {
  double v;

  v = dsda_CalculateVelocity();

  dsda_velocity_color =
    v >= THRESHOLD_3 ?
      0x30 + g_cr_red   :
    v >= THRESHOLD_2 ?
      0x30 + g_cr_blue  :
    v >= THRESHOLD_1 ?
      0x30 + g_cr_green :
    0x30 + g_cr_gray;

  if (v)
    snprintf(text->msg, sizeof(text->msg), "\x1b%cV: %.3f", dsda_velocity_color, v);
  else
    snprintf(text->msg, sizeof(text->msg), "\x1b%cV: 0", dsda_velocity_color);

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

  HUlib_initTextLine(
    &dsda_v_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + 32,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_TOP
  );

  HUlib_initTextLine(
    &dsda_vx_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + 40,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_TOP
  );

  HUlib_initTextLine(
    &dsda_vy_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + 48,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_TOP
  );
}

void dsda_UpdateCoordinateDisplay(void) {
  dsda_WriteCoordinate(&dsda_x_display, &players[displayplayer].mo->x, "X");
  dsda_WriteCoordinate(&dsda_y_display, &players[displayplayer].mo->y, "Y");
  dsda_WriteCoordinate(&dsda_z_display, &players[displayplayer].mo->z, "Z");
  dsda_WriteVelocity(&dsda_v_display);
  dsda_WriteCoordinateSimple(&dsda_vx_display, &players[displayplayer].mo->momx, "X");
  dsda_WriteCoordinateSimple(&dsda_vy_display, &players[displayplayer].mo->momy, "Y");
}

void dsda_DrawCoordinateDisplay(void) {
  HUlib_drawTextLine(&dsda_x_display.text, false);
  HUlib_drawTextLine(&dsda_y_display.text, false);
  HUlib_drawTextLine(&dsda_z_display.text, false);
  HUlib_drawTextLine(&dsda_v_display.text, false);
  HUlib_drawTextLine(&dsda_vx_display.text, false);
  HUlib_drawTextLine(&dsda_vy_display.text, false);
}

void dsda_EraseCoordinateDisplay(void) {
  HUlib_eraseTextLine(&dsda_x_display.text);
  HUlib_eraseTextLine(&dsda_y_display.text);
  HUlib_eraseTextLine(&dsda_z_display.text);
  HUlib_eraseTextLine(&dsda_v_display.text);
  HUlib_eraseTextLine(&dsda_vx_display.text);
  HUlib_eraseTextLine(&dsda_vy_display.text);
}
