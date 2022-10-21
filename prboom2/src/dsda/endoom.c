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
//	DSDA Endoom
//

#include "doomdef.h"
#include "doomtype.h"
#include "lprintf.h"
#include "w_wad.h"

#include "dsda/configuration.h"

#include "endoom.h"

void dsda_DumpEndoom(void) {
  int lump;
  const byte* endoom;

  if (!dsda_IntConfig(dsda_config_ansi_endoom))
    return;

  if (hexen)
    return;

  if (heretic)
    lump = W_CheckNumForName("ENDTEXT");
  else {
    lump = W_CheckNumForName("ENDBOOM");
    if (lump == LUMP_NOT_FOUND)
      lump = W_CheckNumForName("ENDOOM");
  }

  if (W_LumpLength(lump) != 4000)
    return;

  endoom = W_LumpByNum(lump);

  if (endoom) {
    int i;
    const char* color_lookup[] = {
      "0", "4", "2", "6", "1", "5", "3", "7",
      "0;1", "4;1", "2;1", "6;1", "1;1", "5;1", "3;1", "7;1",
    };

    for (i = 0; i < 2000; ++i) {
      byte character;
      byte data;
      const char *foreground;
      const char *background;
      const char *blink;

      character = endoom[i * 2];
      data = endoom[i * 2 + 1];
      foreground = color_lookup[data & 0x0f];
      background = color_lookup[(data >> 4) & 0x07];
      blink = ((data >> 7) & 0x01) ? "5" : "25";

      if (!character)
        character = ' ';

      lprintf(LO_INFO, "\033[3%sm\033[4%sm\033[%sm%c\033[0m",
              foreground, background, blink, character);

      if ((i + 1) % 80 == 0)
        lprintf(LO_INFO, "\n");
    }

    lprintf(LO_INFO, "\n");
  }
}
