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
//	DSDA Command Display
//

#include "st_stuff.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "doomstat.h"

#include "dsda.h"
#include "dsda/build.h"
#include "dsda/global.h"
#include "dsda/settings.h"
#include "dsda/hud.h"

#include "command_display.h"

#define INPUT_TEXT_X 198
#define MAX_HISTORY 20

typedef struct {
  signed char forwardmove;
  signed char sidemove;
  signed char angleturn;
  char use;
  char attack;
  char change;
} dsda_command_t;

typedef struct dsda_command_display_s {
  dsda_command_t command;
  int repeat;
  char text[200];
  hu_textline_t hu_text;
  struct dsda_command_display_s* next;
  struct dsda_command_display_s* prev;
} dsda_command_display_t;

int dsda_command_history_size;
int dsda_hide_empty_commands;

static dsda_command_display_t command_history[MAX_HISTORY];
static dsda_command_display_t* current_command = command_history;
static dsda_command_display_t next_command_display;

static void dsda_TicCmdToCommand(dsda_command_t* command, ticcmd_t* cmd) {
  command->forwardmove = cmd->forwardmove;
  command->sidemove = cmd->sidemove;
  command->angleturn = (cmd->angleturn + 128) >> 8;

  command->use = command->attack = command->change = 0;

  if (cmd->buttons && !(cmd->buttons & BT_SPECIAL)) {
    if (cmd->buttons & BT_ATTACK)
      command->attack = 1;

    if (cmd->buttons & BT_USE)
      command->use = 1;

    if (cmd->buttons & BT_CHANGE)
      command->change = 1 + ((cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT);
  }
}

static int dsda_IsEmptyCommand(dsda_command_t* command) {
  return command->forwardmove == 0 &&
         command->sidemove == 0 &&
         command->angleturn == 0 &&
         command->use == 0 &&
         command->attack == 0 &&
         command->change == 0;
}

static int dsda_IsCommandEqual(dsda_command_t* command, dsda_command_t* other) {
  return command->forwardmove == other->forwardmove &&
         command->sidemove == other->sidemove &&
         command->angleturn == other->angleturn &&
         command->use == other->use &&
         command->attack == other->attack &&
         command->change == other->change;
}

void dsda_InitCommandHistory(void) {
  int i;

  for (i = 1; i < MAX_HISTORY; ++i) {
    command_history[i].prev = &command_history[i - 1];
    command_history[i - 1].next = &command_history[i];
  }

  command_history[0].prev = &command_history[MAX_HISTORY - 1];
  command_history[MAX_HISTORY - 1].next = &command_history[0];
}

void dsda_InitCommandDisplayLine(dsda_command_display_t* command, int offset,
                                 int color, patchnum_t* font) {
  HUlib_initTextLine(
    &command->hu_text,
    INPUT_TEXT_X,
    200 - g_st_height - 8 * offset,
    font,
    HU_FONTSTART,
    color,
    VPT_ALIGN_RIGHT_BOTTOM
  );

  command->hu_text.space_width = 5;
}

void dsda_InitCommandDisplay(patchnum_t* font) {
  static int firsttime = 1;

  if (firsttime) {
    int i;

    firsttime = 0;

    for (i = 0; i < MAX_HISTORY; ++i)
      dsda_InitCommandDisplayLine(&command_history[i], i + 1, g_cr_gray, font);

    dsda_InitCommandDisplayLine(&next_command_display, 1, g_cr_gold, font);
  }
}

void dsda_UpdateCommandText(dsda_command_t* command, dsda_command_display_t* display_command) {
  char* s;
  int length = 0;

  display_command->text[0] = '\0';

  if (display_command->repeat)
    length += sprintf(display_command->text + length, "x%-3d ", display_command->repeat + 1);
  else
    length += sprintf(display_command->text + length, "     ");

  if (command->forwardmove > 0)
    length += sprintf(display_command->text + length, "MF%2d ", command->forwardmove);
  else if (command->forwardmove < 0)
    length += sprintf(display_command->text + length, "MB%2d ", -command->forwardmove);
  else
    length += sprintf(display_command->text + length, "     ");

  if (command->sidemove > 0)
    length += sprintf(display_command->text + length, "SR%2d ", command->sidemove);
  else if (command->sidemove < 0)
    length += sprintf(display_command->text + length, "SL%2d ", -command->sidemove);
  else
    length += sprintf(display_command->text + length, "     ");

  if (command->angleturn > 0)
    length += sprintf(display_command->text + length, "TL%2d ", command->angleturn);
  else if (command->angleturn < 0)
    length += sprintf(display_command->text + length, "TR%2d ", -command->angleturn);
  else
    length += sprintf(display_command->text + length, "     ");

  if (command->attack)
    length += sprintf(display_command->text + length, "A");
  else
    length += sprintf(display_command->text + length, " ");

  if (command->use)
    length += sprintf(display_command->text + length, "U");
  else
    length += sprintf(display_command->text + length, " ");

  if (command->change)
    length += sprintf(display_command->text + length, "C%d", command->change);
  else
    length += sprintf(display_command->text + length, "  ");

  HUlib_clearTextLine(&display_command->hu_text);
  s = display_command->text;
  while (*s) HUlib_addCharToTextLine(&display_command->hu_text, *(s++));
}

void dsda_AddCommandToCommandDisplay(ticcmd_t* cmd) {
  dsda_command_t command;

  dsda_TicCmdToCommand(&command, cmd);

  if (dsda_hide_empty_commands && dsda_IsEmptyCommand(&command))
    return;

  if (dsda_IsCommandEqual(&command, &current_command->command))
    ++current_command->repeat;
  else {
    current_command->repeat = 0;
    current_command = current_command->next;
    current_command->command = command;
  }

  dsda_UpdateCommandText(&command, current_command);
}

void dsda_DrawCommandDisplayLine(dsda_command_display_t* command, int offset) {
  command->hu_text.y = 200 - g_st_height - 8 * offset;
  HUlib_drawTextLine(&command->hu_text, false);
}

void dsda_DrawCommandDisplay(void) {
  int i;
  int offset = 1;
  dsda_command_display_t* command = current_command;

  if (dsda_BuildMode()) {
    ticcmd_t next_cmd;
    dsda_command_t next_command;

    dsda_CopyBuildCmd(&next_cmd);
    dsda_TicCmdToCommand(&next_command, &next_cmd);
    dsda_UpdateCommandText(&next_command, &next_command_display);

    dsda_DrawCommandDisplayLine(&next_command_display, offset);

    ++offset;
  }

  for (i = 0; i < dsda_command_history_size; ++i) {
    dsda_DrawCommandDisplayLine(command, i + offset);
    command = command->prev;
  }
}

void dsda_EraseCommandDisplay(void) {
  int i;
  dsda_command_display_t* command = current_command;

  for (i = 0; i < dsda_command_history_size; ++i) {
    HUlib_eraseTextLine(&command->hu_text);
    command = command->prev;
  }
}
