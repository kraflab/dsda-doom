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
#include "r_main.h"

#include "dsda.h"
#include "dsda/build.h"
#include "dsda/global.h"
#include "dsda/settings.h"
#include "dsda/exhud.h"
#include "dsda/intermission_display.h"

#include "hud.h"

void dsda_RefreshHudText(dsda_text_t* hud_text) {
  const char* s;

  HUlib_clearTextLine(&hud_text->text);

  s = hud_text->msg;
  while (*s) HUlib_addCharToTextLine(&hud_text->text, *(s++));
}

void dsda_InitHud(patchnum_t* font) {
  dsda_InitIntermissionDisplay(font);
  dsda_InitExHud();
}

static dboolean dsda_ExHudVisible(void) {
  return automap_off;
}

void dsda_UpdateHud(void) {
  if (dsda_ExHudVisible()) dsda_UpdateExHud();
}

void dsda_DrawHud(void) {
  if (dsda_ExHudVisible()) dsda_DrawExHud();
}

void dsda_EraseHud(void) {
  dsda_EraseExHud();
}
