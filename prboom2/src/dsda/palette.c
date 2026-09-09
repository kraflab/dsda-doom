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
//	DSDA Palette Management
//

#include <math.h>
#include <string.h>

#include "r_main.h"
#include "w_wad.h"
#include "v_video.h"

#include "palette.h"

static int playpal_index = playpal_default;
static dboolean custom_playpal_active;
static char custom_playpal_name[9];

static dsda_playpal_t playpal_data[NUMPALETTES] = {
  { playpal_default, "PLAYPAL" },
  { playpal_1, "PLAYPAL1" },
  { playpal_2, "PLAYPAL2" },
  { playpal_3, "PLAYPAL3" },
  { playpal_4, "PLAYPAL4" },
  { playpal_5, "PLAYPAL5" },
  { playpal_6, "PLAYPAL6" },
  { playpal_7, "PLAYPAL7" },
  { playpal_8, "PLAYPAL8" },
  { playpal_9, "PLAYPAL9" }
};

static dsda_playpal_t custom_playpal_data = {
  -1, custom_playpal_name
};

dsda_playpal_t* dsda_PlayPalData(void) {
  if (custom_playpal_active)
    return &custom_playpal_data;

  return &playpal_data[playpal_index];
}

void dsda_CyclePlayPal(void) {
  int lump_num = -1;
  int cycle_playpal_index;

  cycle_playpal_index = playpal_index;

  do {
    cycle_playpal_index++;

    if (cycle_playpal_index > playpal_9)
      cycle_playpal_index = playpal_default;

    // Looped around and found nothing
    if (cycle_playpal_index == playpal_index)
      return;

    lump_num = W_CheckNumForName(playpal_data[cycle_playpal_index].lump_name);
  } while (lump_num == LUMP_NOT_FOUND);

  V_SetPlayPal(cycle_playpal_index);
}

void dsda_SetPlayPal(int index) {
  if (index < 0 || index >= NUMPALETTES)
    index = playpal_default;

  playpal_index = index;
  custom_playpal_active = false;
}

static void dsda_FreePlayPalData(dsda_playpal_t* data)
{
  if (data->lump)
  {
    Z_Free(data->lump);
    data->lump = NULL;
  }

  if (data->colours)
  {
    Z_Free(data->colours);
    data->colours = NULL;
  }

  data->length = 0;
}

void dsda_FreePlayPal(void) {
  int playpal_i;

  for (playpal_i = 0; playpal_i < NUMPALETTES; ++playpal_i)
    dsda_FreePlayPalData(&playpal_data[playpal_i]);

  dsda_FreePlayPalData(&custom_playpal_data);
  custom_playpal_active = false;
}

static dboolean dsda_DuplicatePaletteEntry(const byte *playpal, int i, int j) {
  int colormap_i;

  if (
    playpal[3 * i + 0] != playpal[3 * j + 0] ||
    playpal[3 * i + 1] != playpal[3 * j + 1] ||
    playpal[3 * i + 2] != playpal[3 * j + 2]
  )
    return false;

  for (colormap_i = 0; colormap_i < NUMCOLORMAPS; ++colormap_i)
    if (colormaps[0][colormap_i * 256 + i] != colormaps[0][colormap_i * 256 + j])
      return false;

  return true;
}

double dsda_PaletteEntryLightness(const byte *playpal, int i) {
  double L;
  byte pal_r, pal_g, pal_b;
  double r, g, b;
  double y1, y2;

  // this function basically does an RGB to L*a*b* (CIELAB) color
  // space conversion, but only for the L* component since we
  // just want the brightness. probably a bit overkill, but meh.

  // step 0: get colors from palette -- explicitly get it as a byte
  // (i.e. unsigned) so it doesn't get interpreted as a negative value.
  pal_r = playpal[3 * i + 0];
  pal_g = playpal[3 * i + 1];
  pal_b = playpal[3 * i + 2];

  // step 1: RGB to XYZ (...or just Y, I guess :P)
  r = pal_r / 255.0;
  g = pal_g / 255.0;
  b = pal_b / 255.0;

  r = r > 0.04045 ? pow((r + 0.055) / 1.055, 2.4) : r / 12.92;
  g = g > 0.04045 ? pow((g + 0.055) / 1.055, 2.4) : g / 12.92;
  b = b > 0.04045 ? pow((b + 0.055) / 1.055, 2.4) : b / 12.92;

  r *= 100;
  g *= 100;
  b *= 100;

  y1 = 0.2126729 * r + 0.7151522 * g + 0.0721750 * b;

  // step 2: XYZ to Lab
  y2 = y1 / 100.0;
  y2 = y2 > 0.008856 ? pow(y2, 1.0 / 3) : 7.787 * y2 + 4.0 / 29;

  L = 116 * y2 - 16;

  // all done. take the L.
  return L;
}

static void dsda_InitPlayPalData(dsda_playpal_t* data) {
  double lightness;
  double darkest, lightest;
  int lump;
  const byte *playpal;
  int i, j, found = 0;

  lump = W_CheckNumForName(data->lump_name);
  if (lump == LUMP_NOT_FOUND)
    return;

  playpal = W_LumpByNum(lump);

  // find two duplicate palette entries. use one for transparency.
  // rewrite source pixels in patches to the other on composition.
  for (i = 0; i < 256; i++) {
    for (j = i + 1; j < 256; j++) {
      if (dsda_DuplicatePaletteEntry(playpal, i, j)) {
        found = 1;
        break;
      }
    }

    if (found)
      break;
  }

  if (found) { // found duplicate
    data->transparent = i;
    data->duplicate   = j;
  }
  else { // no duplicate: use 255 for transparency, as done previously
    data->transparent = 255;
    data->duplicate   = -1;
  }

  // find the brightness extremes (0-100)
  darkest = 101.0;
  lightest = -1.0;
  for (i = 0; i < 256; i++) {
    lightness = dsda_PaletteEntryLightness(playpal, i);

    if (lightness < darkest) {
      darkest = lightness;
      data->darkest = i;
    }

    if (lightness > lightest) {
      lightest = lightness;
      data->lightest = i;
    }
  }
}

int dsda_SetCustomPlayPal(const char* lump_name)
{
  if (!lump_name || W_CheckNumForName(lump_name) == LUMP_NOT_FOUND)
    return false;

  if (custom_playpal_active && !stricmp(custom_playpal_name, lump_name))
    return true;

  dsda_FreePlayPalData(&custom_playpal_data);
  strncpy(custom_playpal_name, lump_name, 8);
  custom_playpal_name[8] = '\0';
  dsda_InitPlayPalData(&custom_playpal_data);
  custom_playpal_active = true;

  return true;
}

// Moved from r_patch.c
void dsda_InitPlayPal(void) {
  int playpal_i;

  for (playpal_i = 0; playpal_i < NUMPALETTES; ++playpal_i)
    dsda_InitPlayPalData(&playpal_data[playpal_i]);
}
