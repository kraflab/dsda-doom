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
//	DSDA Line Display HUD Component
//

#include "base.h"

#include "line_display.h"

static dsda_text_t line_display[LINE_ACTIVATION_INDEX_MAX];

void dsda_InitLineDisplayHC(int x_offset, int y_offset, int vpt) {
  int i;

  for (i = 0; i < LINE_ACTIVATION_INDEX_MAX; ++i)
    dsda_InitTextHC(&line_display[i], x_offset, y_offset + i * 8, vpt);
}

void dsda_UpdateLineDisplayHC(void) {
  int* line_ids;
  int i;

  line_ids = dsda_PlayerActivatedLines();

  for (i = 0; line_ids[i] != -1; ++i) {
    snprintf(line_display[i].msg, sizeof(line_display[i].msg), "%d", line_ids[i]);
    dsda_RefreshHudText(&line_display[i]);
  }

  if (line_ids[0] != -1 && i < LINE_ACTIVATION_INDEX_MAX) {
    line_display[i].msg[0] = '\0';
    dsda_RefreshHudText(&line_display[i]);
  }
}

void dsda_DrawLineDisplayHC(void) {
  int i;

  for (i = 0; i < LINE_ACTIVATION_INDEX_MAX; ++i) {
    if (!line_display[i].msg[0])
      break;

    dsda_DrawBasicText(&line_display[i]);
  }
}

void dsda_EraseLineDisplayHC(void) {
  int i;

  for (i = 0; i < LINE_ACTIVATION_INDEX_MAX; ++i)
    HUlib_eraseTextLine(&line_display[i].text);
}
