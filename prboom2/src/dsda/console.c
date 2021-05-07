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
//	DSDA Console
//

#include "doomstat.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "m_menu.h"
#include "v_video.h"

#include "dsda/global.h"

#include "console.h"

extern patchnum_t hu_font2[HU_FONTSIZE];

#define CONSOLE_ENTRY_SIZE 64

static char console_prompt[CONSOLE_ENTRY_SIZE + 3] = { '$', ' ' };
static char* console_entry = console_prompt + 2;
static int console_entry_index;
static hu_textline_t hu_console_prompt;

static void dsda_DrawConsole(void) {
  V_FillRect(0, 0, 0, SCREENWIDTH, 8 * SCREENHEIGHT / 200, 0);
  HUlib_drawTextLine(&hu_console_prompt, false);
}

static menuitem_t dsda_ConsoleMenu[] = {};

menu_t dsda_ConsoleDef = {
  0,
  NULL,
  dsda_ConsoleMenu,
  dsda_DrawConsole,
  0, 0
};

static void dsda_UpdateConsoleDisplay(void) {
  char* s = console_prompt;
  HUlib_clearTextLine(&hu_console_prompt);
  while (*s) HUlib_addCharToTextLine(&hu_console_prompt, *(s++));
  HUlib_addCharToTextLine(&hu_console_prompt, '_');
}

static void dsda_ResetConsoleEntry(void) {
  memset(console_entry, 0, CONSOLE_ENTRY_SIZE);
  console_entry_index = 0;
  dsda_UpdateConsoleDisplay();
}

dboolean dsda_OpenConsole(void) {
  static dboolean firsttime = true;

  if (gamestate != GS_LEVEL)
    return false;

  if (firsttime) {
    firsttime = false;

    HUlib_initTextLine(
      &hu_console_prompt,
      0,
      0,
      hu_font2,
      HU_FONTSTART,
      g_cr_gray,
      VPT_ALIGN_LEFT_TOP
    );
  }

  M_StartControlPanel();
  M_SetupNextMenu(&dsda_ConsoleDef);
  dsda_ResetConsoleEntry();

  return true;
}

static void dsda_ExecuteConsole(void) {
  char command[CONSOLE_ENTRY_SIZE];
  char args[CONSOLE_ENTRY_SIZE];

  if (sscanf(console_entry, "%s %s", command, args) == 2) {
    if (!stricmp(command, "player.sethealth")) {
      int health;
      if (sscanf(args, "%i", &health) == 1) {
        players[consoleplayer].mo->health = health;
        players[consoleplayer].health = health;
      }
    }
  }

  dsda_ResetConsoleEntry();
}

void dsda_UpdateConsole(int ch, int action) {
  if (action == MENU_BACKSPACE && console_entry_index > 0) {
    --console_entry_index;
    console_entry[console_entry_index] = '\0';
    dsda_UpdateConsoleDisplay();
  }
  else if (action == MENU_ENTER) {
    dsda_ExecuteConsole();
  }
  else if (ch > 0) {
    ch = tolower(ch);
    if (
      (ch >= 'a' && ch <= 'z') ||
      (ch >= '0' && ch <= '9') ||
      (ch == ' ' || ch == '.' || ch == '-')
    ) {
      console_entry[console_entry_index] = ch;
      if (console_entry_index < CONSOLE_ENTRY_SIZE)
        ++console_entry_index;

      dsda_UpdateConsoleDisplay();
    }
  }
}
