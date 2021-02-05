//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 1993-2008 Raven Software
// Copyright(C) 2005-2014 Simon Howard
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

// MN_menu.c

#include "doomstat.h"
#include "w_wad.h"
#include "v_video.h"
#include "heretic/dstrings.h"
#include "heretic/mn_menu.h"

// heretic

static int SkullBaseLump;
static int MenuTime;

void MN_Init(void)
{
  // InitFonts();
  SkullBaseLump = W_GetNumForName(DEH_String("M_SKL00"));
  //
  // if (gamemode == retail)
  // {                           // Add episodes 4 and 5 to the menu
  //     EpisodeMenu.itemCount = 5;
  //     EpisodeMenu.y -= ITEM_HEIGHT;
  // }
}

void MN_Ticker(void)
{
    if (!menuactive)
    {
        return;
    }
    MenuTime++;
}

void MN_DrawMainMenu(void)
{
  int frame;

  frame = (MenuTime / 3) % 18;
  V_DrawNamePatch(88, 0, 0, DEH_String("M_HTIC"), CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(40, 10, 0, SkullBaseLump + (17 - frame), CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(232, 10, 0, SkullBaseLump + frame, CR_DEFAULT, VPT_STRETCH);
}
