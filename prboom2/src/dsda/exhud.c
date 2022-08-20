//
// Copyright(C) 2021 by Ryan Krafnick
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
//	DSDA Intermission Display
//

#include "dsda/global.h"
#include "dsda/hud_components.h"
#include "dsda/settings.h"

#include "exhud.h"

#define DSDA_EXHUD_X 2

int exhud_color_default;
int exhud_color_warning;
int exhud_color_alert;

void dsda_InitExHud(void) {
  exhud_color_default = g_cr_gray;
  exhud_color_warning = g_cr_green;
  exhud_color_alert = g_cr_red;

  dsda_InitTrackerHC(DSDA_EXHUD_X, 32, VPT_ALIGN_LEFT_BOTTOM | VPT_EX_TEXT);
  dsda_InitCompositeTimeHC(DSDA_EXHUD_X, 16, VPT_ALIGN_LEFT_BOTTOM | VPT_EX_TEXT);
  dsda_InitStatTotalsHC(DSDA_EXHUD_X, 8, VPT_ALIGN_LEFT_BOTTOM | VPT_EX_TEXT);
}

void dsda_UpdateExHud(void) {
  dsda_UpdateTrackerHC();
  dsda_UpdateCompositeTimeHC();
  dsda_UpdateStatTotalsHC();
}

void dsda_DrawExHud(void) {
  int i;

  if (!dsda_StrictMode())
    dsda_DrawTrackerHC();

  dsda_DrawCompositeTimeHC();
  dsda_DrawStatTotalsHC();
}

void dsda_EraseExHud(void) {
  int i;

  dsda_EraseTrackerHC();
  dsda_EraseCompositeTimeHC();
  dsda_EraseStatTotalsHC();
}
