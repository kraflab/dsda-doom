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
#define SELECTOR_XOFFSET (-28)
#define SELECTOR_YOFFSET (-1)
#define SCREENSIZE_INDEX 5

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
extern menu_t NewDef;
extern menu_t OptionsDef;
extern menu_t SetupDef;
extern menuitem_t EpisodeMenu[];
extern menuitem_t NewGameMenu[];
extern short EpiMenuMap[];
extern short EpiMenuEpi[];

void M_DrawThermo(int x, int y, int thermWidth, int thermDot);

void MN_Init(void)
{
  MN_InitFonts();
  SkullBaseLump = W_GetNumForName(DEH_String("M_SKL00"));

  // override doom menu parameters

  MainDef.x = 110;
  MainDef.y = 56;

  EpiDef.x = 80;
  EpiDef.y = 50;

  NewDef.x = 38;
  NewDef.y = 30;

  OptionsDef.x = 88;
  OptionsDef.y = 16;

  SetupDef.x = OptionsDef.x;
  SetupDef.y = OptionsDef.y;

  EpisodeMenu[0].alttext = "CITY OF THE DAMNED";
  EpisodeMenu[1].alttext = "HELL'S MAW";
  EpisodeMenu[2].alttext = "THE DOME OF D'SPARIL";
  EpisodeMenu[3].alttext = "THE OSSUARY";
  EpisodeMenu[4].alttext = "THE STAGNANT DEMESNE";

  NewGameMenu[0].alttext = "THOU NEEDETH A WET-NURSE";
  NewGameMenu[1].alttext = "YELLOWBELLIES-R-US";
  NewGameMenu[2].alttext = "BRINGEST THEM ONETH";
  NewGameMenu[3].alttext = "THOU ART A SMITE-MEISTER";
  NewGameMenu[4].alttext = "BLACK PLAGUE POSSESSES THEE";

  if (gamemode == retail)
  {
    EpiMenuEpi[3] = 4;
    EpiMenuEpi[4] = 5;
    EpiMenuMap[3] = 1;
    EpiMenuMap[4] = 1;
    EpiDef.numitems = 5;
    EpiDef.y -= ITEM_HEIGHT;
  }
  else
  {
    EpiMenuEpi[3] = -1;
    EpiMenuEpi[4] = -1;
    EpiMenuMap[3] = -1;
    EpiMenuMap[4] = -1;
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
extern short itemOn;
extern int showMessages;
extern int screenSize;

void MN_Drawer(void)
{
  int i;
  int x;
  int y;
  int max;
  const char* selName;

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

  y = currentMenu->y + (itemOn * LINEHEIGHT) + SELECTOR_YOFFSET;
  selName = DEH_String(MenuTime & 16 ? "M_SLCTR1" : "M_SLCTR2");
  V_DrawNamePatch(x + SELECTOR_XOFFSET, y, 0, selName, CR_DEFAULT, VPT_STRETCH);
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

void MN_DrawOptions(void)
{
    if (showMessages)
    {
        MN_DrTextB(DEH_String("ON"), 196, OptionsDef.y + 3 * LINEHEIGHT);
    }
    else
    {
        MN_DrTextB(DEH_String("OFF"), 196, OptionsDef.y + 3 * LINEHEIGHT);
    }
    M_DrawThermo(OptionsDef.x, OptionsDef.y + 4 + LINEHEIGHT * SCREENSIZE_INDEX, 9, screenSize);
}

void MN_DrawSetup(void)
{
  // nothing for heretic
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
