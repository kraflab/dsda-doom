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

#include "stdio.h"

#include "doomstat.h"
#include "hu_stuff.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "p_tick.h"
#include "r_main.h"
#include "r_state.h"

#include "dsda.h"
#include "dsda/exhud.h"
#include "dsda/global.h"
#include "dsda/hud.h"
#include "dsda/settings.h"
#include "dsda/utility.h"

extern int exhud_color_default;
extern int exhud_color_warning;
extern int exhud_color_alert;

extern patchnum_t hu_font2[HU_FONTSIZE];

void dsda_InitTextHC(dsda_text_t* component, int x_offset, int y_offset, int vpt);

#endif
