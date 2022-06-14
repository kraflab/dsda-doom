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
#include "m_menu.h"

#include "dsda/hud.h"
#include "dsda/utility.h"

#include "coordinate_display.h"

#define COORDINATE_TEXT_X 2
#define COORDINATE_TEXT_Y 8

#define THRESHOLD_1V 15.11
#define THRESHOLD_2V 19.35
#define THRESHOLD_3V 21.37

#define THRESHOLD_1D 16.67
#define THRESHOLD_2D 21.35
#define THRESHOLD_3D 23.58

static dsda_text_t dsda_x_display;
static dsda_text_t dsda_y_display;
static dsda_text_t dsda_z_display;
static dsda_text_t dsda_a_display;
static dsda_text_t dsda_v_display;
static dsda_text_t dsda_vx_display;
static dsda_text_t dsda_vy_display;
static dsda_text_t dsda_d_display;
static dsda_text_t dsda_dx_display;
static dsda_text_t dsda_dy_display;
static char dsda_velocity_color;
static char dsda_distance_color;

static double dsda_CalculateVelocity(void) {
  double vx, vy;

  vx = (double) players[displayplayer].mo->momx / FRACUNIT;
  vy = (double) players[displayplayer].mo->momy / FRACUNIT;

  return sqrt(vx * vx + vy * vy);
}

static double dsda_CalculateDistance(void) {
  double vx, vy;
  mobj_t* mo;

  mo = players[displayplayer].mo;

  vx = (double) (mo->x - mo->PrevX) / FRACUNIT;
  vy = (double) (mo->y - mo->PrevY) / FRACUNIT;

  return sqrt(vx * vx + vy * vy);
}

static void dsda_WriteCoordinate(dsda_text_t* text, fixed_t x, const char* ch) {
  char str[FIXED_STRING_LENGTH];
  const char* format;

  dsda_FixedToString(str, x);

  snprintf(text->msg, sizeof(text->msg), "%s: %s", ch, str);

  dsda_RefreshHudText(text);
}

static void dsda_WriteAngle(dsda_text_t* text, angle_t x, const char* ch) {
  dsda_angle_t value;
  const char* format;

  value = dsda_SplitAngle(x);

  if (value.frac) {
    if (value.negative && !value.base)
      snprintf(text->msg, sizeof(text->msg), "%s: -%i.%03i", ch, value.base, value.frac);
    else
      snprintf(text->msg, sizeof(text->msg), "%s: %i.%03i", ch, value.base, value.frac);
  }
  else
    snprintf(text->msg, sizeof(text->msg), "%s: %i", ch, value.base);

  dsda_RefreshHudText(text);
}

static void dsda_WriteCoordinateSimple(dsda_text_t* text, fixed_t x, const char* ch, char color) {
  dsda_fixed_t value;
  const char* format;

  value = dsda_SplitFixed(x);

  if (value.frac) {
    if (value.negative && !value.base)
      snprintf(text->msg, sizeof(text->msg), "\x1b%c%s: -%i.%03i",
               color, ch, value.base, 1000 * value.frac / 0xffff);
    else
      snprintf(text->msg, sizeof(text->msg), "\x1b%c%s: %i.%03i",
               color, ch, value.base, 1000 * value.frac / 0xffff);
  }
  else
    snprintf(text->msg, sizeof(text->msg), "\x1b%c%s: %i", color, ch, value.base);

  dsda_RefreshHudText(text);
}

static void dsda_WriteVelocity(dsda_text_t* text) {
  double v;

  v = dsda_CalculateVelocity();

  dsda_velocity_color =
    v >= THRESHOLD_3V ?
      0x30 + g_cr_red   :
    v >= THRESHOLD_2V ?
      0x30 + g_cr_blue  :
    v >= THRESHOLD_1V ?
      0x30 + g_cr_green :
    0x30 + g_cr_gray;

  if (v)
    snprintf(text->msg, sizeof(text->msg), "\x1b%cV: %.3f", dsda_velocity_color, v);
  else
    snprintf(text->msg, sizeof(text->msg), "\x1b%cV: 0", dsda_velocity_color);

  dsda_RefreshHudText(text);
}

static void dsda_WriteDistance(dsda_text_t* text) {
  double v;

  v = dsda_CalculateDistance();

  dsda_distance_color =
    v >= THRESHOLD_3D ?
      0x30 + g_cr_red   :
    v >= THRESHOLD_2D ?
      0x30 + g_cr_blue  :
    v >= THRESHOLD_1D ?
      0x30 + g_cr_green :
    0x30 + g_cr_gray;

  if (v)
    snprintf(text->msg, sizeof(text->msg), "\x1b%cD: %.3f", dsda_distance_color, v);
  else
    snprintf(text->msg, sizeof(text->msg), "\x1b%cD: 0", dsda_distance_color);

  dsda_RefreshHudText(text);
}

void dsda_InitCoordinateDisplay(patchnum_t* font) {
  int offset = 0;

  HUlib_initTextLine(
    &dsda_x_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_green,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );

  offset += DSDA_CHAR_HEIGHT;

  HUlib_initTextLine(
    &dsda_y_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_green,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );

  offset += DSDA_CHAR_HEIGHT;

  HUlib_initTextLine(
    &dsda_z_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_green,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );

  offset += DSDA_CHAR_HEIGHT;

  HUlib_initTextLine(
    &dsda_a_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_green,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );

  offset += 2 * DSDA_CHAR_HEIGHT;

  HUlib_initTextLine(
    &dsda_v_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );

  offset += DSDA_CHAR_HEIGHT;

  HUlib_initTextLine(
    &dsda_vx_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );

  offset += DSDA_CHAR_HEIGHT;

  HUlib_initTextLine(
    &dsda_vy_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );

  offset += 2 * DSDA_CHAR_HEIGHT;

  HUlib_initTextLine(
    &dsda_d_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );

  offset += DSDA_CHAR_HEIGHT;

  HUlib_initTextLine(
    &dsda_dx_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );

  offset += DSDA_CHAR_HEIGHT;

  HUlib_initTextLine(
    &dsda_dy_display.text,
    COORDINATE_TEXT_X,
    COORDINATE_TEXT_Y + offset,
    font,
    HU_FONTSTART,
    g_cr_gray,
    VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
  );
}

void dsda_UpdateCoordinateDisplay(void) {
  mobj_t* mo;

  mo = players[displayplayer].mo;

  dsda_WriteCoordinate(&dsda_x_display, mo->x, "X");
  dsda_WriteCoordinate(&dsda_y_display, mo->y, "Y");
  dsda_WriteCoordinate(&dsda_z_display, mo->z, "Z");
  dsda_WriteAngle(&dsda_a_display, mo->angle, "A");
  dsda_WriteVelocity(&dsda_v_display);
  dsda_WriteCoordinateSimple(&dsda_vx_display, mo->momx, "X", dsda_velocity_color);
  dsda_WriteCoordinateSimple(&dsda_vy_display, mo->momy, "Y", dsda_velocity_color);
  dsda_WriteDistance(&dsda_d_display);
  dsda_WriteCoordinateSimple(&dsda_dx_display, mo->x - mo->PrevX, "X", dsda_distance_color);
  dsda_WriteCoordinateSimple(&dsda_dy_display, mo->y - mo->PrevY, "Y", dsda_distance_color);
}

void dsda_DrawCoordinateDisplay(void) {
  int offset;

  offset = M_ConsoleOpen() ? 2 * DSDA_CHAR_HEIGHT : 0;

  HUlib_drawOffsetTextLine(&dsda_x_display.text, offset);
  HUlib_drawOffsetTextLine(&dsda_y_display.text, offset);
  HUlib_drawOffsetTextLine(&dsda_z_display.text, offset);
  HUlib_drawOffsetTextLine(&dsda_a_display.text, offset);
  HUlib_drawOffsetTextLine(&dsda_v_display.text, offset);
  HUlib_drawOffsetTextLine(&dsda_vx_display.text, offset);
  HUlib_drawOffsetTextLine(&dsda_vy_display.text, offset);
  HUlib_drawOffsetTextLine(&dsda_d_display.text, offset);
  HUlib_drawOffsetTextLine(&dsda_dx_display.text, offset);
  HUlib_drawOffsetTextLine(&dsda_dy_display.text, offset);
}

void dsda_EraseCoordinateDisplay(void) {
  HUlib_eraseTextLine(&dsda_x_display.text);
  HUlib_eraseTextLine(&dsda_y_display.text);
  HUlib_eraseTextLine(&dsda_z_display.text);
  HUlib_eraseTextLine(&dsda_a_display.text);
  HUlib_eraseTextLine(&dsda_v_display.text);
  HUlib_eraseTextLine(&dsda_vx_display.text);
  HUlib_eraseTextLine(&dsda_vy_display.text);
  HUlib_eraseTextLine(&dsda_d_display.text);
  HUlib_eraseTextLine(&dsda_dx_display.text);
  HUlib_eraseTextLine(&dsda_dy_display.text);
}
