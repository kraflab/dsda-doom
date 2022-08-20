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
//	DSDA HUD Component Base
//

#include "base.h"

void dsda_InitTextHC(dsda_text_t* component, int x_offset, int y_offset, int vpt) {
  int x, y, vpt_align;

  x = 0;
  y = 0;

  vpt_align = vpt & VPT_ALIGN_MASK;
  if (
    vpt_align == VPT_ALIGN_BOTTOM ||
    vpt_align == VPT_ALIGN_LEFT_BOTTOM ||
    vpt_align == VPT_ALIGN_RIGHT_BOTTOM
  ) {
    y = 200;
    y_offset = -y_offset;

    // if (viewheight != SCREENHEIGHT)
      y -= g_st_height;
  }

  x += x_offset;
  y += y_offset;

  HUlib_initTextLine(
    &component->text,
    x, y,
    hu_font2,
    HU_FONTSTART,
    g_cr_gray,
    vpt
  );
}

void dsda_InitPatchHC(dsda_patch_component_t* component, int x_offset, int y_offset, int vpt) {
  int x, y, vpt_align;

  x = 0;
  y = 0;

  vpt_align = vpt & VPT_ALIGN_MASK;
  if (
    vpt_align == VPT_ALIGN_BOTTOM ||
    vpt_align == VPT_ALIGN_LEFT_BOTTOM ||
    vpt_align == VPT_ALIGN_RIGHT_BOTTOM
  ) {
    y = 200;
    y_offset = -y_offset;

    // if (viewheight != SCREENHEIGHT)
      y -= g_st_height;
  }

  x += x_offset;
  y += y_offset;

  component->x = x;
  component->y = y;
  component->vpt = vpt;
}
