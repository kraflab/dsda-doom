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
#include "m_menu.h"
#include "heretic/dstrings.h"
#include "heretic/mn_menu.h"

#define LINEHEIGHT  16
#define ITEM_HEIGHT 20

static int FontABaseLump;
static int FontBBaseLump;
static int SkullBaseLump;
static int MenuTime;

static void MN_InitFonts(void)
{
  FontABaseLump = W_GetNumForName(DEH_String("FONTA_S")) + 1;
  FontBBaseLump = W_GetNumForName(DEH_String("FONTB_S")) + 1;
}

extern menu_t MainDef;
extern menu_t EpiDef;
extern menuitem_t EpisodeMenu[];

void MN_Init(void)
{
  MN_InitFonts();
  SkullBaseLump = W_GetNumForName(DEH_String("M_SKL00"));

  // override doom menu parameters

  MainDef.x = 110;
  MainDef.y = 56;

  EpiDef.x = 80;
  EpiDef.y = 50;

  EpisodeMenu[0].alttext = "CITY OF THE DAMNED";
  EpisodeMenu[1].alttext = "HELL'S MAW";
  EpisodeMenu[2].alttext = "THE DOME OF D'SPARIL";
  EpisodeMenu[3].alttext = "THE OSSUARY";
  EpisodeMenu[4].alttext = "THE STAGNANT DEMESNE";

  if (gamemode == retail)
  {
    EpiDef.numitems = 5;
    EpiDef.y -= ITEM_HEIGHT;
  }
  else
  {
    EpiDef.numitems = 3;
  }
}

void MN_Ticker(void)
{
    if (!menuactive)
    {
        return;
    }
    MenuTime++;
}

extern menu_t* currentMenu;

void MN_Drawer(void)
{
  int i;
  int x;
  int y;
  int max;

  x = currentMenu->x;
  y = currentMenu->y;
  max = currentMenu->numitems;

  for (i = 0; i < max; i++)
  {
    const char *text = currentMenu->menuitems[i].alttext;
    if (text)
      MN_DrTextB(DEH_String(text), x, y);
    y += LINEHEIGHT;
  }
  // MenuItem_t *item;
  // const char *message;
  // const char *selName;
  //
  // if (MenuActive == false)
  // {
  //   if (askforquit)
  //   {
  //     message = DEH_String(QuitEndMsg[typeofask - 1]);
  //
  //     MN_DrTextA(message, 160 - MN_TextAWidth(message) / 2, 80);
  //     if (typeofask == 3)
  //     {
  //       MN_DrTextA(SlotText[quicksave - 1], 160 -
  //                  MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
  //       MN_DrTextA(DEH_String("?"), 160 +
  //                  MN_TextAWidth(SlotText[quicksave - 1]) / 2, 90);
  //     }
  //     if (typeofask == 4)
  //     {
  //       MN_DrTextA(SlotText[quickload - 1], 160 -
  //                  MN_TextAWidth(SlotText[quickload - 1]) / 2, 90);
  //       MN_DrTextA(DEH_String("?"), 160 +
  //                  MN_TextAWidth(SlotText[quickload - 1]) / 2, 90);
  //     }
  //     UpdateState |= I_FULLSCRN;
  //   }
  //   return;
  // }
  // else
  // {
  //   UpdateState |= I_FULLSCRN;
  //   if (InfoType)
  //   {
  //     MN_DrawInfo();
  //     return;
  //   }
  //   if (screenblocks < 10)
  //   {
  //     BorderNeedRefresh = true;
  //   }
  //   if (CurrentMenu->drawFunc != NULL)
  //   {
  //     CurrentMenu->drawFunc();
  //   }
  //   x = CurrentMenu->x;
  //   y = CurrentMenu->y;
  //   item = CurrentMenu->items;
  //   for (i = 0; i < CurrentMenu->itemCount; i++)
  //   {
  //     if (item->type != ITT_EMPTY && item->text)
  //     {
  //       MN_DrTextB(DEH_String(item->text), x, y);
  //     }
  //
  //     y += ITEM_HEIGHT;
  //     item++;
  //   }
  //
  //   y = CurrentMenu->y + (CurrentItPos * ITEM_HEIGHT) + SELECTOR_YOFFSET;
  //   selName = DEH_String(MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2");
  //   V_DrawPatch(x + SELECTOR_XOFFSET, y,
  //               W_CacheLumpName(selName, PU_CACHE));
  // }
}

void MN_DrawMainMenu(void)
{
  int frame;

  frame = (MenuTime / 3) % 18;
  V_DrawNamePatch(88, 0, 0, DEH_String("M_HTIC"), CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(40, 10, 0, SkullBaseLump + (17 - frame), CR_DEFAULT, VPT_STRETCH);
  V_DrawNumPatch(232, 10, 0, SkullBaseLump + frame, CR_DEFAULT, VPT_STRETCH);
}

void MN_DrTextA(const char *text, int x, int y)
{
  char c;
  int lump;

  while ((c = *text++) != 0)
  {
    if (c < 33)
    {
      x += 5;
    }
    else
    {
      lump = FontABaseLump + c - 33;
      V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
      x += R_NumPatchWidth(lump) - 1;
    }
  }
}

int MN_TextAWidth(const char *text)
{
  char c;
  int width;
  int lump;

  width = 0;
  while ((c = *text++) != 0)
  {
    if (c < 33)
    {
      width += 5;
    }
    else
    {
      lump = FontABaseLump + c - 33;
      width += R_NumPatchWidth(lump) - 1;
    }
  }
  return (width);
}

void MN_DrTextB(const char *text, int x, int y)
{
  char c;
  int lump;

  while ((c = *text++) != 0)
  {
    if (c < 33)
    {
      x += 8;
    }
    else
    {
      lump = FontBBaseLump + c - 33;
      V_DrawNumPatch(x, y, 0, lump, CR_DEFAULT, VPT_STRETCH);
      x += R_NumPatchWidth(lump) - 1;
    }
  }
}

int MN_TextBWidth(const char *text)
{
  char c;
  int width;
  int lump;

  width = 0;
  while ((c = *text++) != 0)
  {
    if (c < 33)
    {
      width += 5;
    }
    else
    {
      lump = FontBBaseLump + c - 33;
      width += R_NumPatchWidth(lump) - 1;
    }
  }
  return (width);
}
