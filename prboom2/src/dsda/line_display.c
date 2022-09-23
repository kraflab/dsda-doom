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
//	DSDA Line Display
//

#include "hu_lib.h"
#include "hu_stuff.h"
#include "m_menu.h"

#include "dsda.h"
#include "dsda/hud.h"

#include "line_display.h"

#define LINE_TEXT_X (DSDA_CHAR_WIDTH * 16 + 2)
#define LINE_TEXT_Y 8

static dsda_text_t line_display[LINE_ACTIVATION_INDEX_MAX];

void dsda_InitLineDisplay(patchnum_t* font) {
  int i;

  for (i = 0; i < LINE_ACTIVATION_INDEX_MAX; ++i)
    HUlib_initTextLine(
      &line_display[i].text,
      LINE_TEXT_X,
      LINE_TEXT_Y + i * DSDA_CHAR_HEIGHT,
      font,
      HU_FONTSTART,
      CR_GRAY,
      VPT_ALIGN_LEFT_TOP | VPT_EX_TEXT
    );
}

void dsda_UpdateLineDisplay(void) {
  int* line_ids;
  int i;

  line_ids = dsda_PlayerActivatedLines();

  for (i = 0; line_ids[i] != -1; ++i) {
    snprintf(line_display[i].msg, sizeof(line_display[i].msg), "%d", line_ids[i]);
    dsda_RefreshHudText(&line_display[i]);
  }

  if (line_ids[0] != -1 && i < LINE_ACTIVATION_INDEX_MAX)
    line_display[i].msg[0] = '\0';
}

void dsda_DrawLineDisplay(void) {
  int offset, i;

  offset = M_ConsoleOpen() ? 2 * DSDA_CHAR_HEIGHT : 0;

  for (i = 0; i < LINE_ACTIVATION_INDEX_MAX; ++i) {
    if (!line_display[i].msg[0])
      break;

    HUlib_drawOffsetTextLine(&line_display[i].text, offset);
  }
}

void dsda_EraseLineDisplay(void) {
  int i;

  for (i = 0; i < LINE_ACTIVATION_INDEX_MAX; ++i)
    HUlib_eraseTextLine(&line_display[i].text);
}
