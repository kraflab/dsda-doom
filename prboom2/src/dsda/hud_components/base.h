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

#ifndef __DSDA_HUD_COMPONENT_BASE__
#define __DSDA_HUD_COMPONENT_BASE__

#include <stdio.h>

#include "doomdef.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "p_tick.h"
#include "r_data.h"
#include "r_main.h"
#include "r_state.h"
#include "v_video.h"
#include "w_wad.h"

#include "dsda.h"
#include "dsda/exhud.h"
#include "dsda/global.h"
#include "dsda/hud.h"
#include "dsda/settings.h"
#include "dsda/utility.h"

extern int exhud_color_default;
extern int exhud_color_warning;
extern int exhud_color_alert;

extern int health_red;
extern int health_yellow;
extern int health_green;
extern int ammo_red;
extern int ammo_yellow;

extern patchnum_t hu_font2[HU_FONTSIZE];

typedef struct {
  int x;
  int y;
  int vpt;
} dsda_patch_component_t;

void dsda_InitTextHC(dsda_text_t* component, int x_offset, int y_offset, int vpt);
void dsda_InitPatchHC(dsda_patch_component_t* component, int x_offset, int y_offset, int vpt);
void dsda_DrawBigNumber(int x, int y, int delta_x, int delta_y, int cm, int vpt, int count, int n);

#endif
