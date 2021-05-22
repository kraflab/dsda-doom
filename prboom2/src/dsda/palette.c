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

#include "w_wad.h"
#include "v_video.h"

#include "palette.h"

static int playpal_index = playpal_default;

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
  { playpal_9, "PLAYPAL9" },
  { playpal_heretic_e2end, "E2PAL" }
};

dsda_playpal_t* dsda_PlayPalData(void) {
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
  } while (lump_num < 0);

  V_SetPlayPal(cycle_playpal_index);
}

void dsda_SetPlayPal(int index) {
  if (index < 0 || index >= NUMPALETTES)
    index = playpal_default;

  playpal_index = index;
}

void dsda_FreePlayPal(void) {
  int playpal_i;

  for (playpal_i = 0; playpal_i < NUMPALETTES; ++playpal_i)
    if (playpal_data[playpal_i].lump) {
      free(playpal_data[playpal_i].lump);
      playpal_data[playpal_i].lump = NULL;
    }
}

void dsda_FreeTrueColorPlayPal(void) {
  int playpal_i;
  video_mode_t mode;

  mode = V_GetMode();

  for (playpal_i = 0; playpal_i < NUMPALETTES; ++playpal_i) {
    if (mode != VID_MODE32) {
      if (playpal_data[playpal_i].Palettes32)
        free(playpal_data[playpal_i].Palettes32);

      playpal_data[playpal_i].Palettes32 = NULL;
    }
  }
}

// Moved from r_patch.c
void dsda_InitPlayPal(void) {
  int playpal_i;

  for (playpal_i = 0; playpal_i < NUMPALETTES; ++playpal_i) {
    if (!playpal_data[playpal_i].duplicate) {
      int lump;
      const char *playpal;
      int i, j, found = 0;

      lump = W_CheckNumForName(playpal_data[playpal_i].lump_name);
      if (lump < 0)
        continue;

      playpal = W_CacheLumpNum(lump);

      // find two duplicate palette entries. use one for transparency.
      // rewrite source pixels in patches to the other on composition.

      for (i = 0; i < 256; i++) {
        for (j = i + 1; j < 256; j++) {
          if (
            playpal[3 * i + 0] == playpal[3 * j + 0] &&
            playpal[3 * i + 1] == playpal[3 * j + 1] &&
            playpal[3 * i + 2] == playpal[3 * j + 2]
          ) {
            found = 1;
            break;
          }
        }

        if (found)
          break;
      }

      if (found) { // found duplicate
        playpal_data[playpal_i].transparent = i;
        playpal_data[playpal_i].duplicate   = j;
      }
      else { // no duplicate: use 255 for transparency, as done previously
        playpal_data[playpal_i].transparent = 255;
        playpal_data[playpal_i].duplicate   = -1;
      }

      W_UnlockLumpNum(lump);
    }
  }
}
