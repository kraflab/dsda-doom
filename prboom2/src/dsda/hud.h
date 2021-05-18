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

#ifndef __DSDA_HUD__
#define __DSDA_HUD__

#include "r_defs.h"
#include "hu_lib.h"

typedef enum
{
  DSDA_SPLIT_BLUE_KEY,
  DSDA_SPLIT_YELLOW_KEY,
  DSDA_SPLIT_RED_KEY,
  DSDA_SPLIT_USE,
  DSDA_SPLIT_SECRET,
  DSDA_SPLIT_CLASS_COUNT
} dsda_split_class_t;

#define DSDA_TEXT_SIZE 200

typedef struct {
  hu_textline_t text;
  char msg[DSDA_TEXT_SIZE];
} dsda_text_t;

void dsda_RefreshHudText(dsda_text_t* hud_text);
void dsda_InitHud(patchnum_t* font);
void dsda_UpdateHud(void);
void dsda_DrawHud(void);
void dsda_EraseHud(void);
void dsda_AddSplit(dsda_split_class_t split_class);

#endif
