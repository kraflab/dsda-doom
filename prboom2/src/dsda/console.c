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
#include "dsda/tas.h"

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

menu_t dsda_ConsoleDef = {
  0,
  NULL,
  NULL,
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

static dboolean console_PlayerSetHealth(const char* args) {
  int health;

  if (sscanf(args, "%i", &health)) {
    players[consoleplayer].mo->health = health;
    players[consoleplayer].health = health;

    return true;
  }

  return false;
}

static dboolean console_PlayerSetArmor(const char* args) {
  int arg_count;
  int armorpoints, armortype;

  arg_count = sscanf(args, "%i %i", &armorpoints, &armortype);

  if (arg_count != 2 || (armortype != 1 && armortype != 2))
    armortype = players[consoleplayer].armortype;

  if (arg_count) {
    players[consoleplayer].armorpoints = armorpoints;

    if (armortype == 0) armortype = 1;
    players[consoleplayer].armortype = armortype;

    return true;
  }

  return false;
}

static dboolean console_PlayerSetCoordinate(const char* args, int* dest) {
  int x, x_frac = 0;
  double x_double;

  if (sscanf(args, "%i.%i", &x, &x_frac)) {
    *dest = FRACUNIT * x;

    if (args[0] == '-')
      *dest -= x_frac;
    else
      *dest += x_frac;

    return true;
  }

  return false;
}

static dboolean console_PlayerSetX(const char* args) {
  return console_PlayerSetCoordinate(args, &players[consoleplayer].mo->x);
}

static dboolean console_PlayerSetY(const char* args) {
  return console_PlayerSetCoordinate(args, &players[consoleplayer].mo->y);
}

static dboolean console_PlayerSetZ(const char* args) {
  return console_PlayerSetCoordinate(args, &players[consoleplayer].mo->z);
}

static void console_PlayerRoundCoordinate(int* x) {
  int bits = *x & 0xffff;
  if (!bits) return;

  if (*x > 0) {
    if (bits >= 0x8000)
      *x = (*x & ~0xffff) + FRACUNIT;
    else
      *x = *x & ~0xffff;
  }
  else {
    if (bits < 0x8000)
      *x = (*x & ~0xffff) - FRACUNIT;
    else
      *x = *x & ~0xffff;
  }
}

static dboolean console_PlayerRoundX(const char* args) {
  console_PlayerRoundCoordinate(&players[consoleplayer].mo->x);

  return true;
}

static dboolean console_PlayerRoundY(const char* args) {
  console_PlayerRoundCoordinate(&players[consoleplayer].mo->y);

  return true;
}

static dboolean console_PlayerRoundXY(const char* args) {
  console_PlayerRoundCoordinate(&players[consoleplayer].mo->x);
  console_PlayerRoundCoordinate(&players[consoleplayer].mo->y);

  return true;
}

static dboolean console_CommandLock(const char* args) {
  char element[CONSOLE_ENTRY_SIZE];
  int value;

  if (sscanf(args, "%s %i", element, &value) == 2)
    return dsda_UpdatePersistentCommand(element, value);

  return false;
}

static dboolean console_CommandUnlock(const char* args) {
  dsda_DisablePersistentCommand();

  return true;
}

typedef dboolean (*console_command_t)(const char*);

typedef struct {
  const char* command_name;
  console_command_t command;
} console_command_entry_t;

static console_command_entry_t console_commands[] = {
  { "player.sethealth", console_PlayerSetHealth },
  { "player.setarmor", console_PlayerSetArmor },
  { "player.setx", console_PlayerSetX },
  { "player.sety", console_PlayerSetY },
  { "player.setz", console_PlayerSetZ },
  { "player.roundx", console_PlayerRoundX },
  { "player.roundy", console_PlayerRoundY },
  { "player.roundxy", console_PlayerRoundXY },
  { "command.lock", console_CommandLock },
  { "command.unlock", console_CommandUnlock },
  { NULL }
};

static void dsda_ExecuteConsole(void) {
  char command[CONSOLE_ENTRY_SIZE];
  char args[CONSOLE_ENTRY_SIZE];
  int scan_count;

  scan_count = sscanf(console_entry, "%s %[^;]", command, args);

  if (scan_count) {
    console_command_entry_t* entry;

    if (scan_count == 1) args[0] = '\0';

    for (entry = console_commands; entry->command; entry++) {
      if (!stricmp(command, entry->command_name)) {
        entry->command(args);
        break;
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
